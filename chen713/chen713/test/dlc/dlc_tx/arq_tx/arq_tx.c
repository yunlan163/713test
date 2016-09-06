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
//yun:等待ack定时器到
void TimerWaitAck(Arq_Tx* proc)//等待ack定时到
{
	unsigned short btm,i;
	for(i=0;i<proc->size;i++)//查询所有窗口
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
//yun:子函数  arq_tx_rsp发送单播
void ArqTxChkWinProc(Arq_Tx* proc)
{
	unsigned short i,j,btm;
	Signal* sig;
	Data_Sub_Frm *data_sub_frm;
	
	for(i=0;i<proc->size;i++)//查询所有窗口
	{
		if(!proc->tx_cnt[proc->idx])//多次重复失败  yun：某个窗口重传计数  最大重传20.用完了=0
		{
			sig = proc->frm_tx_free_ind;
			((T_Frm_Tx_Free_Ind_Param *)sig->param)->type = SEND_FAIL_TYPE_RESEND;
			AddSignal(sig);

			sig = proc->arq_tx_rsp;
			((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm = NULL;
			AddSignal(sig);

			btm = proc->btm;
			for(j=0;j<proc->size;j++)  //yun:全部释放？？
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
			if(proc->sts[proc->idx] == ARQ_STS_TX)//有数据
			{				
				data_sub_frm =(Data_Sub_Frm *) (proc->frm[proc->idx]);
				if(proc->rst_flag)//窗口刚刚重置，且一次DlcTxInd中，此arq_tx模块第一次发送数据
				{
					data_sub_frm->seg_len_sn_rst |= 0x0001;//arq重置标记  yun：第一次要置位
				}
				else
				{
					data_sub_frm->seg_len_sn_rst &= 0xfffe;//arq重置标记   yun：清零
				}
				sig = proc->arq_tx_rsp;  //yun:发数据
				((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm =(unsigned short *) proc->frm[proc->idx];
				AddSignal(sig);
				printf("arq_tx[%d]::ArqTxChkWinProc() 发第[%d]帧给dlc_tx_ctrl\n", proc->id, proc->idx);
				proc->sts[proc->idx] = ARQ_STS_WAIT_ACK; //yun:发送出去，改变状态
				proc->timer_wait_ack[proc->idx] = 0;//等待ACK定时
				proc->tx_cnt[proc->idx]--;//重发次数记录
				proc->idx++;
				proc->idx %= ARQ_SN_SIZE;
				if(proc->idx == proc->top)   //yun：到达窗顶8
				{
					proc->idx = proc->btm;
				}
				proc->state =IDLE;
				return  ;  //yun：这一窗口发出去了，返回
			}
			else//查找下一个窗口
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
	if(i == proc->size)//窗口无此长度数据
	{
		sig=proc->arq_tx_rsp;
		((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm = NULL;
		AddSignal(sig);

		proc->state =IDLE;
		return ;
	}
}
//yun:来自dlc_tx_ctrl 单播请求
static int ArqTxInd(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
	unsigned short i;
	if(proc->rst_flag)   //重置标志  两个节点第一次进行单播通信时需要带重置标志，收到第一次的ack回复后，清零
	{
		for(i=0;i<proc->size;i++)//查询所有窗口
		{
			if(proc->sts[i] == ARQ_STS_WAIT_ACK)   //yun:等待ack  返回
			{
				sig=proc->arq_tx_rsp;
				((T_Arq_Tx_Rsp_Param*)(sig->param))->xfrm = NULL;  //mac帧地址
				AddSignal(sig);

				proc->state =IDLE;
				return 0;
			}
		}
	}

	if(proc->size < ARQ_WIN_SIZE)//窗口<8，向frm_tx要数据
	{
		proc->frm[proc->top] = (Mac_Frm_1002 *)MemAllocMac();
		if(!proc->frm[proc->top])  //yun:没有申请到内存吗
		{
			if(proc->size)//窗口中有待发数据
			{
				ArqTxChkWinProc(proc);   //arq_tx_rsp
			}
			else//窗口中无待发数据
			{
				sig = proc->arq_tx_rsp;
				((T_Arq_Tx_Rsp_Param*)sig->param)->xfrm = NULL;
				AddSignal(sig);
			}
			return 0; //yun:
		}
		sig = proc->frm_tx_ind;  //yun:要数据
		//yun:要数据，放到申请到的内存地址中
		((T_Frm_Tx_Ind_Param*)sig->param)->dat_sub_frm = (unsigned short *)(proc->frm[proc->top]);
		AddSignal(sig);
		proc->state = WAIT_RSP;
		printf("arq_tx[%d]::ArqTxInd() 窗口<8，向frm_tx要数据\n",proc->id);
	}
	else//窗口=4，不向frm_tx要数据   8
	{
		ArqTxChkWinProc(proc);  //arq_tx_rsp
	}
	return 0;
}
//yun:组mac帧
static void MkMacFrm(Arq_Tx* proc,unsigned short len)//组mac帧
{
	Mac_Frm_1002 *mac_frm =  proc->frm[proc->top];  //yun:proc->frm[proc->top]:申请时的地址
	Data_Sub_Frm *data_sub_frm = (Data_Sub_Frm *)mac_frm;

	mac_frm->common_head = (((P_Entity*)proc->entity)->mib.local_id) << 11;  //yun:发送地址
	mac_frm->common_head |= (proc->id)<<6 ;   //yun：接收地址
	data_sub_frm->seg_len_sn_rst |= (proc->top <<1);  //yun:子帧序号

}
//yun：frm_tx发下来的数据  有、无数据  放进窗中
static int FrmTxRsp(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
	Frm_Tx_Rsp_Param* param=(Frm_Tx_Rsp_Param*)sig->param;
	
	if(param->xlen)//有数据   yun：包括网络2位帧头
	{
		//填MAC帧
		MkMacFrm(proc,param->xlen);
		printf("arq_tx[%d]::FrmTxRsp() 收到数据，填MAC帧\n",proc->id);
		proc->sts[proc->top] = ARQ_STS_TX;  //16个窗
		proc->tx_cnt[proc->top] = MAX_RESEND_CNT;  //20
		proc->frm_len[proc->top] = MAC_FRM_LEN_1002; //63 mac帧的总长度 payload:62+common_head:1
		proc->top ++;//窗顶后移
		proc->top %= ARQ_SN_SIZE;  //16
		proc->size ++;  //yun:已经存进来数据窗的个数
		if(proc->size < ARQ_WIN_SIZE)//窗口<4，向frm_tx要数据  yun:<8  重复程序
		{
			printf("arq_tx::FrmTxRsp() 窗口<8 申请内存，索要数据\n");
			proc->frm[proc->top] = (Mac_Frm_1002 *)MemAllocMac();
			if(!proc->frm[proc->top]) //yun:申请内存
			{
				printf("内存申请失败\n");
				if(proc->size)//窗口中有待发数据
				{
					printf("窗口中有待发数据，发出去\n");
					ArqTxChkWinProc(proc);
				}
				else//窗口中无待发数据
				{
					sig = proc->arq_tx_rsp;
					((T_Arq_Tx_Rsp_Param*)sig->param)->xfrm = NULL;
					AddSignal(sig);
					printf("窗口中没有数据，回复NULL\n");
				}
				return 0;
			}
			sig = proc->frm_tx_ind; //yun:窗不满，要数据
			((T_Frm_Tx_Ind_Param*)sig->param)->dat_sub_frm = (unsigned short *)(proc->frm[proc->top]);
			AddSignal(sig);
			proc->state = WAIT_RSP;
		}
		else//窗口=4，不向frm_tx要数据  yun：=8
		{
			printf("arq_tx[%d]::FrmTxRsp() 窗口为8 发送单播数据\n",proc->id);
			ArqTxChkWinProc(proc);
		}
	}
	else//frm_tx无数据
	{
		printf("arq_tx[%d]::FrmTxRsp() 上层无数据\n",proc->id);
		MemFreeMac((unsigned short *)proc->frm[proc->top]);
		proc->frm[proc->top] = NULL;
		if(proc->size)//窗口中有待发数据
		{
			printf("arq_tx[%d]::FrmTxRsp() 本窗口中有待发数据，发出去\n",proc->id);
			ArqTxChkWinProc(proc);
		
		}
		else//窗口中无待发数据
		{
			sig = proc->arq_tx_rsp;
			((T_Arq_Tx_Rsp_Param*)sig->param)->xfrm = NULL;
			AddSignal(sig);
			printf("arq_tx[%d]::FrmTxRsp() 本窗口中没有数据，回复NULL\n",proc->id);
		}
	}

	return 0;
}
//yun:ArqAckRxInd子函数 重置窗口
void WinRstProc(Arq_Tx* proc)//窗口重置处理
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
	((T_Frm_Tx_Free_Ind_Param *)sig->param)->type = SEND_FAIL_TYPE_RST;//发送失败类型，收到arq重置信令
	AddSignal(sig);

}
//yun:收到回复的ack信令
static int ArqAckRxInd(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
	Arq_Ack_Rx_Ind_Param* param=(Arq_Ack_Rx_Ind_Param*)sig->param;

	unsigned short rcv_sts,sn,succ_cnt,mask,tmp_top,tmp_sn,tmp_btm;
	Ack_Sub_Frm *ack_sgl = (Ack_Sub_Frm *)param->ack_sgl;
	printf("arq_tx[%d]:: ArqAckRxInd 收到ack\n",proc->id);
	if(proc->rst_flag)//发送了arq重置  yun：第一次发送的，收到ack
	{
		if(ack_sgl->flag_ra_rst & 0x0008)//重置响应  rst在bit3  对方回了rst
		{
			proc->rst_flag = 0;
		}
		else//无重置响应
		{
			MemFreeMac((unsigned short *)param->ack_sgl);//释放ack帧
			return 0;   //yun:直接返回
		}
	}
	else//由于收方连续收到两帧重置并回复ack,导致发放窗口状态有可能有成功标记
	{
		if(ack_sgl->flag_ra_rst & 0x0008)//重置响应
		{
			WinRstProc(proc);//需要重置发送窗口
			MemFreeMac((unsigned short *)param->ack_sgl);//释放ack帧
			return 0;
		}
	}
	succ_cnt = 0;
	sn = (ack_sgl->sn_sts>>12) & 0x000f;//ack中期望    arq序号:b15-b12
	rcv_sts = (ack_sgl->sn_sts>>5) & 0x007f;//ack中接收窗口状态   接收标志b11-b5
	tmp_btm = proc->btm;
	tmp_top = (proc->top  >= tmp_btm) ? (proc->top) : (proc->top + ARQ_SN_SIZE);//top差值  //yun:有可能下个窗口（下个16）
	tmp_sn = (sn >= tmp_btm) ? sn : (sn + ARQ_SN_SIZE);//top差值
	if(tmp_sn <= tmp_top && tmp_sn >=tmp_btm)//期望在发窗口中
	{
		while(!(proc->btm == sn))//移动窗底到sn处   不相等时执行,直到相等
		{
			proc->sts[proc->btm] = ARQ_STS_IDLE;
			if(proc->btm == proc->idx)//idx移至窗口中
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
		while(mask)//置窗口中的状态   yun：检查是否出现发送不成功情况，
		{
			sn++;
			sn %= ARQ_SN_SIZE;
			if(rcv_sts & mask) //yun：
			{
				if(proc->sts[sn] == ARQ_STS_IDLE)//不在窗口中，超出
				{
					WinRstProc(proc);//需要重置发送窗口
					MemFreeMac((unsigned short *)param->ack_sgl);//释臿ck帧
					return 0;
				}			
				proc->sts[sn] = ARQ_STS_SUCC;  //状态改变为succ yun:说明那个窗口接收成功了。
			}
			mask >>= 1;
		}
		//proc->sts[proc->btm] = ARQ_STS_TX;
		if(succ_cnt)//发送成功个数succ_cnt
		{
			sig = proc->arq_tx_succ_ind;
			((T_Arq_Tx_Succ_Ind_Param *)sig->param)->succ_cnt = succ_cnt;
			AddSignal(sig);
			printf("arq_tx[%d]:: ArqAckRxInd 是成功ack，给上层frm_tx回复\n",proc->id);
		}
	}
	else//期望不在窗口中 
	{
		 WinRstProc(proc);//窗口重置处理		
	}

	//MemFreeMac((unsigned short *)param->ack_sgl);//释放ack帧???????
	return 0;
}
//yun:收到重置
static int ArqRstRxInd(Signal* sig)
{
	Arq_Tx* proc=(Arq_Tx*)sig->dst;
//	Arq_Rst_Rx_Ind_Param* param=(Arq_Rst_Rx_Ind_Param*)sig->param;
	unsigned short i,btm;
	if(!proc->rst_flag)//非重置状态
	{
		btm = proc->btm;
		for(i=0;i<proc->size;i++)  //yun:所有窗口 释放内存
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
		((T_Frm_Tx_Free_Ind_Param *)sig->param)->type = SEND_FAIL_TYPE_RST;//发送失败类型，收到arq重置信令
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
	proc->rst_flag = 1;//重置标志
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



