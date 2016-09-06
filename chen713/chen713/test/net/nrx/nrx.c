#include "nrx.h"
#include <stdio.h>
enum {IDLE,WAIT_RSP};

typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun:dlc发来的上传数据  普通数据or视频数据
static int NRxInd(Signal* sig)
{
	NRx *proc=(NRx*)sig->dst;
	NRx_Ind_Param * param=(NRx_Ind_Param*)sig->param;
	
	unsigned short *dat,sa,da,sn,pri,tmp_da,mul_flag;
	unsigned short test_nu;  //yun: for test yun
	unsigned short len,uc_bc_flag;

	Net_Frm *net_frm = (Net_Frm *)param->xdat;//网络帧地址
	len = param->xlen - NET_FRM_HEAD_LEN;//净荷长度
	dat = net_frm->net_paload;//净荷地址

	test_nu = len/2;  //for test yun
	sa = ((net_frm->pri_sa_da_mul_sn >> 8) & 0x1f);//源地址
	da = (net_frm->pri_sa_da_mul_sn >>3)  & 0x1f;//目的地址
	sn = ((net_frm->pri_sa_da_mul_sn  & 0x3) << 6)|((net_frm->sn_len >> 10)) ; //yun：两个序号合并才是真正的一个网络包序号
	printf("-------节点[%d]------nrx::NRxInd() 收到第[%d]包\n", ((P_Entity*)proc->entity)->mib.local_id, sn);
	pri = net_frm->pri_sa_da_mul_sn >> 13 ;
	mul_flag = (net_frm->pri_sa_da_mul_sn >>2)  & 0x1;//多播标记
	tmp_da = da;
	if(mul_flag)//多播
	{
		tmp_da = sa;
		//旧网络包，不接收
		if((sa == ((P_Entity*)proc->entity)->mib.local_id)||(!(((sn > proc->nrx_sn_mul[sa]) && ((sn - proc->nrx_sn_mul[sa]) < 128)) || ((sn < proc->nrx_sn_mul[sa]) && ((proc->nrx_sn_mul[sa] - sn)>128)))))
		{
			printf("nrx::NRxInd() 旧的网络包，不接受，释放下层数据\n");
			MemFreeNet(param->xdat);//释放下层数据
			sig = proc->nrx_rsp;//回复下层
			AddSignal(sig);
			proc->state = IDLE;
			if((!(((sn > proc->nrx_sn_mul[sa]) && ((sn - proc->nrx_sn_mul[sa]) < 128)) || ((sn < proc->nrx_sn_mul[sa]) && ((proc->nrx_sn_mul[sa] - sn)>128)))))
			{
				if(sn != proc->nrx_sn_mul[sa])
				{
					//printf("debug");
				}
			}
			return 0;
		}
		else//保存网络包序号
		{
			proc->nrx_sn_mul[sa] = sn;
		}
	}
	else//单播、广播  保存网络包序号
	{
		if(sn == proc->nrx_sn[sa][tmp_da])//重复接收网络包
		{
			printf("nrx::NRxInd()重复接收网络包，释放下层数据\n");
			MemFreeNet(param->xdat);//释放下层数据
			sig = proc->nrx_rsp;//回复下层
			AddSignal(sig);
			proc->state = IDLE;
			return 0;
		}
		else//保存网络包序号
		{
			proc->nrx_sn[sa][tmp_da] = sn;
		}
	}
	
	if(mul_flag)//多播
	{
		uc_bc_flag = MC_NET_FRM_FLAG;
		//AppRxInd(dat,len,sa,uc_bc_flag,pri);//for test 向上层交数据
		proc->dat_wait_free = param->xdat;//保存待释放数据地址
		sig = proc->drelay_chk_req;  //yun：给rt_ctrl 路由控制  查看是否需要转发
		((T_DRelay_Chk_Req_Param*)sig->param)->sa = sa;
		AddSignal(sig);
	}
	else if((da != ((P_Entity*)proc->entity)->mib.local_id) && (da != sa))//单播转发  //不是广播，也不是单播给本节点
	{
		sig=proc->relay_tx_ind; //yun：给ntx 中继
		((T_Relay_Tx_Ind_Param*)sig->param)->xdat=param->xdat;
		((T_Relay_Tx_Ind_Param*)sig->param)->xlen=param->xlen-NET_FRM_HEAD_LEN;
		((T_Relay_Tx_Ind_Param*)sig->param)->xda=da;
		((T_Relay_Tx_Ind_Param*)sig->param)->xpri= pri;
		AddSignal(sig);

		sig=proc->nrx_rsp;  //yun:给下层回复
		AddSignal(sig);	
		proc->state = IDLE;
	}
	else//单播收、广播
	{	
		if(da == sa)//广播
		{
			uc_bc_flag = BC_NET_FRM_FLAG;
		}		
		else //单播
		{
			uc_bc_flag = UC_NET_FRM_FLAG;
		}
		//sig=proc->data_rx_ind;//向上层交数据
		//((T_Data_Rx_Ind_Param*)sig->param)->dat=dat;
		//((T_Data_Rx_Ind_Param*)sig->param)->len=len;
		//((T_Data_Rx_Ind_Param*)sig->param)->sa=sa;
		//AddSignal(sig);
		//AppRxInd(dat,len,sa,uc_bc_flag,pri);//for test 向上层交数据

		//for test yun
		if(1==uc_bc_flag){
			printf("---节点[%d]---nrx::NRxInd() 收到第sn=%d包广播网络包, \n网络包数据为：",((P_Entity*)proc->entity)->mib.local_id,sn);
			while(test_nu--){
				printf("%d,",*(dat++));
			}
			printf("\n");
		}else if(2==uc_bc_flag){
			printf("nrx::NRxInd() 收到第sn=%d包多播网络包\n",sn);
		}else{
			printf("---节点[%d]---nrx::NRxInd() 收到[%d]发来的第sn=%d包单播网络包, \n网络包数据为：",((P_Entity*)proc->entity)->mib.local_id, sa ,sn);
			while(test_nu--){
				printf("%d,",*(dat++));
			}
			printf("\n");
		}

		proc->dat_wait_free = param->xdat;//保存待释放数据地址
		proc->state = WAIT_RSP;
		//回复本模块接收完成，数据必须在AppRxInd中拷贝完成
		sig = &proc->data_rx_rsp;  //yun:给自己本模块，释放数据、向下层dlc回复
		AddSignal(sig);		
	}
	
	return 0;
}
//yun:多播转发查询回复cfm   给ntx转发or释放指示
static int DRelayChkCfm(Signal* sig)
{
	NRx *proc=(NRx*)sig->dst;
	DRelay_Chk_Cfm_Param * param=(DRelay_Chk_Cfm_Param*)sig->param; 

	unsigned short pri,len;
	Net_Frm *net_frm ;
	if(param->succ_flag)//需要转发   //1-中继，0-不中继
	{
		net_frm = (Net_Frm *)(proc->dat_wait_free);//网络帧地址  //yun:再取数据
		len = (net_frm->sn_len) & 0x3ff;//净荷长度
		//sa = ((net_frm->pri_sa_da_mul_sn >> 8) & 0x1f);//源地址
		pri = net_frm->pri_sa_da_mul_sn >> 13 ;
		sig=proc->relay_tx_ind;   //yun:多播转发  给ntx::RelayTxInd()
		((T_Relay_Tx_Ind_Param*)sig->param)->xdat=(unsigned short*)net_frm;
		((T_Relay_Tx_Ind_Param*)sig->param)->xlen=len;
		((T_Relay_Tx_Ind_Param*)sig->param)->xda=MULTICAST_DA;
		((T_Relay_Tx_Ind_Param*)sig->param)->xpri= pri;
		AddSignal(sig);
	}
	else//不转发
	{
		printf("nrx::DRelayChkCfm()多播查询后不转发，释放下层数据\n");
		MemFreeNet(proc->dat_wait_free);//释放数据
		proc->dat_wait_free = NULL;
	}
	sig=proc->nrx_rsp;
	AddSignal(sig);	
	proc->state = IDLE;
	return 0;
}
//yun：本模块给自己的  收到单播、广播网络包后，释放数据，向下层dlc_rx回复，
static int DataRxRsp(Signal* sig)
{
	NRx *proc=(NRx*)sig->dst;
	//Data_Rx_Rsp_Param * param=(Data_Rx_Rsp_Param*)sig->param;
	printf("nrx::DataRxRsp() nrx网络包确认收到，释放数据\n");
	MemFreeNet(proc->dat_wait_free);//释放数据  //yun:指针内存上的数据释放
	proc->dat_wait_free = NULL;   //yun：指针指向NULL
	sig = proc->nrx_rsp;
	AddSignal(sig);
	printf("nrx::DataRxRsp() 回复下层\n");
	proc->state = IDLE;
	return 0;
}

void NRxInit(NRx *proc)
{
	unsigned short i,j;
	proc->state=IDLE;
	proc->dat_wait_free = NULL;
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->nrx_sn_mul[i] = 0;
		for(j=0;j<NODE_MAX_CNT;j++)
		{
			proc->nrx_sn[i][j] = 0;
		}
	}

}

void NRxSetup(NRx *proc)
{
	proc->data_rx_rsp.next=0;
	proc->data_rx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->data_rx_rsp.src=0;
	proc->data_rx_rsp.dst=proc;
	proc->data_rx_rsp.func=DataRxRsp;
	proc->data_rx_rsp.pri=SDL_PRI_URG;

	proc->nrx_ind.next=0;
	proc->nrx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->nrx_ind.src=0;
	proc->nrx_ind.dst=proc;
	proc->nrx_ind.func=NRxInd;
	proc->nrx_ind.pri=SDL_PRI_URG;
	proc->nrx_ind.param=&proc->nrx_ind_param;

	proc->drelay_chk_cfm.next=0;
	proc->drelay_chk_cfm.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->drelay_chk_cfm.src=0;
	proc->drelay_chk_cfm.dst=proc;
	proc->drelay_chk_cfm.func=DRelayChkCfm;
	proc->drelay_chk_cfm.pri=SDL_PRI_URG;
	proc->drelay_chk_cfm.param=&proc->drelay_chk_cfm_param;
}

