#ifndef _DLC_RX_DUMP_H
#define _DLC_RX_DUMP_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"

#define DLC_QUE_MAX_NUM 100//缓存网络包队列大小

typedef struct _ddat_rx_ind_param  
{
	unsigned short ta;//发送节点号
	unsigned short uc_bc_flag;//单播广播标志，0表示单播，1表示广播
	unsigned short *net_frm;//网络帧地址
	unsigned short net_frm_len;//网络帧长度
}Ddat_Rx_Ind_Param;
typedef struct _nrx_rsp_param  
{
	unsigned short rsv;//预留
}Nrx_Rsp_Param;
typedef struct _t_nrx_ind_param
{
	unsigned short ta;//发送节点号
	unsigned short *net_frm;//网络帧地址
	unsigned short net_frm_len;//网络帧长度
}T_NRx_Ind_Param;
typedef struct _t_ddat_rx_rsp_param  
{
	unsigned short rsv;//预留
}T_Ddat_Rx_Rsp_Param;

typedef struct _que_dat
{
	unsigned short *dat;//地址
	unsigned short len;//长度
	unsigned short ta;//发送节点
}Que_Dat;
typedef struct _que
{
	unsigned short top;
	unsigned short btm;
	unsigned short size;
	Que_Dat que_dat[DLC_QUE_MAX_NUM];//网络包信息
}Que;//网络包队列
typedef struct _dlc_rx_dump
{
	void* entity;
	unsigned short state;

	Que que;//收到的网络数据包队列

	Signal ddat_rx_ind[2][NODE_MAX_CNT];
	Ddat_Rx_Ind_Param  ddat_rx_ind_param[2][NODE_MAX_CNT];

	Signal	nrx_rsp; 
	Nrx_Rsp_Param nrx_rsp_param;

	Signal frm_rx_rsp[NODE_MAX_CNT]; 

	Signal* frm_rx_ind[NODE_MAX_CNT];
	Signal* nrx_ind;
	Signal* ddat_rx_rsp[2][NODE_MAX_CNT];

	

}Dlc_Rx_Dump;

extern void  DlcRxDumpSetup(Dlc_Rx_Dump*);
extern void  DlcRxDumpInit(Dlc_Rx_Dump*);

#endif

