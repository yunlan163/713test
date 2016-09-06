#include "topo_ctrl.h"
#include <string.h>
enum {IDLE};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;


static int HelloRxInd(Signal* sig)
{
	Topo_Ctrl* proc = (Topo_Ctrl*)sig->dst;
	Hello_Rx_Ind_Param* param = (Hello_Rx_Ind_Param*)sig->param;
	
	unsigned short tmp_sts,chg_sts,local_id;
	volatile unsigned long tmp_link,nbr_info;
	RTI_Sub_Frm* rti_frm = (RTI_Sub_Frm*)(param->hello_pkt);
	tmp_sts = proc->nbr_table.nbr_sts[param->xta];
	local_id =((P_Entity*)proc->entity)->mib.local_id;
	nbr_info = (((unsigned long)(*(((unsigned short*)rti_frm)+2))) << 16) | (*(((unsigned short*)rti_frm)+3));
	//nbr_info = rti_frm->nbr_info;//2012.8.29 vs适用
	tmp_link = nbr_info & (0x00000001 << local_id);//hello中邻节点信息和本节点关系

	//更新邻节点表
	proc->nbr_table.nbr_ttl[param->xta] = MAX_NBR_TTL;
	if(tmp_link)//hello包发送节点可以收到本节点，置双向
	{
		proc->nbr_table.nbr_sts[param->xta] = 0x3;
		chg_sts = 1;
	}
	else//hello包发送节点不可以收到本节点，置单项
	{
		proc->nbr_table.nbr_sts[param->xta] = 0x1;
		chg_sts = 0;
	}
	if(((proc->nbr_table.nbr_sts[param->xta] == 0x3) && (tmp_sts != 0x3))||((proc->nbr_table.nbr_sts[param->xta] != 0x3) && (tmp_sts == 0x3)))//链路变双向、链路变非双向  
	{
		sig = proc->nbr_sts_chg_ind;//指示上层状态改变
		((T_Nbr_Sts_Chg_Ind_Param*)sig->param)->id = param->xta;
		((T_Nbr_Sts_Chg_Ind_Param*)sig->param)->link_sts = chg_sts;
		AddSignal(sig);
		//sprintf(tmp_short,"链路状态改变，链接状况 %d \n",chg_sts);
		//DbgPrint(tmp_short);
	}
	if(proc->nbr_table.nbr_sts[param->xta] == 0x03)//双向连接
	{
		sig = proc->hello_rt_rx_ind;//将hello包送至上层
		((T_Hello_Rt_Rx_Ind_Param*)sig->param)->hello_pkt = param->hello_pkt;
		((T_Hello_Rt_Rx_Ind_Param*)sig->param)->xta = param->xta;
		AddSignal(sig);
	}
	else
	{	//释放interface中malloc的HELLO包
		MemFreeMac((param->hello_pkt) - NET_FRM_HEAD_LEN - DATA_SUB_FRM_HEAD_LEN);
	}
	return 0;	
}

static int LinkFailInd(Signal* sig)
{
	Topo_Ctrl* proc = (Topo_Ctrl*)sig->dst;
	Link_Fail_Ind_Param* param = (Link_Fail_Ind_Param*)sig->param;

	unsigned short tmp_sts;
	tmp_sts = (proc->nbr_table).nbr_sts[param->xra];
	proc->nbr_table.nbr_sts[param->xra] = 0;
	proc->nbr_table.nbr_ttl[param->xra] = 0;
	if(tmp_sts == 0x03)//双向变断
	{
		sig=proc->nbr_sts_chg_ind;
		((T_Nbr_Sts_Chg_Ind_Param*)sig->param)->id = param->xra;
		((T_Nbr_Sts_Chg_Ind_Param*)sig->param)->link_sts = 0;
		AddSignal(sig);
	}
	return 0;
}

static int HelloTopoTxInd(Signal* sig)
{
	Topo_Ctrl* proc = (Topo_Ctrl*)sig->dst;
	Hello_Topo_Tx_Req_Param* param = (Hello_Topo_Tx_Req_Param*)sig->param;

	unsigned long i,  tmp;
	unsigned long *tmp_addr = param->xdat;
	*tmp_addr = 0;
	//填hello包头邻节点信息
	for(i=0; i<NODE_MAX_CNT; i++)
	{
		tmp = ((proc->nbr_table.nbr_sts[i]) & 0x1);
		//tmp = (tmp <<(i%8));
		*tmp_addr |= tmp <<i;
	}
	
	sig=proc->hello_topo_tx_cfm;
	AddSignal(sig);
	return 0;
}

void TimerTopoCtrl(Topo_Ctrl* proc)
{
	unsigned short i,tmp_sts;
	Signal* sig;
	if(proc->ttl_timer)
	{
		proc->ttl_timer--;
		if(!proc->ttl_timer)
		{
			proc->ttl_timer = MICRO_SLOT_CNT_PER_TTL;
			for(i=0; i<NODE_MAX_CNT; i++)
			{//不对自己的节点进行判断ttl值
				if((i!=((P_Entity*)proc->entity)->mib.local_id) && (proc->nbr_table.nbr_sts[i]) && (proc->nbr_table.nbr_ttl[i])) 
				{
					proc->nbr_table.nbr_ttl[i]--;
					if(!proc->nbr_table.nbr_ttl[i])//TTL到期
					{
						tmp_sts = proc->nbr_table.nbr_sts[i];
						proc->nbr_table.nbr_sts[i] = 0;
						proc->nbr_table.nbr_ttl[i] = 0;
						if(tmp_sts == 0x03)//链路状态变化
						{
							sig=proc->nbr_sts_chg_ind;
							((T_Nbr_Sts_Chg_Ind_Param*)sig->param)->id = i;
							((T_Nbr_Sts_Chg_Ind_Param*)sig->param)->link_sts = 0;
							AddSignal(sig);
							return;
						}
					}
				}
			}
		}
	}
	return;
}


void TopoCtrlInit(Topo_Ctrl* proc)
{
	proc->state = IDLE;
	proc->ttl_timer = MICRO_SLOT_CNT_PER_TTL;
	memset(&proc->nbr_table,0,sizeof(proc->nbr_table));
	proc->nbr_table.nbr_sts[((P_Entity*)proc->entity)->mib.local_id] = 0x03;
}

void  TopoCtrlSetup(Topo_Ctrl* proc)
{
	proc->hello_rx_ind.next=0;
	proc->hello_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->hello_rx_ind.src=0;
	proc->hello_rx_ind.dst=proc;
	proc->hello_rx_ind.func=HelloRxInd;
	proc->hello_rx_ind.pri=SDL_PRI_URG;
	proc->hello_rx_ind.param=&proc->hello_rx_ind_param;

	proc->link_fail_ind.next=0;
	proc->link_fail_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->link_fail_ind.src=0;
	proc->link_fail_ind.dst=proc;
	proc->link_fail_ind.func=LinkFailInd;
	proc->link_fail_ind.pri=SDL_PRI_URG;
	proc->link_fail_ind.param=&proc->link_fail_ind_param;

	proc->hello_topo_tx_req .next=0;
	proc->hello_topo_tx_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->hello_topo_tx_req.src=0;
	proc->hello_topo_tx_req.dst=proc;
	proc->hello_topo_tx_req.func=HelloTopoTxInd;;
	proc->hello_topo_tx_req.pri=SDL_PRI_URG;
	proc->hello_topo_tx_req.param=&proc->hello_topo_tx_req_param;
}
