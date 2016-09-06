#ifndef _DLC_TX_H
#define _DLC_TX_H
#include "dlc_tx_dump\dlc_tx_dump.h"
#include "dlc_tx_ctrl\dlc_tx_ctrl.h"
#include "frm_tx\frm_tx.h"
#include "arq_tx\arq_tx.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
typedef struct _dlc_tx
{
	void* entity;

	Dlc_Tx_Dump dlc_tx_dump;
	Dlc_Tx_Ctrl dlc_tx_ctrl;
	Frm_Tx frm_tx[NODE_MAX_CNT];
	Arq_Tx arq_tx[NODE_MAX_CNT];

}Dlc_Tx;
extern void DlcTxSetup(Dlc_Tx*);
extern void DlcTxInit(Dlc_Tx*);
#endif

