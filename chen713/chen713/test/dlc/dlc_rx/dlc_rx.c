#include "dlc_rx.h"
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;

void DlcRxSetup(Dlc_Rx* proc)
{
	unsigned short i;

	proc->dlc_rx_ctrl.entity=proc->entity;
	DlcRxCtrlSetup(&proc->dlc_rx_ctrl);//dlc_rx_ctrl模块

	proc->dlc_rx_dump.entity=proc->entity;
	DlcRxDumpSetup(&proc->dlc_rx_dump);//dlc_rx_dump模块

	for(i=0;i<NODE_MAX_CNT;i++)//frm_tx模块
	{
		proc->frm_rx[0][i].entity = proc->entity;
		FrmRxSetup(&proc->frm_rx[0][i]);

		proc->frm_rx[1][i].entity = proc->entity;
		FrmRxSetup(&proc->frm_rx[1][i]);

		proc->arq_rx[i].entity = proc->entity;//arq_tx模块
		ArqRxSetup(&proc->arq_rx[i]);
	}

	for(i=0;i<NODE_MAX_CNT;i++)
	{
		//dlc_rx_ctrl模块与arq_rx模块
		proc->dlc_rx_ctrl.arq_rx_ind[i] = & proc->arq_rx[i].arq_rx_ind ;
		proc->arq_rx[i].arq_rx_rsp = & proc->dlc_rx_ctrl.arq_rx_rsp[i];
		proc->arq_rx[i].arq_ack_tx_req = & proc->dlc_rx_ctrl.arq_ack_tx_req[i];
		proc->arq_rx[i].arq_rst_tx_req = & proc->dlc_rx_ctrl.arq_rst_tx_req[i];
		//frm_rx模块与arq_rx模块
		proc->arq_rx[i].frm_rx_ind = & proc->frm_rx[0][i].frm_rx_ind;
		proc->arq_rx[i].frm_rx_free_ind = & proc->frm_rx[0][i].frm_rx_free_ind;
		proc->frm_rx[0][i].frm_rx_rsp = & proc->arq_rx[i].frm_rx_rsp;
		//frm_rx模块与dlc_rx_dump模块
		proc->frm_rx[0][i].ddat_rx_ind = & proc->dlc_rx_dump.ddat_rx_ind[0][i];
		proc->dlc_rx_dump.ddat_rx_rsp[0][i] = & proc->frm_rx[0][i].ddat_rx_rsp;
		proc->frm_rx[1][i].ddat_rx_ind = & proc->dlc_rx_dump.ddat_rx_ind[1][i];
		proc->dlc_rx_dump.ddat_rx_rsp[1][i] = & proc->frm_rx[1][i].ddat_rx_rsp;
		//frm_rx模块与dlc_rx_ctrl模块
		proc->frm_rx[1][i].frm_rx_rsp = & proc->dlc_rx_ctrl.frm_rx_rsp[i];
		proc->dlc_rx_ctrl.frm_rx_ind[i] = & proc->frm_rx[1][i].frm_rx_ind ;  //yun：这里就是广播
	}
	


}
void DlcRxInit(Dlc_Rx *proc)
{
	unsigned short i;
	for(i=0;i<NODE_MAX_CNT;i++)//arq_rx模块
	{
		proc->arq_rx[i].id=i;
		ArqRxInit(&proc->arq_rx[i]);	
	}
	for(i=0;i<NODE_MAX_CNT;i++)//frm_rx模块
	{
		FrmRxInit(&proc->frm_rx[0][i]);
		proc->frm_rx[0][i].id=i;
		proc->frm_rx[0][i].uc_bc_flag = 0;
		FrmRxInit(&proc->frm_rx[1][i]);
		proc->frm_rx[1][i].id=i;
		proc->frm_rx[1][i].uc_bc_flag = 1;
	}
	DlcRxDumpInit(&proc->dlc_rx_dump);//dlc_rx_dump模块
	DlcRxCtrlInit(&proc->dlc_rx_ctrl);//dlc_rx_ctrl模块
}

