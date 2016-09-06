#ifndef _ENTITY_H
#define _ENTITY_H
#include "sdl\sdl.h"
#include "mib\mib.h"
#include "common.h"
#include "dlc\dlc_tx\dlc_tx.h"
#include "dlc\dlc_rx\dlc_rx.h"
#include "net\topo_ctrl\topo_ctrl.h"
#include "net\rt_ctrl\rt_ctrl.h"
#include "net\ntx\ntx.h"
#include "net\nrx\nrx.h"
#include "voice\vrx\vrx.h"
#include "voice\vtx\vtx.h"
#include "voice\vctrl\vctrl.h"

typedef struct  _entity
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	//声明模块实体
	Dlc_Tx dlc_tx;
	Dlc_Rx dlc_rx;

	Topo_Ctrl topo_ctrl;
	Rt_Ctrl rt_ctrl;
	NTx ntx;
	NRx nrx;

	//yun:暂时用不到
	VTx vtx;
	VRx vrx;
	VCtrl vctrl;

}Entity;
extern void EntityInit(Entity *ent);
extern void EntitySetup(Entity *ent);
#endif

