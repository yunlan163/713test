#ifndef _DLC_TX_DUMP_H
#define _DLC_TX_DUMP_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include"..\..\..\mib\mib.h"
typedef struct _ntx_req_param
{
	unsigned short ra;  //yun：单播下一跳的地址，可能有多跳
	unsigned short *xdat;
	unsigned short xlen;

}NTx_Req_Param;

typedef struct _frm_tx_cfm_param
{
	unsigned short id;
	unsigned short succ_fail_flag;
	unsigned short fail_type;//当失败时有效，失败类型重发多次失败，或者收到arq重置信令收方重导致置发送失败
	unsigned short* net_frm;//网络帧地址
	unsigned short da;
	unsigned short sa;
}Frm_Tx_Cfm_Param;

typedef struct _t_ntx_ind_param
{
	unsigned short* dat;
	unsigned short ra;
	unsigned short succ_flag;//成功标记
	unsigned short fail_type;//失败类型
	unsigned short da;
	unsigned short sa;
}T_NTx_Ind_Param;


typedef struct _t_frm_tx_req_param
{
	unsigned short* xdat;
	unsigned short xlen;

}T_Frm_Tx_Req_Param;
typedef struct _dlc_tx_dump
{
	void* entity;
	unsigned short state;

	Signal ntx_req;
	NTx_Req_Param ntx_req_param;

	Signal frm_tx_cfm[NODE_MAX_CNT];
	Frm_Tx_Cfm_Param frm_tx_cfm_param[NODE_MAX_CNT];

	Signal* ntx_ind[NODE_MAX_CNT];
	Signal* frm_tx_req[NODE_MAX_CNT];

}Dlc_Tx_Dump;

extern void DlcTxDumpSetup(Dlc_Tx_Dump*);
extern void DlcTxDumpInit(Dlc_Tx_Dump*);
#endif

