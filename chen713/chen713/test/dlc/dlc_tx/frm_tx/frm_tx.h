#ifndef _FRM_TX_H
#define _FRM_TX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include"..\..\..\mib\mib.h"

#define MAX_SEG_NUM_PER_NET_FRM 30 //一包网络包分段后的最大分段数
typedef struct _frm_tx_req_param
{
	unsigned short* xdat;
	unsigned short xlen;

}Frm_Tx_Req_Param;
typedef struct _frm_tx_free_ind_param
{
	unsigned short fail_type;//当失败时有效，失败类型重发多次失败，或者收到arq重置信令收方重导致置发送失败
}Frm_Tx_Free_Ind_Param;
typedef struct _frm_tx_ind_param
{
	unsigned short *dat_sub_frm;//数据子帧地址   yun：与下层请求的地址指向同一块内存
}Frm_Tx_Ind_Param;
typedef struct _arq_tx_succ_ind_param
{
	unsigned short succ_cnt;//成功个数
}Arq_Tx_Succ_Ind_Param;

typedef struct _t_frm_tx_rsp_param
{
	unsigned short xlen;
}T_Frm_Tx_Rsp_Param;

typedef struct _t_frm_tx_cfm_param
{
	unsigned short id;//本frm_tx编号,即对应接收节点
	unsigned short succ_fail_flag;//成功失败标记1-成功，0失败
	unsigned short fail_type;//当失败时有效，失败类型重发多次失败，或者收到arq重置信令收方重导致置发送失败
	unsigned short *net_frm;//网络帧地址
	unsigned short da;
	unsigned short sa;
}T_Frm_Tx_Cfm_Param;

typedef struct _frm_tx
{
	void* entity;
	unsigned short state;

	unsigned short id;//本frm_tx编号,即对应接收节点

	unsigned short seg_num;//分段总数
	unsigned short seg_idx;//发送到的位置
	unsigned short seg_len[MAX_SEG_NUM_PER_NET_FRM];//所有分段的每段长度
	unsigned short succ_seg_num;//发送成功的段数 

	unsigned short *net_frm;//网络帧地址
	unsigned short net_frm_len;//网络帧长度

	unsigned short bc_seg_num;//bc段序号，用于广播包序号

	unsigned short da;//目的节点
	unsigned short sa;//源节点
	
	Signal frm_tx_req;
	Frm_Tx_Req_Param frm_tx_req_param;
	
	Signal frm_tx_ind;
	Frm_Tx_Ind_Param frm_tx_ind_param;

	Signal frm_tx_free_ind;
	Frm_Tx_Free_Ind_Param frm_tx_free_ind_param;

	Signal arq_tx_succ_ind;
	Arq_Tx_Succ_Ind_Param arq_tx_succ_ind_param;

	Signal* frm_tx_cfm;
	Signal* frm_tx_rsp;

}Frm_Tx;
extern void FrmTxSetup(Frm_Tx*);
extern void FrmTxInit(Frm_Tx*);
#endif
