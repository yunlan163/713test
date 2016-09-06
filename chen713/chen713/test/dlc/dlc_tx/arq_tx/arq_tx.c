#include "arq_tx.h"
#include <stdio.h>
enum {IDLE,WAIT_RSP};
enum {ARQ_STS_IDLE,ARQ_STS_TX,ARQ_STS_SUCC,ARQ_STS_WAIT_ACK};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun:�ȴ�ack��ʱ����
void TimerWaitAck(Arq_Tx* proc)//�ȴ�ack��ʱ��
{
	unsigned short btm,i;
	for(i=0;i<proc->size;i++)//��ѯ���д���
	{
		btm = proc->btm + i;
		btm %= ARQ_SN_SIZE;
		if(proc->sts[btm] == ARQ_STS_WAIT_ACK)
		{
			if(proc->timer_wait_ack[btm])
			{
				proc->timer_wait_ack[btm]--;
				if(!proc->timer_wait_ack[btm])
				{
					proc->sts[btm] = ARQ_STS_TX;
				}
			}
		}
	}
}
//yun:�Ӻ���  arq_tx_rsp���͵���
void ArqTxChkWinProc(Arq_Tx* proc)
{
	unsigned short i,j,btm;
	Signal* sig;
	Data_Sub_Frm *data_sub_frm;
	
	for(i=0;i<proc->size;i++)//��ѯ���д���
	{
		if(!proc->tx_cnt[proc->idx])//����ظ�ʧ��  yun��ĳ�������ش�����  ����ش�20.������=0
		{
			sig = proc->frm_tx_free_ind;
			((T_Frm_Tx_Free_Ind_Param *)sig->param)->type = SEND_FAIL_TYPE_RESEND;
			AddSignal(sig);

			sig = proc->arq_tx_rsp;
			((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm = NULL;
			AddSignal(sig);

			btm = proc->btm;
			for(j=0;j<proc->size;j++)  //yun:ȫ���ͷţ���
			{
				if(proc->sts[btm] != ARQ_STS_IDLE )
				{
					MemFreeMac((unsigned short *)proc->frm[btm]);
					proc->frm[btm] = NULL;
				}
				btm++;
				btm %= ARQ_SN_SIZE;
			}
			proc->top = 0;
			proc->btm = 0;
			proc->idx = 0;
			proc->size = 0;
			proc->rst_flag = 1;
			proc->state = IDLE;
			for(i=0;i<ARQ_SN_SIZE;i++)
			{
				proc->sts[i] = ARQ_STS_IDLE;
			}
			return ;
		}
		else
		{
			if(proc->sts[proc->idx] == ARQ_STS_TX)//������
			{				
				data_sub_frm =(Data_Sub_Frm *) (proc->frm[proc->idx]);
				if(proc->rst_flag)//���ڸո����ã���һ��DlcTxInd�У���arq_txģ���һ�η�������
				{
					data_sub_frm->seg_len_sn_rst |= 0x0001;//arq���ñ��  yun����һ��Ҫ��λ
				}
				else
				{
					data_sub_frm->seg_len_sn_rst &= 0xfffe;//arq���ñ��   yun������
				}
				sig = proc->arq_tx_rsp;  //yun:������
				((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm =(unsigned short *) proc->frm[proc->idx];
				AddSignal(sig);
				printf("arq_tx[%d]::ArqTxChkWinProc() ����[%d]֡��dlc_tx_ctrl\n", proc->id, proc->idx);
				proc->sts[proc->idx] = ARQ_STS_WAIT_ACK; //yun:���ͳ�ȥ���ı�״̬
				proc->timer_wait_ack[proc->idx] = 0;//�ȴ�ACK��ʱ
				proc->tx_cnt[proc->idx]--;//�ط�������¼
				proc->idx++;
				proc->idx %= ARQ_SN_SIZE;
				if(proc->idx == proc->top)   //yun�����ﴰ��8
				{
					proc->idx = proc->btm;
				}
				proc->state =IDLE;
				return  ;  //yun����һ���ڷ���ȥ�ˣ�����
			}
			else//������һ������
			{
				proc->idx++;
				proc->idx %= ARQ_SN_SIZE;
				if(proc->idx == proc->top)
				{
					proc->idx = proc->btm;
				}
			}

		}
	}
	if(i == proc->size)//�����޴˳�������
	{
		sig=proc->arq_tx_rsp;
		((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm = NULL;
		AddSignal(sig);

		proc->state =IDLE;
		return ;
	}
}
//yun:����dlc_tx_ctrl ��������
static int ArqTxInd(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
	unsigned short i;
	if(proc->rst_flag)   //���ñ�־  �����ڵ��һ�ν��е���ͨ��ʱ��Ҫ�����ñ�־���յ���һ�ε�ack�ظ�������
	{
		for(i=0;i<proc->size;i++)//��ѯ���д���
		{
			if(proc->sts[i] == ARQ_STS_WAIT_ACK)   //yun:�ȴ�ack  ����
			{
				sig=proc->arq_tx_rsp;
				((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm = NULL;  //mac֡��ַ
				AddSignal(sig);

				proc->state =IDLE;
				return 0;
			}
		}
	}

	if(proc->size < ARQ_WIN_SIZE)//����<8����frm_txҪ����
	{
		proc->frm[proc->top] = (Mac_Frm_1002 *)MemAllocMac();
		if(!proc->frm[proc->top])  //yun:û�����뵽�ڴ���
		{
			if(proc->size)//�������д�������
			{
				ArqTxChkWinProc(proc);   //arq_tx_rsp
			}
			else//�������޴�������
			{
				sig = proc->arq_tx_rsp;
				((T_Arq_Tx_Rsp_Param*)sig->param)->xfrm = NULL;
				AddSignal(sig);
			}
			return 0; //yun:
		}
		sig = proc->frm_tx_ind;  //yun:Ҫ����
		//yun:Ҫ���ݣ��ŵ����뵽���ڴ��ַ��
		((T_Frm_Tx_Ind_Param*)sig->param)->dat_sub_frm = (unsigned short *)(proc->frm[proc->top]);
		AddSignal(sig);
		proc->state = WAIT_RSP;
		printf("arq_tx[%d]::ArqTxInd() ����<8����frm_txҪ����\n",proc->id);
	}
	else//����=4������frm_txҪ����   8
	{
		ArqTxChkWinProc(proc);  //arq_tx_rsp
	}
	return 0;
}
//yun:��mac֡
static void MkMacFrm(Arq_Tx* proc,unsigned short len)//��mac֡
{
	Mac_Frm_1002 *mac_frm =  proc->frm[proc->top];  //yun:proc->frm[proc->top]:����ʱ�ĵ�ַ
	Data_Sub_Frm *data_sub_frm = (Data_Sub_Frm *)mac_frm;

	mac_frm->common_head = (((P_Entity*)proc->entity)->mib.local_id) << 11;  //yun:���͵�ַ
	mac_frm->common_head |= (proc->id)<<6 ;   //yun�����յ�ַ
	data_sub_frm->seg_len_sn_rst |= (proc->top <<1);  //yun:��֡���

}
//yun��frm_tx������������  �С�������  �Ž�����
static int FrmTxRsp(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
	Frm_Tx_Rsp_Param* param=(Frm_Tx_Rsp_Param*)sig->param;
	
	if(param->xlen)//������   yun����������2λ֡ͷ
	{
		//��MAC֡
		MkMacFrm(proc,param->xlen);
		printf("arq_tx[%d]::FrmTxRsp() �յ����ݣ���MAC֡\n",proc->id);
		proc->sts[proc->top] = ARQ_STS_TX;  //16����
		proc->tx_cnt[proc->top] = MAX_RESEND_CNT;  //20
		proc->frm_len[proc->top] = MAC_FRM_LEN_1002; //63 mac֡���ܳ��� payload:62+common_head:1
		proc->top ++;//��������
		proc->top %= ARQ_SN_SIZE;  //16
		proc->size ++;  //yun:�Ѿ���������ݴ��ĸ���
		if(proc->size < ARQ_WIN_SIZE)//����<4����frm_txҪ����  yun:<8  �ظ�����
		{
			printf("arq_tx::FrmTxRsp() ����<8 �����ڴ棬��Ҫ����\n");
			proc->frm[proc->top] = (Mac_Frm_1002 *)MemAllocMac();
			if(!proc->frm[proc->top]) //yun:�����ڴ�
			{
				printf("�ڴ�����ʧ��\n");
				if(proc->size)//�������д�������
				{
					printf("�������д������ݣ�����ȥ\n");
					ArqTxChkWinProc(proc);
				}
				else//�������޴�������
				{
					sig = proc->arq_tx_rsp;
					((T_Arq_Tx_Rsp_Param*)sig->param)->xfrm = NULL;
					AddSignal(sig);
					printf("������û�����ݣ��ظ�NULL\n");
				}
				return 0;
			}
			sig = proc->frm_tx_ind; //yun:��������Ҫ����
			((T_Frm_Tx_Ind_Param*)sig->param)->dat_sub_frm = (unsigned short *)(proc->frm[proc->top]);
			AddSignal(sig);
			proc->state = WAIT_RSP;
		}
		else//����=4������frm_txҪ����  yun��=8
		{
			printf("arq_tx[%d]::FrmTxRsp() ����Ϊ8 ���͵�������\n",proc->id);
			ArqTxChkWinProc(proc);
		}
	}
	else//frm_tx������
	{
		printf("arq_tx[%d]::FrmTxRsp() �ϲ�������\n",proc->id);
		MemFreeMac((unsigned short *)proc->frm[proc->top]);
		proc->frm[proc->top] = NULL;
		if(proc->size)//�������д�������
		{
			printf("arq_tx[%d]::FrmTxRsp() ���������д������ݣ�����ȥ\n",proc->id);
			ArqTxChkWinProc(proc);
		
		}
		else//�������޴�������
		{
			sig = proc->arq_tx_rsp;
			((T_Arq_Tx_Rsp_Param*)sig->param)->xfrm = NULL;
			AddSignal(sig);
			printf("arq_tx[%d]::FrmTxRsp() ��������û�����ݣ��ظ�NULL\n",proc->id);
		}
	}

	return 0;
}
//yun:ArqAckRxInd�Ӻ��� ���ô���
void WinRstProc(Arq_Tx* proc)//�������ô���
{
	unsigned short btm,i;
	Signal *sig;
	//PrintDat(0,0,0x6666);
	btm = proc->btm;
	for(i=0;i<proc->size;i++)
	{
		if(proc->sts[btm] != ARQ_STS_IDLE)		
		{
			MemFreeMac((unsigned short *)proc->frm[btm]);
			proc->frm[btm] = NULL;
		}
		btm++;
		btm %= ARQ_SN_SIZE;
	}
	for(i=0;i<ARQ_SN_SIZE;i++)
	{
		proc->sts[i] = ARQ_STS_IDLE;
		proc->frm_len[i] = 0;
		proc->frm[i] = 0;
		proc->tx_cnt[i] = 0;
		proc->timer_wait_ack[i] = 0;
	}
	proc->top = 0;
	proc->btm = 0;
	proc->idx = 0;
	proc->size = 0;
	proc->rst_flag = 1;

	sig =proc->frm_tx_free_ind ;
	((T_Frm_Tx_Free_Ind_Param *)sig->param)->type = SEND_FAIL_TYPE_RST;//����ʧ�����ͣ��յ�arq��������
	AddSignal(sig);

}
//yun:�յ��ظ���ack����
static int ArqAckRxInd(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
	Arq_Ack_Rx_Ind_Param* param=(Arq_Ack_Rx_Ind_Param*)sig->param;

	unsigned short rcv_sts,sn,succ_cnt,mask,tmp_top,tmp_sn,tmp_btm;
	Ack_Sub_Frm *ack_sgl = (Ack_Sub_Frm *)param->ack_sgl;
	printf("arq_tx[%d]:: ArqAckRxInd �յ�ack\n",proc->id);
	if(proc->rst_flag)//������arq����  yun����һ�η��͵ģ��յ�ack
	{
		if(ack_sgl->flag_ra_rst & 0x0008)//������Ӧ  rst��bit3  �Է�����rst
		{
			proc->rst_flag = 0;
		}
		else//��������Ӧ
		{
			MemFreeMac((unsigned short *)param->ack_sgl);//�ͷ�ack֡
			return 0;   //yun:ֱ�ӷ���
		}
	}
	else//�����շ������յ���֡���ò��ظ�ack,���·��Ŵ���״̬�п����гɹ����
	{
		if(ack_sgl->flag_ra_rst & 0x0008)//������Ӧ
		{
			WinRstProc(proc);//��Ҫ���÷��ʹ���
			MemFreeMac((unsigned short *)param->ack_sgl);//�ͷ�ack֡
			return 0;
		}
	}
	succ_cnt = 0;
	sn = (ack_sgl->sn_sts>>12) & 0x000f;//ack������    arq���:b15-b12
	rcv_sts = (ack_sgl->sn_sts>>5) & 0x007f;//ack�н��մ���״̬   ���ձ�־b11-b5
	tmp_btm = proc->btm;
	tmp_top = (proc->top  >= tmp_btm) ? (proc->top) : (proc->top + ARQ_SN_SIZE);//top��ֵ  //yun:�п����¸����ڣ��¸�16��
	tmp_sn = (sn >= tmp_btm) ? sn : (sn + ARQ_SN_SIZE);//top��ֵ
	if(tmp_sn <= tmp_top && tmp_sn >=tmp_btm)//�����ڷ�������
	{
		while(!(proc->btm == sn))//�ƶ����׵�sn��   �����ʱִ��,ֱ�����
		{
			proc->sts[proc->btm] = ARQ_STS_IDLE;
			if(proc->btm == proc->idx)//idx����������
			{
				proc->idx++;
				proc->idx %= ARQ_SN_SIZE ;
			}
			MemFreeMac((unsigned short *)proc->frm[proc->btm]);	
			proc->frm[proc->btm] = NULL;
			succ_cnt++;
			proc->size --;
			proc->btm++;
			proc->btm %= ARQ_SN_SIZE;
		
		}
		mask = 0x0040;  //yun:64  7bit
		while(mask)//�ô����е�״̬   yun������Ƿ���ַ��Ͳ��ɹ������
		{
			sn++;
			sn %= ARQ_SN_SIZE;
			if(rcv_sts & mask) //yun��
			{
				if(proc->sts[sn] == ARQ_STS_IDLE)//���ڴ����У�����
				{
					WinRstProc(proc);//��Ҫ���÷��ʹ���
					MemFreeMac((unsigned short *)param->ack_sgl);//���ack֡
					return 0;
				}			
				proc->sts[sn] = ARQ_STS_SUCC;  //״̬�ı�Ϊsucc yun:˵���Ǹ����ڽ��ճɹ��ˡ�
			}
			mask >>= 1;
		}
		//proc->sts[proc->btm] = ARQ_STS_TX;
		if(succ_cnt)//���ͳɹ�����succ_cnt
		{
			sig = proc->arq_tx_succ_ind;
			((T_Arq_Tx_Succ_Ind_Param *)sig->param)->succ_cnt = succ_cnt;
			AddSignal(sig);
			printf("arq_tx[%d]:: ArqAckRxInd �ǳɹ�ack�����ϲ�frm_tx�ظ�\n",proc->id);
		}
	}
	else//�������ڴ����� 
	{
		 WinRstProc(proc);//�������ô���		
	}

	//MemFreeMac((unsigned short *)param->ack_sgl);//�ͷ�ack֡???????
	return 0;
}
//yun:�յ�����
static int ArqRstRxInd(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
//	Arq_Rst_Rx_Ind_Param* param=(Arq_Rst_Rx_Ind_Param*)sig->param;
	unsigned short i,btm;
	if(!proc->rst_flag)//������״̬
	{
		btm = proc->btm;
		for(i=0;i<proc->size;i++)  //yun:���д��� �ͷ��ڴ�
		{
			if(proc->sts[btm] != ARQ_STS_IDLE)			
			{
				MemFreeMac((unsigned short *)proc->frm[btm]);
				proc->frm[btm] = NULL;
			}
			btm++;
			btm %= ARQ_SN_SIZE;
		}
		proc->top = 0;
		proc->btm = 0;
		proc->idx = 0;
		proc->size = 0;
		proc->rst_flag = 1;
		sig =proc->frm_tx_free_ind ;
		((T_Frm_Tx_Free_Ind_Param *)sig->param)->type = SEND_FAIL_TYPE_RST;//����ʧ�����ͣ��յ�arq��������
		AddSignal(sig);
	}
	return 0;
}
void ArqTxInit(Arq_Tx* proc)
{
	unsigned short i;
	proc->state = IDLE;
	proc->top = 0;
	proc->btm = 0;
	proc->idx = 0;
	proc->size = 0;
	proc->rst_flag = 1;//���ñ�־
	for(i=0;i<ARQ_SN_SIZE;i++)
	{
		proc->sts[i] = ARQ_STS_IDLE;
		proc->frm_len[i] = 0;
		proc->frm[i] = 0;
		proc->tx_cnt[i] = 0;
		proc->timer_wait_ack[i] = 0;
	}
}

void ArqTxSetup(Arq_Tx* proc)
{
	proc->arq_tx_ind.next=0;
	proc->arq_tx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->arq_tx_ind.src=0;
	proc->arq_tx_ind.dst=proc;
	proc->arq_tx_ind.func=ArqTxInd;
	proc->arq_tx_ind.pri=SDL_PRI_URG;
	proc->arq_tx_ind.param=&proc->arq_tx_ind_param;

	proc->frm_tx_rsp.next=0;
	proc->frm_tx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_tx_rsp.src=0;
	proc->frm_tx_rsp.dst=proc;
	proc->frm_tx_rsp.func=FrmTxRsp;
	proc->frm_tx_rsp.pri=SDL_PRI_URG;
	proc->frm_tx_rsp.param=&proc->frm_tx_rsp_param;

	proc->arq_ack_rx_ind.next=0;
	proc->arq_ack_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->arq_ack_rx_ind.src=0;
	proc->arq_ack_rx_ind.dst=proc;
	proc->arq_ack_rx_ind.func=ArqAckRxInd;
	proc->arq_ack_rx_ind.pri=SDL_PRI_URG;
	proc->arq_ack_rx_ind.param=&proc->arq_ack_rx_ind_param;

	proc->arq_rst_rx_ind.next=0;
	proc->arq_rst_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->arq_rst_rx_ind.src=0;
	proc->arq_rst_rx_ind.dst=proc;
	proc->arq_rst_rx_ind.func=ArqRstRxInd;
	proc->arq_rst_rx_ind.pri=SDL_PRI_URG;
	proc->arq_rst_rx_ind.param=&proc->arq_ack_rx_ind_param;

}



