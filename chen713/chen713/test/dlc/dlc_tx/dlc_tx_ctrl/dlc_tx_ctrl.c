#include "dlc_tx_ctrl.h"
#include <stdio.h>

enum {IDLE,WAIT_RSP};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun:�յ�ack��������Ӧ��arqģ�顣
static int AckRxInd(Signal *sig)//�յ�ack����
{
	Dlc_Tx_Ctrl* proc=(Dlc_Tx_Ctrl*)sig->dst;
	Ack_Rx_Ind_Param* param=(Ack_Rx_Ind_Param*)sig->param;

	sig = proc->arq_ack_rx_ind[param->id];
	((T_Arq_Ack_Rx_Ind_Param *)sig->param)->ack_sgl = param->ack_sgl;
	AddSignal(sig);
	printf("---�ڵ�[%d]---dlc_tx_ctrl::AckRxInd() �յ�ack������arq_tx[%d]\n",((P_Entity*)proc->entity)->mib.local_id,param->id);
	return 0;
}
//yun:�յ�����
static int RstRxInd(Signal *sig)//�յ�arq��������շ�����
{
	Dlc_Tx_Ctrl* proc=(Dlc_Tx_Ctrl*)sig->dst;
	Rst_Rx_Ind_Param* param=(Rst_Rx_Ind_Param*)sig->param;

	sig = proc->arq_rst_rx_ind[param->id];
	AddSignal(sig);

	return 0;
}
//yun:mac�㷢�� Ҫ����
static int DlcTxInd(Signal *sig)
{
	Dlc_Tx_Ctrl* proc=(Dlc_Tx_Ctrl*)sig->dst;
		
	sig = proc->frm_tx_ind; //yun���ȹ㲥
	
	((T_Bc_Frm_Tx_Ind_Param*)sig->param)->dat_sub_frm = (unsigned short *)(&proc->bc_mac_frm);
	AddSignal(sig);
	proc->state = WAIT_RSP;
	printf("---�ڵ�[%d]---dlc_tx_ctrl::DlcTxInd() ���ϲ�Ҫ����\n", ((P_Entity*)proc->entity)->mib.local_id);
	return 0;
}
//yun���㲥 ������ �ظ�����Ӧ
static int FrmTxRsp(Signal *sig)
{
	Dlc_Tx_Ctrl* proc=(Dlc_Tx_Ctrl*)sig->dst;
	Bc_Frm_Tx_Rsp_Param* param=(Bc_Frm_Tx_Rsp_Param*)sig->param;  //yun:ֻ�����ݳ���
	unsigned short id = ((P_Entity*)proc->entity)->mib.local_id;
	if(param->xlen)//������   yun��û�й㲥��
	{	
		//��MAC֡
		proc->bc_mac_frm.common_head = (id << 11);
		proc->bc_mac_frm.common_head |= (id <<6) ;
		//DlcTxRsp(((unsigned short *)&proc->bc_mac_frm));//for test
		proc->state=IDLE;
		//for test yun  ���� �շ� ֱ����������
		sig = proc->dlc_rx_ind;
		((T_Dlc_Rx_Ind_Param*)sig->param)->frm=(unsigned short*)&proc->bc_mac_frm;
		((T_Dlc_Rx_Ind_Param*)sig->param)->len=63;
		AddSignal(sig);
		printf("---�ڵ�[%d]---dlc_tx_ctrl::FrmTxRsp() ͨ��mac���͹㲥����\n\n" , ((P_Entity*)proc->entity)->mib.local_id);
	}
	else   //yun:û������  �㲥��� �򵥲�Ҫ����
	{
		if(proc->last_tx_id == ((P_Entity*)proc->entity)->mib.local_id)   //yun����ȥ���ڵ� ���ڵ��ǹ㲥
		{
			proc->last_tx_id ++;
			proc->last_tx_id %= NODE_MAX_CNT;
		}
		proc->arq_tx_req_id = proc->last_tx_id; //yun: ���ϴεĵ���Ҫ���� ��ʼ��0��ʼ��
		sig  = proc->arq_tx_ind[proc->arq_tx_req_id];
		AddSignal(sig);
		proc->state = WAIT_RSP;
		printf("dlc_tx_ctrl::FrmTxRsp() �򵥲�[%d]Ҫ����\n",proc->arq_tx_req_id);
	}
	return 0;
}
//yun������arq_tx  ���������� Ҫͨ��mac�㷢��ȥ
static int ArqTxRsp(Signal *sig)
{
	Dlc_Tx_Ctrl* proc=(Dlc_Tx_Ctrl*)sig->dst;
	Arq_Tx_Rsp_Param* param=(Arq_Tx_Rsp_Param*)sig->param;

	if(param->xfrm==NULL)//������ yun:����һ���ڵ㷢��arq_tx_ind
	{
		
		proc->arq_tx_req_id = (proc->arq_tx_req_id + 1) % NODE_MAX_CNT;//Ҫ���ݵ�arq_tx���
		if(proc->arq_tx_req_id == ((P_Entity*)proc->entity)->mib.local_id)
		{
			proc->arq_tx_req_id ++;
			proc->arq_tx_req_id %= NODE_MAX_CNT;
		}
		if(proc->arq_tx_req_id == proc->last_tx_id )
		{
			proc->last_tx_id = (proc->last_tx_id + 1) % NODE_MAX_CNT;//Ҫ���ݵ�arq_tx���
			//DlcTxRsp(NULL);//for test
			proc->state=IDLE;
		}
		else
		{
			sig =proc->arq_tx_ind[proc->arq_tx_req_id];
			AddSignal(sig);
			printf("dlc_tx_ctrl::ArqTxRsp() �������ˣ�����һ��[%d]Ҫ����\n",proc->arq_tx_req_id);
		}
	}
	else//������
	{
		proc->last_tx_id = proc->arq_tx_req_id;
		//DlcTxRsp(param->xfrm);//for test
		proc->state=IDLE;
		//for test
		sig = proc->dlc_rx_ind;
		((T_Dlc_Rx_Ind_Param*)sig->param)->frm=param->xfrm;
		((T_Dlc_Rx_Ind_Param*)sig->param)->len=63;
		AddSignal(sig);
		printf("---�ڵ�[%d]---dlc_tx_ctrl::ArqTxRsp() �յ����ݣ�ͨ��mac����ȥ\n",((P_Entity*)proc->entity)->mib.local_id);
	}
	return 0;
}
void DlcTxCtrlSetup(Dlc_Tx_Ctrl* proc)
{
	int i;
	proc->ack_rx_ind.next=0;
	proc->ack_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->ack_rx_ind.src=0;
	proc->ack_rx_ind.dst=proc;
	proc->ack_rx_ind.func=AckRxInd;
	proc->ack_rx_ind.pri=SDL_PRI_URG;
	proc->ack_rx_ind.param=&proc->ack_rx_ind_param;

	proc->dlc_tx_ind.next=0;
	proc->dlc_tx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->dlc_tx_ind.src=0;
	proc->dlc_tx_ind.dst=proc;
	proc->dlc_tx_ind.func=DlcTxInd;
	proc->dlc_tx_ind.pri=SDL_PRI_URG;
	proc->dlc_tx_ind.param=&proc->dlc_tx_ind_param;

	proc->rst_rx_ind.next=0;
	proc->rst_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->rst_rx_ind.src=0;
	proc->rst_rx_ind.dst=proc;
	proc->rst_rx_ind.func=RstRxInd;
	proc->rst_rx_ind.pri=SDL_PRI_URG;
	proc->rst_rx_ind.param=&proc->rst_rx_ind_param;

	proc->frm_tx_rsp.next=0;
	proc->frm_tx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_tx_rsp.src=0;
	proc->frm_tx_rsp.dst=proc;
	proc->frm_tx_rsp.func=FrmTxRsp;
	proc->frm_tx_rsp.pri=SDL_PRI_URG;
	proc->frm_tx_rsp.param=&proc->frm_tx_rsp_param;

	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->arq_tx_rsp[i].next=0;
		proc->arq_tx_rsp[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->arq_tx_rsp[i].src=0;
		proc->arq_tx_rsp[i].dst=proc;
		proc->arq_tx_rsp[i].func=ArqTxRsp;
		proc->arq_tx_rsp[i].pri=SDL_PRI_URG;
		proc->arq_tx_rsp[i].param=&proc->arq_tx_rsp_param[i];
	}
}

void DlcTxCtrlInit(Dlc_Tx_Ctrl* proc)
{
	proc->state=IDLE;
	proc->arq_tx_req_id = 0;
	proc->last_tx_id = 0;
}
