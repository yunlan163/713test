#ifndef _NRX_H
#define _NRX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"

#define NET_SA_MASK 0x1f
#define NET_PRI_MASK 0x60
#define NET_TYPE_MASK 0x80

typedef struct _nrx_ind_param
{
	unsigned short xta;
	unsigned short *xdat;
	unsigned short xlen;
}NRx_Ind_Param;
typedef struct _data_rx_rsp_param
{
	unsigned short rsv;
}Data_Rx_Rsp_Param;
typedef struct _t_relay_tx_ind_param
{
	unsigned short *xdat;
	unsigned short xlen;
	unsigned short xda;
	unsigned short xpri;
}T_Relay_Tx_Ind_Param;

typedef struct _t_data_rx_ind_param
{
	unsigned short *dat;
	unsigned short len;
	unsigned short sa;

}T_Data_Rx_Ind_Param;

typedef struct _t_drelay_chk_req_param
{
	unsigned short sa;
}T_DRelay_Chk_Req_Param;

typedef struct _drelay_chk_cfm_param
{
	unsigned short succ_flag;//1-中继，0-不中继
}DRelay_Chk_Cfm_Param;
typedef struct _nrx
{
	void *entity;
	unsigned short state;

	unsigned short *dat_wait_free;//保存网络帧地址，当上层接收完成后释放
	unsigned short nrx_sn[NODE_MAX_CNT][NODE_MAX_CNT];//包寸各个接收节点网络包序号
	unsigned short nrx_sn_mul[NODE_MAX_CNT];//多播包的序号

	Signal nrx_ind;
	NRx_Ind_Param nrx_ind_param;

	Signal data_rx_rsp;
	Data_Rx_Rsp_Param data_rx_rsp_param;

	Signal drelay_chk_cfm;
	DRelay_Chk_Cfm_Param drelay_chk_cfm_param;

	Signal* nrx_rsp;
	Signal* data_rx_ind;
	Signal* relay_tx_ind;
	Signal* drelay_chk_req;

}NRx;

extern void NRxSetup(NRx*);
extern void NRxInit(NRx*);

#endif

