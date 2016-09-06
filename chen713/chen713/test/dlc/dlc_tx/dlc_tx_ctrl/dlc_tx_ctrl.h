#ifndef _DLC_TX_CTRL_H
#define _DLC_TX_CTRL_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include"..\..\..\mib\mib.h"

#define MAC_FRM_LEN_TYPE_CNT 4//mac帧长度类型个数,即4种长度的帧
#define DLC_SND_FRM_CNT_MAX 30//dlc层向mac层一次发送帧最大个数
typedef struct _dlc_tx_ind_param
{
	unsigned short rsv;
}Dlc_Tx_Ind_Param;
typedef struct _ack_rx_ind_param
{
	unsigned short* ack_sgl;//信令地址
	unsigned short id;//ack的发送节点
}Ack_Rx_Ind_Param;
typedef struct _rst_rx_ind_param
{
	unsigned short id;//arq重置的发送节点
}Rst_Rx_Ind_Param;
typedef struct _arq_tx_rsp_param
{
	unsigned short* xfrm;
}Arq_Tx_Rsp_Param;
typedef struct _t_dlc_tx_rsp_param
{
	unsigned short * xfrm0;
}T_Dlc_Tx_Rsp_Param;
typedef struct _t_arq_ack_rx_ind_param
{
	unsigned short* ack_sgl;//信令地址
}T_Arq_Ack_Rx_Ind_Param;
typedef struct _t_arq_rst_rx_ind_param
{
	unsigned short rsv;//预留
}T_Arq_Rst_Rx_Ind_Param;
typedef struct _t_arq_tx_ind_param
{
	unsigned short rsv;//预留
}T_Arq_Tx_Ind_Param;
typedef struct _t_bc_frm_tx_ind_param
{
	unsigned short *dat_sub_frm;//数据子帧地址
}T_Bc_Frm_Tx_Ind_Param;

typedef struct _bc_frm_tx_rsp_param
{
	unsigned short xlen;
}Bc_Frm_Tx_Rsp_Param;

typedef struct _t_dlc_rx_ind_param   //for test
{
	unsigned short *frm;
	unsigned short len;
}T_Dlc_Rx_Ind_Param;

typedef struct _dlc_tx_ctrl
{
	void* entity;
	unsigned short state;
	
	unsigned short arq_tx_req_id;//记录arq_tx编号
	unsigned short last_tx_id;//记录上次发送数据的arq_tx编号，下次从下一个arq_tx开始要数据

	Mac_Frm_1002 bc_mac_frm;//广播包缓冲区
	
	Signal dlc_tx_ind;
	Dlc_Tx_Ind_Param dlc_tx_ind_param;

	Signal ack_rx_ind;
	Ack_Rx_Ind_Param ack_rx_ind_param;

	Signal rst_rx_ind;
	Rst_Rx_Ind_Param rst_rx_ind_param;

	Signal arq_tx_rsp[NODE_MAX_CNT];
	Arq_Tx_Rsp_Param arq_tx_rsp_param[NODE_MAX_CNT];

	Signal frm_tx_rsp;
	Bc_Frm_Tx_Rsp_Param frm_tx_rsp_param;

	Signal* frm_tx_ind;
	Signal* dlc_tx_rsp;
	Signal* arq_tx_ind[NODE_MAX_CNT];
	Signal* arq_ack_rx_ind[NODE_MAX_CNT];
	Signal* arq_rst_rx_ind[NODE_MAX_CNT];

	Signal* dlc_rx_ind; //yun: for test
}Dlc_Tx_Ctrl;
extern void DlcTxCtrlSetup(Dlc_Tx_Ctrl*);
extern void DlcTxCtrlInit(Dlc_Tx_Ctrl*);
#endif

