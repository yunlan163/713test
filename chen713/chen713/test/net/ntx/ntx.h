#ifndef _NTX_H
#define _NTX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"
//#include "..\..\..\..\fm_mem.h"

#define NET_QUE_MAX_NUM 20//网络队列长度
#define NET_PKT_MAX_TTL 100//网络包生存时间
#define NET_PKT_RESEND_FROM_FM_TMR  100//网络包从铁电中读出来的时间
#define RT_FAIL_TMR 1000//路由失败时间定时，用于清除铁电中的数据
typedef struct _data_tx_req_param
{
	unsigned short *xdat;//应用层净数据
	unsigned short xlen;//xdat的长度
	unsigned short xda;//目的地址
	unsigned short xpri;//优先级
	unsigned short hello_flag;//Hello包标志
}Data_Tx_Req_Param;
typedef struct _ntx_ind_param
{
	unsigned short* dat;
	unsigned short ra;  //接收地址
	unsigned short succ_flag;//成功标记
	unsigned short fail_type;//失败类型
	unsigned short da;  //目的地址
	unsigned short sa;    //源地址
}NTx_Ind_Param;

typedef struct _rt_tx_rsp_param
{
	unsigned short succ;//是否有路由
	unsigned short ra;//下一跳地址
	unsigned short da;//目的地址
}Rt_Tx_Rsp_Param;
typedef struct _relay_tx_ind_param
{
	unsigned short *xdat;
	unsigned short xlen;
	unsigned short xda;
	unsigned short xpri;
}Relay_Tx_Ind_Param;

typedef struct _t_ntx_req_param
{
	unsigned short ra;
	unsigned short *xdat;
	unsigned short xlen;//此长度为整个网络包的长度，包含网络包净荷和包头

}T_NTx_Req_Param;
typedef struct _t_rt_tx_ind_param
{
	unsigned short da;
}T_Rt_Tx_Ind_Param;
typedef struct _t_link_fail_ind_param
{
	unsigned short ra;//下一跳地址

}T_Link_Fail_Ind_Param;

typedef struct _que_elmt
{
	unsigned short *dat;
	unsigned short len;
	unsigned short ra;
	unsigned short da;
	unsigned short pri;
	unsigned long arrvial_time;
	unsigned short first_send_flag;

}Que_Elmt;
typedef struct _net_que_list
{
	unsigned short top;
	unsigned short btm;
	unsigned short size;
	unsigned short curt_snd_flag;//正在发送标记
	Que_Elmt que_elmt[NET_QUE_MAX_NUM];

}Net_Que_List;

typedef struct _net_que_list_fm
{
//	unsigned short *dat[FLASH_MEM_BLOCK_CNT];
	unsigned short btm;
	unsigned short top;
	unsigned short size;
}Net_Que_List_FM;
typedef struct _ntx
{
	void *entity;
	unsigned short state;

	unsigned short * dat;//整个网络包地址，请求路由时暂时保存
	unsigned short len;//网络包中的净荷长度，请求路由时暂时保存
	unsigned short da;//目的地址，请求路由时暂时保存
	unsigned long  arrvial_time;//网络包到达时间，请求路由时暂时保存

	unsigned long sts;//32个节点的状态各1bit
	unsigned short ntx_sn[NODE_MAX_CNT];//各个节点网络帧序号  初始化=1
	unsigned short ntx_sn_mul;//多播序号  初始化=1
	
	unsigned short curt_node;
	unsigned long ntx_local_time;

	unsigned short data_tx_req_rt_tx_ind_flag;//上层来数据，请求路由标记

	Net_Que_List net_que_list[NODE_MAX_CNT];//网络数据包队列

	Net_Que_List_FM net_que_list_fm;
	unsigned short read_net_pkt_from_fm[NET_FRM_LEN_MAX];//铁电存数据读出暂存
	unsigned short rt_fail_tmr[NODE_MAX_CNT];//路由失败时间定时

	Signal data_tx_req;
	Data_Tx_Req_Param data_tx_req_param;

	Signal rt_tx_rsp;
	Rt_Tx_Rsp_Param rt_tx_rsp_param;

	Signal relay_tx_ind;
	Relay_Tx_Ind_Param relay_tx_ind_param;

	Signal ntx_ind[NODE_MAX_CNT];
	NTx_Ind_Param ntx_ind_param[NODE_MAX_CNT];

	Signal* rt_tx_ind;
	Signal* ntx_req;
	Signal* link_fail_ind;

}NTx;

extern void TimerNTx(NTx* proc);
extern void NTxSetup(NTx*);
extern void NTxInit(NTx*);
#endif
