#ifndef _ARQ_RX_H
#define _ARQ_RX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"

typedef struct _arq_rx_ind_param  
{
	unsigned short *xfrm;//MAC֡��ַ
	unsigned short xlen; //MAC֡����
}Arq_Rx_Ind_Param;

typedef struct _frm_rx_rsp_param
{
	unsigned short rsv;//Ԥ��
}Frm_Rx_Rsp_Param;

typedef struct _t_arq_ack_tx_req_param
{
	unsigned short *ack_sgl;//ack�����ַ
	//unsigned short len;//ack�����
	unsigned short id;
}T_Arq_Ack_Tx_Req_Param;

typedef struct _t_arq_rst_tx_req_param
{
	unsigned short *rst_sgl;//arq���������ַ
	unsigned short len;//arq���������
}T_Arq_Rst_Tx_Req_Param;

typedef struct _t_frm_rx_free_ind_param
{
	unsigned short rsv;//Ԥ���Ժ�ʹ��
}T_Frm_Rx_Free_Ind_Param;

typedef struct _t_frm_rx_ind_param
{
	unsigned short * xdat;//������֡���ɵ�ַ
	unsigned short xlen;//����
	unsigned short seg_flag;//�α�ǣ���ʼ10������01���м�00����ʼ�ҽ���11
	unsigned short bc_uc_falg;//0-������1-�㲥
}T_Frm_Rx_Ind_Param;

typedef struct _arq_rx
{
	void* entity;
	unsigned short state;

	unsigned short id;//arq_rxģ���ţ�����Ӧ�ķ��ͽڵ�

	unsigned short btm;//����
	unsigned short top;//����
	unsigned short idx;//���������ϲ������ݵ�λ��
	Mac_Frm_1002* mac_frm[ARQ_SN_SIZE];//��������
	unsigned short sts[ARQ_SN_SIZE];//��������״̬

	unsigned short rst_flag;//���ñ�־
	unsigned short emergency_ack;//����ack���

	
	Ack_Sub_Frm ack_sub_frm;//������ack����
	Rst_Sub_Frm rst_sub_frm;//������arq��������

	Signal arq_rx_ind;
	Arq_Rx_Ind_Param  arq_rx_ind_param;

	Signal frm_rx_rsp;
	Frm_Rx_Rsp_Param frm_rx_rsp_param;

	Signal* arq_rx_rsp;
	Signal* arq_rst_tx_req;
	Signal* arq_ack_tx_req;
	Signal* frm_rx_ind;
	Signal* frm_rx_free_ind;

}Arq_Rx;

extern void ArqRxSetup(Arq_Rx*);
extern void ArqRxInit(Arq_Rx*);

#endif


