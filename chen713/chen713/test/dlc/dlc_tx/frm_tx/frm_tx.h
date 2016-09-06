#ifndef _FRM_TX_H
#define _FRM_TX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include"..\..\..\mib\mib.h"

#define MAX_SEG_NUM_PER_NET_FRM 30 //һ��������ֶκ�����ֶ���
typedef struct _frm_tx_req_param
{
	unsigned short* xdat;
	unsigned short xlen;

}Frm_Tx_Req_Param;
typedef struct _frm_tx_free_ind_param
{
	unsigned short fail_type;//��ʧ��ʱ��Ч��ʧ�������ط����ʧ�ܣ������յ�arq���������շ��ص����÷���ʧ��
}Frm_Tx_Free_Ind_Param;
typedef struct _frm_tx_ind_param
{
	unsigned short *dat_sub_frm;//������֡��ַ   yun�����²�����ĵ�ַָ��ͬһ���ڴ�
}Frm_Tx_Ind_Param;
typedef struct _arq_tx_succ_ind_param
{
	unsigned short succ_cnt;//�ɹ�����
}Arq_Tx_Succ_Ind_Param;

typedef struct _t_frm_tx_rsp_param
{
	unsigned short xlen;
}T_Frm_Tx_Rsp_Param;

typedef struct _t_frm_tx_cfm_param
{
	unsigned short id;//��frm_tx���,����Ӧ���սڵ�
	unsigned short succ_fail_flag;//�ɹ�ʧ�ܱ��1-�ɹ���0ʧ��
	unsigned short fail_type;//��ʧ��ʱ��Ч��ʧ�������ط����ʧ�ܣ������յ�arq���������շ��ص����÷���ʧ��
	unsigned short *net_frm;//����֡��ַ
	unsigned short da;
	unsigned short sa;
}T_Frm_Tx_Cfm_Param;

typedef struct _frm_tx
{
	void* entity;
	unsigned short state;

	unsigned short id;//��frm_tx���,����Ӧ���սڵ�

	unsigned short seg_num;//�ֶ�����
	unsigned short seg_idx;//���͵���λ��
	unsigned short seg_len[MAX_SEG_NUM_PER_NET_FRM];//���зֶε�ÿ�γ���
	unsigned short succ_seg_num;//���ͳɹ��Ķ��� 

	unsigned short *net_frm;//����֡��ַ
	unsigned short net_frm_len;//����֡����

	unsigned short bc_seg_num;//bc����ţ����ڹ㲥�����

	unsigned short da;//Ŀ�Ľڵ�
	unsigned short sa;//Դ�ڵ�
	
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
