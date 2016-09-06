#ifndef _DLC_RX_H
#define _DLC_RX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "arq_rx\arq_rx.h"
#include "frm_rx\frm_rx.h"
#include "dlc_rx_dump\dlc_rx_dump.h"
#include "dlc_rx_ctrl\dlc_rx_ctrl.h"

typedef struct _dlc_rx
{
	void* entity;

	Dlc_Rx_Ctrl dlc_rx_ctrl;
	Arq_Rx arq_rx[NODE_MAX_CNT];
	Frm_Rx frm_rx[2][NODE_MAX_CNT];//NODE_MAX_CNT个广播，NODE_MAX_CNT个单播
	Dlc_Rx_Dump dlc_rx_dump;

}Dlc_Rx;


extern void DlcRxSetup(Dlc_Rx*);
extern void DlcRxInit(Dlc_Rx*);

#endif

