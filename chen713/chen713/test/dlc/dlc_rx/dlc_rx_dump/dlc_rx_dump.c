#include "dlc_rx_dump.h"
#include <string.h>
#include <stdio.h>
enum{IDLE,WAIT_RSP};

typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;

//д����
static void QueWr(Que *que,unsigned short *dat,unsigned short len,unsigned short ta)
{
	que->que_dat[que->top].dat = dat;
	que->que_dat[que->top].len = len;
	que->que_dat[que->top].ta = ta;
	que->top++;
	que->top %= DLC_QUE_MAX_NUM;
	que->size++;
	if(que->size > DLC_QUE_MAX_NUM)//�����������100��,�����������Ǿ�����
	{
		que->size = DLC_QUE_MAX_NUM;
		que->btm = que->top;
	}
	return ;
}
//������
static int QueRd(Que *que,unsigned short **dat,unsigned short *len,unsigned short *ta)
{
	if(que->size>0)//�������
	{
		*dat = que->que_dat[que->btm].dat;
		*len = que->que_dat[que->btm].len;
		*ta = que->que_dat[que->btm].ta;
		que->btm ++;
		que->btm %= DLC_QUE_MAX_NUM;
		que->size--;
		return 1;
	}
	else//�������
	{
		return 0;
	}
}
//yun���ϲ�������Ļظ��ź�
static int NRxRsp(Signal* sig)
{
	Dlc_Rx_Dump * proc=(Dlc_Rx_Dump*)sig->dst;
//	Nrx_Rsp_Param * param=(Nrx_Rsp_Param *)sig->param;

	int sts;
	unsigned short *dat;
	unsigned short len;
	unsigned short ta;
	sts=QueRd(&proc->que,&dat,&len,&ta);//������   �ظ�1.�����гɹ�
	if(sts==1)//�������
	{
		sig=proc->nrx_ind;  //yun:�����ϴ�
		((T_NRx_Ind_Param *)sig->param)->net_frm = dat;
		((T_NRx_Ind_Param *)sig->param)->net_frm_len = len;
		((T_NRx_Ind_Param *)sig->param)->ta = ta;
		AddSignal(sig);
		printf("dlc_rx_dump::NRxRsp() �յ��ϲ�nrx�Ļظ��źţ���ѯ���У������ݼ����ϴ�\n");
		//NrxInd(dat,len,ta);//for test
	}
	else//�������
	{
		proc->state=IDLE;
		printf("dlc_rx_dump::NRxRsp() �յ��ϲ�nrx�Ļظ��źţ���ѯ���У���ϧû������������ˡ�����\n");
	}
	return 0;
}
//yun������frm_rx���ϴ������ź�  �㲥����һ������������ϴ�
static int DdatRxInd(Signal* sig)
{
	Dlc_Rx_Dump* proc=(Dlc_Rx_Dump*)sig->dst;
	Ddat_Rx_Ind_Param * param=(Ddat_Rx_Ind_Param *)sig->param;

	switch(proc->state)  //yun��ֻ��һ���������˺���ܷ�����һ�����ݣ������ж�״̬
	{
	case IDLE://�������һ��������ɣ����Լ�������������
		sig=proc->nrx_ind;//�������������
		((T_NRx_Ind_Param *)sig->param)->net_frm = param->net_frm;
		((T_NRx_Ind_Param *)sig->param)->net_frm_len = param->net_frm_len;
		((T_NRx_Ind_Param *)sig->param)->ta  = param->ta;
		AddSignal(sig);
		printf("dlc_rx_dump::DdatRxInd() �յ���������ϴ���nrx\n");
		//NrxInd(param->net_frm,param->net_frm_len,param->ta);//for test
		break;
	case WAIT_RSP:
		QueWr(&proc->que,param->net_frm,param->net_frm_len,param->ta); //yun���Ž����еȴ�
		break;
	default:
		break;
	}
	sig=proc->ddat_rx_rsp[param->uc_bc_flag][param->ta];//�ظ��ֶ�ģ��
	AddSignal(sig);
	proc->state=WAIT_RSP;  //yun:���ǵ��²㣬���ǵ��������ظ�
	return 0;
}

void DlcRxDumpInit(Dlc_Rx_Dump* proc)
{
   	proc->state=IDLE;
	memset(&proc->que,0,sizeof(Que));
}
void DlcRxDumpSetup(Dlc_Rx_Dump* proc)
{
	unsigned short i;

	proc->nrx_rsp.next=0;
	proc->nrx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->nrx_rsp.src=0;
	proc->nrx_rsp.dst=proc;
	proc->nrx_rsp.func=NRxRsp;
	proc->nrx_rsp.pri=SDL_PRI_URG;
	proc->nrx_rsp.param=&proc->nrx_rsp_param;

	for(i=0;i<NODE_MAX_CNT;i++)
	{

		proc->ddat_rx_ind[0][i].next=0;
		proc->ddat_rx_ind[0][i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->ddat_rx_ind[0][i].src=0;
		proc->ddat_rx_ind[0][i].dst=proc;
		proc->ddat_rx_ind[0][i].func=DdatRxInd;
		proc->ddat_rx_ind[0][i].pri=SDL_PRI_URG;
		proc->ddat_rx_ind[0][i].param=&proc->ddat_rx_ind_param[0][i];

		proc->ddat_rx_ind[1][i].next=0;
		proc->ddat_rx_ind[1][i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->ddat_rx_ind[1][i].src=0;
		proc->ddat_rx_ind[1][i].dst=proc;
		proc->ddat_rx_ind[1][i].func=DdatRxInd;
		proc->ddat_rx_ind[1][i].pri=SDL_PRI_URG;
		proc->ddat_rx_ind[1][i].param=&proc->ddat_rx_ind_param[1][i];
	}

}

