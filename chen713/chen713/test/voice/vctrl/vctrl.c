#include "vctrl.h"
#include <string.h>
//#include "../../../../j1052_gpio.h"

enum {IDLE, WAIT_CFM};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
extern void RsvCnclReq(unsigned short rsv_bsn_num);
extern void RsvSlotReq(unsigned short rsv_bsn_num,unsigned short ra,unsigned short len);
extern unsigned short curt_ptt_sts;


//预约或取消预约函数
//void RsvCnclProc(VCtrl *proc)
//{
//	Signal *sig;
//	unsigned short type;
//	if(proc->local_node_tx_flag)//本节点在发
//	{
//		if(proc->rsv_type == OFF_RSV)
//		{
//			//RsvSlotReq(RSV_VOICE_BSN_NUM,((P_Entity*)proc->entity)->mib.local_id,3);
//			proc->state = WAIT_CFM;	
//			proc->rsv_type = LOCAL_RSV;			
//		}
//		else if(proc->rsv_type == RELAY_RSV)
//		{
//			proc->rsv_type = LOCAL_RSV;
//		}
//		else if(proc->rsv_type == SUCC_RSV)
//		{
//			if(proc->voice_tx_local_relay_type != VOICE_TX_LOCAL)
//			{
//				if(!add_sig_vtx_req_flag)
//				{
//					add_sig_vtx_req_flag = 1;
//					sig = proc->vtx_req;
//					((T_VTx_Req_Param*)sig->param)->state = LOCAL_ON;
//					AddSignal(sig);	
//					proc->voice_tx_local_relay_type = VOICE_TX_LOCAL;
//				}
//			}
//		}
//	}
//	else//本节点不发
//	{
//		if((!proc->relay_flag[0]) && (!proc->relay_flag[1]) && (!proc->relay_flag[2]))//不需要中继	
//		{
//			if(proc->voice_tx_local_relay_type != VOICE_TX_OFF)
//			{
//				if(!add_sig_vtx_req_flag)
//				{
//					add_sig_vtx_req_flag = 1;
//					sig = proc->vtx_req;
//					((T_VTx_Req_Param*)sig->param)->state = RELAY_LOCAL_OFF;
//					AddSignal(sig);
//					proc->voice_tx_local_relay_type = VOICE_TX_OFF;
//				}
//			}
//			if(proc->rsv_type == SUCC_RSV)
//			{
//				//RsvCnclReq(RSV_VOICE_BSN_NUM);
//			}
//			proc->rsv_type = OFF_RSV;	
//		}
//		else//需要中继
//		{		
//			if(proc->rsv_type == OFF_RSV)
//			{
//				//RsvSlotReq(RSV_VOICE_BSN_NUM,((P_Entity*)proc->entity)->mib.local_id,3);
//				proc->state = WAIT_CFM;	
//				proc->rsv_type = RELAY_RSV;			
//			}
//			else if(proc->rsv_type == SUCC_RSV)
//			{
//				if(proc->voice_tx_local_relay_type != VOICE_TX_RELAY)
//				{
//					if(!add_sig_vtx_req_flag)
//					{
//						add_sig_vtx_req_flag = 1;
//						sig = proc->vtx_req;
//						((T_VTx_Req_Param*)sig->param)->state = RELAY_RSV;
//						AddSignal(sig);
//						proc->voice_tx_local_relay_type = VOICE_TX_RELAY;
//					}
//				}
//			}
//		}
//	}
//}

void VoiceReq(VCtrl* proc, unsigned short on_off_flag)
{
	Signal *sig;
	unsigned short i,tmp_sa,tmp_id,info_flag = 0;
	unsigned short cncl_flag = 0;
	if(on_off_flag)//1表示ON，0表示OFF
	{
		switch(proc->voice_road_cnt)
		{
			case 0:
			case 1:
			case 2:		    
				for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
				{
					if(!proc->voice_info[i].en)
					{
						info_flag = 1;
						tmp_id = i;
						break;
					}
				}
				break;
			default://当前大于等于3路
				tmp_sa = ((P_Entity*)proc->entity)->mib.local_id;
				tmp_id = 0;
				for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
				{
					if(proc->voice_info[i].sa > tmp_sa)
					{
						tmp_sa = proc->voice_info[i].sa;
						tmp_id = i;
					}
				}
				if(tmp_sa != ((P_Entity*)proc->entity)->mib.local_id)
				{
					info_flag = 1;
					proc->voice_road_cnt--;
					
				}
				break;	
		}
		if(info_flag)
		{
			proc->local_node_tx_flag = 1;
			proc->voice_info[tmp_id].en = 1;
			proc->voice_info[tmp_id].sa = ((P_Entity*)proc->entity)->mib.local_id;
			proc->voice_info[tmp_id].cncl_timer = 0;
			proc->relay_flag[tmp_id] = 0 ;
			proc->voice_road_cnt++;	
			//RsvCnclProc(proc);
		}
		else
		{
			//voice_busy_flag = 1;
		}
	}
	else//on_off_flag==0
	{
		proc->local_node_tx_flag = 0;
		for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
		{
			if(proc->voice_info[i].en)
			{
				if(proc->voice_info[i].sa == ((P_Entity*)proc->entity)->mib.local_id)
				{
					proc->voice_info[i].en = 0;
					proc->voice_info[i].cncl_timer = 0;
					proc->relay_flag[i] = 0 ;
					proc->voice_road_cnt--;
					cncl_flag = 1;
					break;
				}
			}
		}
		if(cncl_flag)
		{
			//RsvCnclProc(proc);
		}
	}
	return;
}
//static int RsvSlotCfm(Signal *sig)
//{
//	unsigned short i;
//	VCtrl *proc = (VCtrl*)sig->dst;
//	Rsv_Slot_Cfm_Param *param = (Rsv_Slot_Cfm_Param*)sig->param;
//	unsigned short tmp_flag,tmp_flag_1;
//	if(param->succ_flag)
//	{
//		if(proc->rsv_type == RELAY_RSV)
//		{
//			tmp_flag = RELAY_ON;
//			tmp_flag_1 = VOICE_TX_RELAY;
//		}
//		else if(proc->rsv_type == LOCAL_RSV)
//		{
//			tmp_flag = LOCAL_ON;
//			tmp_flag_1 = VOICE_TX_LOCAL;
//		}
//		else if(proc->rsv_type == OFF_RSV )//预约完成后已经不需要了
//		{
//			//RsvCnclReq(RSV_VOICE_BSN_NUM);
//			proc->state = IDLE;
//			return 0;
//		}
//		proc->rsv_type = SUCC_RSV;
//		if(proc->voice_tx_local_relay_type != tmp_flag_1)
//		{
//			if(!add_sig_vtx_req_flag)
//			{
//				add_sig_vtx_req_flag = 1;
//				sig = proc->vtx_req;
//				((T_VTx_Req_Param*)sig->param)->state = tmp_flag;
//				AddSignal(sig);
//				proc->voice_tx_local_relay_type = tmp_flag_1;
//			}
//		}
//	}
//	else
//	{
//		proc->local_node_tx_flag = 0;
//		proc->rsv_type = OFF_RSV;
//		for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
//		{
//			if(proc->voice_info[i].en)
//			{
//				if(proc->voice_info[i].sa == ((P_Entity*)proc->entity)->mib.local_id)
//				{
//					proc->voice_info[i].en = 0;
//					proc->relay_flag[i] = 0 ;
//					proc->voice_info[i].cncl_timer = 0;
//					proc->voice_road_cnt--;
//					break;
//				}
//			}
//		}
//	}
//	proc->state = IDLE;
//	return 0 ;
//} 


static int VoiceCtrlInd(Signal *sig)
{

	VCtrl *proc = (VCtrl*)sig->dst;
	Voice_Ctrl_Ind_Param *param = (Voice_Ctrl_Ind_Param*)sig->param;
	unsigned short i,tmp_sa,tmp_id,tmp_succ_flag,sn_new_flag;
	unsigned short tmp_road_cnt_before,tmp_road_cnt_after,tmp_vroad_idx;
	sn_new_flag = 0;
	tmp_succ_flag = 0;
	tmp_road_cnt_before = proc->voice_road_cnt;
	tmp_vroad_idx = 0;
	//处理旧话音
	for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
	{
		if(proc->voice_info[i].en)
		{
			if(proc->voice_info[i].sa == param->sa)
			{
				/*if((param->sn > proc->voice_info[i].sn) && ((param->sn - proc->voice_info[i].sn) < 128))
				{
					sn_new_flag = 1;
				}
				else if((param->sn < proc->voice_info[i].sn) && ((proc->voice_info[i].sn - param->sn)>128))
				{
					sn_new_flag = 1;
				}*/
				sn_new_flag = 1;
				if(sn_new_flag)
				{
					proc->voice_info[i].cncl_timer = V_CNCL_TIMER_MAX_CNT;
					proc->voice_info[i].sn = param->sn;
					if(param->relay_flag != proc->relay_flag[i])//拓扑变化，重新决定中继预约
					{
						proc->relay_flag[i] = param->relay_flag ;
						//RsvCnclProc(proc);
					}
					//AGPIODATA ^= 0x2000;
				}						
				sig = proc->voice_ctrl_rsp;
				((T_Voice_Ctrl_Rsp_Param*)sig->param)->succ_flag = sn_new_flag;
				((T_Voice_Ctrl_Rsp_Param*)sig->param)->rx_road_cnt = proc->voice_road_cnt - proc->local_node_tx_flag;
				((T_Voice_Ctrl_Rsp_Param*)sig->param)->sa_road_idx = tmp_vroad_idx;
				AddSignal(sig);
				
				return 0;
			}
			if(proc->voice_info[i].sa != ((P_Entity*)proc->entity)->mib.local_id)//找到此节点的位置,用于向上层传话音拷贝
			{
				tmp_vroad_idx++;
			}
		}
	}
	tmp_vroad_idx = 0;
	//处理新话音
	if(proc->voice_road_cnt < MAX_VOICE_ROAD_CNT)//路数<3
	{
		for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
		{
			if(!proc->voice_info[i].en)
			{
				break;
			}
		}
		proc->voice_info[i].en = 1;
		proc->voice_info[i].sa = param->sa;
		proc->voice_info[i].cncl_timer = V_CNCL_TIMER_MAX_CNT;
		proc->voice_info[i].sn = param->sn;
		proc->relay_flag[i] = param->relay_flag ;
		proc->voice_road_cnt++;
		tmp_succ_flag = 1;	
	}
	else if(proc->voice_road_cnt >= MAX_VOICE_ROAD_CNT)//路数=3
	{
		tmp_sa = param->sa;
		tmp_id = 0;
		for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)//找出最大的节点号
		{
			if(proc->voice_info[i].sa > tmp_sa)
			{
				tmp_sa = proc->voice_info[i].sa;
				tmp_id = i;
			}
		}
		if(tmp_sa > param->sa)//优先级高的节点插入
		{
			proc->voice_info[tmp_id].en = 1;
			proc->voice_info[tmp_id].sa = param->sa;
			proc->voice_info[tmp_id].cncl_timer = V_CNCL_TIMER_MAX_CNT;
			proc->voice_info[tmp_id].sn = param->sn;
			proc->relay_flag[tmp_id] = param->relay_flag ;
			tmp_succ_flag = 1;

			if(tmp_sa == ((P_Entity*)proc->entity)->mib.local_id)
			{
				proc->local_node_tx_flag = 0;
				//voice_busy_flag = 1;
			}	
		}
	}
	if(tmp_succ_flag)//找到此节点的位置,用于向上层传话音拷贝
	{
		for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
		{
			if(proc->voice_info[i].en)
			{
				if(proc->voice_info[i].sa == param->sa)
				{
					break;
				}
				else if(proc->voice_info[i].sa != ((P_Entity*)proc->entity)->mib.local_id)
				{
					tmp_vroad_idx++;
				}
			}
		}
	}
	if(tmp_succ_flag)//预约操作
	{
		//RsvCnclProc(proc);
	}
	sig = proc->voice_ctrl_rsp;
	((T_Voice_Ctrl_Rsp_Param*)sig->param)->succ_flag = tmp_succ_flag;
	((T_Voice_Ctrl_Rsp_Param*)sig->param)->rx_road_cnt = proc->voice_road_cnt - proc->local_node_tx_flag;
	((T_Voice_Ctrl_Rsp_Param*)sig->param)->sa_road_idx = tmp_vroad_idx;
	AddSignal(sig);

	tmp_road_cnt_after = proc->voice_road_cnt;
	//test_info.vctrl_road_cnt[test_info.vctrl_road_cnt_idx++]=tmp_road_cnt_after;
	//test_info.vctrl_road_cnt_idx%=10;
	if(((!tmp_road_cnt_before) && (tmp_road_cnt_after)) || ((tmp_road_cnt_before==1) && (proc->local_node_tx_flag) && (tmp_road_cnt_after>1)))//从没有收话路，到有收话路,开始定时80ms
	{
		//period_80ms_timer = PERIOD_40MS_TIMER;//40ms
	}

	return 0;

}

//void TimerVoiceCnclRoad(VCtrl* proc)
//{
//	unsigned short i;
//	Signal* sig;
//	for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
//	{
//		if(proc->voice_info[i].cncl_timer)
//		{
//			proc->voice_info[i].cncl_timer--;
//			if(!proc->voice_info[i].cncl_timer)
//			{
//				proc->voice_info[i].en = 0;
//				proc->relay_flag[i] = 0 ;
//				proc->voice_road_cnt--;
//				if(curt_ptt_sts && (!proc->local_node_tx_flag))
//				{
//					proc->local_node_tx_flag = 1;
//					proc->voice_info[i].en = 1;
//					proc->voice_info[i].sa = ((P_Entity*)proc->entity)->mib.local_id;
//					proc->voice_info[i].cncl_timer = 0;
//					proc->relay_flag[i] = 0 ;
//					proc->voice_road_cnt++;					
//				}
//				RsvCnclProc(proc);	
//								
//			}
//		}
//	}
//
//	return;
//}
void VCtrlInit(VCtrl* proc)
{
	proc->state = IDLE;
	proc->voice_road_cnt = 0;
	proc->local_node_tx_flag = 0;
	proc->rsv_type = OFF_RSV;
	proc->voice_tx_local_relay_type = VOICE_TX_OFF;
	memset(proc->relay_flag,0,sizeof(proc->relay_flag));
	memset(proc->voice_info,0,(sizeof(Voice_Info))*MAX_VOICE_ROAD_CNT);
	return;
}
void VCtrlSetup(VCtrl *proc)
{
	proc->rsv_slot_cfm.next=0;
	proc->rsv_slot_cfm.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->rsv_slot_cfm.src=0;
	proc->rsv_slot_cfm.dst=proc;
	//proc->rsv_slot_cfm.func=RsvSlotCfm;
	proc->rsv_slot_cfm.pri=SDL_PRI_URG;
	proc->rsv_slot_cfm.param=&proc->rsv_slot_cfm_param;

	proc->voice_ctrl_ind.next=0;
	proc->voice_ctrl_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->voice_ctrl_ind.src=0;
	proc->voice_ctrl_ind.dst=proc;
	proc->voice_ctrl_ind.func=VoiceCtrlInd;
	proc->voice_ctrl_ind.pri=SDL_PRI_URG;
	proc->voice_ctrl_ind.param=&proc->voice_ctrl_ind_param;
}







