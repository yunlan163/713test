#ifndef _VCTRL_H
#define _VCTRL_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"

#define V_CNCL_TIMER_MAX_CNT 500//多次没有收到话音数据


#define LOCAL_RSV 0
#define RELAY_RSV 1
#define OFF_RSV 2
#define SUCC_RSV 3

#define VOICE_TX_OFF 0
#define VOICE_TX_LOCAL 1
#define VOICE_TX_RELAY 2
#define VOICE_TX_LOCAL_RELAY 3

typedef struct _voice_ctrl_ind_param
{
	unsigned short sa;//话音源节点
	unsigned short sn;
	unsigned short relay_flag;

}Voice_Ctrl_Ind_Param;
typedef struct _t_rsv_slot_req_param
{
	unsigned short rsv_bsn_num;

}T_Rsv_Slot_Req_Param;
typedef struct _t_rsv_cncl_req_param
{
	unsigned short cncl_bsn_num;

}T_Rsv_Cncl_Req_Param;


typedef struct _t_vtx_req_param
{
	unsigned short state;

}T_VTx_Req_Param;

typedef struct _t_voice_ctrl_rsp_param
{
	unsigned short succ_flag;
	unsigned short rx_road_cnt;//总路数
	unsigned short sa_road_idx;//本话音包发送节点向上送的位置

}T_Voice_Ctrl_Rsp_Param;


typedef struct _rsv_slot_cfm_param
{
	unsigned short succ_flag;

}Rsv_Slot_Cfm_Param;

typedef struct _voice_info
{
	unsigned short en;//本结构体是否有效
	unsigned short sa;//本路话音源节点
	unsigned short cncl_timer;//话音超时定时器
	unsigned short sn;//话路序号

}Voice_Info;
typedef struct _vctrl
{
	void* entity;
	unsigned short state;
	unsigned short voice_road_cnt;//本节点记录的话音路数
	unsigned short rsv_type;
	Voice_Info voice_info[MAX_VOICE_ROAD_CNT];

	unsigned short local_node_tx_flag;//本节点正在发送标记
	unsigned short relay_flag[MAX_VOICE_ROAD_CNT];//中继标志
	
	unsigned short voice_tx_local_relay_type;//话音发生中继类型

	Signal rsv_slot_cfm;
	Rsv_Slot_Cfm_Param rsv_slot_cfm_param;

	//Signal rsv_cncl_cfm;
	//Rsv_Cncl_Cfm_Param rsv_cncl_cfm_param;

	Signal voice_ctrl_ind;
	Voice_Ctrl_Ind_Param voice_ctrl_ind_param;

	Signal *vtx_req;
	Signal *voice_ctrl_rsp;
	Signal *rsv_slot_req;
	Signal *rsv_cncl_req;	

}VCtrl;

extern void VCtrlSetup(VCtrl*);
extern void VCtrlInit(VCtrl*);
extern void VoiceReq(VCtrl* proc, unsigned short on_off_flag);
extern void TimerVoiceCnclRoad(VCtrl* proc);
#endif

