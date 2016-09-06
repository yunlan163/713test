#ifndef _TOPO_CTRL_H
#define _TOPO_CTRL_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"

#define MAX_NBR_TTL 20//ttl���ֵ
#define MICRO_SLOT_CNT_PER_TTL 800//ÿ��ttl��ʱ���Ƶ


typedef struct _t_hello_rt_rx_ind_param
{
	unsigned short *hello_pkt;//hello����ַ
	unsigned short xta;//���ͽڵ�
}T_Hello_Rt_Rx_Ind_Param;

typedef struct _t_nbr_sts_chg_ind_param
{
	unsigned short id;
	unsigned short link_sts;
}T_Nbr_Sts_Chg_Ind_Param;

typedef struct _t_hello_topo_tx_cfm_param
{
	unsigned short rsv;//Ԥ��

}T_HEllo_Topo_Tx_Cfm_Param;

typedef struct _link_fail_ind_param
{
	unsigned short xra;//�ϵ�id��

}Link_Fail_Ind_Param;

typedef struct _hello_topo_tx_req_param
{
	unsigned long *xdat;//��������Ϣ��ַ

}Hello_Topo_Tx_Req_Param;

typedef struct _hello_rx_ind_param
{
	unsigned short *hello_pkt;//hello����ַ
	unsigned short xta;//���ͽڵ�
}Hello_Rx_Ind_Param;


typedef struct _nbr_table//�ڽڵ��
{
	unsigned short nbr_sts[NODE_MAX_CNT];//״̬0-�ϣ�1-������3-˫��
	unsigned short nbr_ttl[NODE_MAX_CNT];//ttl

}Nbr_Table;

typedef struct _topo_ctrl
{
	void *entity;
	unsigned short state;
	
	Nbr_Table nbr_table;//�ڽڵ��
	unsigned short ttl_timer;//��ʱ��ʱ��ttl��ȥ1

	Signal link_fail_ind;//�ط�ʧ�ܣ���·�Ͽ�
	Link_Fail_Ind_Param link_fail_ind_param;

	Signal hello_rx_ind;//�յ�hello��
	Hello_Rx_Ind_Param hello_rx_ind_param;

	Signal hello_topo_tx_req;//����hello����������topo��Ϣ
	Hello_Topo_Tx_Req_Param hello_topo_tx_req_param;

	Signal* nbr_sts_chg_ind;//����״���ı�
	Signal* hello_topo_tx_cfm;//����topo��Ϣ,�ظ�
	Signal* hello_rt_rx_ind;//��hello������rt_ctrl

}Topo_Ctrl;

extern void  TopoCtrlSetup(Topo_Ctrl*);
extern void  TopoCtrlInit(Topo_Ctrl*);
extern void TimerTopoCtrl(Topo_Ctrl* );
#endif

