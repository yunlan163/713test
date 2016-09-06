#include "vtx.h"
#include <string.h>
enum {IDLE};
typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
static int VTxReq(Signal *sig)
{
	VTx *proc= (VTx*)sig->dst;
	VTx_Req_Param* param = (VTx_Req_Param*)sig->param;
	proc->vtx_sts = param->state;
	//add_sig_vtx_req_flag = 0;
	return 0;
}
static int VoiceRelay(Signal *sig)
{
	VTx *proc= (VTx*)sig->dst;
	Voice_Relay_Param* param = (Voice_Relay_Param*)sig->param;
	unsigned short cnt;
	if(proc->vtx_sts != RELAY_LOCAL_OFF)
	{
		memcpy(proc->voice_mac_frm[proc->btm].payload,param->vdat,VOICE_DATA_LEN + VOICE_SUB_FRM_HEAD_LEN);
		proc->btm++;
		proc->btm %= MAX_VOICE_ROAD_CNT;
		proc->size++;
		if(proc->size > MAX_VOICE_ROAD_CNT)
		{
			cnt = proc->size - MAX_VOICE_ROAD_CNT;
			proc->size = MAX_VOICE_ROAD_CNT;
			while(cnt--)
			{
				proc->top++;
				proc->top %= MAX_VOICE_ROAD_CNT;
			}
		}
	}
	return 0;

}
void VoiceTx(VTx* proc,unsigned short *vdat)
{
	unsigned short cnt;
	if(proc->vtx_sts == LOCAL_ON)
	{
		proc->local_voice_sn ++;
		proc->local_voice_sn &= 0xff;
		((Voice_Sub_Frm*)&proc->voice_mac_frm[proc->btm])->flag_sa_sn = SUB_FRM_TYPE_VOICE | ((((P_Entity*)proc->entity)->mib.local_id) << 9) | proc->local_voice_sn;
		memcpy((((Voice_Sub_Frm*)&proc->voice_mac_frm[proc->btm])->voice_payload),vdat,VOICE_DATA_LEN);
		proc->btm++;
		proc->btm %= MAX_VOICE_ROAD_CNT;
		proc->size++;
		if(proc->size > MAX_VOICE_ROAD_CNT)
		{
			cnt = proc->size - MAX_VOICE_ROAD_CNT;
			proc->size = MAX_VOICE_ROAD_CNT;
			while(cnt--)
			{
				proc->top++;
				proc->top %= MAX_VOICE_ROAD_CNT;
			}
		}	
	}
	return;
}
void VTxInit(VTx* proc)
{
	unsigned short i;
	proc->state = IDLE;
	proc->vtx_sts = RELAY_LOCAL_OFF;
	proc->btm = 0;
	proc->top = 0;
	proc->size = 0;
	memset(proc->voice_mac_frm,0,(sizeof(Mac_Frm_1002))*MAX_VOICE_ROAD_CNT);
	proc->local_voice_sn = 0;
	for(i = 0; i < MAX_VOICE_ROAD_CNT; i++)
	{
		proc->voice_mac_frm[i].common_head = (((P_Entity*)proc->entity)->mib.local_id) << 11;
		proc->voice_mac_frm[i].common_head |= (((P_Entity*)proc->entity)->mib.local_id) <<6;
	}
}
void VTxSetup(VTx* proc)
{
	proc->voice_relay.next=0;
	proc->voice_relay.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->voice_relay.src=0;
	proc->voice_relay.dst=proc;
	proc->voice_relay.func=VoiceRelay;
	proc->voice_relay.pri=SDL_PRI_URG;
	proc->voice_relay.param=&proc->voice_relay_param;

	proc->vtx_req.next=0;
	proc->vtx_req.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->vtx_req.src=0;
	proc->vtx_req.dst=proc;
	proc->vtx_req.func=VTxReq;
	proc->vtx_req.pri=SDL_PRI_URG;
	proc->vtx_req.param=&proc->vtx_req_param;
	
}

