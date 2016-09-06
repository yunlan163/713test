#include "dlc_tx.h"

enum {IDLE};
typedef struct
{

	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
void DlcTxSetup(Dlc_Tx* proc)
{
	unsigned short i;
	
	//proc->entity = proc;
	proc->dlc_tx_dump.entity = proc->entity;//dlc_rx_dump模块
	DlcTxDumpSetup(&proc->dlc_tx_dump);

	proc->dlc_tx_ctrl.entity = proc->entity;//dlc_tx_ctrl模块
	DlcTxCtrlSetup(&proc->dlc_tx_ctrl);

	for(i=0;i<NODE_MAX_CNT;i++)//frm_tx模块
	{
		proc->frm_tx[i].entity = proc->entity;
		FrmTxSetup(&proc->frm_tx[i]);

		proc->arq_tx[i].entity = proc->entity;//arq_tx模块
		ArqTxSetup(&proc->arq_tx[i]);
	}

	for(i=0;i<NODE_MAX_CNT;i++)
	{	
		if(((P_Entity*)proc->entity)->mib.local_id != i)   //yun：本节点号！=i时。单播，给其他节点
		{
			//dlc_tx_ctrl模块与arq_tx模块
			proc->dlc_tx_ctrl.arq_tx_ind[i] = &proc->arq_tx[i].arq_tx_ind;
			proc->dlc_tx_ctrl.arq_ack_rx_ind[i] = &proc->arq_tx[i].arq_ack_rx_ind;
			proc->dlc_tx_ctrl.arq_rst_rx_ind[i] = &proc->arq_tx[i].arq_rst_rx_ind;
			proc->arq_tx[i].arq_tx_rsp = &proc->dlc_tx_ctrl.arq_tx_rsp[i];
			//frm_tx模块与arq_tx模块
			proc->arq_tx[i].frm_tx_free_ind = &proc->frm_tx[i].frm_tx_free_ind;
			proc->arq_tx[i].arq_tx_succ_ind = &proc->frm_tx[i].arq_tx_succ_ind;
			proc->arq_tx[i].frm_tx_ind = &proc->frm_tx[i].frm_tx_ind;
			proc->frm_tx[i].frm_tx_rsp = &proc->arq_tx[i].frm_tx_rsp;    //单播
		}
		//frm_tx模块与dlc_tx_dump模块   //yun:本节点号=i时，本节点自己
		proc->dlc_tx_dump.frm_tx_req[i] = &proc->frm_tx[i].frm_tx_req;
		proc->frm_tx[i].frm_tx_cfm = &proc->dlc_tx_dump.frm_tx_cfm[i];

	}
	//dlc_tx_ctrl模块与frm_tx模块   本节点自己是否有广播
	proc->dlc_tx_ctrl.frm_tx_ind = &proc->frm_tx[((P_Entity*)proc->entity)->mib.local_id].frm_tx_ind;
	proc->frm_tx[((P_Entity*)proc->entity)->mib.local_id].frm_tx_rsp = &proc->dlc_tx_ctrl.frm_tx_rsp;   //yun:
}
void DlcTxInit(Dlc_Tx* proc)
{
	unsigned i;

	for(i=0;i<NODE_MAX_CNT;i++)
	{  
		proc->frm_tx[i].id=i;  //yun:在这里给自己的模块id赋值
		FrmTxInit(&proc->frm_tx[i]);//frm_tx模块
		proc->arq_tx[i].id=i;
		ArqTxInit(&proc->arq_tx[i]);//arq_tx模块	
	}
	DlcTxDumpInit(&proc->dlc_tx_dump);//dlc_rx_dump模块
	DlcTxCtrlInit(&proc->dlc_tx_ctrl);//dlc_tx_ctrl模块
}


