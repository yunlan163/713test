#ifndef _NTX_H
#define _NTX_H
#include "..\..\sdl\sdl.h"
#include "..\..\common.h"
#include "..\..\mib\mib.h"
#include "..\..\..\mymem.h"
//#include "..\..\..\..\fm_mem.h"

#define NET_QUE_MAX_NUM 20//������г���
#define NET_PKT_MAX_TTL 100//���������ʱ��
#define NET_PKT_RESEND_FROM_FM_TMR  100//������������ж�������ʱ��
#define RT_FAIL_TMR 1000//·��ʧ��ʱ�䶨ʱ��������������е�����
typedef struct _data_tx_req_param
{
	unsigned short *xdat;//Ӧ�ò㾻����
	unsigned short xlen;//xdat�ĳ���
	unsigned short xda;//Ŀ�ĵ�ַ
	unsigned short xpri;//���ȼ�
	unsigned short hello_flag;//Hello����־
}Data_Tx_Req_Param;
typedef struct _ntx_ind_param
{
	unsigned short* dat;
	unsigned short ra;  //���յ�ַ
	unsigned short succ_flag;//�ɹ����
	unsigned short fail_type;//ʧ������
	unsigned short da;  //Ŀ�ĵ�ַ
	unsigned short sa;    //Դ��ַ
}NTx_Ind_Param;

typedef struct _rt_tx_rsp_param
{
	unsigned short succ;//�Ƿ���·��
	unsigned short ra;//��һ����ַ
	unsigned short da;//Ŀ�ĵ�ַ
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
	unsigned short xlen;//�˳���Ϊ����������ĳ��ȣ�������������ɺͰ�ͷ

}T_NTx_Req_Param;
typedef struct _t_rt_tx_ind_param
{
	unsigned short da;
}T_Rt_Tx_Ind_Param;
typedef struct _t_link_fail_ind_param
{
	unsigned short ra;//��һ����ַ

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
	unsigned short curt_snd_flag;//���ڷ��ͱ��
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

	unsigned short * dat;//�����������ַ������·��ʱ��ʱ����
	unsigned short len;//������еľ��ɳ��ȣ�����·��ʱ��ʱ����
	unsigned short da;//Ŀ�ĵ�ַ������·��ʱ��ʱ����
	unsigned long  arrvial_time;//���������ʱ�䣬����·��ʱ��ʱ����

	unsigned long sts;//32���ڵ��״̬��1bit
	unsigned short ntx_sn[NODE_MAX_CNT];//�����ڵ�����֡���  ��ʼ��=1
	unsigned short ntx_sn_mul;//�ಥ���  ��ʼ��=1
	
	unsigned short curt_node;
	unsigned long ntx_local_time;

	unsigned short data_tx_req_rt_tx_ind_flag;//�ϲ������ݣ�����·�ɱ��

	Net_Que_List net_que_list[NODE_MAX_CNT];//�������ݰ�����

	Net_Que_List_FM net_que_list_fm;
	unsigned short read_net_pkt_from_fm[NET_FRM_LEN_MAX];//��������ݶ����ݴ�
	unsigned short rt_fail_tmr[NODE_MAX_CNT];//·��ʧ��ʱ�䶨ʱ

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
