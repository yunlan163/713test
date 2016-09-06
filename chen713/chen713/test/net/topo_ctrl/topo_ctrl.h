#ifndef _TOPO_CTRL_H
#define _TOPO_CTRL_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"

#define MAX_NBR_TTL 20//ttl最大值
#define MICRO_SLOT_CNT_PER_TTL 800//每个ttl的时间分频


typedef struct _t_hello_rt_rx_ind_param
{
	unsigned short *hello_pkt;//hello包地址
	unsigned short xta;//发送节点
}T_Hello_Rt_Rx_Ind_Param;

typedef struct _t_nbr_sts_chg_ind_param
{
	unsigned short id;
	unsigned short link_sts;
}T_Nbr_Sts_Chg_Ind_Param;

typedef struct _t_hello_topo_tx_cfm_param
{
	unsigned short rsv;//预留

}T_HEllo_Topo_Tx_Cfm_Param;

typedef struct _link_fail_ind_param
{
	unsigned short xra;//断的id号

}Link_Fail_Ind_Param;

typedef struct _hello_topo_tx_req_param
{
	unsigned long *xdat;//填拓扑信息地址

}Hello_Topo_Tx_Req_Param;

typedef struct _hello_rx_ind_param
{
	unsigned short *hello_pkt;//hello包地址
	unsigned short xta;//发送节点
}Hello_Rx_Ind_Param;


typedef struct _nbr_table//邻节点表
{
	unsigned short nbr_sts[NODE_MAX_CNT];//状态0-断，1-单链，3-双链
	unsigned short nbr_ttl[NODE_MAX_CNT];//ttl

}Nbr_Table;

typedef struct _topo_ctrl
{
	void *entity;
	unsigned short state;
	
	Nbr_Table nbr_table;//邻节点表
	unsigned short ttl_timer;//定时到时，ttl减去1

	Signal link_fail_ind;//重发失败，链路断开
	Link_Fail_Ind_Param link_fail_ind_param;

	Signal hello_rx_ind;//收到hello包
	Hello_Rx_Ind_Param hello_rx_ind_param;

	Signal hello_topo_tx_req;//发送hello包，请求填topo信息
	Hello_Topo_Tx_Req_Param hello_topo_tx_req_param;

	Signal* nbr_sts_chg_ind;//链接状况改变
	Signal* hello_topo_tx_cfm;//填完topo信息,回复
	Signal* hello_rt_rx_ind;//将hello包送至rt_ctrl

}Topo_Ctrl;

extern void  TopoCtrlSetup(Topo_Ctrl*);
extern void  TopoCtrlInit(Topo_Ctrl*);
extern void TimerTopoCtrl(Topo_Ctrl* );
#endif

