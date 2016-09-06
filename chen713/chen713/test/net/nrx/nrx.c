#include "nrx.h"
#include <stdio.h>
enum {IDLE,WAIT_RSP};

typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun:dlc�������ϴ�����  ��ͨ����or��Ƶ����
static int NRxInd(Signal* sig)
{
	NRx *proc=(NRx*)sig->dst;
	NRx_Ind_Param * param=(NRx_Ind_Param*)sig->param;
	
	unsigned short *dat,sa,da,sn,pri,tmp_da,mul_flag;
	unsigned short test_nu;  //yun: for test yun
	unsigned short len,uc_bc_flag;

	Net_Frm *net_frm = (Net_Frm *)param->xdat;//����֡��ַ
	len = param->xlen - NET_FRM_HEAD_LEN;//���ɳ���
	dat = net_frm->net_paload;//���ɵ�ַ

	test_nu = len/2;  //for test yun
	sa = ((net_frm->pri_sa_da_mul_sn >> 8) & 0x1f);//Դ��ַ
	da = (net_frm->pri_sa_da_mul_sn >>3)  & 0x1f;//Ŀ�ĵ�ַ
	sn = ((net_frm->pri_sa_da_mul_sn  & 0x3) << 6)|((net_frm->sn_len >> 10)) ; //yun��������źϲ�����������һ����������
	printf("-------�ڵ�[%d]------nrx::NRxInd() �յ���[%d]��\n", ((P_Entity*)proc->entity)->mib.local_id, sn);
	pri = net_frm->pri_sa_da_mul_sn >> 13 ;
	mul_flag = (net_frm->pri_sa_da_mul_sn >>2)  & 0x1;//�ಥ���
	tmp_da = da;
	if(mul_flag)//�ಥ
	{
		tmp_da = sa;
		//���������������
		if((sa == ((P_Entity*)proc->entity)->mib.local_id)||(!(((sn > proc->nrx_sn_mul[sa]) && ((sn - proc->nrx_sn_mul[sa]) < 128)) || ((sn < proc->nrx_sn_mul[sa]) && ((proc->nrx_sn_mul[sa] - sn)>128)))))
		{
			printf("nrx::NRxInd() �ɵ�������������ܣ��ͷ��²�����\n");
			MemFreeNet(param->xdat);//�ͷ��²�����
			sig = proc->nrx_rsp;//�ظ��²�
			AddSignal(sig);
			proc->state = IDLE;
			if((!(((sn > proc->nrx_sn_mul[sa]) && ((sn - proc->nrx_sn_mul[sa]) < 128)) || ((sn < proc->nrx_sn_mul[sa]) && ((proc->nrx_sn_mul[sa] - sn)>128)))))
			{
				if(sn != proc->nrx_sn_mul[sa])
				{
					//printf("debug");
				}
			}
			return 0;
		}
		else//������������
		{
			proc->nrx_sn_mul[sa] = sn;
		}
	}
	else//�������㲥  ������������
	{
		if(sn == proc->nrx_sn[sa][tmp_da])//�ظ����������
		{
			printf("nrx::NRxInd()�ظ�������������ͷ��²�����\n");
			MemFreeNet(param->xdat);//�ͷ��²�����
			sig = proc->nrx_rsp;//�ظ��²�
			AddSignal(sig);
			proc->state = IDLE;
			return 0;
		}
		else//������������
		{
			proc->nrx_sn[sa][tmp_da] = sn;
		}
	}
	
	if(mul_flag)//�ಥ
	{
		uc_bc_flag = MC_NET_FRM_FLAG;
		//AppRxInd(dat,len,sa,uc_bc_flag,pri);//for test ���ϲ㽻����
		proc->dat_wait_free = param->xdat;//������ͷ����ݵ�ַ
		sig = proc->drelay_chk_req;  //yun����rt_ctrl ·�ɿ���  �鿴�Ƿ���Ҫת��
		((T_DRelay_Chk_Req_Param*)sig->param)->sa = sa;
		AddSignal(sig);
	}
	else if((da != ((P_Entity*)proc->entity)->mib.local_id) && (da != sa))//����ת��  //���ǹ㲥��Ҳ���ǵ��������ڵ�
	{
		sig=proc->relay_tx_ind; //yun����ntx �м�
		((T_Relay_Tx_Ind_Param*)sig->param)->xdat=param->xdat;
		((T_Relay_Tx_Ind_Param*)sig->param)->xlen=param->xlen-NET_FRM_HEAD_LEN;
		((T_Relay_Tx_Ind_Param*)sig->param)->xda=da;
		((T_Relay_Tx_Ind_Param*)sig->param)->xpri= pri;
		AddSignal(sig);

		sig=proc->nrx_rsp;  //yun:���²�ظ�
		AddSignal(sig);	
		proc->state = IDLE;
	}
	else//�����ա��㲥
	{	
		if(da == sa)//�㲥
		{
			uc_bc_flag = BC_NET_FRM_FLAG;
		}		
		else //����
		{
			uc_bc_flag = UC_NET_FRM_FLAG;
		}
		//sig=proc->data_rx_ind;//���ϲ㽻����
		//((T_Data_Rx_Ind_Param*)sig->param)->dat=dat;
		//((T_Data_Rx_Ind_Param*)sig->param)->len=len;
		//((T_Data_Rx_Ind_Param*)sig->param)->sa=sa;
		//AddSignal(sig);
		//AppRxInd(dat,len,sa,uc_bc_flag,pri);//for test ���ϲ㽻����

		//for test yun
		if(1==uc_bc_flag){
			printf("---�ڵ�[%d]---nrx::NRxInd() �յ���sn=%d���㲥�����, \n���������Ϊ��",((P_Entity*)proc->entity)->mib.local_id,sn);
			while(test_nu--){
				printf("%d,",*(dat++));
			}
			printf("\n");
		}else if(2==uc_bc_flag){
			printf("nrx::NRxInd() �յ���sn=%d���ಥ�����\n",sn);
		}else{
			printf("---�ڵ�[%d]---nrx::NRxInd() �յ�[%d]�����ĵ�sn=%d�����������, \n���������Ϊ��",((P_Entity*)proc->entity)->mib.local_id, sa ,sn);
			while(test_nu--){
				printf("%d,",*(dat++));
			}
			printf("\n");
		}

		proc->dat_wait_free = param->xdat;//������ͷ����ݵ�ַ
		proc->state = WAIT_RSP;
		//�ظ���ģ�������ɣ����ݱ�����AppRxInd�п������
		sig = &proc->data_rx_rsp;  //yun:���Լ���ģ�飬�ͷ����ݡ����²�dlc�ظ�
		AddSignal(sig);		
	}
	
	return 0;
}
//yun:�ಥת����ѯ�ظ�cfm   ��ntxת��or�ͷ�ָʾ
static int DRelayChkCfm(Signal* sig)
{
	NRx *proc=(NRx*)sig->dst;
	DRelay_Chk_Cfm_Param * param=(DRelay_Chk_Cfm_Param*)sig->param; 

	unsigned short pri,len;
	Net_Frm *net_frm ;
	if(param->succ_flag)//��Ҫת��   //1-�м̣�0-���м�
	{
		net_frm = (Net_Frm *)(proc->dat_wait_free);//����֡��ַ  //yun:��ȡ����
		len = (net_frm->sn_len) & 0x3ff;//���ɳ���
		//sa = ((net_frm->pri_sa_da_mul_sn >> 8) & 0x1f);//Դ��ַ
		pri = net_frm->pri_sa_da_mul_sn >> 13 ;
		sig=proc->relay_tx_ind;   //yun:�ಥת��  ��ntx::RelayTxInd()
		((T_Relay_Tx_Ind_Param*)sig->param)->xdat=(unsigned short*)net_frm;
		((T_Relay_Tx_Ind_Param*)sig->param)->xlen=len;
		((T_Relay_Tx_Ind_Param*)sig->param)->xda=MULTICAST_DA;
		((T_Relay_Tx_Ind_Param*)sig->param)->xpri= pri;
		AddSignal(sig);
	}
	else//��ת��
	{
		printf("nrx::DRelayChkCfm()�ಥ��ѯ��ת�����ͷ��²�����\n");
		MemFreeNet(proc->dat_wait_free);//�ͷ�����
		proc->dat_wait_free = NULL;
	}
	sig=proc->nrx_rsp;
	AddSignal(sig);	
	proc->state = IDLE;
	return 0;
}
//yun����ģ����Լ���  �յ��������㲥��������ͷ����ݣ����²�dlc_rx�ظ���
static int DataRxRsp(Signal* sig)
{
	NRx *proc=(NRx*)sig->dst;
	//Data_Rx_Rsp_Param * param=(Data_Rx_Rsp_Param*)sig->param;
	printf("nrx::DataRxRsp() nrx�����ȷ���յ����ͷ�����\n");
	MemFreeNet(proc->dat_wait_free);//�ͷ�����  //yun:ָ���ڴ��ϵ������ͷ�
	proc->dat_wait_free = NULL;   //yun��ָ��ָ��NULL
	sig = proc->nrx_rsp;
	AddSignal(sig);
	printf("nrx::DataRxRsp() �ظ��²�\n");
	proc->state = IDLE;
	return 0;
}

void NRxInit(NRx *proc)
{
	unsigned short i,j;
	proc->state=IDLE;
	proc->dat_wait_free = NULL;
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->nrx_sn_mul[i] = 0;
		for(j=0;j<NODE_MAX_CNT;j++)
		{
			proc->nrx_sn[i][j] = 0;
		}
	}

}

void NRxSetup(NRx *proc)
{
	proc->data_rx_rsp.next=0;
	proc->data_rx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->data_rx_rsp.src=0;
	proc->data_rx_rsp.dst=proc;
	proc->data_rx_rsp.func=DataRxRsp;
	proc->data_rx_rsp.pri=SDL_PRI_URG;

	proc->nrx_ind.next=0;
	proc->nrx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->nrx_ind.src=0;
	proc->nrx_ind.dst=proc;
	proc->nrx_ind.func=NRxInd;
	proc->nrx_ind.pri=SDL_PRI_URG;
	proc->nrx_ind.param=&proc->nrx_ind_param;

	proc->drelay_chk_cfm.next=0;
	proc->drelay_chk_cfm.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->drelay_chk_cfm.src=0;
	proc->drelay_chk_cfm.dst=proc;
	proc->drelay_chk_cfm.func=DRelayChkCfm;
	proc->drelay_chk_cfm.pri=SDL_PRI_URG;
	proc->drelay_chk_cfm.param=&proc->drelay_chk_cfm_param;
}

