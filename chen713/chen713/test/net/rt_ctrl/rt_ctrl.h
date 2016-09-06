#ifndef _RT_CTRL_TX_H
#define _RT_CTRL_TX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"

#define METRIC_INFINITE 0x1f//���������


#define HELLO_PKT_NODE_CNT_PER_MAC_FRM 11//ÿ�η���HELLO�������ͽڵ���

typedef struct _t_rt_tx_rsp_param
{
	unsigned short succ;//�Ƿ���·��
	unsigned short ra;//��һ����ַ
	unsigned short da;//Ŀ�ĵ�ַ
}T_Rt_Tx_Rsp_Param;

typedef struct _hello_rt_rx_ind_param
{
	unsigned short *hello_pkt;//hello����ַ
	unsigned short xta;//���ͽڵ�
}Hello_Rt_Rx_Ind_Param;

typedef struct _t_hello_tx_req_param
{
	unsigned short *rti_sub_frm;
	unsigned short xlen;

}T_Hello_Tx_Req_Param;

typedef struct _t_hello_topo_tx_req_param
{
	unsigned long *dat;

}T_Hello_Topo_Tx_Req_Param;

typedef struct _nbr_sts_chg_ind_param
{	
	unsigned short xnbr;
	unsigned short xsucc;

}Nbr_Sts_Chg_Ind_Param;

typedef struct _rt_tx_ind_param
{
	unsigned short da;

}Rt_Tx_Ind_Param;

typedef struct _hello_topo_tx_cfm_param
{
	unsigned short rsv;//����

}Hello_Topo_Tx_Cfm_Param;

typedef struct _drelay_chk_req_param
{
	unsigned short sa;
}DRelay_Chk_Req_Param;

typedef struct _t_drelay_chk_cfm_param
{
	unsigned short succ_flag;
}T_DRelay_Chk_Cfm_Param;

typedef struct _vrelay_chk_req_param
{
	unsigned short sa;
}VRelay_Chk_Req_Param;

typedef struct _t_vrelay_chk_cfm_param
{
	unsigned short succ_flag;
}T_VRelay_Chk_Cfm_Param;
typedef struct _hello_tx_cfm_param
{
	unsigned short rsv;//����

}Hello_Tx_Cfm_Param;

typedef struct _hello_info_table//HELLOԭʼ��Ϣ��
{
	unsigned short metric_next;//metricΪ���ֽڵĵ�6λ��nextΪ���ֽڵĵ�5λ
	unsigned short seq;

}Hello_Info_Table;

typedef struct _rt_table//·�ɱ�
{
	unsigned short tflag_metric_next;//tflagΪ���λ��metricΪ���ֽڵĵ�6λ��nextΪ���ֽڵĵ�5λ
	unsigned short seq;

}Rt_Table;

typedef struct _rt_ctrl
{
	void* entity;
	unsigned short state;
	unsigned short rt_table_chg_tx_to_arm_flag;//·�ɱ仯��·�ɱ�����ARM��־
	unsigned short ip_table_chg_tx_to_arm_flag;//·�ɱ仯��·�ɱ�����ARM��־

	unsigned short hello_snd_timer_idx;//Hello����ʱ���͵���λ��

	Hello_Info_Table hello_info_table[NODE_MAX_CNT][NODE_MAX_CNT];//HELLOԭʼ��Ϣ��
	Rt_Table rt_table[NODE_MAX_CNT];//·�ɱ�
	unsigned short hello_send_timer;//HELLO�����Ͷ�ʱ��
	RTI_Sub_Frm rti_sub_frm;//hello��

	unsigned short hello_topo_tx_req_add_sig_flag;//��־hello_topo_tx_req�ź��Ѿ���ӣ���δִ��
	unsigned long ip_map_table[NODE_MAX_CNT];//IPӰ���
	
	Signal rt_tx_ind;//·�ɲ�ѯ
	Rt_Tx_Ind_Param rt_tx_ind_param;

	Signal hello_rt_rx_ind;//hello������
	Hello_Rt_Rx_Ind_Param hello_rt_rx_ind_param;

	Signal nbr_sts_chg_ind;//�ڽڵ�״̬�仯
	Nbr_Sts_Chg_Ind_Param nbr_sts_chg_ind_param;

	Signal hello_topo_tx_cfm;//��hello��ͷ�ڽڵ���Ϣ�ظ�
	Hello_Topo_Tx_Cfm_Param hello_topo_tx_cfm_param;

	//Signal hello_tx_cfm;//hello���������
	//Hello_Tx_Cfm_Param hello_tx_cfm_param;

	Signal drelay_chk_req;//�ಥ·�ɲ�ѯ
	DRelay_Chk_Req_Param drelay_chk_req_param;

	Signal vrelay_chk_req;//�����ಥ·�ɲ�ѯ
	VRelay_Chk_Req_Param vrelay_chk_req_param;

	Signal* rt_tx_rsp;//·�ɲ�ѯ�ظ�
	Signal* hello_tx_req;//����hello������
	Signal* hello_topo_tx_req;//������hello��ͷ�ڽڵ���Ϣ
	Signal* drelay_chk_cfm;//�ಥ·�ɲ�ѯ�ظ�
	Signal* vrelay_chk_cfm;//�����ಥ·�ɲ�ѯ�ظ�

}Rt_Ctrl;

extern void RtCtrlSetup(Rt_Ctrl*);
extern void RtCtrlInit(Rt_Ctrl*);
extern void TimerRtCtrl(Rt_Ctrl* );

#endif

