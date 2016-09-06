#ifndef _DLC_RX_CTRL_H
#define _DLC_RX_CTRL_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"

typedef struct _dlc_rx_ind_param 
{
	unsigned short *frm;//MAC֡��ַ
	unsigned short len;//MAC֡����
}Dlc_Rx_Ind_Param;

typedef struct _arq_rx_rsp_param 
{
	unsigned short rsv;//Ԥ��
}Arq_Rx_Rsp_Param;

typedef struct _arq_ack_tx_req_param
{
	unsigned short *ack_sgl;//ack�����ַ
	//unsigned short len;//ack�����
	unsigned short id;//yun�� ���ڵ�id
}Arq_Ack_Tx_Req_Param;

typedef struct _arq_rst_tx_req_param
{
	unsigned short *rst_sgl;//arq���������ַ
	unsigned short len;//arq���������
}Arq_Rst_Tx_Req_Param;

typedef struct _t_ack_tx_req_param
{
	unsigned short *ack_sgl;//ack�����ַ
	//unsigned short len;//ack�����
	unsigned short id;//yun�� ���ڵ�id
}T_Ack_Tx_Req_Param;

typedef struct _t_rst_tx_req_param
{
	unsigned short *rst_sgl;//arq���������ַ
	unsigned short len;//arq���������
}T_Rst_Tx_Req_Param;

typedef struct _t_arq_rx_ind_param  
{
	unsigned short *frm;//MAC֡��ַ
	unsigned short len; //MAC֡����
}T_Arq_Rx_Ind_Param;

typedef struct _t_dlc_rx_rsp_param  
{
	unsigned short rsv;//Ԥ��
}T_Dlc_Rx_Rsp_Param;

typedef struct _t_bc_frm_rx_ind_param
{
	unsigned short * xdat;//������֡���ɵ�ַ
	unsigned short xlen;//����
	unsigned short seg_flag;//�α�ǣ���ʼ10������01���м�00����ʼ�ҽ���11
	unsigned short bc_uc_falg;//0-������1-�㲥
	unsigned short bc_seg_num;//�㲥�����
}T_Bc_Frm_Rx_Ind_Param;

typedef struct _dlc_rx_ctrl
{
	void* entity;
	unsigned short state;
	unsigned short * bc_frm_wait_free;

	Signal dlc_rx_ind;
	Dlc_Rx_Ind_Param dlc_rx_ind_param;

	Signal arq_rx_rsp[NODE_MAX_CNT];
	Arq_Rx_Rsp_Param  arq_rx_rsp_param[NODE_MAX_CNT];

	Signal arq_rst_tx_req[NODE_MAX_CNT];
	Arq_Rst_Tx_Req_Param arq_rst_tx_req_param[NODE_MAX_CNT];

	Signal arq_ack_tx_req[NODE_MAX_CNT];
	Arq_Ack_Tx_Req_Param arq_ack_tx_req_param[NODE_MAX_CNT];
	
	Signal frm_rx_rsp[NODE_MAX_CNT]; 
	
	Signal* frm_rx_ind[NODE_MAX_CNT];
	Signal* dlc_rx_rsp;
	Signal* arq_rx_ind[NODE_MAX_CNT];
	Signal* ack_tx_req;  //yun���ظ�ack�� 
	Signal* rst_tx_req;

}Dlc_Rx_Ctrl;

extern void DlcRxCtrlSetup(Dlc_Rx_Ctrl* );
extern void DlcRxCtrlInit(Dlc_Rx_Ctrl*);

#endif

