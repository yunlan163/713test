#ifndef _VTX_H
#define _VTX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"




typedef struct _vtx_req_param
{
	unsigned short state;

}VTx_Req_Param;

typedef struct _voice_relay_param
{
	unsigned short *vdat;

}Voice_Relay_Param;

typedef struct _vtx
{
	void* entity;
	unsigned short state;
	unsigned short vtx_sts;//取值与vtx_req信号参数一致
	unsigned short local_voice_sn;

	Mac_Frm_1002 voice_mac_frm[MAX_VOICE_ROAD_CNT];
	unsigned short btm;
	unsigned short top;
	unsigned short size;//表示待发的话音包个数

	Signal voice_relay;
	Voice_Relay_Param voice_relay_param;

	Signal vtx_req;
	VTx_Req_Param vtx_req_param;

	//Signal *vdat_tx_req;

}VTx;
extern void VTxInit(VTx* proc);
extern void VTxSetup(VTx* proc);
extern void VoiceTx(VTx* proc,unsigned short *vdat);
#endif

