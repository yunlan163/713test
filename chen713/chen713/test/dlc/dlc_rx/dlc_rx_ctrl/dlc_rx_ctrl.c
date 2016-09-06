#include "dlc_rx_ctrl.h"
#include <stdio.h>

enum {IDLE,WAIT_RSP};

typedef struct
{
	void* myself;//must be at the first 
	Sdl_Core sdlc;//must be at the second
	Mib mib;//must be at the third
	
}P_Entity;
//yun:收到mac层的dlc_rx_ind信号
static int DlcRxInd(Signal* sig)
{
	Dlc_Rx_Ctrl* proc=(Dlc_Rx_Ctrl*)sig->dst;
	Dlc_Rx_Ind_Param *param=(Dlc_Rx_Ind_Param *)sig->param;
	unsigned short paload_len,sn;

	unsigned short ta = (((Mac_Frm_1002 * )(param->frm))->common_head) >> 11;//取公共头发送地址   yun：发送地址
	unsigned short ra = ((((Mac_Frm_1002 * )(param->frm))->common_head) >> 6) & 0x1f;//取公共头发送地址    yun：接收地址
	unsigned short seg_flag = (((Data_Sub_Frm *)(param->frm))->seg_len_sn_rst) & 0x6000;
	if(ta == ra )//广播
	{	 
		proc->bc_frm_wait_free = param->frm;  //yun:帧地址
		paload_len = ((((Data_Sub_Frm *)(param->frm))->seg_len_sn_rst) >> 5) & 0x3f;  //yun:长度
		sn = ((((Data_Sub_Frm *)(param->frm))->seg_len_sn_rst) >> 1) & 0xf;   //yun:序号
		sig = proc->frm_rx_ind[ta];//数据送至frm模块
		((T_Bc_Frm_Rx_Ind_Param*)sig->param)->xdat = ((Data_Sub_Frm *)(param->frm))->data;
		((T_Bc_Frm_Rx_Ind_Param*)sig->param)->xlen = paload_len;
		((T_Bc_Frm_Rx_Ind_Param*)sig->param)->seg_flag = seg_flag;
		((T_Bc_Frm_Rx_Ind_Param*)sig->param)->bc_uc_falg = 1;//广播
		((T_Bc_Frm_Rx_Ind_Param*)sig->param)->bc_seg_num = sn;
		AddSignal(sig);
		printf("---节点[%d]----dlc_rx_ctrl::DlcRxInd() 收到广播包[%d], 上传至frm_rx \n",((P_Entity*)proc->entity)->mib.local_id,sn);
	}
	else//单播
	{
		printf("---节点[%d]---dlc_rx_ctrl::DlcRxInd() 收到单播包\n",((P_Entity*)proc->entity)->mib.local_id);
		sig = proc->arq_rx_ind[ta];//数据送至arq模块  yun:ta:相对应的发送的地址模块  小包
		((T_Arq_Rx_Ind_Param *)sig->param)->frm = param->frm;
		((T_Arq_Rx_Ind_Param *)sig->param)->len = param->len;
		AddSignal(sig);
		printf("dlc_rx_ctrl::DlcRxInd() 根据发送地址上传至arq_rx_ind[%d] \n", ta);
	}
	proc->state = WAIT_RSP;
	return 0;
}
//yun:收到arq_rx的回复，上传完毕一个位置
static int ArqRxRsp(Signal* sig)
{
	Dlc_Rx_Ctrl* proc=(Dlc_Rx_Ctrl*)sig->dst;
//	Arq_Rx_Rsp_Param *param=(Arq_Rx_Rsp_Param *)sig->param;

	//sig=proc->dlc_rx_rsp;//回复mac层接收完成
	//AddSignal(sig);//for test

	proc->state=IDLE;	
	return 0;
}

static int ArqAckTxReq(Signal* sig)
{
	Dlc_Rx_Ctrl* proc=(Dlc_Rx_Ctrl*)sig->dst;
	Arq_Ack_Tx_Req_Param *param=(Arq_Ack_Tx_Req_Param *)sig->param;

	sig=proc->ack_tx_req;//发送ack信令  在这里填写
	((T_Ack_Tx_Req_Param *)sig->param)->ack_sgl = param->ack_sgl;
	((T_Ack_Tx_Req_Param *)sig->param)->id = param->id;
	//((T_Ack_Tx_Req_Param *)sig->param)->len = param->len;

	AddSignal(sig);	
	printf("dlc_rx_ctrl:: ArqAckTxReq 发送ack给 发送方的模块\n");
	//AckTxReq(param->ack_sgl);//for test
	return 0;
}

static int ArqRstTxReq(Signal* sig)
{
	Dlc_Rx_Ctrl* proc=(Dlc_Rx_Ctrl*)sig->dst;
	Arq_Rst_Tx_Req_Param *param=(Arq_Rst_Tx_Req_Param *)sig->param;

	sig=proc->rst_tx_req ;//发送arq重置信令
	//((T_Rst_Tx_Req_Param *)sig->param)->rst_sgl = param->rst_sgl;
	//((T_Rst_Tx_Req_Param *)sig->param)->len = param->len;
	//AddSignal(sig);
	//RstTxReq(param->rst_sgl);//for test
	return 0;
}
//yun：收到frm_rx的广播回复、释放
static int FrmRxRsp(Signal* sig)
{
	Dlc_Rx_Ctrl* proc=(Dlc_Rx_Ctrl*)sig->dst;
	//Frm_Rx_Rsp_Param *param=(Frm_Rx_Rsp_Param *)sig->param;   //yun:没有参数
	MemFreeMac(proc->bc_frm_wait_free);  //yun:释放
	proc->bc_frm_wait_free = NULL;
	return 0;
}

void DlcRxCtrlInit(Dlc_Rx_Ctrl *proc)
{
	proc->state=IDLE;
	proc->bc_frm_wait_free = NULL;
}

void DlcRxCtrlSetup(Dlc_Rx_Ctrl *proc)
{
	unsigned short i;
	proc->dlc_rx_ind.next=0;
	proc->dlc_rx_ind.sdlc=&((P_Entity*)proc->entity)->sdlc;
	proc->dlc_rx_ind.src=0;
	proc->dlc_rx_ind.dst=proc;
	proc->dlc_rx_ind.func=DlcRxInd;
	proc->dlc_rx_ind.pri=SDL_PRI_URG;
	proc->dlc_rx_ind.param=&proc->dlc_rx_ind_param;
	
	for(i=0;i<NODE_MAX_CNT;i++)
	{
		proc->arq_rx_rsp[i].next=0;
		proc->arq_rx_rsp[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->arq_rx_rsp[i].src=0;
		proc->arq_rx_rsp[i].dst=proc;
		proc->arq_rx_rsp[i].func=ArqRxRsp;
		proc->arq_rx_rsp[i].pri=SDL_PRI_URG;
		proc->arq_rx_rsp[i].param=&proc->arq_rx_rsp_param;

		proc->arq_ack_tx_req[i].next=0;
		proc->arq_ack_tx_req[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->arq_ack_tx_req[i].src=0;
		proc->arq_ack_tx_req[i].dst=proc;
		proc->arq_ack_tx_req[i].func=ArqAckTxReq;
		proc->arq_ack_tx_req[i].pri=SDL_PRI_URG;
		proc->arq_ack_tx_req[i].param=&proc->arq_ack_tx_req_param;

		proc->arq_rst_tx_req[i].next=0;
		proc->arq_rst_tx_req[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->arq_rst_tx_req[i].src=0;
		proc->arq_rst_tx_req[i].dst=proc;
		proc->arq_rst_tx_req[i].func=ArqRstTxReq;
		proc->arq_rst_tx_req[i].pri=SDL_PRI_URG;
		proc->arq_rst_tx_req[i].param=&proc->arq_rst_tx_req_param;

		proc->frm_rx_rsp[i].next=0;
		proc->frm_rx_rsp[i].sdlc=&((P_Entity*)proc->entity)->sdlc;
		proc->frm_rx_rsp[i].src=0;
		proc->frm_rx_rsp[i].dst=proc;
		proc->frm_rx_rsp[i].func=FrmRxRsp;
		proc->frm_rx_rsp[i].pri=SDL_PRI_URG;
	}	
}
