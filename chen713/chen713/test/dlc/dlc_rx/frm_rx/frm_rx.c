#include "frm_rx.h"
#include <stdio.h>
enum{IDLE,RX,WAIT_RSP};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun���յ�dlc_rx_ctrl�����Ĺ㲥���ݡ�  arq_rx�ĵ���
static int FrmRxInd(Signal* sig) 
{
	Frm_Rx *proc=(Frm_Rx*)sig->dst;  //yun:�޸���
	Frm_Rx_Ind_Param * param=(Frm_Rx_Ind_Param*)sig->param;
	
	unsigned short len, i;
	unsigned short* p_frm;
//	Net_Frm* net_frm = (Net_Frm*)param->xdat ;

	/*if(param->xdat[param->xlen-1] != param->xdat[param->xlen - 2])
	{
		sig=proc->ddat_rx_ind;//����dlc_rx_dump
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm = proc->net_frm;
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm_len = proc->net_frm_len;
		((T_Ddat_Rx_Ind_Param *)sig->param)->ta = proc->id;
		((T_Ddat_Rx_Ind_Param *)sig->param)->uc_bc_flag = proc->uc_bc_flag;
		AddSignal(sig);

	}*/


	switch(proc->state)
	{
	case IDLE://�����ڽ�������
		if((param->seg_flag & FIRST_SEG_FLAG) == FIRST_SEG_FLAG)//�׶�  seg_flag;//�α�ǣ���ʼ10������01���м�00����ʼ�ҽ���11
		{
		   	proc->net_frm = MemAllocNet();  //yun�����ٿռ�洢
			if(!proc->net_frm)  //yun:fail
			{
				sig=proc->frm_rx_rsp;
				AddSignal(sig);
				return 0;
			}
   			proc->dat_idx = proc->net_frm; //dat_idx:����֡���յ���λ��  proc->net_frmָ����յ������ݿ�ͷ
			proc->net_frm_len = 0;
			if(param->bc_uc_falg)//�㲥
			{
				proc->bc_seg_num = param ->bc_seg_num;//��¼���
				proc->bc_seg_num ++;
				proc->bc_seg_num &= 0xf;
			}
		}
		else//���׶�
		{
			printf("��ô���ܽ������\n");
			sig=proc->frm_rx_rsp;  //yun��֪ͨ�ͷ�
			AddSignal(sig);
			return 0;
		}
		break;
	case RX://���ڽ�������
		if((param->seg_flag & FIRST_SEG_FLAG) == FIRST_SEG_FLAG)//�׶�
		{
			MemFreeNet(proc->net_frm);
			proc->net_frm = NULL;
			proc->dat_idx =NULL;
		  	proc->net_frm = MemAllocNet();//���½���
			if(!proc->net_frm)
			{
				sig=proc->frm_rx_rsp;
				AddSignal(sig);
				return 0;
			}
			proc->dat_idx = proc->net_frm;
			proc->net_frm_len = 0;
			if(param->bc_uc_falg)//�㲥
			{
				proc->bc_seg_num = param ->bc_seg_num;//��¼���
				proc->bc_seg_num ++;
				proc->bc_seg_num &= 0xf;
			}
		}
		else//no�׶�
		{
			if(param->bc_uc_falg)//�㲥
			{
				if(proc->bc_seg_num == param->bc_seg_num)//��¼�������
				{
					proc->bc_seg_num ++;
					proc->bc_seg_num &= 0xf;
				}
				else//�㲥�����������˰�ʧ��
				{
					printf("�㲥�����ղ��������˰�ʧ�ܣ�proc->state=IDLE\n");
					MemFreeNet(proc->net_frm);
					proc->net_frm = NULL;
					proc->dat_idx =NULL;
					sig=proc->frm_rx_rsp;
					AddSignal(sig);
					proc->state=IDLE;
					return 0;
				}
			}
		}
		break;
	default://�����ܷ��� 
		break;
	}	
	
	len = param->xlen;
	p_frm = param->xdat;
	//memcpy(proc->dat_idx,p_frm,len);
	for(i = 0; i < len; i++)
	{
		proc->dat_idx[i] = p_frm[i];  //yun���ݴ�����
	}

	proc->dat_idx += len;
	proc->net_frm_len+= len;
	if((param->seg_flag & LAST_SEG_FLAG) == LAST_SEG_FLAG)//β�� ������
	{
		sig=proc->ddat_rx_ind;//����dlc_rx_dump
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm = proc->net_frm;//proc->dat_idx = proc->net_frm;
		((T_Ddat_Rx_Ind_Param *)sig->param)->net_frm_len = proc->net_frm_len;
		((T_Ddat_Rx_Ind_Param *)sig->param)->ta = proc->id;
		((T_Ddat_Rx_Ind_Param *)sig->param)->uc_bc_flag = proc->uc_bc_flag;
		AddSignal(sig);
		proc->state=WAIT_RSP;
		proc->net_frm = NULL;
		printf("frm_rx::FrmRxInd() ������һ�������ݣ��ϴ���dlc_rx_dump\n");
	}
	else//��β��
	{
		sig=proc->frm_rx_rsp; //yun���յ��ˣ��ͷŸղŵĽ�������ָ��
		AddSignal(sig);
		proc->state=RX;
		printf("frm_rx[%d]::FrmRxInd() �յ����ݣ��ظ��²��ͷ�\n",proc->id);
	}
	return 0;
}
//yun���յ��ϲ�dlc_rx_dump�Ļظ�
static int DdatRxRsp(Signal* sig) 
{
	Frm_Rx *proc=(Frm_Rx*)sig->dst;
	//Ddat_Rx_Rsp_Param * param=(Ddat_Rx_Rsp_Param*)sig->param;

	sig=proc->frm_rx_rsp;
	AddSignal(sig);
	proc->net_frm = NULL;
	proc->dat_idx =NULL;

	proc->state=IDLE;
	printf("frm_rx::DdatRxRsp() �յ�dlc_rx_dump�Ļظ���״̬��ΪIDLE\n");
	return 0;
}
//yun:�յ�arq_rx.c���ͷ�ָʾ ����ʱ
static int FrmRxFreeInd(Signal* sig)
{
	Frm_Rx *proc=(Frm_Rx*)sig->dst;
	//Frm_Rx_Free_Ind_Param * param=(Frm_Rx_Free_Ind_Param*)sig->param;

	if(proc->net_frm!=NULL)
	{
		MemFreeNet(proc->net_frm);
	}
	proc->dat_idx = NULL;
	proc->net_frm = NULL;
	proc->net_frm_len = 0;
	proc->state=IDLE;
	printf("frm_rx::FrmRxFreeInd() �յ�arq_rx.c�Ļ���ָʾ��״̬��ΪIDLE\n");
	return 0;

}
void FrmRxInit(Frm_Rx *proc)
{
	proc->state=IDLE;
	proc->net_frm = NULL;
	proc->net_frm_len = 0;
	proc->dat_idx = NULL;
}

void FrmRxSetup(Frm_Rx *proc)
{
	proc->frm_rx_ind.next=0;
	proc->frm_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_rx_ind.src=0;
	proc->frm_rx_ind.dst=proc;
	proc->frm_rx_ind.func=FrmRxInd;
	proc->frm_rx_ind.pri=SDL_PRI_URG;
	proc->frm_rx_ind.param=&proc->frm_rx_ind_param;

	proc->frm_rx_free_ind.next=0;
	proc->frm_rx_free_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_rx_free_ind.src=0;
	proc->frm_rx_free_ind.dst=proc;
	proc->frm_rx_free_ind.func=FrmRxFreeInd;
	proc->frm_rx_free_ind.pri=SDL_PRI_URG;
	proc->frm_rx_free_ind.param=&proc->frm_rx_free_ind_param;


	proc->ddat_rx_rsp.next=0;
	proc->ddat_rx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->ddat_rx_rsp.src=0;
	proc->ddat_rx_rsp.dst=proc;
	proc->ddat_rx_rsp.func=DdatRxRsp;
	proc->ddat_rx_rsp.pri=SDL_PRI_URG;
	proc->ddat_rx_rsp.param=&proc->ddat_rx_rsp_param;
}
