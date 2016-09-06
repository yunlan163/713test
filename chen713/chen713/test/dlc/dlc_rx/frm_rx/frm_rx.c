#include "frm_rx.h"
#include <stdio.h>
enum{IDLE,RX,WAIT_RSP};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun：收到dlc_rx_ctrl发来的广播数据、  arq_rx的单播
static int FrmRxInd(Signal* sig) 
{
	Frm_Rx *proc=(Frm_Rx*)sig->dst;  //yun:修改了
	Frm_Rx_Ind_Param * param=(Frm_Rx_Ind_Param*)sig->param;
	
	unsigned short len, i;
	unsigned short* p_frm;
//	Net_Frm* net_frm = (Net_Frm*)param->xdat ;

	/*if(param->xdat[param->xlen-1] != param->xdat[param->xlen - 2])
	{
		sig=proc->ddat_rx_ind;//送至dlc_rx_dump
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm = proc->net_frm;
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm_len = proc->net_frm_len;
		((T_Ddat_Rx_Ind_Param *)sig->param)->ta = proc->id;
		((T_Ddat_Rx_Ind_Param *)sig->param)->uc_bc_flag = proc->uc_bc_flag;
		AddSignal(sig);

	}*/


	switch(proc->state)
	{
	case IDLE://无正在接收数据
		if((param->seg_flag & FIRST_SEG_FLAG) == FIRST_SEG_FLAG)//首段  seg_flag;//段标记：开始10、结束01、中间00、开始且结束11
		{
		   	proc->net_frm = MemAllocNet();  //yun：开辟空间存储
			if(!proc->net_frm)  //yun:fail
			{
				sig=proc->frm_rx_rsp;
				AddSignal(sig);
				return 0;
			}
   			proc->dat_idx = proc->net_frm; //dat_idx:网络帧接收到的位置  proc->net_frm指向接收到的数据开头
			proc->net_frm_len = 0;
			if(param->bc_uc_falg)//广播
			{
				proc->bc_seg_num = param ->bc_seg_num;//记录序号
				proc->bc_seg_num ++;
				proc->bc_seg_num &= 0xf;
			}
		}
		else//非首段
		{
			printf("怎么可能进到这里？\n");
			sig=proc->frm_rx_rsp;  //yun：通知释放
			AddSignal(sig);
			return 0;
		}
		break;
	case RX://正在接收数据
		if((param->seg_flag & FIRST_SEG_FLAG) == FIRST_SEG_FLAG)//首段
		{
			MemFreeNet(proc->net_frm);
			proc->net_frm = NULL;
			proc->dat_idx =NULL;
		  	proc->net_frm = MemAllocNet();//重新接收
			if(!proc->net_frm)
			{
				sig=proc->frm_rx_rsp;
				AddSignal(sig);
				return 0;
			}
			proc->dat_idx = proc->net_frm;
			proc->net_frm_len = 0;
			if(param->bc_uc_falg)//广播
			{
				proc->bc_seg_num = param ->bc_seg_num;//记录序号
				proc->bc_seg_num ++;
				proc->bc_seg_num &= 0xf;
			}
		}
		else//no首段
		{
			if(param->bc_uc_falg)//广播
			{
				if(proc->bc_seg_num == param->bc_seg_num)//记录期望序号
				{
					proc->bc_seg_num ++;
					proc->bc_seg_num &= 0xf;
				}
				else//广播包不连续，此包失败
				{
					printf("广播包接收不连续，此包失败，proc->state=IDLE\n");
					MemFreeNet(proc->net_frm);
					proc->net_frm = NULL;
					proc->dat_idx =NULL;
					sig=proc->frm_rx_rsp;
					AddSignal(sig);
					proc->state=IDLE;
					return 0;
				}
			}
		}
		break;
	default://不可能发生 
		break;
	}	
	
	len = param->xlen;
	p_frm = param->xdat;
	//memcpy(proc->dat_idx,p_frm,len);
	for(i = 0; i < len; i++)
	{
		proc->dat_idx[i] = p_frm[i];  //yun：暂存数据
	}

	proc->dat_idx += len;
	proc->net_frm_len+= len;
	if((param->seg_flag & LAST_SEG_FLAG) == LAST_SEG_FLAG)//尾段 接受完
	{
		sig=proc->ddat_rx_ind;//送至dlc_rx_dump
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm = proc->net_frm;//proc->dat_idx = proc->net_frm;
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm_len = proc->net_frm_len;
		((T_Ddat_Rx_Ind_Param *)sig->param)->ta = proc->id;
		((T_Ddat_Rx_Ind_Param *)sig->param)->uc_bc_flag = proc->uc_bc_flag;
		AddSignal(sig);
		proc->state=WAIT_RSP;
		proc->net_frm = NULL;
		printf("frm_rx::FrmRxInd() 接收完一整包数据，上传至dlc_rx_dump\n");
	}
	else//非尾段
	{
		sig=proc->frm_rx_rsp; //yun：收到了，释放刚才的接收数据指针
		AddSignal(sig);
		proc->state=RX;
		printf("frm_rx[%d]::FrmRxInd() 收到数据，回复下层释放\n",proc->id);
	}
	return 0;
}
//yun：收到上层dlc_rx_dump的回复
static int DdatRxRsp(Signal* sig) 
{
	Frm_Rx *proc=(Frm_Rx*)sig->dst;
	//Ddat_Rx_Rsp_Param * param=(Ddat_Rx_Rsp_Param*)sig->param;

	sig=proc->frm_rx_rsp;
	AddSignal(sig);
	proc->net_frm = NULL;
	proc->dat_idx =NULL;

	proc->state=IDLE;
	printf("frm_rx::DdatRxRsp() 收到dlc_rx_dump的回复，状态变为IDLE\n");
	return 0;
}
//yun:收到arq_rx.c的释放指示 滑窗时
static int FrmRxFreeInd(Signal* sig)
{
	Frm_Rx *proc=(Frm_Rx*)sig->dst;
	//Frm_Rx_Free_Ind_Param * param=(Frm_Rx_Free_Ind_Param*)sig->param;

	if(proc->net_frm!=NULL)
	{
		MemFreeNet(proc->net_frm);
	}
	proc->dat_idx = NULL;
	proc->net_frm = NULL;
	proc->net_frm_len = 0;
	proc->state=IDLE;
	printf("frm_rx::FrmRxFreeInd() 收到arq_rx.c的滑窗指示，状态变为IDLE\n");
	return 0;

}
void FrmRxInit(Frm_Rx *proc)
{
	proc->state=IDLE;
	proc->net_frm = NULL;
	proc->net_frm_len = 0;
	proc->dat_idx = NULL;
}

void FrmRxSetup(Frm_Rx *proc)
{
	proc->frm_rx_ind.next=0;
	proc->frm_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_rx_ind.src=0;
	proc->frm_rx_ind.dst=proc;
	proc->frm_rx_ind.func=FrmRxInd;
	proc->frm_rx_ind.pri=SDL_PRI_URG;
	proc->frm_rx_ind.param=&proc->frm_rx_ind_param;

	proc->frm_rx_free_ind.next=0;
	proc->frm_rx_free_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_rx_free_ind.src=0;
	proc->frm_rx_free_ind.dst=proc;
	proc->frm_rx_free_ind.func=FrmRxFreeInd;
	proc->frm_rx_free_ind.pri=SDL_PRI_URG;
	proc->frm_rx_free_ind.param=&proc->frm_rx_free_ind_param;


	proc->ddat_rx_rsp.next=0;
	proc->ddat_rx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->ddat_rx_rsp.src=0;
	proc->ddat_rx_rsp.dst=proc;
	proc->ddat_rx_rsp.func=DdatRxRsp;
	proc->ddat_rx_rsp.pri=SDL_PRI_URG;
	proc->ddat_rx_rsp.param=&proc->ddat_rx_rsp_param;
}
