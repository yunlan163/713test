#include "arq_rx.h"
#include <stdio.h>
enum {IDLE,WAIT_RSP};//模块状态
enum {ARQ_STS_IDLE,ARQ_STS_RXW,ARQ_STS_DATAW};//窗口状态:非接收窗、接收窗没数据、接收窗有数据

typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;

//yun:发送ack的子程序
void SndAckProc(Arq_Rx *proc)
{
	Signal* sig;
	unsigned short ack,sn,mask,sts;
	ack = proc->btm << 12;//期望
	sn = proc->btm +1;
	sn %= ARQ_SN_SIZE;
	sts = 0;
	mask = 0x0040;
	while(mask)//期望后7窗的状态
	{
		if(ARQ_STS_DATAW == proc->sts[sn])//已经收到数据
		{
			sts |= mask;
		}
		sn++;
		sn %= ARQ_SN_SIZE;
		mask >>= 1;
	}
	ack = (ack|(sts<<5));
	proc->ack_sub_frm.sn_sts = ack;
	sig = proc->arq_ack_tx_req;
	
	((T_Arq_Ack_Tx_Req_Param *)sig->param)->ack_sgl = (unsigned short*)&proc->ack_sub_frm;
	//((T_Arq_Ack_Tx_Req_Param *)sig->param)->len = SUB_FRM_LEN_ACK;//2
	((T_Arq_Ack_Tx_Req_Param *)sig->param)->id = ((P_Entity*)proc->entity)->mib.local_id;  //
	AddSignal(sig);
	printf("arq_rx[%d]::SndAckProc() 发送ack给dlc_rx_ctrl\n",proc->id);
	return;
	//第一次已经
	//proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK; //0xd000
	//proc->ack_sub_frm.flag_ra_rst |= (proc->id) << 4;//填ack中的接收节点号
	//proc->ack_sub_frm.flag_ra_rst |= 0x0008;//ack中响应发方重置
}
//yun：收到来自dlc_rx_ctrl的单播数据
static int ArqRxInd(Signal* sig)
{
	Arq_Rx* proc=(Arq_Rx*)sig->dst;
	Arq_Rx_Ind_Param *param=(Arq_Rx_Ind_Param*)sig->param;

	Data_Sub_Frm* data_sub_frm;
	unsigned short sn,tmp_rst_flag,seg_flag;
	unsigned short* tmp_dat;
	unsigned short tmp_len,i,btm;
	data_sub_frm = ((Data_Sub_Frm*)param->xfrm);  //yun:帧地址
	sn = (data_sub_frm->seg_len_sn_rst>>1) & 0x000f;//序号
	tmp_rst_flag = data_sub_frm->seg_len_sn_rst & 0x0001;//发方重置标志

	////////////////////////////////重置处理///////////////////////////////////////////
	if(tmp_rst_flag)//发方重置标记
	{
		if(proc->rst_flag)//收方已经重置过
		{
			proc->rst_flag = 0;
		}
		//窗口初始化    yun：发方重置，收方没有重置的情况下，要重置
		btm = proc->btm;
		for(i=0;i<ARQ_WIN_SIZE;i++)  //8
		{
			if(proc->sts[btm] == ARQ_STS_DATAW)
			{
				MemFreeMac((unsigned short *)proc->mac_frm[btm]);
				proc->mac_frm[btm] = NULL;
			}
			btm++;
			btm %= ARQ_SN_SIZE; //16
		}
		proc->state = IDLE;
		proc->btm = 0;
		proc->top = ARQ_WIN_SIZE;
		proc->idx = 0;
		//窗口状态初始化
		for(i=0;i<proc->top;i++)
		{
			proc->sts[i]=ARQ_STS_RXW;
		}
		for(i=proc->top;i<ARQ_SN_SIZE;i++)
		{
			proc->sts[i]=ARQ_STS_IDLE;
		}
		sig = proc->frm_rx_free_ind;//清空分段模块保存的数据  yun:在发方重置的情况下
		AddSignal(sig);
		//yun：ack
		proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK;
		proc->ack_sub_frm.flag_ra_rst |= (proc->id) << 4;//填ack中的接收节点号
		proc->ack_sub_frm.flag_ra_rst |= 0x0008;//ack中响应发方重置
	}
	else  //yun：发方没有重置，
	{
		//proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK;
		//proc->ack_sub_frm.flag_ra_rst |= proc->id<<4;//填ack中的接收节点号
		//yun：添加的下一行
		proc->ack_sub_frm.flag_ra_rst &= 0xFFF7;//ack中响应发方没有重置  发方没有重置，说明这是不是第一次收到单播，srt=0
		if(proc->rst_flag)//收方重置
		{
			MemFreeMac(param->xfrm);
			//sig = proc->arq_rst_tx_req;//发重置信令
			//((T_Arq_Rst_Tx_Req_Param*)sig->param)->rst_sgl = (unsigned short *)&proc->rst_sub_frm;
			//((T_Arq_Rst_Tx_Req_Param*)sig->param)->len = SUB_FRM_LEN_RST;
			//AddSignal(sig);
			sig = proc->arq_rx_rsp;
			AddSignal(sig);
			proc->state = IDLE;
			return 0;
		}
	}

	///////////////////////////////////arq窗口接收数据处理/////////////////////////////
	if(proc->sts[sn] == ARQ_STS_RXW)//接收窗且无数据
	{
		//保存数据
		proc->mac_frm[sn] = (Mac_Frm_1002*)param->xfrm;//memcpy(&proc->mac_frm[sn],param->xfrm,param->xlen);//保存数据
		proc->sts[sn] = ARQ_STS_DATAW;//把数据写进对应序号的窗口，变状态为ARQ_STS_DATAW
		if(sn == proc->btm)//收到窗底
		{			
			do//移动窗口
			{
				proc->btm++;
				proc->btm %=ARQ_SN_SIZE;
				proc->sts[proc->top] = ARQ_STS_RXW;
				proc->top++;
				proc->top %=ARQ_SN_SIZE;
			}while(proc->sts[proc->btm] == ARQ_STS_DATAW);
			//数据送至frm_rx
			
			data_sub_frm = (Data_Sub_Frm *)proc->mac_frm[proc->idx];  //yun:在收到frm_rx回复的rsq后++
			tmp_dat = data_sub_frm->data;
			tmp_len = ((data_sub_frm->seg_len_sn_rst >> 5) & 0x003f);
			seg_flag = (data_sub_frm->seg_len_sn_rst) & 0x6000;
			sig = proc->frm_rx_ind;  //yun：单播给frm_rx
			((T_Frm_Rx_Ind_Param*)sig->param)->xdat = tmp_dat;
			((T_Frm_Rx_Ind_Param*)sig->param)->xlen = tmp_len;
			((T_Frm_Rx_Ind_Param*)sig->param)->seg_flag = seg_flag;
			((T_Frm_Rx_Ind_Param*)sig->param)->bc_uc_falg = 0;//单播
			AddSignal(sig);
			proc->state = WAIT_RSP;
			printf("arq_rx[%d]::ArqRxInd() 发送数据给上层frm_rx\n",proc->id);
		/*	if(tmp_len == 58)
			{
				sig = proc->frm_rx_ind;
			((T_Frm_Rx_Ind_Param*)sig->param)->xdat = tmp_dat;
			((T_Frm_Rx_Ind_Param*)sig->param)->xlen = tmp_len;
			((T_Frm_Rx_Ind_Param*)sig->param)->seg_flag = seg_flag;
			((T_Frm_Rx_Ind_Param*)sig->param)->bc_uc_falg = 0;//单播

			}*/
			return 0;  //yun:结束
		}
	}
	else
	{
		MemFreeMac(param->xfrm);
	}	
	//SndAckProc(proc);//发送ack处理 yun for test 
	sig = proc->arq_rx_rsp;
	AddSignal(sig);
	proc->state = IDLE;
	return 0;
}
//yun：收到frm_rx的回复
static int FrmRxRsp(Signal* sig)
{
	Arq_Rx* proc=(Arq_Rx*)sig->dst;
	//Frm_Rx_Rsp_Param *param=(Frm_Rx_Rsp_Param*)sig->param;
	
	Data_Sub_Frm* data_sub_frm;
	unsigned short seg_flag;
	unsigned short* tmp_dat;
	unsigned short tmp_len;
	MemFreeMac((unsigned short *)proc->mac_frm[proc->idx]);//释放MAC包
	proc->mac_frm[proc->idx] = NULL;
	proc->sts[proc->idx] = ARQ_STS_IDLE;//yun:接收成功了，改变标志位
	proc->idx ++;
	proc->idx %= ARQ_SN_SIZE;
	printf("arq_rx[%d]::FrmRxRsp() 收到上层frm_rx的回复\n",proc->id);
	if(ARQ_STS_DATAW == proc->sts[proc->idx])//仍有数据可以送至frm_rx
	{
		
		data_sub_frm = (Data_Sub_Frm *)proc->mac_frm[proc->idx];  //yun:已经是下一帧了
		tmp_dat = data_sub_frm->data;
		tmp_len = ((data_sub_frm->seg_len_sn_rst >> 5) & 0x003f);
		seg_flag = (data_sub_frm->seg_len_sn_rst) & 0x6000;
		sig = proc->frm_rx_ind;//数据送至frm_rx  yun：再次发送
		((T_Frm_Rx_Ind_Param*)sig->param)->xdat = tmp_dat;
		((T_Frm_Rx_Ind_Param*)sig->param)->xlen = tmp_len;
		((T_Frm_Rx_Ind_Param*)sig->param)->seg_flag = seg_flag;
		AddSignal(sig);
		printf("arq_rx[%d]::FrmRxRsp() 还有数据，继续发送给frm_rx\n",proc->id);
	}
	else//无数据可以送至frm_rx,即idx == btm
	{
		printf("arq_rx[%d]::FrmRxRsp() 没有数据了，准备回复ack\n",proc->id);
		SndAckProc(proc);//发送ack处理  yun for test ？？？？
		sig = proc->arq_rx_rsp;
		AddSignal(sig);
		proc->state = IDLE;	
	}
	return 0;
}

void ArqRxInit(Arq_Rx *proc)
{
	unsigned short i;
	proc->state = IDLE;
	proc->rst_flag = 1;
	proc->btm = 0;
	proc->top = ARQ_WIN_SIZE;
	proc->idx = 0;
	proc->emergency_ack = 0;
	//窗口状态初始化
	for(i=0;i<proc->top;i++)
	{
		proc->sts[i]=ARQ_STS_RXW;
	}
	for(i=proc->top;i<ARQ_SN_SIZE;i++)
	{
		proc->sts[i]=ARQ_STS_IDLE;
	}
	for(i=0;i<ARQ_SN_SIZE;i++)
	{
		proc->mac_frm[i] = 0;
	}
	//待发ack信令部分初始化
	proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK;
	proc->ack_sub_frm.flag_ra_rst |= (proc->id)<<4;
	//待发arq重置信令初始化
	proc->rst_sub_frm.flag_ra = SUB_FRM_TYPE_RST;
	proc->rst_sub_frm.flag_ra |= (proc->id)<<4;
}

void ArqRxSetup(Arq_Rx *proc)
{
	proc->arq_rx_ind.next=0;
	proc->arq_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->arq_rx_ind.src=0;
	proc->arq_rx_ind.dst=proc;
	proc->arq_rx_ind.func=ArqRxInd;
	proc->arq_rx_ind.pri=SDL_PRI_URG;
	proc->arq_rx_ind.param=&proc->arq_rx_ind_param;

	proc->frm_rx_rsp.next=0;
	proc->frm_rx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_rx_rsp.src=0;
	proc->frm_rx_rsp.dst=proc;
	proc->frm_rx_rsp.func=FrmRxRsp;
	proc->frm_rx_rsp.pri=SDL_PRI_URG;
	proc->frm_rx_rsp.param=&proc->frm_rx_rsp_param;

}
