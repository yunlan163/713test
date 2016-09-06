#ifndef _ARQ_TX_H
#define _ARQ_TX_H
#include "..\..\..\sdl\sdl.h"
#include "..\..\..\common.h"
#include"..\..\..\mib\mib.h"
#include "..\..\..\..\mymem.h"


typedef struct _arq_ack_rx_ind_param
{
	unsigned short* ack_sgl;//ack����
}Arq_Ack_Rx_Ind_Param;
typedef struct _arq_rst_rx_ind_param
{
	unsigned short rsv;//Ԥ��
}Arq_Rst_Rx_Ind_Param;
typedef struct _arq_tx_ind_param
{
	unsigned short rsv;//Ԥ��
}Arq_Tx_Ind_Param;

typedef struct _frm_tx_rsp_param
{
	unsigned short xlen;
}Frm_Tx_Rsp_Param;

typedef struct _t_arq_tx_rsp_param
{
	unsigned short* xfrm;//mac֡��ַ
}T_Arq_Tx_Rsp_Param;
typedef struct _t_frm_tx_free_ind_param
{
	unsigned short type;//�����ط�ʧ��/�շ�����

}T_Frm_Tx_Free_Ind_Param;

typedef struct _t_frm_tx_ind_param
{
	unsigned short *dat_sub_frm;//������֡��ַ
}T_Frm_Tx_Ind_Param;
typedef struct _t_arq_tx_succ_ind_param
{
	unsigned short succ_cnt;//�ɹ�����
}T_Arq_Tx_Succ_Ind_Param;
typedef struct _arq_tx
{
	void* entity;
	unsigned short state;

	unsigned short id;//arq_txģ���ţ������ڽ��սڵ�

	unsigned short top;//����
	unsigned short btm;//����
	unsigned short idx;//���͵����ڵ�λ��
	unsigned short size;//���ڴ�С
	Mac_Frm_1002 *frm[ARQ_SN_SIZE];//��������   16
	unsigned short sts[ARQ_SN_SIZE];//����������ÿ������״̬��������ARQ_STS_IDLE��������δ���ɹ�ARQ_STS_TX���������ҷ��ɹ�ARQ_STS_SUCC,�����ݵ��ڵ�ack ARQ_STS_WAIT_ACK��
	unsigned short frm_len[ARQ_SN_SIZE];//����������ÿ�����ڵ�MAC֡����
	unsigned short tx_cnt[ARQ_SN_SIZE];//����������ÿ�����ڵ��ط���������

	unsigned short timer_wait_ack[ARQ_SN_SIZE];//�ȴ�ack��ʱ
	unsigned short rst_flag;//���ñ�־  ��һ����Ҫ����

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

