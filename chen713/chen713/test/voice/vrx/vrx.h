#ifndef _VRX_H
#define _VRX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"



typedef struct _vrx_ind_param
{
	unsigned short *vdat;

}VRx_Ind_Param;


typedef struct _voice_ctrl_rsp_param
{
	unsigned short succ_flag;
	unsigned short rx_road_cnt;//总路数
	unsigned short sa_road_idx;//本话音包发送节点向上送的位置
}Voice_Ctrl_Rsp_Param;

typedef struct _vrelay_chk_cfm_param
{
	unsigned short succ_flag;//1-中继，0-不中继
}VRelay_Chk_Cfm_Param;

typedef struct _t_vrelay_chk_req_param
{
	unsigned short sa;
}T_VRelay_Chk_Req_Param;

typedef struct _t_voice_ctrl_ind_param
{
	unsigned short sa;//话音源节点
	unsigned short sn;
	unsigned short relay_flag;

}T_Voice_Ctrl_Ind_Param;

typedef struct _t_voice_relay_param
{
	unsigned short *vdat;

}T_Voice_Relay_Param;

typedef struct _vrx
{
	void* entity;
	unsigned short state;

	unsigned short voice_sn[NODE_MAX_CNT];//话音序号记录

	unsigned short current_vrx_road_cnt;//本周期80ms,接受到的路数
	unsigned short current_vrx_road_flag[MAX_VOICE_ROAD_CNT];//记录本80ms是否接收话音各个路数

	Mac_Frm_1002 voice_mac_frm;

	unsigned short relay_flag;//中继标记

	Signal vrx_ind;
	VRx_Ind_Param vrx_ind_param;

	Signal vrelay_chk_cfm;
	VRelay_Chk_Cfm_Param vrelay_chk_cfm_param;

	Signal voice_ctrl_rsp;
	Voice_Ctrl_Rsp_Param voice_ctrl_rsp_param;

	Signal* vrelay_chk_req;	
	Signal* voice_ctrl_ind;
	Signal* voice_relay;

}VRx;
extern void VRxInit(VRx* proc);
extern void VRxSetup(VRx* proc);

#endif
