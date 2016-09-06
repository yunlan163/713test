#include "dlc_tx_dump.h"
#include <stdio.h>
enum {IDLE};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun:网络数据发送请求 来自上层网络
static int NTxReq(Signal *sig)
{
	Dlc_Tx_Dump* proc=(Dlc_Tx_Dump*)sig->dst;
	NTx_Req_Param* param=(NTx_Req_Param*)sig->param;
	
	//proc->dlc_tx_dump.frm_tx_req[i] = &proc->frm_tx[i].frm_tx_req;
	sig = proc->frm_tx_req[param->ra];//yun:根据数据包的目的地址，加信号给对应模块进行处理  yun：有32个
	((T_Frm_Tx_Req_Param*)sig->param)->xdat = param->xdat;
	((T_Frm_Tx_Req_Param*)sig->param)->xlen = param->xlen;
	AddSignal(sig);
	printf("dlc_tx_dump::NTxReq() 收到网络数据发送请求，根据目的地址给frm_tx[%d]\n",param->ra);
	//sig->func(sig);
	return 0 ;
}
//yun：收到frm_tx的回复，给上层  (单播、广播)
static int FrmTxCfm(Signal *sig)
{
	Dlc_Tx_Dump* proc=(Dlc_Tx_Dump*)sig->dst;
	Frm_Tx_Cfm_Param* param=(Frm_Tx_Cfm_Param*)sig->param;

	sig = proc->ntx_ind[param->id];
	((T_NTx_Ind_Param*)sig->param)->ra = param->id;
	((T_NTx_Ind_Param*)sig->param)->succ_flag= param->succ_fail_flag;
	((T_NTx_Ind_Param*)sig->param)->fail_type = param->fail_type;
	((T_NTx_Ind_Param*)sig->param)->dat = param->net_frm;
	((T_NTx_Ind_Param*)sig->param)->da = param->da;
	((T_NTx_Ind_Param*)sig->param)->sa = param->sa;
	AddSignal(sig);
	printf("dlc_tx_dump::FrmTxCfm() 收到frm_tx的发送成功回复，给上层ntx[%d]\n",param->id);
	//NTxInd(param->id,param->succ_fail_flag,param->fail_type);//for test
	return 0 ;
}
void DlcTxDumpSetup(Dlc_Tx_Dump* proc)
{
	int i;
	proc->ntx_req.next=0;
	proc->ntx_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->ntx_req.src=0;
	proc->ntx_req.dst=proc;
	proc->ntx_req.func=NTxReq;
	proc->ntx_req.pri=SDL_PRI_URG;
	proc->ntx_req.param=&proc->ntx_req_param;


	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->frm_tx_cfm[i].next=0;
		proc->frm_tx_cfm[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->frm_tx_cfm[i].src=0;
		proc->frm_tx_cfm[i].dst=proc;
		proc->frm_tx_cfm[i].func=FrmTxCfm;
		proc->frm_tx_cfm[i].pri=SDL_PRI_URG;
		proc->frm_tx_cfm[i].param=&proc->frm_tx_cfm_param[i];
	}
}
void DlcTxDumpInit(Dlc_Tx_Dump* proc)
{
	proc->state=IDLE;
}

