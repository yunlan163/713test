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
//yun:来自dlc_tx_dump的网络包数据发送请求  单播、广播
static int FrmTxReq(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Frm_Tx_Req_Param* param=(Frm_Tx_Req_Param*)sig->param;
	
	unsigned short tmp_seg,i,len;
	unsigned short tmp_rem;
	proc->net_frm = param->xdat; //yun:数据地址
	proc->net_frm_len = param->xlen;  //yun:数据长度
	proc->seg_idx = 0;
	proc->succ_seg_num = 0;
	proc->da = ((((Net_Frm*)(proc->net_frm))->pri_sa_da_mul_sn)>>3) &0x1f; //源地址,目的地址,序号
	proc->sa = ((((Net_Frm*)(proc->net_frm))->pri_sa_da_mul_sn)>>8) &0x1f;
	if( proc->net_frm_len  == 0)//无数据
	{
		return 0;
	}
	//计算分段长度
	tmp_seg = proc->net_frm_len / DATA_SUB_FRM_PALOAD_LEN_1002; //61
	tmp_rem = proc->net_frm_len % DATA_SUB_FRM_PALOAD_LEN_1002;
	if(tmp_rem)
	{
		tmp_seg += 1;
	}
	//if(tmp_rem > (DATA_SUB_FRM_PALOAD_LEN_1002 - 15))//每个MAC包末尾省空间用于携带信令
	//{
	//	tmp_seg += 1;
	//}
	proc->seg_num = tmp_seg;
	//保存分段长度
	len = proc->net_frm_len / proc->seg_num;
	tmp_rem = proc->net_frm_len % proc->seg_num;
	for(i=0;i<tmp_seg;i++)  //yun:平均长度
	{
		proc->seg_len[i] = len;
	}
	for(i=0;i<tmp_rem;i++)
	{
		proc->seg_len[i] +=1;  //yun：前tmp_rem个长度+1；
	}
	proc->state = TX;//发送数据状态   yun：分包完，改变至TX状态
	printf("frm_tx[%d]::FrmTxReq() 分包完毕，等待发送\n",proc->id);
//	printdat_once[18]= proc->seg_num + 0x30;
	return 0;
}
//yun:下层dlc_tx_ctrl（广播）或者arq_tx（单播）请求索要数据
static int FrmTxInd(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Frm_Tx_Ind_Param* param=(Frm_Tx_Ind_Param*)sig->param;
	
	unsigned short i;
	Data_Sub_Frm* data_sub_frm;
	unsigned short* data = proc->net_frm;  //yun:已经保存的网络帧地址
	if(proc->state == IDLE)//无待发数据
	{
		sig = proc->frm_tx_rsp;
		((T_Frm_Tx_Rsp_Param*)sig->param)->xlen = 0;
		AddSignal(sig);
		printf("---节点[%d]---frm_tx[%d]::FrmTxInd() 没有数据要给\n",((P_Entity*)proc->entity)->mib.local_id,proc->id);
	}
	else//TX状态，有待发数据
	{
		data_sub_frm = (Data_Sub_Frm *)param->dat_sub_frm;
		data_sub_frm ->seg_len_sn_rst = 0;
		data_sub_frm ->seg_len_sn_rst |= (proc->seg_len[proc->seg_idx])<<5;
		//填帧头的段标记
		if(proc->seg_idx == 0)//头帧    yun：seg_idx;//发送到哪个位置
		{
			data_sub_frm ->seg_len_sn_rst |= FIRST_SEG_FLAG;  //0x4000
		}
		if(proc->seg_idx == proc->seg_num - 1)//尾帧
		{
			data_sub_frm ->seg_len_sn_rst |= LAST_SEG_FLAG;   //0x2000
		}
		if(proc->id == ((P_Entity*)proc->entity)->mib.local_id)//广播数据序号
		{
			data_sub_frm ->seg_len_sn_rst |= (proc->bc_seg_num  << 1);
			proc->bc_seg_num ++;//bc段序号，用于广播包序号
			proc->bc_seg_num &= 0xf;//yun:最大15包,不是同一网络数据包序号也是直接累加的
		}
		for(i=0;i<proc->seg_idx;i++)//数据发送到的位置
		{
			data += proc->seg_len[i];
		}
		for(i=0;i<DATA_SUB_FRM_PALOAD_LEN_1002;i++)//将数据保存入窗口  yun：放进地址中
		{
			if(i<proc->seg_len[proc->seg_idx])
			{
				data_sub_frm->data[i] = *(data++); //yun：拷贝到传上来的地址中  前两位是网络帧头
			}
			else
			{
				data_sub_frm->data[i] = 0;//yun：后面的为0
			}
		}
		sig = proc->frm_tx_rsp;//回复单播arq_tx/广播dlc_tx_ctrl
		((T_Frm_Tx_Rsp_Param*)sig->param)->xlen = proc->seg_len[proc->seg_idx];
		AddSignal(sig);
		printf("frm_tx[%d]::FrmTxInd() 有数据，给第[%d]帧\n", proc->id, proc->seg_idx);
		proc->seg_idx ++;   //yun:发送到的位置+1  帧的位置

		if(proc->seg_idx == proc->seg_num)//本包网络数据向arq_tx发送结束
		{
			proc->state = IDLE;
			printf("frm_tx[%d]::FrmTxInd() 数据帧给完了\n",proc->id);
			printf("\n");
		}
		//广播数据认为发送成功   yun：广播数据发下去就认为成功，没有ack确认
		if(proc->id == ((P_Entity*)proc->entity)->mib.local_id)
		{
			printf("frm_tx::FrmTxInd() 广播包成功发送第[%d]个\n",proc->succ_seg_num);
			proc->succ_seg_num ++;//发送成功个数更新
			if(proc->succ_seg_num == proc->seg_num)//本网络包发送成功
			{
				sig = proc->frm_tx_cfm;
				((T_Frm_Tx_Cfm_Param*)sig->param)->id = proc->id;
				((T_Frm_Tx_Cfm_Param*)sig->param)->succ_fail_flag = 1;//成功
				((T_Frm_Tx_Cfm_Param*)sig->param)->fail_type = 0;//成功时无效
				((T_Frm_Tx_Cfm_Param*)sig->param)->net_frm = proc->net_frm;
				((T_Frm_Tx_Cfm_Param*)sig->param)->da = proc->da;
				((T_Frm_Tx_Cfm_Param*)sig->param)->sa = proc->sa;
				AddSignal(sig);
				printf("frm_tx::FrmTxInd() 本网络包广播发送完毕，回复上层dlc_tx_dump\n");
			}
		}
		if((proc->id == ((P_Entity*)proc->entity)->mib.local_id) && (proc->state == IDLE)&&(proc->succ_seg_num != proc->seg_num))
		{
			//PrintDat(0,0,0xaaaa);
		}
	}
	return 0;
}
//yun:请求释放数据包
static int FrmTxFreeInd(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Frm_Tx_Free_Ind_Param* param=(Frm_Tx_Free_Ind_Param*)sig->param;

	proc->seg_idx = 0;
	proc->seg_num = 0;
	proc->succ_seg_num = 0;

	sig = proc->frm_tx_cfm;
	((T_Frm_Tx_Cfm_Param*)sig->param)->id = proc->id;
	((T_Frm_Tx_Cfm_Param*)sig->param)->succ_fail_flag = 0;//失败
	((T_Frm_Tx_Cfm_Param*)sig->param)->fail_type = param->fail_type ;//失败类型
	((T_Frm_Tx_Cfm_Param*)sig->param)->net_frm = proc->net_frm;
	((T_Frm_Tx_Cfm_Param*)sig->param)->da = proc->da;
	((T_Frm_Tx_Cfm_Param*)sig->param)->sa = proc->sa;
	AddSignal(sig);
	proc->state = IDLE;
	return 0;
}
//yun:单播帧发送成功处理函数  成功发送一次就返回一次，记录成功次数  接收方返回ack后
static int ArqTxSuccInd(Signal *sig)
{
	Frm_Tx* proc=(Frm_Tx*)sig->dst;
	Arq_Tx_Succ_Ind_Param* param=(Arq_Tx_Succ_Ind_Param*)sig->param;
	printf("frm_tx[%d]:: ArqTxSuccInd 单播发送成功[%d]\n",proc->id,proc->succ_seg_num);
	proc->succ_seg_num += param->succ_cnt;//发送成功个数更新
	
	if(proc->succ_seg_num == proc->seg_num)//本网络包发送成功
	{
		sig = proc->frm_tx_cfm;
		((T_Frm_Tx_Cfm_Param*)sig->param)->id = proc->id;
		((T_Frm_Tx_Cfm_Param*)sig->param)->succ_fail_flag = 1;//成功
		((T_Frm_Tx_Cfm_Param*)sig->param)->fail_type = 0;//成功时无效
		((T_Frm_Tx_Cfm_Param*)sig->param)->net_frm = proc->net_frm;
		((T_Frm_Tx_Cfm_Param*)sig->param)->da = proc->da;
		((T_Frm_Tx_Cfm_Param*)sig->param)->sa = proc->sa;
		AddSignal(sig);
		printf("frm_tx[%d]:: ArqTxSuccInd 单播网络包发送完毕，给上层发送确认\n",proc->id);
	}else{
		//yun:如果没有发送完，等着要数据继续发送
		printf("frm_tx[%d]:: ArqTxSuccInd 等着下层要数据\n", proc->id);
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

