#include "rt_ctrl.h"

unsigned long HELLO_SEND_TIMER = 800;//hello包发送周期

enum {IDLE,WAIT_CFM};

typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
extern void UploadRtInfo(Rt_Ctrl* proc);
extern void UploadIPInfo(Rt_Ctrl* proc);
unsigned short IpAddrToMacAddr(Rt_Ctrl* proc, unsigned short ip)
{
	unsigned short i;
	for(i = 0; i < NODE_MAX_CNT; i++)
	{
	 	if((proc->ip_map_table[i] & 0xff) == ip)
		{
			return i;
		}
	}
	return 0xff;
}
//在hello原始信息表重新查找路由
unsigned short  HelloInfoTableChkRt(Rt_Ctrl* proc,unsigned short id)
{
	unsigned short tmp_rt_metric = (proc->rt_table[id].tflag_metric_next >> 8) & 0x1f;
	unsigned short tmp_rt_seq = proc->rt_table[id].seq;
	unsigned short replace_seq,replace_id,replace_metric,i;
	//跳数相同时无论序号新旧，跳数大时选序号必须新的（为了防止环路）
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		if(((proc->hello_info_table[i][id].metric_next>>8) & 0x1f) == METRIC_INFINITE)//i节点不可达
		{
			continue;
		}
		if(((((proc->hello_info_table[i][id].metric_next>>8) & 0x1f)+1) == tmp_rt_metric)||(((unsigned short)(proc->hello_info_table[i][id].seq - tmp_rt_seq)<128) &&((unsigned short)(proc->hello_info_table[i][id].seq != tmp_rt_seq))))//跳数相同或者序号新
		{
			replace_seq = proc->hello_info_table[i][id].seq;
			replace_id = i;
			replace_metric = ((proc->hello_info_table[i][id].metric_next>>8) & 0x1f)+1;
			for(i=0;i<NODE_MAX_CNT;i++)//查找最优，跳数小、序号新
			{
				if(((proc->hello_info_table[i][id].metric_next>>8) & 0x1f) == METRIC_INFINITE)
				{
					continue;
				}
				if((((proc->hello_info_table[i][id].metric_next>>8) & 0x1f) + 1) > replace_metric)//，序号是新的，但是跳数大，不用
				{
					continue;
				}
				else if((((proc->hello_info_table[i][id].metric_next>>8) & 0x1f)+1) < replace_metric)//跳数小，比临时替代节点的跳数小，而且也比路由表中的跳数也小，但是序号是旧的，也不用。
				{
					if((((proc->hello_info_table[i][id].metric_next>>8) & 0x1f)+1) > tmp_rt_metric)//找到小的跳数，但是序号是旧的不用
					{
						if((unsigned short)(proc->hello_info_table[i][id].seq - tmp_rt_seq) > 128)//序号旧
						{
							continue;
						}
					}
				}
				else if((((proc->hello_info_table[i][id].metric_next>>8) & 0x1f)+1) == replace_metric)//跳数相等，序号为旧的，不用
				{
					if(proc->hello_info_table[i][id].seq <= replace_seq)//序号旧
					{
						continue;
					}
				}
				replace_metric = ((proc->hello_info_table[i][id].metric_next>>8) & 0x1f)+1;
				replace_id = i;
				replace_seq = proc->hello_info_table[i][id].seq;
			}
			//找到，更新路由表
			proc->rt_table[id].tflag_metric_next &= 0x8000;
			proc->rt_table[id].tflag_metric_next |= replace_metric<<8;
			proc->rt_table[id].tflag_metric_next |= replace_id;
			proc->rt_table[id].seq = replace_seq;
			return 1;
		}
	}
	//没找到，更新路由表
	proc->rt_table[id].tflag_metric_next = 0x8000;
	proc->rt_table[id].tflag_metric_next |= METRIC_INFINITE<<8;
	proc->rt_table[id].tflag_metric_next |= 255;
	proc->rt_table[id].seq++;
	return 0;
}

static int HelloRtRxInd(Signal* sig)//在topoctrl中，已确定是双向连接关系的，所以路由信息直接填写1跳
{
	Rt_Ctrl* proc = (Rt_Ctrl*)sig->dst;
	Hello_Rt_Rx_Ind_Param* param = (Hello_Rt_Rx_Ind_Param*)sig->param;

	RTI_Sub_Frm* hello_pkt = (RTI_Sub_Frm*)param->hello_pkt;
	unsigned short i,succ,tmp_id,hello_cnt = ((hello_pkt->flag_cnt_pb >> 1) & 0xff);
	unsigned short ta = param->xta;
	unsigned short seq_cnt;
	//更新hello包的发送节点路由信息
	proc->rt_table[ta].tflag_metric_next &= ~(0x7fff);//把本地路由表中该发节点的路由信息清除，下面重新填写相关信息。
	proc->rt_table[ta].tflag_metric_next |= 1<<8;//标记跳数为1跳
	proc->rt_table[ta].tflag_metric_next |= ta;//下一跳为发送节点
	proc->rt_table[ta].seq = hello_pkt->seq;//序号为hello包中序号
	proc->ip_map_table[ta] = (((unsigned long)(*(((unsigned short*)hello_pkt)+4))) << 16) | (*(((unsigned short*)hello_pkt)+5));//5509
	//proc->ip_map_table[ta] = hello_pkt->ip;//vs2005适用
	//操作hello包中各路由项的节点路由
	for(i=0; i<hello_cnt; i++)
	{
		//更新原始hello信息表
		tmp_id = ((hello_pkt->rt_items[i].dest_next_metric)>>11);
		proc->hello_info_table[ta][tmp_id].metric_next = (hello_pkt->rt_items[i].dest_next_metric & 0x001f)<<8;
		proc->hello_info_table[ta][tmp_id].metric_next |= ((hello_pkt->rt_items[i].dest_next_metric) >> 6 ) & 0x1f;
		proc->hello_info_table[ta][tmp_id].seq = hello_pkt->rt_items[i].seq;
		
		if(tmp_id == ((P_Entity*)proc->entity)->mib.local_id)
		{
			continue;
		}
		if((proc->hello_info_table[ta][tmp_id].metric_next >> 8) != METRIC_INFINITE)//hello包发送节点可以到达
		{
			if(((proc->rt_table[tmp_id].tflag_metric_next) & 0x1f) == ta)//下一跳为hello包发送节点,更新序号和跳数
			{
				proc->rt_table[tmp_id].tflag_metric_next &= 0x8000;
				proc->rt_table[tmp_id].tflag_metric_next |= (((proc->hello_info_table[ta][tmp_id].metric_next)>>8) + 1) << 8;
				proc->rt_table[tmp_id].tflag_metric_next |= ta;
				proc->rt_table[tmp_id].seq = proc->hello_info_table[ta][tmp_id].seq;
				//更新ip影射表
				proc->ip_map_table[tmp_id] = hello_pkt->rt_items[i].ip;
			}
			//if((((proc->hello_info_table[ta][tmp_id].metric_next)>>8) + 1) < ((proc->rt_table[tmp_id].tflag_metric_next >> 8) & 0x1f))//跳数小（不选序号新，避免路由波动）
			seq_cnt = (proc->hello_info_table[ta][tmp_id].seq > proc->rt_table[tmp_id].seq ) ? (proc->hello_info_table[ta][tmp_id].seq - proc->rt_table[tmp_id].seq) : (proc->hello_info_table[ta][tmp_id].seq + 0xffff - proc->rt_table[tmp_id].seq);//2013.1.1
			if((0xff > seq_cnt && seq_cnt> 2)||(((proc->hello_info_table[ta][tmp_id].metric_next)>>8) + 1) < ((proc->rt_table[tmp_id].tflag_metric_next >> 8) & 0x1f))//2013.1.1 条数小 or 路由新 
			{
				//更新路由表该项
				proc->rt_table[tmp_id].tflag_metric_next &= 0x8000;
				proc->rt_table[tmp_id].tflag_metric_next |= (((proc->hello_info_table[ta][tmp_id].metric_next)>>8) + 1) << 8;
				proc->rt_table[tmp_id].tflag_metric_next |= ta;
				proc->rt_table[tmp_id].seq = proc->hello_info_table[ta][tmp_id].seq;
				//更新ip影射表
				proc->ip_map_table[tmp_id] = hello_pkt->rt_items[i].ip;
				//sprintf(tmp_short,"路由表 %d 条目由更新，序号%d 、跳数%d、下一跳%d\n",tmp_id,proc->hello_info_table[ta][tmp_id].seq ,(proc->rt_table[tmp_id].tflag_metric_next >> 8) & 0x1f,ta);
				//DbgPrint(tmp_short);
			}
		}
		else//hello包发送节点不可以到达
		{
			if((proc->rt_table[tmp_id].tflag_metric_next & 0x1f) == ta)//下一跳为hello包发送节点,重新查找路由
			{
				succ = HelloInfoTableChkRt(proc,tmp_id);//在hello原始信息表重新查找路由并更新路由表
				//sprintf(tmp_short,"路由表 %d 条目由失效，更新与否%d，更新后序号%d 、跳数%d、下一跳%d\n",tmp_id,succ,proc->rt_table[tmp_id].seq ,(proc->rt_table[tmp_id].tflag_metric_next >> 8) & 0x1f,proc->rt_table[tmp_id].tflag_metric_next & 0x1f);
				//DbgPrint(tmp_short);
			}
		}
	}
	//判断是否需要发送hello包
	if(!proc->hello_topo_tx_req_add_sig_flag)
	{
		for(i=0; i<NODE_MAX_CNT; i++)
		{
			if(proc->rt_table[i].tflag_metric_next & 0x8000)
			{
				sig = proc->hello_topo_tx_req;//请求填写邻节点信息
				((T_Hello_Topo_Tx_Req_Param*)sig->param)->dat = &proc->rti_sub_frm.nbr_info;
				AddSignal(sig);
				proc->state = WAIT_CFM;
				proc->hello_topo_tx_req_add_sig_flag = 1;
				proc->rt_table_chg_tx_to_arm_flag = 1;//路由变化，送至ARM
				proc->ip_table_chg_tx_to_arm_flag = 1;//路由变化，送至ARM
				break;
			}
		}
	}
	//释放interface中malloc的HELLO包
	MemFreeMac((param->hello_pkt) - NET_FRM_HEAD_LEN - DATA_SUB_FRM_HEAD_LEN);
	return 0;
}

static int HelloTopoTxCfm(Signal* sig)//topo 控制模块填写完邻节点信息后，回复rtctrl，rtctrl填写更改的节点路由信息，组成hello包
{
	Rt_Ctrl* proc=(Rt_Ctrl*)sig->dst;
	//Hello_Topo_Tx_Cfm_Param* param=(Hello_Topo_Tx_Cfm_Param*)sig->param;

	unsigned short i,k=0;

	proc->hello_topo_tx_req_add_sig_flag = 0;
	//PrintDat(0,0,0x0007);
	//填hello包中的路由条目
	for(i=0; i<NODE_MAX_CNT; i++)
	{
		if(proc->rt_table[i].tflag_metric_next & 0x8000)
		{
			proc->rti_sub_frm.rt_items[k].dest_next_metric = i<<11;
			proc->rti_sub_frm.rt_items[k].dest_next_metric |= ((proc->rt_table[i].tflag_metric_next)&0x1f) <<6;
			proc->rti_sub_frm.rt_items[k].dest_next_metric |= (proc->rt_table[i].tflag_metric_next>>8) &0x1f;
			proc->rti_sub_frm.rt_items[k].seq = proc->rt_table[i].seq;
			proc->rti_sub_frm.rt_items[k].ip = proc->ip_map_table[i];
			k++;
			proc->rt_table[i].tflag_metric_next &= 0x7fff;
			if((k * 4 )>(DATA_SUB_FRM_PALOAD_LEN_1002 - RTI_SUB_FRM_HEAD_LEN - NET_FRM_HEAD_LEN - 4))
			{
				break;
			}
		}
	}
	//填hello包中的帧头
	proc->rti_sub_frm.flag_cnt_pb = SUB_FRM_TYPE_RTI;
	proc->rti_sub_frm.flag_cnt_pb |= k<<1;
	proc->rti_sub_frm.seq = proc->rt_table[((P_Entity*)proc->entity)->mib.local_id].seq;
	proc->rti_sub_frm.ip = proc->ip_map_table[((P_Entity*)proc->entity)->mib.local_id];
	proc->rt_table[((P_Entity*)proc->entity)->mib.local_id].seq += 2;

	//sig=proc->hello_tx_req;//发送hello包
	//((T_Hello_Tx_Req_Param *)sig->param)->rti_sub_frm = (unsigned short *)&proc->rti_sub_frm;
	//((T_Hello_Tx_Req_Param *)sig->param)->xlen = k + k + 4;
	//AddSignal(sig);
	//Hello_Tx_Req((unsigned short *)&proc->rti_sub_frm,k + k + 4,((P_Entity*)proc->entity)->mib.local_id);//for test
	if(!dat_tx_req_add_sig_flag)
	{
		//AppTxReq((unsigned short *)&proc->rti_sub_frm,k * 4 + 6,((P_Entity*)proc->entity)->mib.local_id,0,1);//应用层请求网络层发送数据
	}
	proc->state=IDLE;
	return 0;
}

static int NbrStsChgInd(Signal* sig)////如果topo控制发现为单项链接，则把该节点的路由信息全部设置为无效;如果本地路由表中下一跳有该节点，则需要重新查找路由;给topo发请求，主要用于填写hello包中的邻节点信息
{
	unsigned short rt_table_chg_flag = 0;
	Rt_Ctrl* proc = (Rt_Ctrl*)sig->dst;
	Nbr_Sts_Chg_Ind_Param* param = (Nbr_Sts_Chg_Ind_Param*)sig->param;
	
	unsigned short  i,succ;
	if(!param->xsucc)
	{
		for(i=0;i<NODE_MAX_CNT;i++)//如果topo控制发现为单项链接，则把该节点的路由信息全部设置为无效
		{
			proc->hello_info_table[param->xnbr][i].metric_next = METRIC_INFINITE<<8;
			proc->hello_info_table[param->xnbr][i].metric_next |= 255;
			proc->hello_info_table[param->xnbr][i].seq++;
			
		}
		for(i=0;i<NODE_MAX_CNT;i++)
		{
			if(((proc->rt_table[i].tflag_metric_next>>8) & 0x1f) != METRIC_INFINITE)
			{
				if((proc->rt_table[i].tflag_metric_next & 0x1f) == param->xnbr)//如果本地路由表中下一跳有该节点，则需要重新查找路由
				{
					succ = HelloInfoTableChkRt(proc,i);
					//sprintf(tmp_short,"路由表 %d 条目由失效，更新与否%d，更新后序号%d 、跳数%d、下一跳%d\n",i,succ,proc->rt_table[i].seq ,(proc->rt_table[i].tflag_metric_next >> 8) &0x1f,proc->rt_table[i].tflag_metric_next & 0x1f);
					//DbgPrint(tmp_short);
				}
			}
		}
		if(!proc->hello_topo_tx_req_add_sig_flag)//邻节点链路变为单向，则需要给topo发请求，主要用于填写hello包中的邻节点信息
		{
			sig = proc->hello_topo_tx_req;//请求填写邻节点信息
			((T_Hello_Topo_Tx_Req_Param*)sig->param)->dat = &proc ->rti_sub_frm.nbr_info;
			AddSignal(sig);
			proc->state = WAIT_CFM;
			proc->hello_topo_tx_req_add_sig_flag = 1;
			proc->rt_table_chg_tx_to_arm_flag = 1;//路由变化，送至ARM
		}
	}

	return 0;
}
//yun:ntx发送的单播数据，需要查询路由
static int RtTxInd(Signal* sig)//网络发模块查找下一跳地址
{
	Rt_Ctrl* proc=(Rt_Ctrl*)sig->dst;
	Rt_Tx_Ind_Param* param=(Rt_Tx_Ind_Param*)sig->param;
	unsigned short i,succ=0;
	unsigned short tmp_ra;
	//PrintDat(0,0,0x0008);
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		if(i==param->da)//yun:找到目的地址  路由表
		{
			if(((proc->rt_table[i].tflag_metric_next >> 8) & 0x1f) != METRIC_INFINITE)
			{
				tmp_ra = proc->rt_table[i].tflag_metric_next  & 0x1f;  //yun:查到了路由路径
				succ = 1;
			}
			else
			{
				tmp_ra=METRIC_INFINITE;  // 31 跳数无穷大，没有路由能通
				succ = 0;
			}
			break;
		}
		
	}
	sig=proc->rt_tx_rsp;
	//((T_Rt_Tx_Rsp_Param*)sig->param)->ra=tmp_ra;  //yun:下一跳的地址
	//((T_Rt_Tx_Rsp_Param*)sig->param)->succ = succ;
	((T_Rt_Tx_Rsp_Param*)sig->param)->ra=param->da;//for tmp test 2012.10.4  yun
	((T_Rt_Tx_Rsp_Param*)sig->param)->succ = 1;//for tmp test 2012.10.4  yun
	((T_Rt_Tx_Rsp_Param*)sig->param)->da=param->da;//目的地址
	AddSignal(sig);
	return 0;
}

static int VRelayChkReq(Signal* sig)
{
	Rt_Ctrl* proc=(Rt_Ctrl*)sig->dst;
	VRelay_Chk_Req_Param* param=(VRelay_Chk_Req_Param*)sig->param;

	unsigned short sa = param->sa;
	unsigned short i,succ_falg = 0;

	for(i=0; i<NODE_MAX_CNT; i++)
	{
		//邻节点
		if((proc->rt_table[i].tflag_metric_next & 0x1f) == i)//从邻节点中查找可以到达目的节点
		{
			//我为中继到源
			if((proc->hello_info_table[i][sa].metric_next & 0x1f) == ((P_Entity*)proc->entity)->mib.local_id)
			{
				succ_falg = 1;
				break;
			}
		}

	}	
	sig = proc->vrelay_chk_cfm;
	((T_VRelay_Chk_Cfm_Param*)sig->param)->succ_flag = succ_falg;
	AddSignal(sig);
	return 0;
}
static int DRelayChkReq(Signal* sig)
{
	Rt_Ctrl* proc=(Rt_Ctrl*)sig->dst;
	DRelay_Chk_Req_Param* param=(DRelay_Chk_Req_Param*)sig->param;

	unsigned short sa = param->sa;
	unsigned short i,succ_falg = 0;

	for(i=0; i<NODE_MAX_CNT; i++)
	{
		//邻节点
		if((proc->rt_table[i].tflag_metric_next & 0x1f) == i)
		{
			//我为中继到源
			if((proc->hello_info_table[i][sa].metric_next & 0x1f) == ((P_Entity*)proc->entity)->mib.local_id)
			{
				succ_falg = 1;
				break;
			}
		}
	}	
	sig = proc->drelay_chk_cfm;
	((T_DRelay_Chk_Cfm_Param*)sig->param)->succ_flag = succ_falg;
	AddSignal(sig);
	return 0;
	
}
//void TimerRtCtrl(Rt_Ctrl* proc)
//{
//	unsigned short i,end_node;
//	Signal *sig;
//	if(!proc->hello_topo_tx_req_add_sig_flag)
//	{
//		if(proc->hello_send_timer)
//		{
//			proc->hello_send_timer--;
//			if(!proc->hello_send_timer)//hello包发送周期到，发送hello包
//			{
//				proc->hello_send_timer = HELLO_SEND_TIMER; //+ rand()%(HELLO_SEND_TIMER/10);
//				end_node = (proc->hello_snd_timer_idx + 1) * HELLO_PKT_NODE_CNT_PER_MAC_FRM;
//				end_node = end_node > NODE_MAX_CNT ? NODE_MAX_CNT : end_node;
//				for(i = proc->hello_snd_timer_idx * HELLO_PKT_NODE_CNT_PER_MAC_FRM;i<end_node;i++)
//				{
//					if(i != ((P_Entity*)proc->entity)->mib.local_id)
//					{
//						proc->rt_table[i].tflag_metric_next |= 0x8000;
//					}
//				}
//				proc->hello_snd_timer_idx ++;
//				proc->hello_snd_timer_idx = proc->hello_snd_timer_idx % ((NODE_MAX_CNT/HELLO_PKT_NODE_CNT_PER_MAC_FRM) + 1);
//				sig = proc->hello_topo_tx_req;//请求填写邻节点信息
//				((T_Hello_Topo_Tx_Req_Param*)sig->param)->dat = &proc->rti_sub_frm.nbr_info;
//				AddSignal(sig);
//				proc->state = WAIT_CFM;
//				proc->hello_topo_tx_req_add_sig_flag = 1;
//				//if(proc->rt_table_chg_tx_to_arm_flag)
//				{
//					UploadRtInfo(proc);
//					proc->rt_table_chg_tx_to_arm_flag = 0;
//				}
//				//else if(proc->ip_table_chg_tx_to_arm_flag)
//				{
//					UploadIPInfo(proc);
//					proc->ip_table_chg_tx_to_arm_flag = 0;
//				}
//			}
//		}
//	}
//
//}
void RtCtrlInit(Rt_Ctrl* proc)
{
	int i,j;
	proc->state=IDLE;
	proc->hello_send_timer = HELLO_SEND_TIMER; //+ rand()%(HELLO_SEND_TIMER/10);
	proc->hello_topo_tx_req_add_sig_flag = 0;
	proc->rt_table_chg_tx_to_arm_flag = 1;
	proc->ip_table_chg_tx_to_arm_flag = 0;
	proc->hello_snd_timer_idx = 0;
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		for(j=0;j<NODE_MAX_CNT;j++)
		{
			proc->hello_info_table[i][j].metric_next = (METRIC_INFINITE<<8) | 255;
			proc->hello_info_table[i][j].seq = 1;
		}
		proc->rt_table[i].tflag_metric_next = (METRIC_INFINITE<<8) | 255;
		proc->rt_table[i].seq = 1;
		proc->ip_map_table[i] = 0xffffffff;
		
	}
	proc->rt_table[((P_Entity*)proc->entity)->mib.local_id].tflag_metric_next = (((P_Entity*)proc->entity)->mib.local_id);
	proc->rt_table[((P_Entity*)proc->entity)->mib.local_id].seq = 2;
	proc->ip_map_table[((P_Entity*)proc->entity)->mib.local_id] = ((P_Entity*)proc->entity)->mib.local_id;
	
}


void RtCtrlSetup(Rt_Ctrl* proc)
{
	proc->hello_rt_rx_ind.next=0;
	proc->hello_rt_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->hello_rt_rx_ind.src=0;
	proc->hello_rt_rx_ind.dst=proc;
	proc->hello_rt_rx_ind.func=HelloRtRxInd;
	proc->hello_rt_rx_ind.pri=SDL_PRI_URG;
	proc->hello_rt_rx_ind.param=&proc->hello_rt_rx_ind_param;

	proc->nbr_sts_chg_ind.next=0;
	proc->nbr_sts_chg_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->nbr_sts_chg_ind.src=0;
	proc->nbr_sts_chg_ind.dst=proc;
	proc->nbr_sts_chg_ind.func=NbrStsChgInd;
	proc->nbr_sts_chg_ind.pri=SDL_PRI_URG;
	proc->nbr_sts_chg_ind.param=&proc->nbr_sts_chg_ind_param;
	
	proc->hello_topo_tx_cfm.next=0;
	proc->hello_topo_tx_cfm.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->hello_topo_tx_cfm.src=0;
	proc->hello_topo_tx_cfm.dst=proc;
	proc->hello_topo_tx_cfm.func=HelloTopoTxCfm;
	proc->hello_topo_tx_cfm.pri=SDL_PRI_URG;
	proc->hello_topo_tx_cfm.param=&proc->hello_topo_tx_cfm_param;

	proc->rt_tx_ind.next=0;
	proc->rt_tx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->rt_tx_ind.src=0;
	proc->rt_tx_ind.dst=proc;
	proc->rt_tx_ind.func=RtTxInd;
	proc->rt_tx_ind.pri=SDL_PRI_URG;
	proc->rt_tx_ind.param=&proc->rt_tx_ind_param;

	proc->drelay_chk_req.next=0;
	proc->drelay_chk_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->drelay_chk_req.src=0;
	proc->drelay_chk_req.dst=proc;
	proc->drelay_chk_req.func=DRelayChkReq;
	proc->drelay_chk_req.pri=SDL_PRI_URG;
	proc->drelay_chk_req.param=&proc->drelay_chk_req_param;

	proc->vrelay_chk_req.next=0;
	proc->vrelay_chk_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->vrelay_chk_req.src=0;
	proc->vrelay_chk_req.dst=proc;
	proc->vrelay_chk_req.func=VRelayChkReq;
	proc->vrelay_chk_req.pri=SDL_PRI_URG;
	proc->vrelay_chk_req.param=&proc->vrelay_chk_req_param;

	//proc->hello_tx_cfm.next=0;
	//proc->hello_tx_cfm.sdlc=&((P_Entity*)proc->entity)->sdlc;
	//proc->hello_tx_cfm.src=0;
	//proc->hello_tx_cfm.dst=proc;
	//proc->hello_tx_cfm.func=HelloTxCfm;
	//proc->hello_tx_cfm.pri=SDL_PRI_URG;
	//proc->hello_tx_cfm.param=&proc->hello_tx_cfm_param;
}

