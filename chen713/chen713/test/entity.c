#include "entity.h"



void EntityInit(Entity *ent)
{
	SdlCoreInit(&ent->sdlc);
	MibInit(&ent->mib);

	DlcTxInit(&ent->dlc_tx);
	DlcRxInit(&ent->dlc_rx);
	
	TopoCtrlInit(&ent->topo_ctrl);
	RtCtrlInit(&ent->rt_ctrl);
	NTxInit(&ent->ntx);
	NRxInit(&ent->nrx);
	
	VTxInit(&ent->vtx);
	VRxInit(&ent->vrx);
	VCtrlInit(&ent->vctrl);
}
void EntitySetup(Entity *ent)
{
	unsigned short i;
	ent->myself = ent;

	//yun:dlc_tx/dlc_rx
	ent->dlc_tx.entity=ent;
	DlcTxSetup(&ent->dlc_tx);
	ent->dlc_rx.entity=ent;
	DlcRxSetup(&ent->dlc_rx);

	//yun:rt
	ent->rt_ctrl.entity=ent;
	RtCtrlSetup(&ent->rt_ctrl);
	ent->topo_ctrl.entity=ent;
	TopoCtrlSetup(&ent->topo_ctrl);

	//yun:ntx/nrx
	ent->ntx.entity=ent;
	NTxSetup(&ent->ntx);
	ent->nrx.entity=ent;
	NRxSetup(&ent->nrx);

	//yun:vtx/vrx
	ent->vtx.entity = ent;
	VTxSetup(&ent->vtx);
	ent->vrx.entity = ent;
	VRxSetup(&ent->vrx);
	ent->vctrl.entity = ent;
	VCtrlSetup(&ent->vctrl);

	//link betwin ntx and dlc_tx
	ent->ntx.ntx_req = &ent->dlc_tx.dlc_tx_dump.ntx_req;
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		ent->dlc_tx.dlc_tx_dump.ntx_ind[i] = &ent->ntx.ntx_ind[i];
	}

	//link betwin nrx and dlc_rx
	ent->nrx.nrx_rsp = &ent->dlc_rx.dlc_rx_dump.nrx_rsp;
	ent->dlc_rx.dlc_rx_dump.nrx_ind = &ent->nrx.nrx_ind;

	//link betwin rt_ctrl and topo_ctrl
	ent->topo_ctrl.hello_rt_rx_ind = &ent->rt_ctrl.hello_rt_rx_ind;
	ent->topo_ctrl.nbr_sts_chg_ind = &ent->rt_ctrl.nbr_sts_chg_ind;
	ent->topo_ctrl.hello_topo_tx_cfm  = &ent->rt_ctrl.hello_topo_tx_cfm;
	ent->rt_ctrl.hello_topo_tx_req = &ent->topo_ctrl.hello_topo_tx_req;

	//Signal connection between NTx and NRx
	ent->nrx.relay_tx_ind=&ent->ntx.relay_tx_ind;

	//Signal connection between NTx and topo_ctrl
	ent->ntx.link_fail_ind=&ent->topo_ctrl.link_fail_ind;

	//Signal connection between NTx and rt_ctrl
	ent->ntx.rt_tx_ind=&ent->rt_ctrl.rt_tx_ind;
	ent->rt_ctrl.rt_tx_rsp=&ent->ntx.rt_tx_rsp;

	//Signal connection between NRx and rt_ctrl
	ent->nrx.drelay_chk_req = &ent->rt_ctrl.drelay_chk_req;
	ent->rt_ctrl.drelay_chk_cfm = &ent->nrx.drelay_chk_cfm;

	//Signal connection between VRx and rt_ctrl
	ent->vrx.vrelay_chk_req = &ent->rt_ctrl.vrelay_chk_req;
	ent->rt_ctrl.vrelay_chk_cfm = &ent->vrx.vrelay_chk_cfm;

	//vctrl and vtx
	ent->vctrl.vtx_req = &ent->vtx.vtx_req;
	//vctrl and vrx
	ent->vctrl.voice_ctrl_rsp = &ent->vrx.voice_ctrl_rsp;
	ent->vrx.voice_ctrl_ind = &ent->vctrl.voice_ctrl_ind;
	//vrx and vtx
	ent->vrx.voice_relay = &ent->vtx.voice_relay;

	//dlc_tx and dlc_rx    yun:for test
	//ent->dlc_tx.dlc_tx_ctrl.dlc_rx_ind=&ent->dlc_rx.dlc_rx_ctrl.dlc_rx_ind;

}

