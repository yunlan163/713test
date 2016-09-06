#ifndef _FRM_RX_H
#define _FRM_RX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"
//#include "..\..\..\..\..\fm_mem.h"

typedef struct _t_frm_rx_rsp_param
{
	unsigned short rsv;//Ԥ��
}T_Frm_Rx_Rsp_Param;
typedef struct _t_ddat_rx_ind_param  
{
	unsigned short ta;//���ͽڵ��
	unsigned short uc_bc_flag;//�����㲥��־��0��ʾ������1��ʾ�㲥
	unsigned short *net_frm;//����֡��ַ
	unsigned short net_frm_len;//����֡����
	
}T_Ddat_Rx_Ind_Param;

typedef struct _ddat_rx_rsp_param  
{
	unsigned short rsv;//Ԥ��
}Ddat_Rx_Rsp_Param;

typedef struct _frm_rx_free_ind_param
{
	unsigned short rsv;//Ԥ���Ժ�ʹ��
}Frm_Rx_Free_Ind_Param;

typedef struct _frm_rx_ind_param
{
	unsigned short * xdat;//������֡���ɵ�ַ
	unsigned short xlen;//����
	unsigned short seg_flag;//�α�ǣ���ʼ10������01���м�00����ʼ�ҽ���11
	unsigned short bc_uc_falg;//0-������1-�㲥
	unsigned short bc_seg_num;//�㲥�����
}Frm_Rx_Ind_Param;

typedef struct _frm_rx
{
	void* entity;
	unsigned short state;

	unsigned short id;//frm_rxģ���ţ������ͽڵ��
	unsigned short uc_bc_flag;//�����㲥��־��0��ʾ����ģ�飬1��ʾ�㲥ģ��
	unsigned short bc_seg_num;//bc����ţ����ڹ㲥�����

	unsigned short* net_frm;//����֡��ַ
	unsigned short net_frm_len;//����֡����
	unsigned short*  dat_idx;//����֡���յ���λ��

	Signal frm_rx_ind;
	Frm_Rx_Ind_Param frm_rx_ind_param;

	Signal frm_rx_free_ind;
	Frm_Rx_Free_Ind_Param frm_rx_free_ind_param;

	Signal ddat_rx_rsp;
	Ddat_Rx_Rsp_Param ddat_rx_rsp_param;

	Signal* ddat_rx_ind;
	Signal* frm_rx_rsp;
	
}Frm_Rx;

extern void FrmRxSetup(Frm_Rx*);
extern void FrmRxInit(Frm_Rx*);

#endif


