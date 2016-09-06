#include "arq_rx.h"
#include <stdio.h>
enum {IDLE,WAIT_RSP};//ģ��״̬
enum {ARQ_STS_IDLE,ARQ_STS_RXW,ARQ_STS_DATAW};//����״̬:�ǽ��մ������մ�û���ݡ����մ�������

typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;

//yun:����ack���ӳ���
void SndAckProc(Arq_Rx *proc)
{
	Signal* sig;
	unsigned short ack,sn,mask,sts;
	ack = proc->btm << 12;//����
	sn = proc->btm +1;
	sn %= ARQ_SN_SIZE;
	sts = 0;
	mask = 0x0040;
	while(mask)//������7����״̬
	{
		if(ARQ_STS_DATAW == proc->sts[sn])//�Ѿ��յ�����
		{
			sts |= mask;
		}
		sn++;
		sn %= ARQ_SN_SIZE;
		mask >>= 1;
	}
	ack = (ack|(sts<<5));
	proc->ack_sub_frm.sn_sts = ack;
	sig = proc->arq_ack_tx_req;
	
	((T_Arq_Ack_Tx_Req_Param *)sig->param)->ack_sgl = (unsigned short*)&proc->ack_sub_frm;
	//((T_Arq_Ack_Tx_Req_Param *)sig->param)->len = SUB_FRM_LEN_ACK;//2
	((T_Arq_Ack_Tx_Req_Param *)sig->param)->id = ((P_Entity*)proc->entity)->mib.local_id;  //
	AddSignal(sig);
	printf("arq_rx[%d]::SndAckProc() ����ack��dlc_rx_ctrl\n",proc->id);
	return;
	//��һ���Ѿ�
	//proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK; //0xd000
	//proc->ack_sub_frm.flag_ra_rst |= (proc->id) << 4;//��ack�еĽ��սڵ��
	//proc->ack_sub_frm.flag_ra_rst |= 0x0008;//ack����Ӧ��������
}
//yun���յ�����dlc_rx_ctrl�ĵ�������
static int ArqRxInd(Signal* sig)
{
	Arq_Rx* proc=(Arq_Rx*)sig->dst;
	Arq_Rx_Ind_Param *param=(Arq_Rx_Ind_Param*)sig->param;

	Data_Sub_Frm* data_sub_frm;
	unsigned short sn,tmp_rst_flag,seg_flag;
	unsigned short* tmp_dat;
	unsigned short tmp_len,i,btm;
	data_sub_frm = ((Data_Sub_Frm*)param->xfrm);  //yun:֡��ַ
	sn = (data_sub_frm->seg_len_sn_rst>>1) & 0x000f;//���
	tmp_rst_flag = data_sub_frm->seg_len_sn_rst & 0x0001;//�������ñ�־

	////////////////////////////////���ô���///////////////////////////////////////////
	if(tmp_rst_flag)//�������ñ��
	{
		if(proc->rst_flag)//�շ��Ѿ����ù�
		{
			proc->rst_flag = 0;
		}
		//���ڳ�ʼ��    yun���������ã��շ�û�����õ�����£�Ҫ����
		btm = proc->btm;
		for(i=0;i<ARQ_WIN_SIZE;i++)  //8
		{
			if(proc->sts[btm] == ARQ_STS_DATAW)
			{
				MemFreeMac((unsigned short *)proc->mac_frm[btm]);
				proc->mac_frm[btm] = NULL;
			}
			btm++;
			btm %= ARQ_SN_SIZE; //16
		}
		proc->state = IDLE;
		proc->btm = 0;
		proc->top = ARQ_WIN_SIZE;
		proc->idx = 0;
		//����״̬��ʼ��
		for(i=0;i<proc->top;i++)
		{
			proc->sts[i]=ARQ_STS_RXW;
		}
		for(i=proc->top;i<ARQ_SN_SIZE;i++)
		{
			proc->sts[i]=ARQ_STS_IDLE;
		}
		sig = proc->frm_rx_free_ind;//��շֶ�ģ�鱣�������  yun:�ڷ������õ������
		AddSignal(sig);
		//yun��ack
		proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK;
		proc->ack_sub_frm.flag_ra_rst |= (proc->id) << 4;//��ack�еĽ��սڵ��
		proc->ack_sub_frm.flag_ra_rst |= 0x0008;//ack����Ӧ��������
	}
	else  //yun������û�����ã�
	{
		//proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK;
		//proc->ack_sub_frm.flag_ra_rst |= proc->id<<4;//��ack�еĽ��սڵ��
		//yun����ӵ���һ��
		proc->ack_sub_frm.flag_ra_rst &= 0xFFF7;//ack����Ӧ����û������  ����û�����ã�˵�����ǲ��ǵ�һ���յ�������srt=0
		if(proc->rst_flag)//�շ�����
		{
			MemFreeMac(param->xfrm);
			//sig = proc->arq_rst_tx_req;//����������
			//((T_Arq_Rst_Tx_Req_Param*)sig->param)->rst_sgl = (unsigned short *)&proc->rst_sub_frm;
			//((T_Arq_Rst_Tx_Req_Param*)sig->param)->len = SUB_FRM_LEN_RST;
			//AddSignal(sig);
			sig = proc->arq_rx_rsp;
			AddSignal(sig);
			proc->state = IDLE;
			return 0;
		}
	}

	///////////////////////////////////arq���ڽ������ݴ���/////////////////////////////
	if(proc->sts[sn] == ARQ_STS_RXW)//���մ���������
	{
		//��������
		proc->mac_frm[sn] = (Mac_Frm_1002*)param->xfrm;//memcpy(&proc->mac_frm[sn],param->xfrm,param->xlen);//��������
		proc->sts[sn] = ARQ_STS_DATAW;//������д����Ӧ��ŵĴ��ڣ���״̬ΪARQ_STS_DATAW
		if(sn == proc->btm)//�յ�����
		{			
			do//�ƶ�����
			{
				proc->btm++;
				proc->btm %=ARQ_SN_SIZE;
				proc->sts[proc->top] = ARQ_STS_RXW;
				proc->top++;
				proc->top %=ARQ_SN_SIZE;
			}while(proc->sts[proc->btm] == ARQ_STS_DATAW);
			//��������frm_rx
			
			data_sub_frm = (Data_Sub_Frm *)proc->mac_frm[proc->idx];  //yun:���յ�frm_rx�ظ���rsq��++
			tmp_dat = data_sub_frm->data;
			tmp_len = ((data_sub_frm->seg_len_sn_rst >> 5) & 0x003f);
			seg_flag = (data_sub_frm->seg_len_sn_rst) & 0x6000;
			sig = proc->frm_rx_ind;  //yun��������frm_rx
			((T_Frm_Rx_Ind_Param*)sig->param)->xdat = tmp_dat;
			((T_Frm_Rx_Ind_Param*)sig->param)->xlen = tmp_len;
			((T_Frm_Rx_Ind_Param*)sig->param)->seg_flag = seg_flag;
			((T_Frm_Rx_Ind_Param*)sig->param)->bc_uc_falg = 0;//����
			AddSignal(sig);
			proc->state = WAIT_RSP;
			printf("arq_rx[%d]::ArqRxInd() �������ݸ��ϲ�frm_rx\n",proc->id);
		/*	if(tmp_len == 58)
			{
				sig = proc->frm_rx_ind;
			((T_Frm_Rx_Ind_Param*)sig->param)->xdat = tmp_dat;
			((T_Frm_Rx_Ind_Param*)sig->param)->xlen = tmp_len;
			((T_Frm_Rx_Ind_Param*)sig->param)->seg_flag = seg_flag;
			((T_Frm_Rx_Ind_Param*)sig->param)->bc_uc_falg = 0;//����

			}*/
			return 0;  //yun:����
		}
	}
	else
	{
		MemFreeMac(param->xfrm);
	}	
	//SndAckProc(proc);//����ack���� yun for test 
	sig = proc->arq_rx_rsp;
	AddSignal(sig);
	proc->state = IDLE;
	return 0;
}
//yun���յ�frm_rx�Ļظ�
static int FrmRxRsp(Signal* sig)
{
	Arq_Rx* proc=(Arq_Rx*)sig->dst;
	//Frm_Rx_Rsp_Param *param=(Frm_Rx_Rsp_Param*)sig->param;
	
	Data_Sub_Frm* data_sub_frm;
	unsigned short seg_flag;
	unsigned short* tmp_dat;
	unsigned short tmp_len;
	MemFreeMac((unsigned short *)proc->mac_frm[proc->idx]);//�ͷ�MAC��
	proc->mac_frm[proc->idx] = NULL;
	proc->sts[proc->idx] = ARQ_STS_IDLE;//yun:���ճɹ��ˣ��ı��־λ
	proc->idx ++;
	proc->idx %= ARQ_SN_SIZE;
	printf("arq_rx[%d]::FrmRxRsp() �յ��ϲ�frm_rx�Ļظ�\n",proc->id);
	if(ARQ_STS_DATAW == proc->sts[proc->idx])//�������ݿ�������frm_rx
	{
		
		data_sub_frm = (Data_Sub_Frm *)proc->mac_frm[proc->idx];  //yun:�Ѿ�����һ֡��
		tmp_dat = data_sub_frm->data;
		tmp_len = ((data_sub_frm->seg_len_sn_rst >> 5) & 0x003f);
		seg_flag = (data_sub_frm->seg_len_sn_rst) & 0x6000;
		sig = proc->frm_rx_ind;//��������frm_rx  yun���ٴη���
		((T_Frm_Rx_Ind_Param*)sig->param)->xdat = tmp_dat;
		((T_Frm_Rx_Ind_Param*)sig->param)->xlen = tmp_len;
		((T_Frm_Rx_Ind_Param*)sig->param)->seg_flag = seg_flag;
		AddSignal(sig);
		printf("arq_rx[%d]::FrmRxRsp() �������ݣ��������͸�frm_rx\n",proc->id);
	}
	else//�����ݿ�������frm_rx,��idx == btm
	{
		printf("arq_rx[%d]::FrmRxRsp() û�������ˣ�׼���ظ�ack\n",proc->id);
		SndAckProc(proc);//����ack����  yun for test ��������
		sig = proc->arq_rx_rsp;
		AddSignal(sig);
		proc->state = IDLE;	
	}
	return 0;
}

void ArqRxInit(Arq_Rx *proc)
{
	unsigned short i;
	proc->state = IDLE;
	proc->rst_flag = 1;
	proc->btm = 0;
	proc->top = ARQ_WIN_SIZE;
	proc->idx = 0;
	proc->emergency_ack = 0;
	//����״̬��ʼ��
	for(i=0;i<proc->top;i++)
	{
		proc->sts[i]=ARQ_STS_RXW;
	}
	for(i=proc->top;i<ARQ_SN_SIZE;i++)
	{
		proc->sts[i]=ARQ_STS_IDLE;
	}
	for(i=0;i<ARQ_SN_SIZE;i++)
	{
		proc->mac_frm[i] = 0;
	}
	//����ack����ֳ�ʼ��
	proc->ack_sub_frm.flag_ra_rst = SUB_FRM_TYPE_ACK;
	proc->ack_sub_frm.flag_ra_rst |= (proc->id)<<4;
	//����arq���������ʼ��
	proc->rst_sub_frm.flag_ra = SUB_FRM_TYPE_RST;
	proc->rst_sub_frm.flag_ra |= (proc->id)<<4;
}

void ArqRxSetup(Arq_Rx *proc)
{
	proc->arq_rx_ind.next=0;
	proc->arq_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->arq_rx_ind.src=0;
	proc->arq_rx_ind.dst=proc;
	proc->arq_rx_ind.func=ArqRxInd;
	proc->arq_rx_ind.pri=SDL_PRI_URG;
	proc->arq_rx_ind.param=&proc->arq_rx_ind_param;

	proc->frm_rx_rsp.next=0;
	proc->frm_rx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->frm_rx_rsp.src=0;
	proc->frm_rx_rsp.dst=proc;
	proc->frm_rx_rsp.func=FrmRxRsp;
	proc->frm_rx_rsp.pri=SDL_PRI_URG;
	proc->frm_rx_rsp.param=&proc->frm_rx_rsp_param;

}
