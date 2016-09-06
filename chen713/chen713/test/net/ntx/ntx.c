#include "ntx.h"
#include <string.h>
#include <stdio.h>
#include "..\..\common.h"
//#include "../../../../hpi.h"
//#include "../../../../j1052_gpio.h"

enum {IDLE,WAIT_RT_RSP};
extern short getcrc_short( unsigned short* buf,  int len);
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;

//������������������д��ָ���ĵ�ַ��  yun:�Ӻ��� 7param
static void WtNetPkt(NTx* proc,unsigned short *sdat,unsigned short *dat,unsigned short len,unsigned short da,unsigned short pri,unsigned short hello_flag)//д�����ͷ����������
{
	memset(dat,0,(len + NET_FRM_HEAD_LEN));  //yun:������ռ� ����
	//������֡ͷ,������ţ�����ڷ��͵�ʱ��Ż���д��
	((Net_Frm*)dat)->pri_sa_da_mul_sn = ((P_Entity*)proc->entity)->mib.local_id << 8;  //yun:Դ��ַ
	((Net_Frm*)dat)->pri_sa_da_mul_sn |= (da & 0x1f)<<3;  //yun:Ŀ�ĵ�ַ
	((Net_Frm*)dat)->pri_sa_da_mul_sn |= pri<<13;  //yun:���ȼ�
	((Net_Frm*)dat)->sn_len |= (len &0x3ff);  //yun:���� ��û���
	if(da == MULTICAST_DA)   //�ಥ��λ
	{
		((Net_Frm*)dat)->pri_sa_da_mul_sn |= (0x1<<2);
	}
	memcpy(((Net_Frm*)dat)->net_paload, sdat, len);  //yun�����ݿ���!!!! ע�ⳤ�����⣺ û�������ͷ
	if(hello_flag)//HELLO����֡ͷ�ñ��
	{
		((Net_Frm*)dat)->pri_sa_da_mul_sn |= 0x8000;	
	}
	return;
}

//�����Ӧ��Ŀ�Ľڵ����������
static int NetQueWt(NTx* proc,unsigned short *dat,unsigned short len,unsigned short ra,unsigned short da,unsigned long  arrvial_time,unsigned short pri)
{
	unsigned short btm,idx,tmp_idx2,tmp_idx1;
	short i,j,size;
	//top = proc->net_que_list[da].top;
	btm = proc->net_que_list[da].btm;
	size = proc->net_que_list[da].size; 
	if(size >= NET_QUE_MAX_NUM)  //20
	{
		printf("ntx::NetQueWt() ���������ͷ�����\n");
		MemFreeNet(dat);//���ݹ��࣬�ͷ�����
		return 0;
	}
	
	//ת���ಥ���������ȼ������Ŷ�,ֱ�ӷŵ�������buf��
	if((((*dat)>>2)&0x1) && ((((*dat)>>8)&0x1f) != ((P_Entity*)proc->entity)->mib.local_id))//�ಥ&&Դ�ڵ㲻���Լ�
	{
		idx = (btm + size)%NET_QUE_MAX_NUM;
	}
	else//�������������ȼ��Ŷӣ����ȼ��ߵ����ڿ���btm�ķ������ȼ��͵�����Զ��btm�ķ���
	{
		idx = btm;
		//yun��ע�⣬i��1��ʼ��Ҳ���ǽ�����һ���ͷ�����0��
		for(i=1;i<=size;i++)   //yun���ҵ������ȼ�С����һ����ȷ����λ�� ����С�����ȼ���
		{
			idx = (btm + i)%NET_QUE_MAX_NUM;
			if(i<size)
			{
				if(pri < proc->net_que_list[da].que_elmt[idx].pri)//�ҵ�idxΪ����λ��  yun�������ȼ���
				{
					break;
				}
			}
		}
		for(j=0;j<size-i;j++)  //yun:�ҵ���λ�����ĵ�Ԫһ��һ��������
		{
			tmp_idx1 = (btm + size -j)%NET_QUE_MAX_NUM;
			tmp_idx2 = (btm + size -j -1)%NET_QUE_MAX_NUM;
			proc->net_que_list[da].que_elmt[tmp_idx1].dat = proc->net_que_list[da].que_elmt[tmp_idx2].dat;
			proc->net_que_list[da].que_elmt[tmp_idx1].len = proc->net_que_list[da].que_elmt[tmp_idx2].len;
			proc->net_que_list[da].que_elmt[tmp_idx1].ra = proc->net_que_list[da].que_elmt[tmp_idx2].ra ;
			proc->net_que_list[da].que_elmt[tmp_idx1].da = proc->net_que_list[da].que_elmt[tmp_idx2].da;
			proc->net_que_list[da].que_elmt[tmp_idx1].pri = proc->net_que_list[da].que_elmt[tmp_idx2].pri;
			proc->net_que_list[da].que_elmt[tmp_idx1].arrvial_time = proc->net_que_list[da].que_elmt[tmp_idx2].arrvial_time ;
			proc->net_que_list[da].que_elmt[tmp_idx1].first_send_flag = proc->net_que_list[da].que_elmt[tmp_idx2].first_send_flag;
		}
	}
	//�����ݷŵ��ҵ��Ĳ���λ����
	proc->net_que_list[da].que_elmt[idx].dat=dat;
	proc->net_que_list[da].que_elmt[idx].len=len;
	proc->net_que_list[da].que_elmt[idx].ra=ra;
	proc->net_que_list[da].que_elmt[idx].da=da;
	proc->net_que_list[da].que_elmt[idx].pri=pri;
	proc->net_que_list[da].que_elmt[idx].arrvial_time = arrvial_time;
	proc->net_que_list[da].que_elmt[idx].first_send_flag=1;
	proc->net_que_list[da].top++;
	proc->net_que_list[da].top%=NET_QUE_MAX_NUM;
	proc->net_que_list[da].size++;
	return 1;
}

//ɾ�������е������   yun���²�ظ�����������·��ʧ��  ɾ���ײ�һ��
void DeleteQueElmt(NTx* proc,unsigned short da)
{
	unsigned short btm=proc->net_que_list[da].btm;
	//unsigned char printdat_once[] = " DeleteQueElmt";
	//PrintDat(printdat_once,14,1);
	if(proc->net_que_list[da].size)
	{
		printf("ntx::DeleteQueElmt() ɾ����������������ͷ�����\n");
		MemFreeNet(proc->net_que_list[da].que_elmt[btm].dat);
		proc->net_que_list[da].que_elmt[btm].dat = NULL;
		proc->net_que_list[da].que_elmt[btm].len = 0;
		proc->net_que_list[da].que_elmt[btm].ra = 0;
		proc->net_que_list[da].que_elmt[btm].da = 0;
		proc->net_que_list[da].que_elmt[btm].arrvial_time = 0;
		proc->net_que_list[da].que_elmt[btm].first_send_flag = 0;
		proc->net_que_list[da].btm ++;
		proc->net_que_list[da].btm %= NET_QUE_MAX_NUM;
		proc->net_que_list[da].size --;
	}

}

//����ĳ�ڵ��������ж�ȡ
static int NetQueRd(NTx* proc,unsigned short **dat,unsigned short *len,unsigned short *ra,unsigned short *da,unsigned long * arrvial_time,unsigned short dat_in)
{
	unsigned short btm;
	unsigned short k,dat_valid,size;
	//unsigned long time_len;
	dat_valid=0;
	if(proc->net_que_list[dat_in].size!=0)
	{
		size=proc->net_que_list[dat_in].size;
		for(k=0;k<size;k++)
		{
			btm=proc->net_que_list[dat_in].btm;			
			if(proc->net_que_list[dat_in].que_elmt[btm].first_send_flag)//�״η������������¼ʱ�����ڳ�ʱɾ��
			{
				proc->net_que_list[dat_in].que_elmt[btm].first_send_flag = 0;
				proc->net_que_list[dat_in].que_elmt[btm].arrvial_time = proc->ntx_local_time;
				dat_valid=1;
				break;
			}
			//if(proc->ntx_local_time > proc->net_que_list[dat_in].que_elmt[btm].arrvial_time)
			//{
			//	time_len = proc->ntx_local_time - proc->net_que_list[dat_in].que_elmt[btm].arrvial_time;
			//}
			//else
			//{
			//	time_len = proc->ntx_local_time + (0xffffffff - proc->net_que_list[dat_in].que_elmt[btm].arrvial_time);
			//}
			//if(time_len < NET_PKT_MAX_TTL)//δ��ʱ�����״������
			//{
				dat_valid=1;
				break;	
			//}
			//else//��������ݳ�ʱ,ɾ��
			//{
			//	sprintf(str,"Timer Net Pkt FAIL: Node %d ��> Node %d   len=%d  Data: %x %x %x %x %x %x %x %x %x %x",((P_Entity*)proc->entity)->mib.local_id,dat_in,proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].len,proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[3],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[4],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[5],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[6],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[7],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[8],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[9],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[10],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[11],proc->net_que_list[dat_in].que_elmt[proc->net_que_list[dat_in].btm].dat[12]);		
			//	DbgPrint(str);
			//	DeleteQueElmt(proc,dat_in);
			//}
		}		
	}
	if(!dat_valid)//�����������
	{
		*dat=0;
		*len=0;
		return 0;
	}	
	else//�����������
	{
		btm=proc->net_que_list[dat_in].btm;
		*dat=proc->net_que_list[dat_in].que_elmt[btm].dat;
		*len=proc->net_que_list[dat_in].que_elmt[btm].len;
		*ra=proc->net_que_list[dat_in].que_elmt[btm].ra;
		*da=proc->net_que_list[dat_in].que_elmt[btm].da;
		*arrvial_time=proc->net_que_list[dat_in].que_elmt[btm].arrvial_time;
		return 1;
	}
}

//����ڵ��������ж�ȡ
static int NetQueRdNext(NTx* proc,unsigned short **dat,unsigned short *len,unsigned short *ra,unsigned short *da,unsigned long * arrvial_time)
{
	unsigned short j,dat_valid;
	dat_valid=0;
	proc->curt_node++;
	proc->curt_node %= NODE_MAX_CNT;
	for(j = proc->curt_node;j < NODE_MAX_CNT; j++)
	{
		if(proc->net_que_list[j].size > 0)//�˽ڵ��²���,���Է���
		{
			dat_valid = NetQueRd(proc,dat,len,ra,da,arrvial_time,j);
		}
		if(dat_valid)
		{
			break;
		}
	}
	proc->curt_node = j;
	if(proc->curt_node == NODE_MAX_CNT)
	{
		proc->curt_node--;
	}
	return dat_valid;
}

//yun:�ϲ�ҵ�����ݷ�������
static int DataTxReq(Signal* sig)
{
	NTx* proc = (NTx*)sig->dst;
	Data_Tx_Req_Param *param = (Data_Tx_Req_Param*)sig->param;
	//unsigned char printdat_once[] = " DatTxReq";
	unsigned short *dat;
	unsigned short len;
	unsigned short da;
	unsigned short ra;
	unsigned long arrival_time;
	unsigned short tmp_sn;
	Net_Frm* net_frm;  //yun:Ϊ �������
	len = param->xlen;//lenΪ������������ɳ���
	printf("-----�ڵ�[%d]-----ntx::DataTxReq() �յ�ҵ�����ݷ�������\n",((P_Entity*)proc->entity)->mib.local_id);
	//yun:���»���û��------------------------------------------------------------
	if((proc->state==WAIT_RT_RSP)||(len == 0) || (len>NET_PAYLOAD_SIZE))   //yun�����ܷ���
	{
		//yun��testע�͵�
		//if(param->xdat == (unsigned short *)HPI_FRAME_RX_16BUF.DATA)
		//{
		//	SET_MCU_GPIO2_HIGH;//����ARM�·�����
		//}
		//dat_tx_req_add_sig_flag = 0;//�ź�dat_tx_req�Ѿ�������б��
		return 0;  // yun�����ܷ��ͣ�ֱ�ӷ���
	}
	dat = (unsigned short *)MemAllocNet();  //yun:�����ڴ�
	if(!dat)//��������ռ�   yun�����ܷ��ͣ��ظ��������
	{
		//yun��testע�͵�
		//if(param->xdat == (unsigned short *)HPI_FRAME_RX_16BUF.DATA)
		//{
		//	SET_MCU_GPIO2_HIGH;//����ARM�·�����
		//}
		//dat_tx_req_add_sig_flag = 0;//�ź�dat_tx_req�Ѿ�������б��
		return 0;
	}
	if(param->xda == MULTICAST_DA)//yun���ಥ
	{
		test_info.snd_net_pkt_gc++;
	}
	else if(param->xda == ((P_Entity*)proc->entity)->mib.local_id)   //yun:�㲥
	{
		test_info.snd_net_pkt_bc++;
	} 
	else   //yun:����
	{
		test_info.snd_net_pkt_uc ++;
	}
	//----------------------------------------------------------------------------
	
	da = param->xda;  //Ŀ�ĵ�ַ
	ra = 0;
	arrival_time = proc->ntx_local_time;
	//������������ݿ��� 7-param
	WtNetPkt(proc, param->xdat, dat, len, da, param->xpri, param->hello_flag);
	
	if(da == MULTICAST_DA)//�ಥ
	{
		da = ((P_Entity*)proc->entity)->mib.local_id;   //yun:���ǹ㲥������  Ҳ�ǹ㲥
	}
	//д�������/β  7-param
	NetQueWt(proc, dat, len, ra, da, arrival_time, param->xpri);
	printf("ntx::DataTxReq() д��������\n");
	if(proc->net_que_list[da].size > 1)//�˳��˰����δ�����ݣ�Ӧ�÷���֮ǰ������?
	{
		NetQueRd(proc,&dat,&len,&ra,&da,&arrival_time,da);//������
		printf("ntx::DataTxReq() size>1��������\n");
	}
	proc->dat = dat;  //yun:size=1,��һ�����ݣ����ö���
	proc->len = len;
	proc->da = da;
	proc->arrvial_time = arrival_time;
	proc->curt_node = da;//��¼�ϲ㷢���ݴ���,����·��ʧ�ܻ��²�æ������� NetQueRdNext
	
	if((da !=((P_Entity*)proc->entity)->mib.local_id))//����  ��Ҫ�ȼ��·��
	{
		if(proc->state != WAIT_RT_RSP)//û����������·��
		{
			sig = proc->rt_tx_ind;//����·��
			((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
			AddSignal(sig);
			proc->state = WAIT_RT_RSP;
			proc->data_tx_req_rt_tx_ind_flag = 1; //yun:���·��  ��·������ռ��
		}
	}
	else//�㲥���ಥ
	{
		if((!(proc->sts & (1<<da)))&& (!(proc->net_que_list[proc->da].curt_snd_flag))) //yun��״̬λ�����������ͱ�־
		{			
			//������ͷ���
			printf("ntx::DataTxReq() �㲥/�ಥ ��д����ͷ���\n");
			net_frm = (Net_Frm* )(proc->dat);
			//yun��Դ��ַ=���ڵ��
			if((((net_frm->pri_sa_da_mul_sn) >> 8) &0x1f) == ((P_Entity*)proc->entity)->mib.local_id)//���ڵ㷢�͵�����������м�
			{
				if(((net_frm->pri_sa_da_mul_sn) >> 2) &0x1)//�ಥ��־
				{
					tmp_sn = proc->ntx_sn_mul;//�ಥ���   ���յ��ظ�ʱ++��
				}
				else//�㲥
				{
					tmp_sn = proc->ntx_sn[proc->da];  // ���յ��ظ�ʱ++��
				}
				net_frm->pri_sa_da_mul_sn |= ((tmp_sn>> 6) & 0x3);  //yun:���  2��ƴ������Ϊ���������
				net_frm->sn_len |= (tmp_sn << 10);
			}
			proc->net_que_list[proc->da].curt_snd_flag = 1;  //yun:�յ��²�ظ�ʱ����
			sig=proc->ntx_req;//���²�dlc_tx��������
			((T_NTx_Req_Param*)sig->param)->ra = proc->da;
			((T_NTx_Req_Param*)sig->param)->xdat = proc->dat;
			((T_NTx_Req_Param*)sig->param)->xlen = proc->len + NET_FRM_HEAD_LEN;
			AddSignal(sig);
			printf("ntx::DataTxReq() ���²㷢�͹㲥�����,��[%d]��\n",tmp_sn);
			proc->sts |= (1<<(proc->da));//�²�˽ڵ�æ
		}
		else{
			printf("ntx::DataTxReq() net_que_list[%d]����ռ�ã��ȴ�\n",proc->da);
		}
		//dat_tx_req_add_sig_flag = 0;//�ź�dat_tx_req�Ѿ�������б��
		//yun��testע�͵�
		//if(param->xdat == (unsigned short *)HPI_FRAME_RX_16BUF.DATA)
		//{
		//	SET_MCU_GPIO2_HIGH;//����ARM�·�����
		//}
	}
	return 0;
}

//·�ɲ�ѯ���ص��źŴ�����
static int RtTxRsp(Signal* sig)
{
	NTx* proc = (NTx*)sig->dst;
	Rt_Tx_Rsp_Param *param = (Rt_Tx_Rsp_Param*)sig->param;
	unsigned short *dat;
	unsigned long  arrvial_time;
	unsigned short len = 0;
	unsigned short da,ra;
	unsigned short size,i,tmp_sn;
	unsigned short test_succ_yun;
	unsigned short test_ra_yun;
	Net_Frm* net_frm;

	//yun��testע�͵�
	//if(proc->data_tx_req_rt_tx_ind_flag)
	//{
	//	proc->data_tx_req_rt_tx_ind_flag = 0;
	//	SET_MCU_GPIO2_HIGH;//����ARM�·�����
	//}
	//dat_tx_req_add_sig_flag = 0;//�ź�dat_tx_req�Ѿ�������б��
	proc->state=IDLE;
	proc->da = param->da;
	printf("ntx.c::RtTxRsp() ·�ɲ�ѯ���ؽ��\n");
	if((param->succ) && (!(proc->sts & (1<<param->ra))) && (!(proc->net_que_list[proc->da].curt_snd_flag)))//�鵽·�ɣ��²㣨ra��һ�����˽��սڵ���
	{
		//proc->rt_fail_tmr[proc->da] = 0;
		NetQueRd(proc,&dat,&len,&ra,&da,&arrvial_time,proc->da);//2012.8.31���¶����У���ֹ������·��ʱ��������շ����ͷţ���ʱproc->datΪ�ͷŵ������
		if(len)//������Ҫ��2012.8.31   yun�����ִ�����return��
		{	

			proc->dat = dat;//2012.8.31
			proc->len = len;//2012.8.31
			//������ͷ���
			net_frm = (Net_Frm* )(proc->dat);

			if((((net_frm->pri_sa_da_mul_sn) >> 8) &0x1f) == ((P_Entity*)proc->entity)->mib.local_id)//���ڵ㷢�͵�����������м�  Դ��ַ����8λ
			{
				if(((net_frm->pri_sa_da_mul_sn) >> 2) &0x1)//�ಥ
				{
					tmp_sn = proc->ntx_sn_mul;
				}
				else//�㵥��
				{
					tmp_sn = proc->ntx_sn[proc->da];  //yun:ntx_sn��ʼ��Ϊ1
				}
				net_frm->pri_sa_da_mul_sn |= ((tmp_sn>> 6) & 0x3); //yun:??????????
				net_frm->sn_len |= (tmp_sn << 10);
				printf("ntx.c::RtTxRsp() �����ݷ�����д����ͷ���[%d]\n",tmp_sn);
			}
			proc->net_que_list[proc->da].curt_snd_flag = 1;  //yun:�յ��²�ظ�ʱ����
			sig=proc->ntx_req;//���²㷢������
			printf("ntx.c::RtTxRsp()�����²�dlc_tx_dump\n");
			((T_NTx_Req_Param*)sig->param)->ra = param->ra; //yun:�����ַ�ǵ���ʱ ��һ���ĵ�ַ ,������Ŀ�ĵ�ַ������֡ͷ��
			((T_NTx_Req_Param*)sig->param)->xdat = proc->dat;
			((T_NTx_Req_Param*)sig->param)->xlen = proc->len + NET_FRM_HEAD_LEN;
			AddSignal(sig);   // 
			//NTx_Req((((P_Entity*)proc->entity)->mib.local_id),param->ra,proc->dat,proc->len + NET_FRM_HEAD_LEN);//for test
			proc->sts |= (1<<param->ra);//�²�˽ڵ�æ
			return 0;
		}
	}
	if(!param->succ)//δ�鵽·��
	{
		/*if(!proc->rt_fail_tmr[proc->da])
		{
			proc->rt_fail_tmr[proc->da] = RT_FAIL_TMR;
		}*/
		size = proc->net_que_list[proc->da].size;//2012.8.31
		for(i=0; i<size; i++)//2012.8.31
		{
			/*proc->net_que_list_fm.dat[proc->net_que_list_fm.top] = FlashMemAlloc();
			if(proc->net_que_list_fm.dat[proc->net_que_list_fm.top])
			{
				WriteFlash(proc->net_que_list_fm.dat[proc->net_que_list_fm.top], proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].dat, proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].len  + NET_FRM_HEAD_LEN);			
				proc->net_que_list_fm.top++;
				proc->net_que_list_fm.top %= FLASH_MEM_BLOCK_CNT;
				proc->net_que_list_fm.size++;
			}*/
			DeleteQueElmt(proc,proc->da);//2012.8.31 ��·��ֱ��ɾ��
			test_info.snd_fail_net_pkt_uc++;
		}
	}
	NetQueRdNext(proc,&dat,&len,&ra,&da,&arrvial_time);   //yun����ڵ��������ж�ȡ
	if(len)//������Ҫ��
	{
		proc->dat = dat;
		proc->len = len;
		proc->da = da;
		proc->arrvial_time = arrvial_time;
		sig=proc->rt_tx_ind;//����·�� 
		((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
		AddSignal(sig);
		proc->state = WAIT_RT_RSP;
	}
	return 0;
}

volatile unsigned short rely_err_cnt = 0;

//yun��ת�� ָʾ ����Ҫ�������������DataTxReq����
static int RelayTxInd(Signal* sig)
{
	NTx* proc=(NTx*)sig->dst;
	Relay_Tx_Ind_Param *param=(Relay_Tx_Ind_Param*)sig->param;
	unsigned short *dat;
	unsigned short len, link_cnt, delay;
	unsigned short da,tmp_da,tmp_sn;
	//unsigned short ra;
	unsigned long  arrvial_time;
	Net_Frm* net_frm;
	arrvial_time=proc->ntx_local_time;
	da = param->xda;
	dat = param->xdat;
	len = param->xlen;
	tmp_da = da;
	if(da == MULTICAST_DA)
	{
		tmp_da = ((P_Entity*)proc->entity)->mib.local_id;
	}
	/*if(malloc_cnt1 > RELAY_TX_MALLOC_CNT_MAX)
	{
		proc->net_que_list_fm.dat[proc->net_que_list_fm.top] = FlashMemAlloc();
		if(proc->net_que_list_fm.dat[proc->net_que_list_fm.top])
		{
			WriteFlash(proc->net_que_list_fm.dat[proc->net_que_list_fm.top],dat, len  + NET_FRM_HEAD_LEN);			
			proc->net_que_list_fm.top++;
			proc->net_que_list_fm.top %= FLASH_MEM_BLOCK_CNT;
			proc->net_que_list_fm.size++;
			return 0;
		}
	}*/

	proc->dat = dat;
	proc->len = len;
	proc->da = tmp_da;
	proc->arrvial_time = arrvial_time;

	NetQueWt(proc, param->xdat, param->xlen, 0, tmp_da, arrvial_time, param->xpri);//д�������/β
	if((da !=((P_Entity*)proc->entity)->mib.local_id) && (da != MULTICAST_DA))//����
	{
		if(proc->state != WAIT_RT_RSP)//û����������·��
		{
			sig = proc->rt_tx_ind;//����·��
			((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
			AddSignal(sig);
			proc->state = WAIT_RT_RSP;
		}
	}
	else//�㲥���ಥ
	{
		if((!(proc->sts & (1<<da))) && (!(proc->net_que_list[proc->da].curt_snd_flag)))
		{			
			//������ͷ���
			net_frm = (Net_Frm* )(proc->dat);
			if((((net_frm->pri_sa_da_mul_sn) >> 8) &0x1f) == ((P_Entity*)proc->entity)->mib.local_id)//���ڵ㷢�͵�����������м�
			{
				if(((net_frm->pri_sa_da_mul_sn) >> 2) &0x1)//�ಥ
				{
					tmp_sn = proc->ntx_sn_mul;
				}
				else//�㲥
				{
					tmp_sn = proc->ntx_sn[proc->da];
				}
				net_frm->pri_sa_da_mul_sn |= ((tmp_sn>> 6) & 0x3);
				net_frm->sn_len |= (tmp_sn << 10);
			}
			proc->net_que_list[proc->da].curt_snd_flag = 1;
			sig=proc->ntx_req;//���²㷢������
			((T_NTx_Req_Param*)sig->param)->ra = proc->da;
			((T_NTx_Req_Param*)sig->param)->xdat = proc->dat;
			((T_NTx_Req_Param*)sig->param)->xlen = proc->len + NET_FRM_HEAD_LEN;
			AddSignal(sig);
			//NTx_Req((((P_Entity*)proc->entity)->mib.local_id),param->ra,proc->dat,proc->len + NET_FRM_HEAD_LEN);//for test
			proc->sts |= (1<<(proc->da));//�²�˽ڵ�æ
		}
	}

	return 0;
}

//yun���յ��²�dlc�����Ļظ�
static int NTxInd(Signal* sig)
{
	NTx* proc=(NTx*)sig->dst;
	NTx_Ind_Param *param=(NTx_Ind_Param*)sig->param;

	unsigned short *dat;
	unsigned short len = 0;
	unsigned short is_vaid = 0;
	unsigned short ra;
	unsigned short da;
	unsigned short* tmp_dat;
	unsigned short tmp_ra,tmp_da,tmp_sa;
	unsigned long arrvial_time;
	unsigned short i,size,mul_flag;

	Net_Frm* net_frm = (Net_Frm* )param->dat;
	unsigned char printdat_once[20] = " NTxInd";
	//PrintDat(printdat_once,7,1);
	//PrintDat(0,0,0x0006);
	if(param->dat == NULL)  //yun:����Ϊ��
	{
		//PrintDat(0,0,0xaaaa);
		return 0;
	}

	tmp_da = param->da;  //yun��Ŀ�Ľڵ�
	tmp_sa = param->sa;  //yun:Դ�ڵ�
	mul_flag = ((net_frm->pri_sa_da_mul_sn)>>2) & 0x1;  //yun:�ಥ��־
	if(mul_flag)
	{
		tmp_da = ((P_Entity*)proc->entity)->mib.local_id;
	}
	
	tmp_dat=param->dat;
	proc->sts &= (~(1<<param->ra));//�²�˽ڵ���
	proc->net_que_list[tmp_da].curt_snd_flag = 0;//�Ŀ�Ľڵ������ڷ��Ͱ?
	printf("ntx.c::NTxInd() �յ�dlc_tx_dump����������ͳɹ��ظ�\n");
	if(param->succ_flag)//�ײ㷢�ͳɹ�
	{
		if(tmp_sa == ((P_Entity*)proc->entity)->mib.local_id)//���ڵ㷢�͵�����������м�
		{
			if(mul_flag)//�ಥ
			{
				proc->ntx_sn_mul ++;
			}
			else
			{
				proc->ntx_sn[tmp_da]++;//�������+1
				//printf("ntx::NTxInd() �������+1����ǰ���Ϊ��[%d]\n", proc->ntx_sn[tmp_da]);
			}
		}
		DeleteQueElmt(proc,tmp_da);//ɾ�����������
		if((proc->net_que_list[tmp_da].size) && ((proc->state!=WAIT_RT_RSP))) //|| (tmp_da==((P_Entity*)proc->entity)->mib.local_id)))//��Ŀ�Ľڵ�����δ������
		{
			is_vaid = NetQueRd(proc,&dat,&len,&ra,&da,&arrvial_time,tmp_da);//������
			printf("ntx.c::NTxInd() ����Ŀ�Ľڵ�����Ƿ������ݣ� %d\n",is_vaid);
		}			
	}
	else//�²㷢��ʧ��
	{
		if(param->fail_type == SEND_FAIL_TYPE_RESEND)//�ط�ʧ�� 2012.9.10
		{
			/*if(!proc->rt_fail_tmr[tmp_da])
			{
				proc->rt_fail_tmr[tmp_da] = RT_FAIL_TMR;
			}*/
			size = proc->net_que_list[tmp_da].size;//2012.8.31
			for(i=0; i<size; i++)//2012.8.31
			{
				/*proc->net_que_list_fm.dat[proc->net_que_list_fm.top] = FlashMemAlloc();
				if(proc->net_que_list_fm.dat[proc->net_que_list_fm.top])
				{
					WriteFlash(proc->net_que_list_fm.dat[proc->net_que_list_fm.top], proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].dat, proc->net_que_list[proc->da].que_elmt[proc->net_que_list[proc->da].btm].len  + NET_FRM_HEAD_LEN);			
					proc->net_que_list_fm.top++;
					proc->net_que_list_fm.top %= FLASH_MEM_BLOCK_CNT;
					proc->net_que_list_fm.size++;
				}*/
				DeleteQueElmt(proc,tmp_da);//2012.8.31ɾ����������� 
				test_info.snd_fail_net_pkt_uc++;
			}
			//tmp_ra = param->ra;	
			//sig=proc->link_fail_ind;//ָʾ·��ʧ��
			//((T_Link_Fail_Ind_Param*)sig->param)->ra = tmp_ra;
			//AddSignal(sig);	
		}
	}
	
	if((!is_vaid) && (proc->state!=WAIT_RT_RSP))
	{
		proc->curt_node = tmp_da;//����㷢�����ݴ���,���ڶ����� NetQueRdNext
		is_vaid = NetQueRdNext(proc,&dat,&len,&ra,&da,&arrvial_time);//�������ڵ�����
		if(!is_vaid)//�Ȳ飨tmp_da+1������NODE_MAX_CNT-1�����ٲ� 0��tmp_da�Ƿ��������
		{
			is_vaid = NetQueRdNext(proc,&dat,&len,&ra,&da,&arrvial_time);//�������ڵ�����
			printf("ntx.c::NTxInd() ��������ڵ��Ƿ������ݷ��� ��%d\n",is_vaid);
			if(da == tmp_da)
			{
				is_vaid = 0;
			}
		}
	}
	
	if(is_vaid)//���Է�������
	{
		proc->dat=dat;
		proc->len=len;
		proc->da=da;
		proc->arrvial_time = arrvial_time;
		sig = proc->rt_tx_ind;//����·��
		((T_Rt_Tx_Ind_Param*)sig->param)->da = da;
		AddSignal(sig);
		proc->state=WAIT_RT_RSP;
		printf("ntx.c::NTxInd()	���Է������ݣ�����·�ɣ��ȴ�ָʾ\n");
	}
	else//�����Է�������
	{
		proc->state=IDLE;
		printf("ntx.c::NTxInd() û�����ݷ�����\n");
	}
	return 0;
}
		
//void TimerNTx(NTx* proc)
//{
//	unsigned short i,size,k,btm,len,da,pri,sa;
//	unsigned long time_len;
//	Signal *sig;
//	proc->ntx_local_time++;//��ʱ��
//	
//	/*for(i=0;i<NODE_MAX_CNT;i++)
//	{
//		if(proc->rt_fail_tmr[i])
//		{
//			proc->rt_fail_tmr[i]--;
//		}
//	}*/
//
//	if(proc->ntx_local_time == NET_PKT_RESEND_FROM_FM_TMR)
//	{
//		proc->ntx_local_time = 0;
//		size = proc->net_que_list_fm.size;
//		if(size)
//		{
//			if((malloc_cnt1 < READ_FM_MALLOC_CNT_MAX)&&(!dat_tx_req_add_sig_flag))
//			{
//				btm = proc->net_que_list_fm.btm;
//				//ReadFlash(proc->read_net_pkt_from_fm, proc->net_que_list_fm.dat[btm], NET_FRM_LEN_MAX);
//				proc->net_que_list_fm.btm++;
//				//proc->net_que_list_fm.btm %= FLASH_MEM_BLOCK_CNT;
//				proc->net_que_list_fm.size--;
//				len = ((Net_Frm*)(proc->read_net_pkt_from_fm))->sn_len & 0x1ff;
//				da = (((Net_Frm*)(proc->read_net_pkt_from_fm))->pri_sa_da_mul_sn >>3)  & 0x1f;//Ŀ�ĵ�ַ
//				pri = (((Net_Frm*)(proc->read_net_pkt_from_fm))->pri_sa_da_mul_sn)>> 13 ;
//				sa = ((((Net_Frm*)(proc->read_net_pkt_from_fm))->pri_sa_da_mul_sn >> 8) & 0x1f);//Դ��ַ
//				if(sa != ((P_Entity*)proc->entity)->mib.local_id)//����ת��
//				{
//					sig=&proc->relay_tx_ind;
//					((Relay_Tx_Ind_Param*)sig->param)->xdat = proc->read_net_pkt_from_fm;
//					((Relay_Tx_Ind_Param*)sig->param)->xlen = len;
//					((Relay_Tx_Ind_Param*)sig->param)->xda=da;
//					((Relay_Tx_Ind_Param*)sig->param)->xpri= pri;
//					RelayTxInd(sig);
//				}
//				else
//				{
//					//AppTxReq(&proc->read_net_pkt_from_fm[NET_FRM_HEAD_LEN],len, da, pri,0);
//
//				}
//				//if(proc->rt_fail_tmr[da])
//				//{
//				//	AppTxReq(&proc->read_net_pkt_from_fm[NET_FRM_HEAD_LEN],len, da, pri,0);
//				//}
//			}
//		}
//		/*for(i=0;i<NODE_MAX_CNT;i++)
//		{
//			if(proc->net_que_list[i].size!=0)
//			{
//				size=proc->net_que_list[i].size;
//				for(k=0;k<size;k++)
//				{
//					btm=proc->net_que_list[i].btm;			
//					if(proc->ntx_local_time > proc->net_que_list[i].que_elmt[btm].arrvial_time)
//					{
//						time_len = proc->ntx_local_time - proc->net_que_list[i].que_elmt[btm].arrvial_time;
//					}
//					else
//					{
//						time_len = proc->ntx_local_time + (0xffffffff - proc->net_que_list[i].que_elmt[btm].arrvial_time);
//					}
//					if(time_len < NET_PKT_MAX_TTL)//δ��ʱ�����״������
//					{
//						break;	
//					}
//					else//��������ݳ�ʱ,ɾ��
//					{						
//						DeleteQueElmt(proc,i);
//					}
//				}
//			}		
//		}*/
//	}
//	return ;
//}

void NTxInit(NTx* proc)
{
	int i,k;
	proc->state=IDLE;
	proc->dat=0;
	proc->len=0;
	proc->da=0;
	proc->arrvial_time=0;
	proc->curt_node=0;
	proc->ntx_local_time=0;
	proc->sts=0;
	proc->ntx_sn_mul = 1;//��ʼ��ţ�=0
	proc->data_tx_req_rt_tx_ind_flag = 0;
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->ntx_sn[i]=1;//��ʼ��ţ�=0
		proc->net_que_list[i].btm=0;
		proc->net_que_list[i].top=0;
		proc->net_que_list[i].size=0;
		proc->net_que_list[i].curt_snd_flag=0;
		for(k=0;k<NET_QUE_MAX_NUM;k++)
		{
			proc->net_que_list[i].que_elmt[k].dat=0;
			proc->net_que_list[i].que_elmt[k].len=0;
			proc->net_que_list[i].que_elmt[k].ra=0;
			proc->net_que_list[i].que_elmt[k].da=0;
			proc->net_que_list[i].que_elmt[k].arrvial_time=0;
			proc->net_que_list[i].que_elmt[k].first_send_flag=0;
		}
	}
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->ntx_ind[i].next=0;
		proc->ntx_ind[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->ntx_ind[i].src=0;
		proc->ntx_ind[i].dst=proc;
		proc->ntx_ind[i].func=NTxInd;
		proc->ntx_ind[i].pri=SDL_PRI_URG;
		proc->ntx_ind[i].param=&proc->ntx_ind_param[i];
		//proc->rt_fail_tmr[i] = 0;
	}
	proc->net_que_list_fm.btm = 0;
	proc->net_que_list_fm.top = 0;
	proc->net_que_list_fm.size = 0;
}
void NTxSetup(NTx* proc)
{
	proc->data_tx_req.next=0;
	proc->data_tx_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->data_tx_req.src=0;
	proc->data_tx_req.dst=proc;
	proc->data_tx_req.func=DataTxReq;
	proc->data_tx_req.pri=SDL_PRI_URG;
	proc->data_tx_req.param=&proc->data_tx_req_param;

	proc->rt_tx_rsp.next=0;
	proc->rt_tx_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->rt_tx_rsp.src=0;
	proc->rt_tx_rsp.dst=proc;
	proc->rt_tx_rsp.func=RtTxRsp;
	proc->rt_tx_rsp.pri=SDL_PRI_URG;
	proc->rt_tx_rsp.param=&proc->rt_tx_rsp_param;

	proc->relay_tx_ind.next=0;
	proc->relay_tx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->relay_tx_ind.src=0;
	proc->relay_tx_ind.dst=proc;
	proc->relay_tx_ind.func=RelayTxInd;
	proc->relay_tx_ind.pri=SDL_PRI_URG;
	proc->relay_tx_ind.param=&proc->relay_tx_ind_param;


   
}
