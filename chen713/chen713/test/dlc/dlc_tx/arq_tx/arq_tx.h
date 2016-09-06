#ifndef _ARQ_TX_H
#define _ARQ_TX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include"..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"


typedef struct _arq_ack_rx_ind_param
{
	unsigned short* ack_sgl;//ack信令
}Arq_Ack_Rx_Ind_Param;
typedef struct _arq_rst_rx_ind_param
{
	unsigned short rsv;//预留
}Arq_Rst_Rx_Ind_Param;
typedef struct _arq_tx_ind_param
{
	unsigned short rsv;//预留
}Arq_Tx_Ind_Param;

typedef struct _frm_tx_rsp_param
{
	unsigned short xlen;
}Frm_Tx_Rsp_Param;

typedef struct _t_arq_tx_rsp_param
{
	unsigned short* xfrm;//mac帧地址
}T_Arq_Tx_Rsp_Param;
typedef struct _t_frm_tx_free_ind_param
{
	unsigned short type;//类型重发失败/收方重置

}T_Frm_Tx_Free_Ind_Param;

typedef struct _t_frm_tx_ind_param
{
	unsigned short *dat_sub_frm;//数据子帧地址
}T_Frm_Tx_Ind_Param;
typedef struct _t_arq_tx_succ_ind_param
{
	unsigned short succ_cnt;//成功个数
}T_Arq_Tx_Succ_Ind_Param;
typedef struct _arq_tx
{
	void* entity;
	unsigned short state;

	unsigned short id;//arq_tx模块编号，即对于接收节点

	unsigned short top;//窗顶
	unsigned short btm;//窗底
	unsigned short idx;//发送到窗口的位置
	unsigned short size;//窗口大小
	Mac_Frm_1002 *frm[ARQ_SN_SIZE];//窗口区域   16
	unsigned short sts[ARQ_SN_SIZE];//窗口区域中每个窗口状态（无数据ARQ_STS_IDLE，有数据未发成功ARQ_STS_TX，有数据且发成功ARQ_STS_SUCC,有数据但在等ack ARQ_STS_WAIT_ACK）
	unsigned short frm_len[ARQ_SN_SIZE];//窗口区域中每个窗口的MAC帧长度
	unsigned short tx_cnt[ARQ_SN_SIZE];//窗口区域中每个窗口的重发个数计数

	unsigned short timer_wait_ack[ARQ_SN_SIZE];//等待ack定时
	unsigned short rst_flag;//重置标志  第一次需要重置

	Signal arq_tx_ind;
	Arq_Tx_Ind_Param arq_tx_ind_param;

	Signal arq_ack_rx_ind;
	Arq_Ack_Rx_Ind_Param arq_ack_rx_ind_param;

	Signal arq_rst_rx_ind;
	Arq_Rst_Rx_Ind_Param arq_rst_rx_ind_param;

	Signal frm_tx_rsp;
	Frm_Tx_Rsp_Param frm_tx_rsp_param;


	Signal* frm_tx_ind;
	Signal* arq_tx_rsp;
	Signal* frm_tx_free_ind;
	Signal* arq_tx_succ_ind;



}Arq_Tx;
extern void ArqTxSetup(Arq_Tx*);
extern void ArqTxInit(Arq_Tx*);

#endif

