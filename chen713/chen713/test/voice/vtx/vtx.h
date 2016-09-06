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
	unsigned short vtx_sts;//ȡֵ��vtx_req�źŲ���һ��
	unsigned short local_voice_sn;

	Mac_Frm_1002 voice_mac_frm[MAX_VOICE_ROAD_CNT];
	unsigned short btm;
	unsigned short top;
	unsigned short size;//��ʾ�����Ļ���������

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

