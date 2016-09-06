#include "vrx.h"
#include <string.h>
//#include "../../../../dma_r1x4_mcbsp2_mpct.h"
//#include "../../../../g729_init.h"

extern unsigned int DMA_start_hDmaXmt3_flag;

enum {IDLE,WAIT_RSP,WAIT_CFM};
extern void VoiceRx(unsigned short *vdat, unsigned short sa);
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
static int VRxInd(Signal* sig)
{
	VRx* proc = (VRx*)sig->dst;
	VRx_Ind_Param* param = (VRx_Ind_Param*)sig->param;
	unsigned short sa,sn;
	memcpy(&proc->voice_mac_frm,param->vdat,MAC_FRM_LEN_1002);
	MemFreeMac(param->vdat);
	sa = (((Voice_Sub_Frm*)(proc->voice_mac_frm.payload))->flag_sa_sn >> 9) & 0x1f;
	sn = (((Voice_Sub_Frm*)(proc->voice_mac_frm.payload))->flag_sa_sn) & 0xff;
	//经转发的旧话音包
	if(sa == ((P_Entity*)proc->entity)->mib.local_id)
	{
			return 0;
	}
	if(((proc->voice_sn[sa] >= sn) && (proc->voice_sn[sa] - sn < 30)) || ((proc->voice_sn[sa] < sn) && ( sn - proc->voice_sn[sa] > 225)))
	{
		return 0;
	}
	proc->voice_sn[sa] = sn;

	//用于中继
	sig = proc->vrelay_chk_req;
	((T_VRelay_Chk_Req_Param*)sig->param)->sa = sa;
	AddSignal(sig);
	proc->state = WAIT_CFM;

	return 0;
}

static int VoiceCtrlRsp(Signal* sig)
{  
	unsigned short *vdat;
	unsigned short sa;
	VRx* proc = (VRx*)sig->dst;
	Voice_Ctrl_Rsp_Param* param = (Voice_Ctrl_Rsp_Param*)sig->param;
	

	if(param->succ_flag)
	{
		vdat = ((Voice_Sub_Frm*)(proc->voice_mac_frm.payload))->voice_payload;
		sa = (((Voice_Sub_Frm*)(proc->voice_mac_frm.payload))->flag_sa_sn >> 9) & 0x1f;
		//VoiceRx(vdat,sa);

		//memcpy(AUDIO_rcv729,vdat,VOICE_DATA_LEN);
		//task_decode_flag = decode;
		//MPCT_FRAME_RX_BUF.FRAME_TYPE = 0xAA01;
		//DMA_start_hDmaXmt3_flag = 1;

		if(proc->current_vrx_road_cnt < param->rx_road_cnt)
		{
			//memcpy(AUDIO_rcv729 + (param->sa_road_idx * 40) ,vdat,VOICE_DATA_LEN);//拷贝到相应的位置
			proc->current_vrx_road_flag[param->sa_road_idx] = 1;
			proc->current_vrx_road_cnt ++;
		}
		if(proc->relay_flag)
		{
			sig = proc->voice_relay;
			((T_Voice_Relay_Param*)sig->param)->vdat = (unsigned short*)(&(proc->voice_mac_frm));
			AddSignal(sig);
		}
	
	}
	else
	{
		proc->state = IDLE;
	}
	return 0;
}
static int VRelayChkCfm(Signal *sig)
{
	VRx* proc = (VRx*)sig->dst;
	VRelay_Chk_Cfm_Param* param = (VRelay_Chk_Cfm_Param*)sig->param;
	unsigned short sa,sn;
	
	proc->relay_flag = param->succ_flag;
	sa = (((Voice_Sub_Frm*)(proc->voice_mac_frm.payload))->flag_sa_sn >> 9) & 0x1f;
	sn = (((Voice_Sub_Frm*)(proc->voice_mac_frm.payload))->flag_sa_sn) & 0xff;

	sig = proc->voice_ctrl_ind;
	((T_Voice_Ctrl_Ind_Param*)sig->param)->sa = sa;
	((T_Voice_Ctrl_Ind_Param*)sig->param)->sn = sn;
	((T_Voice_Ctrl_Ind_Param*)sig->param)->relay_flag = param->succ_flag;
	AddSignal(sig);
	proc->state = WAIT_RSP;

	return 0;	
}
void VRxInit(VRx* proc)
{
	proc->state = IDLE;	
	proc->current_vrx_road_cnt = 0;
	memset(proc->voice_sn,0x1ff,sizeof(	proc->voice_sn));
	memset(proc->current_vrx_road_flag,0,MAX_VOICE_ROAD_CNT);	
	memset(&proc->voice_mac_frm, 0, sizeof(Mac_Frm_1002));
}
void VRxSetup(VRx* proc)
{
	proc->vrx_ind.next=0;
	proc->vrx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->vrx_ind.src=0;
	proc->vrx_ind.dst=proc;
	proc->vrx_ind.func=VRxInd;
	proc->vrx_ind.pri=SDL_PRI_URG;
	proc->vrx_ind.param=&proc->vrx_ind_param;

	proc->vrelay_chk_cfm.next=0;
	proc->vrelay_chk_cfm.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->vrelay_chk_cfm.src=0;
	proc->vrelay_chk_cfm.dst=proc;
	proc->vrelay_chk_cfm.func=VRelayChkCfm;
	proc->vrelay_chk_cfm.pri=SDL_PRI_URG;
	proc->vrelay_chk_cfm.param=&proc->vrelay_chk_cfm_param;

	proc->voice_ctrl_rsp.next=0;
	proc->voice_ctrl_rsp.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->voice_ctrl_rsp.src=0;
	proc->voice_ctrl_rsp.dst=proc;
	proc->voice_ctrl_rsp.func=VoiceCtrlRsp;
	proc->voice_ctrl_rsp.pri=SDL_PRI_URG;
	proc->voice_ctrl_rsp.param=&proc->voice_ctrl_rsp_param;
}

