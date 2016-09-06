#include "frm_tx.h"
#include <string.h>
#include <stdio.h>
enum {IDLE,TX};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun:����dlc_tx_dump����������ݷ�������  �������㲥
static int FrmTxReq(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Frm_Tx_Req_Param* param=(Frm_Tx_Req_Param*)sig->param;
	
	unsigned short tmp_seg,i,len;
	unsigned short tmp_rem;
	proc->net_frm = param->xdat; //yun:���ݵ�ַ
	proc->net_frm_len = param->xlen;  //yun:���ݳ���
	proc->seg_idx = 0;
	proc->succ_seg_num = 0;
	proc->da = ((((Net_Frm*)(proc->net_frm))->pri_sa_da_mul_sn)>>3) &0x1f; //Դ��ַ,Ŀ�ĵ�ַ,���
	proc->sa = ((((Net_Frm*)(proc->net_frm))->pri_sa_da_mul_sn)>>8) &0x1f;
	if( proc->net_frm_len  == 0)//������
	{
		return 0;
	}
	//����ֶγ���
	tmp_seg = proc->net_frm_len / DATA_SUB_FRM_PALOAD_LEN_1002; //61
	tmp_rem = proc->net_frm_len % DATA_SUB_FRM_PALOAD_LEN_1002;
	if(tmp_rem)
	{
		tmp_seg += 1;
	}
	//if(tmp_rem > (DATA_SUB_FRM_PALOAD_LEN_1002 - 15))//ÿ��MAC��ĩβʡ�ռ�����Я������
	//{
	//	tmp_seg += 1;
	//}
	proc->seg_num = tmp_seg;
	//����ֶγ���
	len = proc->net_frm_len / proc->seg_num;
	tmp_rem = proc->net_frm_len % proc->seg_num;
	for(i=0;i<tmp_seg;i++)  //yun:ƽ������
	{
		proc->seg_len[i] = len;
	}
	for(i=0;i<tmp_rem;i++)
	{
		proc->seg_len[i] +=1;  //yun��ǰtmp_rem������+1��
	}
	proc->state = TX;//��������״̬   yun���ְ��꣬�ı���TX״̬
	printf("frm_tx[%d]::FrmTxReq() �ְ���ϣ��ȴ�����\n",proc->id);
//	printdat_once[18]= proc->seg_num + 0x30;
	return 0;
}
//yun:�²�dlc_tx_ctrl���㲥������arq_tx��������������Ҫ����
static int FrmTxInd(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Frm_Tx_Ind_Param* param=(Frm_Tx_Ind_Param*)sig->param;
	
	unsigned short i;
	Data_Sub_Frm* data_sub_frm;
	unsigned short* data = proc->net_frm;  //yun:�Ѿ����������֡��ַ
	if(proc->state == IDLE)//�޴�������
	{
		sig = proc->frm_tx_rsp;
		((T_Frm_Tx_Rsp_Param*)sig->param)->xlen = 0;
		AddSignal(sig);
		printf("---�ڵ�[%d]---frm_tx[%d]::FrmTxInd() û������Ҫ��\n",((P_Entity*)proc->entity)->mib.local_id,proc->id);
	}
	else//TX״̬���д�������
	{
		data_sub_frm = (Data_Sub_Frm *)param->dat_sub_frm;
		data_sub_frm ->seg_len_sn_rst = 0;
		data_sub_frm ->seg_len_sn_rst |= (proc->seg_len[proc->seg_idx])<<5;
		//��֡ͷ�Ķα��
		if(proc->seg_idx == 0)//ͷ֡    yun��seg_idx;//���͵��ĸ�λ��
		{
			data_sub_frm ->seg_len_sn_rst |= FIRST_SEG_FLAG;  //0x4000
		}
		if(proc->seg_idx == proc->seg_num - 1)//β֡
		{
			data_sub_frm ->seg_len_sn_rst |= LAST_SEG_FLAG;   //0x2000
		}
		if(proc->id == ((P_Entity*)proc->entity)->mib.local_id)//�㲥�������
		{
			data_sub_frm ->seg_len_sn_rst |= (proc->bc_seg_num  << 1);
			proc->bc_seg_num ++;//bc����ţ����ڹ㲥�����
			proc->bc_seg_num &= 0xf;//yun:���15��,����ͬһ�������ݰ����Ҳ��ֱ���ۼӵ�
		}
		for(i=0;i<proc->seg_idx;i++)//���ݷ��͵���λ��
		{
			data += proc->seg_len[i];
		}
		for(i=0;i<DATA_SUB_FRM_PALOAD_LEN_1002;i++)//�����ݱ����봰��  yun���Ž���ַ��
		{
			if(i<proc->seg_len[proc->seg_idx])
			{
				data_sub_frm->data[i] = *(data++); //yun���������������ĵ�ַ��  ǰ��λ������֡ͷ
			}
			else
			{
				data_sub_frm->data[i] = 0;//yun�������Ϊ0
			}
		}
		sig = proc->frm_tx_rsp;//�ظ�����arq_tx/�㲥dlc_tx_ctrl
		((T_Frm_Tx_Rsp_Param*)sig->param)->xlen = proc->seg_len[proc->seg_idx];
		AddSignal(sig);
		printf("frm_tx[%d]::FrmTxInd() �����ݣ�����[%d]֡\n", proc->id, proc->seg_idx);
		proc->seg_idx ++;   //yun:���͵���λ��+1  ֡��λ��

		if(proc->seg_idx == proc->seg_num)//��������������arq_tx���ͽ���
		{
			proc->state = IDLE;
			printf("frm_tx[%d]::FrmTxInd() ����֡������\n",proc->id);
			printf("\n");
		}
		//�㲥������Ϊ���ͳɹ�   yun���㲥���ݷ���ȥ����Ϊ�ɹ���û��ackȷ��
		if(proc->id == ((P_Entity*)proc->entity)->mib.local_id)
		{
			printf("frm_tx::FrmTxInd() �㲥���ɹ����͵�[%d]��\n",proc->succ_seg_num);
			proc->succ_seg_num ++;//���ͳɹ���������
			if(proc->succ_seg_num == proc->seg_num)//����������ͳɹ�
			{
				sig = proc->frm_tx_cfm;
				((T_Frm_Tx_Cfm_Param*)sig->param)->id = proc->id;
				((T_Frm_Tx_Cfm_Param*)sig->param)->succ_fail_flag = 1;//�ɹ�
				((T_Frm_Tx_Cfm_Param*)sig->param)->fail_type = 0;//�ɹ�ʱ��Ч
				((T_Frm_Tx_Cfm_Param*)sig->param)->net_frm = proc->net_frm;
				((T_Frm_Tx_Cfm_Param*)sig->param)->da = proc->da;
				((T_Frm_Tx_Cfm_Param*)sig->param)->sa = proc->sa;
				AddSignal(sig);
				printf("frm_tx::FrmTxInd() ��������㲥������ϣ��ظ��ϲ�dlc_tx_dump\n");
			}
		}
		if((proc->id == ((P_Entity*)proc->entity)->mib.local_id) && (proc->state == IDLE)&&(proc->succ_seg_num != proc->seg_num))
		{
			//PrintDat(0,0,0xaaaa);
		}
	}
	return 0;
}
//yun:�����ͷ����ݰ�
static int FrmTxFreeInd(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Frm_Tx_Free_Ind_Param* param=(Frm_Tx_Free_Ind_Param*)sig->param;

	proc->seg_idx = 0;
	proc->seg_num = 0;
	proc->succ_seg_num = 0;

	sig = proc->frm_tx_cfm;
	((T_Frm_Tx_Cfm_Param*)sig->param)->id = proc->id;
	((T_Frm_Tx_Cfm_Param*)sig->param)->succ_fail_flag = 0;//ʧ��
	((T_Frm_Tx_Cfm_Param*)sig->param)->fail_type = param->fail_type ;//ʧ������
	((T_Frm_Tx_Cfm_Param*)sig->param)->net_frm = proc->net_frm;
	((T_Frm_Tx_Cfm_Param*)sig->param)->da = proc->da;
	((T_Frm_Tx_Cfm_Param*)sig->param)->sa = proc->sa;
	AddSignal(sig);
	proc->state = IDLE;
	return 0;
}
//yun:����֡���ͳɹ�������  �ɹ�����һ�ξͷ���һ�Σ���¼�ɹ�����  ���շ�����ack��
static int ArqTxSuccInd(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Arq_Tx_Succ_Ind_Param* param=(Arq_Tx_Succ_Ind_Param*)sig->param;
	printf("frm_tx[%d]:: ArqTxSuccInd �������ͳɹ�[%d]\n",proc->id,proc->succ_seg_num);
	proc->succ_seg_num += param->succ_cnt;//���ͳɹ���������
	
	if(proc->succ_seg_num == proc->seg_num)//����������ͳɹ�
	{
		sig = proc->frm_tx_cfm;
		((T_Frm_Tx_Cfm_Param*)sig->param)->id = proc->id;
		((T_Frm_Tx_Cfm_Param*)sig->param)->succ_fail_flag = 1;//�ɹ�
		((T_Frm_Tx_Cfm_Param*)sig->param)->fail_type = 0;//�ɹ�ʱ��Ч
		((T_Frm_Tx_Cfm_Param*)sig->param)->net_frm = proc->net_frm;
		((T_Frm_Tx_Cfm_Param*)sig->param)->da = proc->da;
		((T_Frm_Tx_Cfm_Param*)sig->param)->sa = proc->sa;
		AddSignal(sig);
		printf("frm_tx[%d]:: ArqTxSuccInd ���������������ϣ����ϲ㷢��ȷ��\n",proc->id);
	}else{
		//yun:���û�з����꣬����Ҫ���ݼ�������
		printf("frm_tx[%d]:: ArqTxSuccInd �����²�Ҫ����\n", proc->id);
		printf("\n");
	}
	
	return 0;
}

void FrmTxInit(Frm_Tx* proc)
{
	proc->state=IDLE;
	proc->seg_num = 0;
	proc->seg_idx = 0;
	proc->succ_seg_num = 0;
	memset(proc->seg_len,0,MAX_SEG_NUM_PER_NET_FRM);
	proc->net_frm = NULL;
	proc->net_frm_len = 0;
	proc->bc_seg_num = 0;
}
void FrmTxSetup(Frm_Tx* proc)
{
	proc->frm_tx_req.next=0;
	proc->frm_tx_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_tx_req.src=0;
	proc->frm_tx_req.dst=proc;
	proc->frm_tx_req.func=FrmTxReq;
	proc->frm_tx_req.pri=SDL_PRI_URG;
	proc->frm_tx_req.param=&proc->frm_tx_req_param;


	proc->frm_tx_ind.next=0;
	proc->frm_tx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_tx_ind.src=0;
	proc->frm_tx_ind.dst=proc;
	proc->frm_tx_ind.func=FrmTxInd;
	proc->frm_tx_ind.pri=SDL_PRI_URG;
	proc->frm_tx_ind.param=&proc->frm_tx_ind_param;

	proc->frm_tx_free_ind.next=0;
	proc->frm_tx_free_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_tx_free_ind.src=0;
	proc->frm_tx_free_ind.dst=proc;
	proc->frm_tx_free_ind.func=FrmTxFreeInd;
	proc->frm_tx_free_ind.pri=SDL_PRI_URG;
	proc->frm_tx_free_ind.param=&proc->frm_tx_free_ind_param;

	proc->arq_tx_succ_ind.next=0;
	proc->arq_tx_succ_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->arq_tx_succ_ind.src=0;
	proc->arq_tx_succ_ind.dst=proc;
	proc->arq_tx_succ_ind.func=ArqTxSuccInd;
	proc->arq_tx_succ_ind.pri=SDL_PRI_URG;
	proc->arq_tx_succ_ind.param=&proc->arq_tx_succ_ind_param;

}

