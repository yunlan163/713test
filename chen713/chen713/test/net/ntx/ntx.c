#include "ntx.h"
#include <string.h>
#include <stdio.h>
#include "..\..\common.h"
//#include "../../../../hpi.h"
//#include "../../../../j1052_gpio.h"

enum {IDLE,WAIT_RT_RSP};
extern short getcrc_short( unsigned short* buf,  int len);
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;

//组网络包，并把网络包写到指定的地址中  yun:子函数 7param
static void WtNetPkt(NTx* proc,unsigned short *sdat,unsigned short *dat,unsigned short len,unsigned short da,unsigned short pri,unsigned short hello_flag)//写网络包头，拷贝数据
{
	memset(dat,0,(len + NET_FRM_HEAD_LEN));  //yun:网络包空间 置零
	//填网络帧头,除了序号（序号在发送的时候才会填写）
	((Net_Frm*)dat)->pri_sa_da_mul_sn = ((P_Entity*)proc->entity)->mib.local_id << 8;  //yun:源地址
	((Net_Frm*)dat)->pri_sa_da_mul_sn |= (da & 0x1f)<<3;  //yun:目的地址
	((Net_Frm*)dat)->pri_sa_da_mul_sn |= pri<<13;  //yun:优先级
	((Net_Frm*)dat)->sn_len |= (len &0x3ff);  //yun:长度 ，没序号
	if(da == MULTICAST_DA)   //多播置位
	{
		((Net_Frm*)dat)->pri_sa_da_mul_sn |= (0x1<<2);
	}
	memcpy(((Net_Frm*)dat)->net_paload, sdat, len);  //yun：数据拷贝!!!! 注意长度问题： 没有网络包头
	if(hello_flag)//HELLO包，帧头置标记
	{
		((Net_Frm*)dat)->pri_sa_da_mul_sn |= 0x8000;	
	}
	return;
}

//插入对应的目的节点网络队列中
static int NetQueWt(NTx* proc,unsigned short *dat,unsigned short len,unsigned short ra,unsigned short da,unsigned long  arrvial_time,unsigned short pri)
{
	unsigned short btm,idx,tmp_idx2,tmp_idx1;
	short i,j,size;
	//top = proc->net_que_list[da].top;
	btm = proc->net_que_list[da].btm;
	size = proc->net_que_list[da].size; 
	if(size >= NET_QUE_MAX_NUM)  //20
	{
		printf("ntx::NetQueWt() 队列满，释放数据\n");
		MemFreeNet(dat);//数据过多，释放数据
		return 0;
	}
	
	//转发多播，不按优先级重新排队,直接放到最后面的buf中
	if((((*dat)>>2)&0x1) && ((((*dat)>>8)&0x1f) != ((P_Entity*)proc->entity)->mib.local_id))//多播&&源节点不是自己
	{
		idx = (btm + size)%NET_QUE_MAX_NUM;
	}
	else//其他，按照优先级排队，优先级高的排在靠近btm的方向；优先级低的排在远离btm的方向
	{
		idx = btm;
		//yun：注意，i从1开始，也就是进来第一个就放在了0处
		for(i=1;i<=size;i++)   //yun：找到该优先级小于哪一个，确定其位置 （数小，优先级大）
		{
			idx = (btm + i)%NET_QUE_MAX_NUM;
			if(i<size)
			{
				if(pri < proc->net_que_list[da].que_elmt[idx].pri)//找到idx为插入位置  yun：该优先级高
				{
					break;
				}
			}
		}
		for(j=0;j<size-i;j++)  //yun:找到的位置其后的单元一个一个往后移
		{
			tmp_idx1 = (btm + size -j)%NET_QUE_MAX_NUM;
			tmp_idx2 = (btm + size -j -1)%NET_QUE_MAX_NUM;
			proc->net_que_list[da].que_elmt[tmp_idx1].dat = proc->net_que_list[da].que_elmt[tmp_idx2].dat;
			proc->net_que_list[da].que_elmt[tmp_idx1].len = proc->net_que_list[da].que_elmt[tmp_idx2].len;
			proc->net_que_list[da].que_elmt[tmp_idx1].ra = proc->net_que_list[da].que_elmt[tmp_idx2].ra ;
			proc->net_que_list[da].que_elmt[tmp_idx1].da = proc->net_que_list[da].que_elmt[tmp_idx2].da;
			proc->net_que_list[da].que_elmt[tmp_idx1].pri = proc->net_que_list[da].que_elmt[tmp_idx2].pri;
			proc->net_que_list[da].que_elmt[tmp_idx1].arrvial_time = proc->net_que_list[da].que_elmt[tmp_idx2].arrvial_time ;
			proc->net_que_list[da].que_elmt[tmp_idx1].first_send_flag = proc->net_que_list[da].que_elmt[tmp_idx2].first_send_flag;
		}
	}
	//把数据放到找到的插入位置中
	proc->net_que_list[da].que_elmt[idx].dat=dat;
	proc->net_que_list[da].que_elmt[idx].len=len;
	proc->net_que_list[da].que_elmt[idx].ra=ra;
	proc->net_que_list[da].que_elmt[idx].da=da;
	proc->net_que_list[da].que_elmt[idx].pri=pri;
	proc->net_que_list[da].que_elmt[idx].arrvial_time = arrvial_time;
	proc->net_que_list[da].que_elmt[idx].first_send_flag=1;
	proc->net_que_list[da].top++;
	proc->net_que_list[da].top%=NET_QUE_MAX_NUM;
	proc->net_que_list[da].size++;
	return 1;
}

//删除队列中的网络包   yun：下层回复、单播查找路由失败  删除底部一个
void DeleteQueElmt(NTx* proc,unsigned short da)
{
	unsigned short btm=proc->net_que_list[da].btm;
	//unsigned char printdat_once[] = " DeleteQueElmt";
	//PrintDat(printdat_once,14,1);
	if(proc->net_que_list[da].size)
	{
		printf("ntx::DeleteQueElmt() 删除队列中网络包，释放数据\n");
		MemFreeNet(proc->net_que_list[da].que_elmt[btm].dat);
		proc->net_que_list[da].que_elmt[btm].dat = NULL;
		proc->net_que_list[da].que_elmt[btm].len = 0;
		proc->net_que_list[da].que_elmt[btm].ra = 0;
		proc->net_que_list[da].que_elmt[btm].da = 0;
		proc->net_que_list[da].que_elmt[btm].arrvial_time = 0;
		proc->net_que_list[da].que_elmt[btm].first_send_flag = 0;
		proc->net_que_list[da].btm ++;
		proc->net_que_list[da].btm %= NET_QUE_MAX_NUM;
		proc->net_que_list[da].size --;
	}

}

//具体某节点的网络队列读取
static int NetQueRd(NTx* proc,unsigned short **dat,unsigned short *len,unsigned short *ra,unsigned short *da,unsigned long * arrvial_time,unsigned short dat_in)
{
	unsigned short btm;
	unsigned short k,dat_valid,size;
	//unsigned long time_len;
	dat_valid=0;
	if(proc->net_que_list[dat_in].size!=0)
	{
		size=proc->net_que_list[dat_in].size;
		for(k=0;k<size;k++)
		{
			btm=proc->net_que_list[dat_in].btm;			
			if(proc->net_que_list[dat_in].que_elmt[btm].first_send_flag)//首次发送网络包，记录时间用于超时删除
			{
				proc->net_que_list[dat_in].que_elmt[btm].first_send_flag = 0;
				proc->net_que_list[dat_in].que_elmt[btm].arrvial_time = proc->ntx_local_time;
				dat_valid=1;
				break;
			}
			//if(proc->ntx_local_time > proc->net_que_list[dat_in].que_elmt[btm].arrvial_time)
			//{
			//	time_len = proc->ntx_local_time - proc->net_que_list[dat_in].que_elmt[btm].arrvial_time;
			//}
			//else
			//{
			//	time_len = proc->ntx_local_time + (0xffffffff - proc->net_que_list[dat_in].que_elmt[btm].arrvial_time);
			//}
			//if(time_len < NET_PKT_MAX_TTL)//未超时，非首次网络包
			//{
				dat_valid=1;
				break;	
			//}
			//else//网络包数据超时,删除
			//{
			//	sprintf(str,"Timer Net Pkt FAIL: Node %d —> Node %d   len=%d  Data: %x %x %x %x %x %x %x %x %x %x",((P_Entity*)proc->entity)->mib.local_id,dat_in,proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].len,proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[3],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[4],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[5],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[6],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[7],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[8],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[9],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[10],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[11],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[12]);		
			//	DbgPrint(str);
			//	DeleteQueElmt(proc,dat_in);
			//}
		}		
	}
	if(!dat_valid)//无网络包数据
	{
		*dat=0;
		*len=0;
		return 0;
	}	
	else//有网络包数据
	{
		btm=proc->net_que_list[dat_in].btm;
		*dat=proc->net_que_list[dat_in].que_elmt[btm].dat;
		*len=proc->net_que_list[dat_in].que_elmt[btm].len;
		*ra=proc->net_que_list[dat_in].que_elmt[btm].ra;
		*da=proc->net_que_list[dat_in].que_elmt[btm].da;
		*arrvial_time=proc->net_que_list[dat_in].que_elmt[btm].arrvial_time;
		return 1;
	}
}

//任意节点的网络队列读取
static int NetQueRdNext(NTx* proc,unsigned short **dat,unsigned short *len,unsigned short *ra,unsigned short *da,unsigned long * arrvial_time)
{
	unsigned short j,dat_valid;
	dat_valid=0;
	proc->curt_node++;
	proc->curt_node %= NODE_MAX_CNT;
	for(j = proc->curt_node;j < NODE_MAX_CNT; j++)
	{
		if(proc->net_que_list[j].size > 0)//此节点下层闲,可以发送
		{
			dat_valid = NetQueRd(proc,dat,len,ra,da,arrvial_time,j);
		}
		if(dat_valid)
		{
			break;
		}
	}
	proc->curt_node = j;
	if(proc->curt_node == NODE_MAX_CNT)
	{
		proc->curt_node--;
	}
	return dat_valid;
}

//yun:上层业务数据发送请求
static int DataTxReq(Signal* sig)
{
	NTx* proc = (NTx*)sig->dst;
	Data_Tx_Req_Param *param = (Data_Tx_Req_Param*)sig->param;
	//unsigned char printdat_once[] = " DatTxReq";
	unsigned short *dat;
	unsigned short len;
	unsigned short da;
	unsigned short ra;
	unsigned long arrival_time;
	unsigned short tmp_sn;
	Net_Frm* net_frm;  //yun:为 组网络包
	len = param->xlen;//len为整个网络包净荷长度
	printf("-----节点[%d]-----ntx::DataTxReq() 收到业务数据发送请求\n",((P_Entity*)proc->entity)->mib.local_id);
	//yun:以下基本没用------------------------------------------------------------
	if((proc->state==WAIT_RT_RSP)||(len == 0) || (len>NET_PAYLOAD_SIZE))   //yun：不能发送
	{
		//yun：test注释掉
		//if(param->xdat == (unsigned short *)HPI_FRAME_RX_16BUF.DATA)
		//{
		//	SET_MCU_GPIO2_HIGH;//允许ARM下发数据
		//}
		//dat_tx_req_add_sig_flag = 0;//信号dat_tx_req已经加入队列标记
		return 0;  // yun：不能发送，直接返回
	}
	dat = (unsigned short *)MemAllocNet();  //yun:开辟内存
	if(!dat)//无网络包空间   yun：不能发送，重复上面程序
	{
		//yun：test注释掉
		//if(param->xdat == (unsigned short *)HPI_FRAME_RX_16BUF.DATA)
		//{
		//	SET_MCU_GPIO2_HIGH;//允许ARM下发数据
		//}
		//dat_tx_req_add_sig_flag = 0;//信号dat_tx_req已经加入队列标记
		return 0;
	}
	if(param->xda == MULTICAST_DA)//yun：多播
	{
		test_info.snd_net_pkt_gc++;
	}
	else if(param->xda == ((P_Entity*)proc->entity)->mib.local_id)   //yun:广播
	{
		test_info.snd_net_pkt_bc++;
	} 
	else   //yun:单播
	{
		test_info.snd_net_pkt_uc ++;
	}
	//----------------------------------------------------------------------------
	
	da = param->xda;  //目的地址
	ra = 0;
	arrival_time = proc->ntx_local_time;
	//填网络包，数据拷贝 7-param
	WtNetPkt(proc, param->xdat, dat, len, da, param->xpri, param->hello_flag);
	
	if(da == MULTICAST_DA)//多播
	{
		da = ((P_Entity*)proc->entity)->mib.local_id;   //yun:不是广播？？？  也是广播
	}
	//写入队列首/尾  7-param
	NetQueWt(proc, dat, len, ra, da, arrival_time, param->xpri);
	printf("ntx::DataTxReq() 写到队列中\n");
	if(proc->net_que_list[da].size > 1)//此除此包杂形捶⑹荩Ω梅⑺椭暗耐绨?
	{
		NetQueRd(proc,&dat,&len,&ra,&da,&arrival_time,da);//读队列
		printf("ntx::DataTxReq() size>1，读队列\n");
	}
	proc->dat = dat;  //yun:size=1,就一个数据，不用读了
	proc->len = len;
	proc->da = da;
	proc->arrvial_time = arrival_time;
	proc->curt_node = da;//记录上层发数据触发,用于路由失败或下层忙后读队列 NetQueRdNext
	
	if((da !=((P_Entity*)proc->entity)->mib.local_id))//单播  需要先检查路由
	{
		if(proc->state != WAIT_RT_RSP)//没有正在请求路由
		{
			sig = proc->rt_tx_ind;//请求路由
			((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
			AddSignal(sig);
			proc->state = WAIT_RT_RSP;
			proc->data_tx_req_rt_tx_ind_flag = 1; //yun:标记路由  此路由正在占用
		}
	}
	else//广播、多播
	{
		if((!(proc->sts & (1<<da)))&& (!(proc->net_que_list[proc->da].curt_snd_flag))) //yun：状态位？？？，发送标志
		{			
			//填网络头序号
			printf("ntx::DataTxReq() 广播/多播 填写网络头序号\n");
			net_frm = (Net_Frm* )(proc->dat);
			//yun：源地址=本节点号
			if((((net_frm->pri_sa_da_mul_sn) >> 8) &0x1f) == ((P_Entity*)proc->entity)->mib.local_id)//本节点发送的网络包，非中继
			{
				if(((net_frm->pri_sa_da_mul_sn) >> 2) &0x1)//多播标志
				{
					tmp_sn = proc->ntx_sn_mul;//多播序号   （收到回复时++）
				}
				else//广播
				{
					tmp_sn = proc->ntx_sn[proc->da];  // （收到回复时++）
				}
				net_frm->pri_sa_da_mul_sn |= ((tmp_sn>> 6) & 0x3);  //yun:序号  2个拼接起来为真正的序号
				net_frm->sn_len |= (tmp_sn << 10);
			}
			proc->net_que_list[proc->da].curt_snd_flag = 1;  //yun:收到下层回复时清零
			sig=proc->ntx_req;//向下层dlc_tx发送数据
			((T_NTx_Req_Param*)sig->param)->ra = proc->da;
			((T_NTx_Req_Param*)sig->param)->xdat = proc->dat;
			((T_NTx_Req_Param*)sig->param)->xlen = proc->len + NET_FRM_HEAD_LEN;
			AddSignal(sig);
			printf("ntx::DataTxReq() 向下层发送广播网络包,第[%d]包\n",tmp_sn);
			proc->sts |= (1<<(proc->da));//下层此节点忙
		}
		else{
			printf("ntx::DataTxReq() net_que_list[%d]正在占用，等待\n",proc->da);
		}
		//dat_tx_req_add_sig_flag = 0;//信号dat_tx_req已经加入队列标记
		//yun：test注释掉
		//if(param->xdat == (unsigned short *)HPI_FRAME_RX_16BUF.DATA)
		//{
		//	SET_MCU_GPIO2_HIGH;//允许ARM下发数据
		//}
	}
	return 0;
}

//路由查询返回的信号处理函数
static int RtTxRsp(Signal* sig)
{
	NTx* proc = (NTx*)sig->dst;
	Rt_Tx_Rsp_Param *param = (Rt_Tx_Rsp_Param*)sig->param;
	unsigned short *dat;
	unsigned long  arrvial_time;
	unsigned short len = 0;
	unsigned short da,ra;
	unsigned short size,i,tmp_sn;
	unsigned short test_succ_yun;
	unsigned short test_ra_yun;
	Net_Frm* net_frm;

	//yun：test注释掉
	//if(proc->data_tx_req_rt_tx_ind_flag)
	//{
	//	proc->data_tx_req_rt_tx_ind_flag = 0;
	//	SET_MCU_GPIO2_HIGH;//允许ARM下发数据
	//}
	//dat_tx_req_add_sig_flag = 0;//信号dat_tx_req已经加入队列标记
	proc->state=IDLE;
	proc->da = param->da;
	printf("ntx.c::RtTxRsp() 路由查询返回结果\n");
	if((param->succ) && (!(proc->sts & (1<<param->ra))) && (!(proc->net_que_list[proc->da].curt_snd_flag)))//查到路由，下层（ra下一跳）此接收节点闲
	{
		//proc->rt_fail_tmr[proc->da] = 0;
		NetQueRd(proc,&dat,&len,&ra,&da,&arrvial_time,proc->da);//2012.8.31重新读队列，防止当请求路由时，网络包刚发完释放，这时proc->dat为释放的网络包
		if(len)//有数据要发2012.8.31   yun：如果执行最后return；
		{	

			proc->dat = dat;//2012.8.31
			proc->len = len;//2012.8.31
			//填网络头序号
			net_frm = (Net_Frm* )(proc->dat);

			if((((net_frm->pri_sa_da_mul_sn) >> 8) &0x1f) == ((P_Entity*)proc->entity)->mib.local_id)//本节点发送的网络包，非中继  源地址右移8位
			{
				if(((net_frm->pri_sa_da_mul_sn) >> 2) &0x1)//多播
				{
					tmp_sn = proc->ntx_sn_mul;
				}
				else//广单播
				{
					tmp_sn = proc->ntx_sn[proc->da];  //yun:ntx_sn初始化为1
				}
				net_frm->pri_sa_da_mul_sn |= ((tmp_sn>> 6) & 0x3); //yun:??????????
				net_frm->sn_len |= (tmp_sn << 10);
				printf("ntx.c::RtTxRsp() 有数据发，填写网络头序号[%d]\n",tmp_sn);
			}
			proc->net_que_list[proc->da].curt_snd_flag = 1;  //yun:收到下层回复时清零
			sig=proc->ntx_req;//向下层发送数据
			printf("ntx.c::RtTxRsp()发给下层dlc_tx_dump\n");
			((T_NTx_Req_Param*)sig->param)->ra = param->ra; //yun:这个地址是单播时 下一跳的地址 ,真正的目的地址在网络帧头里
			((T_NTx_Req_Param*)sig->param)->xdat = proc->dat;
			((T_NTx_Req_Param*)sig->param)->xlen = proc->len + NET_FRM_HEAD_LEN;
			AddSignal(sig);   // 
			//NTx_Req((((P_Entity*)proc->entity)->mib.local_id),param->ra,proc->dat,proc->len + NET_FRM_HEAD_LEN);//for test
			proc->sts |= (1<<param->ra);//下层此节点忙
			return 0;
		}
	}
	if(!param->succ)//未查到路由
	{
		/*if(!proc->rt_fail_tmr[proc->da])
		{
			proc->rt_fail_tmr[proc->da] = RT_FAIL_TMR;
		}*/
		size = proc->net_que_list[proc->da].size;//2012.8.31
		for(i=0; i<size; i++)//2012.8.31
		{
			/*proc->net_que_list_fm.dat[proc->net_que_list_fm.top] = FlashMemAlloc();
			if(proc->net_que_list_fm.dat[proc->net_que_list_fm.top])
			{
				WriteFlash(proc->net_que_list_fm.dat[proc->net_que_list_fm.top], proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].dat, proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].len  + NET_FRM_HEAD_LEN);			
				proc->net_que_list_fm.top++;
				proc->net_que_list_fm.top %= FLASH_MEM_BLOCK_CNT;
				proc->net_que_list_fm.size++;
			}*/
			DeleteQueElmt(proc,proc->da);//2012.8.31 无路由直接删除
			test_info.snd_fail_net_pkt_uc++;
		}
	}
	NetQueRdNext(proc,&dat,&len,&ra,&da,&arrvial_time);   //yun任意节点的网络队列读取
	if(len)//有数据要发
	{
		proc->dat = dat;
		proc->len = len;
		proc->da = da;
		proc->arrvial_time = arrvial_time;
		sig=proc->rt_tx_ind;//查找路由 
		((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
		AddSignal(sig);
		proc->state = WAIT_RT_RSP;
	}
	return 0;
}

volatile unsigned short rely_err_cnt = 0;

//yun：转发 指示 不需要组包，流程类似DataTxReq（）
static int RelayTxInd(Signal* sig)
{
	NTx* proc=(NTx*)sig->dst;
	Relay_Tx_Ind_Param *param=(Relay_Tx_Ind_Param*)sig->param;
	unsigned short *dat;
	unsigned short len, link_cnt, delay;
	unsigned short da,tmp_da,tmp_sn;
	//unsigned short ra;
	unsigned long  arrvial_time;
	Net_Frm* net_frm;
	arrvial_time=proc->ntx_local_time;
	da = param->xda;
	dat = param->xdat;
	len = param->xlen;
	tmp_da = da;
	if(da == MULTICAST_DA)
	{
		tmp_da = ((P_Entity*)proc->entity)->mib.local_id;
	}
	/*if(malloc_cnt1 > RELAY_TX_MALLOC_CNT_MAX)
	{
		proc->net_que_list_fm.dat[proc->net_que_list_fm.top] = FlashMemAlloc();
		if(proc->net_que_list_fm.dat[proc->net_que_list_fm.top])
		{
			WriteFlash(proc->net_que_list_fm.dat[proc->net_que_list_fm.top],dat, len  + NET_FRM_HEAD_LEN);			
			proc->net_que_list_fm.top++;
			proc->net_que_list_fm.top %= FLASH_MEM_BLOCK_CNT;
			proc->net_que_list_fm.size++;
			return 0;
		}
	}*/

	proc->dat = dat;
	proc->len = len;
	proc->da = tmp_da;
	proc->arrvial_time = arrvial_time;

	NetQueWt(proc, param->xdat, param->xlen, 0, tmp_da, arrvial_time, param->xpri);//写入队列首/尾
	if((da !=((P_Entity*)proc->entity)->mib.local_id) && (da != MULTICAST_DA))//单播
	{
		if(proc->state != WAIT_RT_RSP)//没有正在请求路由
		{
			sig = proc->rt_tx_ind;//请求路由
			((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
			AddSignal(sig);
			proc->state = WAIT_RT_RSP;
		}
	}
	else//广播、多播
	{
		if((!(proc->sts & (1<<da))) && (!(proc->net_que_list[proc->da].curt_snd_flag)))
		{			
			//填网络头序号
			net_frm = (Net_Frm* )(proc->dat);
			if((((net_frm->pri_sa_da_mul_sn) >> 8) &0x1f) == ((P_Entity*)proc->entity)->mib.local_id)//本节点发送的网络包，非中继
			{
				if(((net_frm->pri_sa_da_mul_sn) >> 2) &0x1)//多播
				{
					tmp_sn = proc->ntx_sn_mul;
				}
				else//广播
				{
					tmp_sn = proc->ntx_sn[proc->da];
				}
				net_frm->pri_sa_da_mul_sn |= ((tmp_sn>> 6) & 0x3);
				net_frm->sn_len |= (tmp_sn << 10);
			}
			proc->net_que_list[proc->da].curt_snd_flag = 1;
			sig=proc->ntx_req;//向下层发送数据
			((T_NTx_Req_Param*)sig->param)->ra = proc->da;
			((T_NTx_Req_Param*)sig->param)->xdat = proc->dat;
			((T_NTx_Req_Param*)sig->param)->xlen = proc->len + NET_FRM_HEAD_LEN;
			AddSignal(sig);
			//NTx_Req((((P_Entity*)proc->entity)->mib.local_id),param->ra,proc->dat,proc->len + NET_FRM_HEAD_LEN);//for test
			proc->sts |= (1<<(proc->da));//下层此节点忙
		}
	}

	return 0;
}

//yun：收到下层dlc发来的回复
static int NTxInd(Signal* sig)
{
	NTx* proc=(NTx*)sig->dst;
	NTx_Ind_Param *param=(NTx_Ind_Param*)sig->param;

	unsigned short *dat;
	unsigned short len = 0;
	unsigned short is_vaid = 0;
	unsigned short ra;
	unsigned short da;
	unsigned short* tmp_dat;
	unsigned short tmp_ra,tmp_da,tmp_sa;
	unsigned long arrvial_time;
	unsigned short i,size,mul_flag;

	Net_Frm* net_frm = (Net_Frm* )param->dat;
	unsigned char printdat_once[20] = " NTxInd";
	//PrintDat(printdat_once,7,1);
	//PrintDat(0,0,0x0006);
	if(param->dat == NULL)  //yun:数据为空
	{
		//PrintDat(0,0,0xaaaa);
		return 0;
	}

	tmp_da = param->da;  //yun：目的节点
	tmp_sa = param->sa;  //yun:源节点
	mul_flag = ((net_frm->pri_sa_da_mul_sn)>>2) & 0x1;  //yun:多播标志
	if(mul_flag)
	{
		tmp_da = ((P_Entity*)proc->entity)->mib.local_id;
	}
	
	tmp_dat=param->dat;
	proc->sts &= (~(1<<param->ra));//下层此节点闲
	proc->net_que_list[tmp_da].curt_snd_flag = 0;//四康慕诘阄拚诜⑺桶?
	printf("ntx.c::NTxInd() 收到dlc_tx_dump的网络包发送成功回复\n");
	if(param->succ_flag)//底层发送成功
	{
		if(tmp_sa == ((P_Entity*)proc->entity)->mib.local_id)//本节点发送的网络包，非中继
		{
			if(mul_flag)//多播
			{
				proc->ntx_sn_mul ++;
			}
			else
			{
				proc->ntx_sn[tmp_da]++;//网络序号+1
				//printf("ntx::NTxInd() 网络序号+1，当前序号为：[%d]\n", proc->ntx_sn[tmp_da]);
			}
		}
		DeleteQueElmt(proc,tmp_da);//删除网络包队列
		if((proc->net_que_list[tmp_da].size) && ((proc->state!=WAIT_RT_RSP))) //|| (tmp_da==((P_Entity*)proc->entity)->mib.local_id)))//此目的节点仍有未发数据
		{
			is_vaid = NetQueRd(proc,&dat,&len,&ra,&da,&arrvial_time,tmp_da);//读队列
			printf("ntx.c::NTxInd() 检查该目的节点队列是否还有数据： %d\n",is_vaid);
		}			
	}
	else//下层发送失败
	{
		if(param->fail_type == SEND_FAIL_TYPE_RESEND)//重发失败 2012.9.10
		{
			/*if(!proc->rt_fail_tmr[tmp_da])
			{
				proc->rt_fail_tmr[tmp_da] = RT_FAIL_TMR;
			}*/
			size = proc->net_que_list[tmp_da].size;//2012.8.31
			for(i=0; i<size; i++)//2012.8.31
			{
				/*proc->net_que_list_fm.dat[proc->net_que_list_fm.top] = FlashMemAlloc();
				if(proc->net_que_list_fm.dat[proc->net_que_list_fm.top])
				{
					WriteFlash(proc->net_que_list_fm.dat[proc->net_que_list_fm.top], proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].dat, proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].len  + NET_FRM_HEAD_LEN);			
					proc->net_que_list_fm.top++;
					proc->net_que_list_fm.top %= FLASH_MEM_BLOCK_CNT;
					proc->net_que_list_fm.size++;
				}*/
				DeleteQueElmt(proc,tmp_da);//2012.8.31删除网络包队列 
				test_info.snd_fail_net_pkt_uc++;
			}
			//tmp_ra = param->ra;	
			//sig=proc->link_fail_ind;//指示路由失败
			//((T_Link_Fail_Ind_Param*)sig->param)->ra = tmp_ra;
			//AddSignal(sig);	
		}
	}
	
	if((!is_vaid) && (proc->state!=WAIT_RT_RSP))
	{
		proc->curt_node = tmp_da;//网络层发完数据触发,用于读队列 NetQueRdNext
		is_vaid = NetQueRdNext(proc,&dat,&len,&ra,&da,&arrvial_time);//读其他节点数据
		if(!is_vaid)//先查（tmp_da+1）至（NODE_MAX_CNT-1），再查 0至tmp_da是否有网络包
		{
			is_vaid = NetQueRdNext(proc,&dat,&len,&ra,&da,&arrvial_time);//读其他节点数据
			printf("ntx.c::NTxInd() 检查其他节点是否有数据发送 ：%d\n",is_vaid);
			if(da == tmp_da)
			{
				is_vaid = 0;
			}
		}
	}
	
	if(is_vaid)//可以发送数据
	{
		proc->dat=dat;
		proc->len=len;
		proc->da=da;
		proc->arrvial_time = arrvial_time;
		sig = proc->rt_tx_ind;//请求路由
		((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
		AddSignal(sig);
		proc->state=WAIT_RT_RSP;
		printf("ntx.c::NTxInd()	可以发送数据，请求路由，等待指示\n");
	}
	else//不可以发送数据
	{
		proc->state=IDLE;
		printf("ntx.c::NTxInd() 没有数据发送了\n");
	}
	return 0;
}
		
//void TimerNTx(NTx* proc)
//{
//	unsigned short i,size,k,btm,len,da,pri,sa;
//	unsigned long time_len;
//	Signal *sig;
//	proc->ntx_local_time++;//定时器
//	
//	/*for(i=0;i<NODE_MAX_CNT;i++)
//	{
//		if(proc->rt_fail_tmr[i])
//		{
//			proc->rt_fail_tmr[i]--;
//		}
//	}*/
//
//	if(proc->ntx_local_time == NET_PKT_RESEND_FROM_FM_TMR)
//	{
//		proc->ntx_local_time = 0;
//		size = proc->net_que_list_fm.size;
//		if(size)
//		{
//			if((malloc_cnt1 < READ_FM_MALLOC_CNT_MAX)&&(!dat_tx_req_add_sig_flag))
//			{
//				btm = proc->net_que_list_fm.btm;
//				//ReadFlash(proc->read_net_pkt_from_fm, proc->net_que_list_fm.dat[btm], NET_FRM_LEN_MAX);
//				proc->net_que_list_fm.btm++;
//				//proc->net_que_list_fm.btm %= FLASH_MEM_BLOCK_CNT;
//				proc->net_que_list_fm.size--;
//				len = ((Net_Frm*)(proc->read_net_pkt_from_fm))->sn_len & 0x1ff;
//				da = (((Net_Frm*)(proc->read_net_pkt_from_fm))->pri_sa_da_mul_sn >>3)  & 0x1f;//目的地址
//				pri = (((Net_Frm*)(proc->read_net_pkt_from_fm))->pri_sa_da_mul_sn)>> 13 ;
//				sa = ((((Net_Frm*)(proc->read_net_pkt_from_fm))->pri_sa_da_mul_sn >> 8) & 0x1f);//源地址
//				if(sa != ((P_Entity*)proc->entity)->mib.local_id)//单播转发
//				{
//					sig=&proc->relay_tx_ind;
//					((Relay_Tx_Ind_Param*)sig->param)->xdat = proc->read_net_pkt_from_fm;
//					((Relay_Tx_Ind_Param*)sig->param)->xlen = len;
//					((Relay_Tx_Ind_Param*)sig->param)->xda=da;
//					((Relay_Tx_Ind_Param*)sig->param)->xpri= pri;
//					RelayTxInd(sig);
//				}
//				else
//				{
//					//AppTxReq(&proc->read_net_pkt_from_fm[NET_FRM_HEAD_LEN],len, da, pri,0);
//
//				}
//				//if(proc->rt_fail_tmr[da])
//				//{
//				//	AppTxReq(&proc->read_net_pkt_from_fm[NET_FRM_HEAD_LEN],len, da, pri,0);
//				//}
//			}
//		}
//		/*for(i=0;i<NODE_MAX_CNT;i++)
//		{
//			if(proc->net_que_list[i].size!=0)
//			{
//				size=proc->net_que_list[i].size;
//				for(k=0;k<size;k++)
//				{
//					btm=proc->net_que_list[i].btm;			
//					if(proc->ntx_local_time > proc->net_que_list[i].que_elmt[btm].arrvial_time)
//					{
//						time_len = proc->ntx_local_time - proc->net_que_list[i].que_elmt[btm].arrvial_time;
//					}
//					else
//					{
//						time_len = proc->ntx_local_time + (0xffffffff - proc->net_que_list[i].que_elmt[btm].arrvial_time);
//					}
//					if(time_len < NET_PKT_MAX_TTL)//未超时，非首次网络包
//					{
//						break;	
//					}
//					else//网络包数据超时,删除
//					{						
//						DeleteQueElmt(proc,i);
//					}
//				}
//			}		
//		}*/
//	}
//	return ;
//}

void NTxInit(NTx* proc)
{
	int i,k;
	proc->state=IDLE;
	proc->dat=0;
	proc->len=0;
	proc->da=0;
	proc->arrvial_time=0;
	proc->curt_node=0;
	proc->ntx_local_time=0;
	proc->sts=0;
	proc->ntx_sn_mul = 1;//初始序号！=0
	proc->data_tx_req_rt_tx_ind_flag = 0;
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->ntx_sn[i]=1;//初始序号！=0
		proc->net_que_list[i].btm=0;
		proc->net_que_list[i].top=0;
		proc->net_que_list[i].size=0;
		proc->net_que_list[i].curt_snd_flag=0;
		for(k=0;k<NET_QUE_MAX_NUM;k++)
		{
			proc->net_que_list[i].que_elmt[k].dat=0;
			proc->net_que_list[i].que_elmt[k].len=0;
			proc->net_que_list[i].que_elmt[k].ra=0;
			proc->net_que_list[i].que_elmt[k].da=0;
			proc->net_que_list[i].que_elmt[k].arrvial_time=0;
			proc->net_que_list[i].que_elmt[k].first_send_flag=0;
		}
	}
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->ntx_ind[i].next=0;
		proc->ntx_ind[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->ntx_ind[i].src=0;
		proc->ntx_ind[i].dst=proc;
		proc->ntx_ind[i].func=NTxInd;
		proc->ntx_ind[i].pri=SDL_PRI_URG;
		proc->ntx_ind[i].param=&proc->ntx_ind_param[i];
		//proc->rt_fail_tmr[i] = 0;
	}
	proc->net_que_list_fm.btm = 0;
	proc->net_que_list_fm.top = 0;
	proc->net_que_list_fm.size = 0;
}
void NTxSetup(NTx* proc)
{
	proc->data_tx_req.next=0;
	proc->data_tx_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->data_tx_req.src=0;
	proc->data_tx_req.dst=proc;
	proc->data_tx_req.func=DataTxReq;
	proc->data_tx_req.pri=SDL_PRI_URG;
	proc->data_tx_req.param=&proc->data_tx_req_param;

	proc->rt_tx_rsp.next=0;
	proc->rt_tx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->rt_tx_rsp.src=0;
	proc->rt_tx_rsp.dst=proc;
	proc->rt_tx_rsp.func=RtTxRsp;
	proc->rt_tx_rsp.pri=SDL_PRI_URG;
	proc->rt_tx_rsp.param=&proc->rt_tx_rsp_param;

	proc->relay_tx_ind.next=0;
	proc->relay_tx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->relay_tx_ind.src=0;
	proc->relay_tx_ind.dst=proc;
	proc->relay_tx_ind.func=RelayTxInd;
	proc->relay_tx_ind.pri=SDL_PRI_URG;
	proc->relay_tx_ind.param=&proc->relay_tx_ind_param;


   
}
