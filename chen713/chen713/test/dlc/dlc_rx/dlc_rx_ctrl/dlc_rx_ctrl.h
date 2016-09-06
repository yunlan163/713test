#ifndef _DLC_RX_CTRL_H
#define _DLC_RX_CTRL_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"

typedef struct _dlc_rx_ind_param 
{
	unsigned short *frm;//MAC帧地址
	unsigned short len;//MAC帧长度
}Dlc_Rx_Ind_Param;

typedef struct _arq_rx_rsp_param 
{
	unsigned short rsv;//预留
}Arq_Rx_Rsp_Param;

typedef struct _arq_ack_tx_req_param
{
	unsigned short *ack_sgl;//ack信令地址
	//unsigned short len;//ack信令长度
	unsigned short id;//yun： 本节点id
}Arq_Ack_Tx_Req_Param;

typedef struct _arq_rst_tx_req_param
{
	unsigned short *rst_sgl;//arq重置信令地址
	unsigned short len;//arq重置信令长度
}Arq_Rst_Tx_Req_Param;

typedef struct _t_ack_tx_req_param
{
	unsigned short *ack_sgl;//ack信令地址
	//unsigned short len;//ack信令长度
	unsigned short id;//yun： 本节点id
}T_Ack_Tx_Req_Param;

typedef struct _t_rst_tx_req_param
{
	unsigned short *rst_sgl;//arq重置信令地址
	unsigned short len;//arq重置信令长度
}T_Rst_Tx_Req_Param;

typedef struct _t_arq_rx_ind_param  
{
	unsigned short *frm;//MAC帧地址
	unsigned short len; //MAC帧长度
}T_Arq_Rx_Ind_Param;

typedef struct _t_dlc_rx_rsp_param  
{
	unsigned short rsv;//预留
}T_Dlc_Rx_Rsp_Param;

typedef struct _t_bc_frm_rx_ind_param
{
	unsigned short * xdat;//数据子帧净荷地址
	unsigned short xlen;//长度
	unsigned short seg_flag;//段标记：开始10、结束01、中间00、开始且结束11
	unsigned short bc_uc_falg;//0-单播，1-广播
	unsigned short bc_seg_num;//广播段序号
}T_Bc_Frm_Rx_Ind_Param;

typedef struct _dlc_rx_ctrl
{
	void* entity;
	unsigned short state;
	unsigned short * bc_frm_wait_free;

	Signal dlc_rx_ind;
	Dlc_Rx_Ind_Param dlc_rx_ind_param;

	Signal arq_rx_rsp[NODE_MAX_CNT];
	Arq_Rx_Rsp_Param  arq_rx_rsp_param[NODE_MAX_CNT];

	Signal arq_rst_tx_req[NODE_MAX_CNT];
	Arq_Rst_Tx_Req_Param arq_rst_tx_req_param[NODE_MAX_CNT];

	Signal arq_ack_tx_req[NODE_MAX_CNT];
	Arq_Ack_Tx_Req_Param arq_ack_tx_req_param[NODE_MAX_CNT];
	
	Signal frm_rx_rsp[NODE_MAX_CNT]; 
	
	Signal* frm_rx_ind[NODE_MAX_CNT];
	Signal* dlc_rx_rsp;
	Signal* arq_rx_ind[NODE_MAX_CNT];
	Signal* ack_tx_req;  //yun：回复ack的 
	Signal* rst_tx_req;

}Dlc_Rx_Ctrl;

extern void DlcRxCtrlSetup(Dlc_Rx_Ctrl* );
extern void DlcRxCtrlInit(Dlc_Rx_Ctrl*);

#endif

