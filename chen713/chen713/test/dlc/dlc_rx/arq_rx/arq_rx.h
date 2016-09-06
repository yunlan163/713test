#ifndef _ARQ_RX_H
#define _ARQ_RX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"

typedef struct _arq_rx_ind_param  
{
	unsigned short *xfrm;//MAC帧地址
	unsigned short xlen; //MAC帧长度
}Arq_Rx_Ind_Param;

typedef struct _frm_rx_rsp_param
{
	unsigned short rsv;//预留
}Frm_Rx_Rsp_Param;

typedef struct _t_arq_ack_tx_req_param
{
	unsigned short *ack_sgl;//ack信令地址
	//unsigned short len;//ack信令长度
	unsigned short id;
}T_Arq_Ack_Tx_Req_Param;

typedef struct _t_arq_rst_tx_req_param
{
	unsigned short *rst_sgl;//arq重置信令地址
	unsigned short len;//arq重置信令长度
}T_Arq_Rst_Tx_Req_Param;

typedef struct _t_frm_rx_free_ind_param
{
	unsigned short rsv;//预留以后使用
}T_Frm_Rx_Free_Ind_Param;

typedef struct _t_frm_rx_ind_param
{
	unsigned short * xdat;//数据子帧净荷地址
	unsigned short xlen;//长度
	unsigned short seg_flag;//段标记：开始10、结束01、中间00、开始且结束11
	unsigned short bc_uc_falg;//0-单播，1-广播
}T_Frm_Rx_Ind_Param;

typedef struct _arq_rx
{
	void* entity;
	unsigned short state;

	unsigned short id;//arq_rx模块编号，即对应的发送节点

	unsigned short btm;//窗底
	unsigned short top;//窗顶
	unsigned short idx;//窗口中向上层送数据的位置
	Mac_Frm_1002* mac_frm[ARQ_SN_SIZE];//窗口区域
	unsigned short sts[ARQ_SN_SIZE];//窗口区域状态

	unsigned short rst_flag;//重置标志
	unsigned short emergency_ack;//紧急ack标记

	
	Ack_Sub_Frm ack_sub_frm;//待发的ack信令
	Rst_Sub_Frm rst_sub_frm;//待发的arq重置信令

	Signal arq_rx_ind;
	Arq_Rx_Ind_Param  arq_rx_ind_param;

	Signal frm_rx_rsp;
	Frm_Rx_Rsp_Param frm_rx_rsp_param;

	Signal* arq_rx_rsp;
	Signal* arq_rst_tx_req;
	Signal* arq_ack_tx_req;
	Signal* frm_rx_ind;
	Signal* frm_rx_free_ind;

}Arq_Rx;

extern void ArqRxSetup(Arq_Rx*);
extern void ArqRxInit(Arq_Rx*);

#endif


