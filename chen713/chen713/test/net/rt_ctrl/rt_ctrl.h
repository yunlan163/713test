#ifndef _RT_CTRL_TX_H
#define _RT_CTRL_TX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"

#define METRIC_INFINITE 0x1f//跳数无穷大


#define HELLO_PKT_NODE_CNT_PER_MAC_FRM 11//每次发送HELLO包，发送节点数

typedef struct _t_rt_tx_rsp_param
{
	unsigned short succ;//是否有路由
	unsigned short ra;//下一跳地址
	unsigned short da;//目的地址
}T_Rt_Tx_Rsp_Param;

typedef struct _hello_rt_rx_ind_param
{
	unsigned short *hello_pkt;//hello包地址
	unsigned short xta;//发送节点
}Hello_Rt_Rx_Ind_Param;

typedef struct _t_hello_tx_req_param
{
	unsigned short *rti_sub_frm;
	unsigned short xlen;

}T_Hello_Tx_Req_Param;

typedef struct _t_hello_topo_tx_req_param
{
	unsigned long *dat;

}T_Hello_Topo_Tx_Req_Param;

typedef struct _nbr_sts_chg_ind_param
{	
	unsigned short xnbr;
	unsigned short xsucc;

}Nbr_Sts_Chg_Ind_Param;

typedef struct _rt_tx_ind_param
{
	unsigned short da;

}Rt_Tx_Ind_Param;

typedef struct _hello_topo_tx_cfm_param
{
	unsigned short rsv;//保留

}Hello_Topo_Tx_Cfm_Param;

typedef struct _drelay_chk_req_param
{
	unsigned short sa;
}DRelay_Chk_Req_Param;

typedef struct _t_drelay_chk_cfm_param
{
	unsigned short succ_flag;
}T_DRelay_Chk_Cfm_Param;

typedef struct _vrelay_chk_req_param
{
	unsigned short sa;
}VRelay_Chk_Req_Param;

typedef struct _t_vrelay_chk_cfm_param
{
	unsigned short succ_flag;
}T_VRelay_Chk_Cfm_Param;
typedef struct _hello_tx_cfm_param
{
	unsigned short rsv;//保留

}Hello_Tx_Cfm_Param;

typedef struct _hello_info_table//HELLO原始信息表
{
	unsigned short metric_next;//metric为高字节的低6位，next为低字节的低5位
	unsigned short seq;

}Hello_Info_Table;

typedef struct _rt_table//路由表
{
	unsigned short tflag_metric_next;//tflag为最高位，metric为高字节的低6位，next为低字节的低5位
	unsigned short seq;

}Rt_Table;

typedef struct _rt_ctrl
{
	void* entity;
	unsigned short state;
	unsigned short rt_table_chg_tx_to_arm_flag;//路由变化将路由表送至ARM标志
	unsigned short ip_table_chg_tx_to_arm_flag;//路由变化将路由表送至ARM标志

	unsigned short hello_snd_timer_idx;//Hello包定时发送到的位置

	Hello_Info_Table hello_info_table[NODE_MAX_CNT][NODE_MAX_CNT];//HELLO原始信息表
	Rt_Table rt_table[NODE_MAX_CNT];//路由表
	unsigned short hello_send_timer;//HELLO包发送定时器
	RTI_Sub_Frm rti_sub_frm;//hello包

	unsigned short hello_topo_tx_req_add_sig_flag;//标志hello_topo_tx_req信号已经添加，但未执行
	unsigned long ip_map_table[NODE_MAX_CNT];//IP影射表
	
	Signal rt_tx_ind;//路由查询
	Rt_Tx_Ind_Param rt_tx_ind_param;

	Signal hello_rt_rx_ind;//hello包接收
	Hello_Rt_Rx_Ind_Param hello_rt_rx_ind_param;

	Signal nbr_sts_chg_ind;//邻节点状态变化
	Nbr_Sts_Chg_Ind_Param nbr_sts_chg_ind_param;

	Signal hello_topo_tx_cfm;//填hello包头邻节点信息回复
	Hello_Topo_Tx_Cfm_Param hello_topo_tx_cfm_param;

	//Signal hello_tx_cfm;//hello包发送完成
	//Hello_Tx_Cfm_Param hello_tx_cfm_param;

	Signal drelay_chk_req;//多播路由查询
	DRelay_Chk_Req_Param drelay_chk_req_param;

	Signal vrelay_chk_req;//话音多播路由查询
	VRelay_Chk_Req_Param vrelay_chk_req_param;

	Signal* rt_tx_rsp;//路由查询回复
	Signal* hello_tx_req;//发送hello包请求
	Signal* hello_topo_tx_req;//请求填hello包头邻节点信息
	Signal* drelay_chk_cfm;//多播路由查询回复
	Signal* vrelay_chk_cfm;//话音多播路由查询回复

}Rt_Ctrl;

extern void RtCtrlSetup(Rt_Ctrl*);
extern void RtCtrlInit(Rt_Ctrl*);
extern void TimerRtCtrl(Rt_Ctrl* );

#endif

