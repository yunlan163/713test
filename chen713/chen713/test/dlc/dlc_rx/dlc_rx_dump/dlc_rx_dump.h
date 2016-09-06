#ifndef _DLC_RX_DUMP_H
#define _DLC_RX_DUMP_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include "..\..\..\mib\mib.h"

#define DLC_QUE_MAX_NUM 100//������������д�С

typedef struct _ddat_rx_ind_param  
{
	unsigned short ta;//���ͽڵ��
	unsigned short uc_bc_flag;//�����㲥��־��0��ʾ������1��ʾ�㲥
	unsigned short *net_frm;//����֡��ַ
	unsigned short net_frm_len;//����֡����
}Ddat_Rx_Ind_Param;
typedef struct _nrx_rsp_param  
{
	unsigned short rsv;//Ԥ��
}Nrx_Rsp_Param;
typedef struct _t_nrx_ind_param
{
	unsigned short ta;//���ͽڵ��
	unsigned short *net_frm;//����֡��ַ
	unsigned short net_frm_len;//����֡����
}T_NRx_Ind_Param;
typedef struct _t_ddat_rx_rsp_param  
{
	unsigned short rsv;//Ԥ��
}T_Ddat_Rx_Rsp_Param;

typedef struct _que_dat
{
	unsigned short *dat;//��ַ
	unsigned short len;//����
	unsigned short ta;//���ͽڵ�
}Que_Dat;
typedef struct _que
{
	unsigned short top;
	unsigned short btm;
	unsigned short size;
	Que_Dat que_dat[DLC_QUE_MAX_NUM];//�������Ϣ
}Que;//���������
typedef struct _dlc_rx_dump
{
	void* entity;
	unsigned short state;

	Que que;//�յ����������ݰ�����

	Signal ddat_rx_ind[2][NODE_MAX_CNT];
	Ddat_Rx_Ind_Param  ddat_rx_ind_param[2][NODE_MAX_CNT];

	Signal	nrx_rsp; 
	Nrx_Rsp_Param nrx_rsp_param;

	Signal frm_rx_rsp[NODE_MAX_CNT]; 

	Signal* frm_rx_ind[NODE_MAX_CNT];
	Signal* nrx_ind;
	Signal* ddat_rx_rsp[2][NODE_MAX_CNT];

	

}Dlc_Rx_Dump;

extern void  DlcRxDumpSetup(Dlc_Rx_Dump*);
extern void  DlcRxDumpInit(Dlc_Rx_Dump*);

#endif

