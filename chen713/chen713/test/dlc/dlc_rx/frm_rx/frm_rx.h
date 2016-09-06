#ifndef _FRM_RX_H
#define _FRM_RX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"
//#include "..\..\..\..\..\fm_mem.h"

typedef struct _t_frm_rx_rsp_param
{
	unsigned short rsv;//预留
}T_Frm_Rx_Rsp_Param;
typedef struct _t_ddat_rx_ind_param  
{
	unsigned short ta;//发送节点号
	unsigned short uc_bc_flag;//单播广播标志，0表示单播，1表示广播
	unsigned short *net_frm;//网络帧地址
	unsigned short net_frm_len;//网络帧长度
	
}T_Ddat_Rx_Ind_Param;

typedef struct _ddat_rx_rsp_param  
{
	unsigned short rsv;//预留
}Ddat_Rx_Rsp_Param;

typedef struct _frm_rx_free_ind_param
{
	unsigned short rsv;//预留以后使用
}Frm_Rx_Free_Ind_Param;

typedef struct _frm_rx_ind_param
{
	unsigned short * xdat;//数据子帧净荷地址
	unsigned short xlen;//长度
	unsigned short seg_flag;//段标记：开始10、结束01、中间00、开始且结束11
	unsigned short bc_uc_falg;//0-单播，1-广播
	unsigned short bc_seg_num;//广播段序号
}Frm_Rx_Ind_Param;

typedef struct _frm_rx
{
	void* entity;
	unsigned short state;

	unsigned short id;//frm_rx模块编号，即发送节点号
	unsigned short uc_bc_flag;//单播广播标志，0表示单播模块，1表示广播模块
	unsigned short bc_seg_num;//bc段序号，用于广播包序号

	unsigned short* net_frm;//网络帧地址
	unsigned short net_frm_len;//网络帧长度
	unsigned short*  dat_idx;//网络帧接收到的位置

	Signal frm_rx_ind;
	Frm_Rx_Ind_Param frm_rx_ind_param;

	Signal frm_rx_free_ind;
	Frm_Rx_Free_Ind_Param frm_rx_free_ind_param;

	Signal ddat_rx_rsp;
	Ddat_Rx_Rsp_Param ddat_rx_rsp_param;

	Signal* ddat_rx_ind;
	Signal* frm_rx_rsp;
	
}Frm_Rx;

extern void FrmRxSetup(Frm_Rx*);
extern void FrmRxInit(Frm_Rx*);

#endif


