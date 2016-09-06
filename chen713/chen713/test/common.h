#ifndef _COMMON_H
#define _COMMON_H


#define NODE_MAX_CNT 32
//#define NODE_SIM_CNT 2

#define CHK_CNT_PER_TIME 300

//MAC层发送接收模块
#define MAC_FRM_LEN_234          15 //mac帧总长度234bit 14字余10bit   15字 
#define MAC_FRM_LEN_1002         63 //mac帧总长度1002bit 62字余10bit  63字
#define MAC_FRM_COMMOM_HEAD_LEN  1 //mac公共头长度,其实是10bit
#define MAC_FRM_PAYLOAD_1002     62//(MAC_FRM_LEN_1002 - MAC_FRM_COMMOM_HEAD_LEN) //mac除公共头外其他部分长度
#define MAC_FRM_PAYLOAD_234      14//(MAC_FRM_LEN_234 - MAC_FRM_COMMOM_HEAD_LEN) //mac除公共头外其他部分长度


#define DATA_MODE_0_DATA_LEN   15   //数据类型0，mac帧总长度234bit,14字余10bit
//#define DATA_MODE_1_MAC_FRM_LEN 63//数据类型1,mac帧总长度1002bit 62字余10bit
#define DATA_MODE_2_DATA_LEN   126//数据类型2,mac帧总长度2004bit,125字余4
#define DATA_MODE_3_DATA_LEN   251//数据类型3,mac帧总长度4008bit,250字余8
#define DATA_MODE_2_MAC_FRM_CNT   2//数据类型2含MAC帧个数
#define DATA_MODE_3_MAC_FRM_CNT   4//数据类型3含MAC帧个数

#define DATA_MODE_0_TIME_LEN    800   //数据类型0，mac帧234bit时间0.8ms 
//#define DATA_MODE_1_TIME_LEN 1300//数据类型1,mac帧1002bit时间1.3ms
#define DATA_MODE_2_TIME_LEN    1800  //数据类型2,2个mac帧1002bit时间1.8ms
#define DATA_MODE_3_TIME_LEN    2900  //数据类型3,4mac帧1002bit时间2.9ms

#define DATA_MODE_0_MICRO_CNT    2   //数据类型0，mac帧234bit微时隙个数5
//#define DATA_MODE_1_MICRO_CNT  3//数据类型1，mac帧1002bit微时隙个数7
#define DATA_MODE_2_MICRO_CNT    4   //数据类型2，2个mac帧1002bit微时隙个数10
#define DATA_MODE_3_MICRO_CNT    6  //数据类型3，4mac帧1002bit微时隙个数15

#define DATA_MODE_0 0//发送数据模式类型定义
#define DATA_MODE_1 1
#define DATA_MODE_2 2
#define DATA_MODE_3 3

#ifndef NULL
#define NULL 0
#endif


//子帧类型
#define SUB_FRM_TYPE_RTS 0xc000//高8位有效
#define SUB_FRM_TYPE_CTS 0xc200
#define SUB_FRM_TYPE_SRR 0xc800
#define SUB_FRM_TYPE_SRF 0xca00
#define SUB_FRM_TYPE_SRC 0xcc00
#define SUB_FRM_TYPE_SCI 0xce00
#define SUB_FRM_TYPE_ACK 0xd000
#define SUB_FRM_TYPE_RST 0xd200
#define SUB_FRM_TYPE_VLR 0xd800
#define SUB_FRM_TYPE_VLC 0xda00
#define SUB_FRM_TYPE_VCI 0xdc00
#define SUB_FRM_TYPE_RTI 0xe000
#define SUB_FRM_TYPE_NJR 0xe800
#define SUB_FRM_TYPE_NSI 0xea00
#define SUB_FRM_TYPE_NFI 0xec00
#define SUB_FRM_TYPE_DATA 0x0000//高1位有效
#define SUB_FRM_TYPE_VOICE 0x8000//高2位有效

//子帧长度字
#define SUB_FRM_LEN_RTS 2
#define SUB_FRM_LEN_CTS 2
#define SUB_FRM_LEN_SRR 2
#define SUB_FRM_LEN_SRF 2
#define SUB_FRM_LEN_SRC 2
#define SUB_FRM_LEN_SCI 2
#define SUB_FRM_LEN_ACK 2
#define SUB_FRM_LEN_RST 1
#define SUB_FRM_LEN_VLR 2
#define SUB_FRM_LEN_VLC 2
#define SUB_FRM_LEN_VCI 2
#define SUB_FRM_LEN_NJR 1
#define SUB_FRM_LEN_NSI 3
#define SUB_FRM_LEN_NFI 1
#define SUB_FRM_LEN_RTI 6
#define DATA_SUB_FRM_HEAD_LEN 1 //数据子帧帧头长度
#define RTI_SUB_FRM_HEAD_LEN 6//rti子帧头长度
#define VOICE_SUB_FRM_HEAD_LEN 1 //话音子帧帧头长度

//vtx_req信号参数
#define RELAY_LOCAL_OFF 0
#define LOCAL_ON 1
#define RELAY_ON 2

#define MAX_VOICE_ROAD_CNT 3//最多允许3路话音
#define VOICE_DATA_LEN 40//话音净荷长度

#define RSV_VOICE_BSN_NUM 0//预约业务号话音
#define RSV_DATA_BSN_NUM 1 //预约业务号数据(CTS RTS)

typedef struct _data_sub_frm//数据子帧
{
	unsigned short seg_len_sn_rst;
	unsigned short data[MAC_FRM_PAYLOAD_1002 - DATA_SUB_FRM_HEAD_LEN]; //62-1
}Data_Sub_Frm;

typedef struct _ack_sub_frm//ack子帧
{
	unsigned short flag_ra_rst;//帧标志:b15-b9（1101000）,接收节点b8-b4,arq重置标记b3
	unsigned short sn_sts;//arq序号:b15-b12,接收标志b11-b5
}Ack_Sub_Frm;

typedef struct _rst_sub_frm//arq重置子帧
{
	unsigned short flag_ra;//帧标志:b15-b9（1101010）,接收节点b8-b4
}Rst_Sub_Frm;

typedef struct _nsi_sub_frm//NSI子帧
{
	unsigned short flag_sn;//子帧标记b15-b9
	unsigned short group_level_parent;
	unsigned short fn;
}NSI_Sub_Frm;

typedef struct _mac_frm_234//MAC帧结构
{
	unsigned short payload[MAC_FRM_PAYLOAD_234];  //14
	unsigned short common_head;
}Mac_Frm_234;

typedef struct _mac_frm_1002//MAC帧结构
{
	unsigned short payload[MAC_FRM_PAYLOAD_1002];  //62
	unsigned short common_head;
}Mac_Frm_1002;

typedef struct _voice_sub_frm
{
	unsigned short flag_sa_sn;
	unsigned short voice_payload[VOICE_DATA_LEN];

}Voice_Sub_Frm;


////////////////////////////////DLC层宏定义////////////////////////////////////
#define ARQ_SN_SIZE 16//窗口区域总大小
#define ARQ_WIN_SIZE 8//窗口大小
#define SEND_FAIL_TYPE_RESEND 0 //发送失败类型，重发失败
#define SEND_FAIL_TYPE_RST 1 //发送失败类型，收到arq重置信令失败
#define MAX_RESEND_CNT 20//arq_tx最大重发次数
#define TIMER_WAIT_ACK_MAX 500//等等ack定时,暂时置为很大的值，用于防止DlcTxCfm中断丢



//分段开始结束标志定义
#define FIRST_SEG_FLAG 0x4000
#define MID_SEG_FLAG   0x0000
#define LAST_SEG_FLAG  0x2000  


//数据子帧净荷
#define DATA_SUB_FRM_PALOAD_LEN_1002  (MAC_FRM_PAYLOAD_1002 - DATA_SUB_FRM_HEAD_LEN)//1002bitMAC帧的数据子帧净荷   62-1
#define DATA_SUB_FRM_PALOAD_LEN_234   (MAC_FRM_PAYLOAD_234 - DATA_SUB_FRM_HEAD_LEN)//234bitMAC帧的数据子帧净荷
#define DLC_SEG_LEN_MAX                DATA_SUB_FRM_PALOAD_LEN_1002//最大DLC分段长度


///////////////////////////////////NET层定义//////////////////////////////////////


#define NET_FRM_HEAD_LEN 2//网络帧头长度
#define NET_FRM_LEN_MAX 750//最大网络帧长度,字
#define NET_PAYLOAD_SIZE NET_FRM_LEN_MAX - NET_FRM_HEAD_LEN //网络帧净荷最大长度  750-2

#define UC_NET_FRM_FLAG 0//单播
#define BC_NET_FRM_FLAG 1//广播
#define MC_NET_FRM_FLAG 2//多播

#define HELLO_PKT_PTI 0//hello包优先级
#define MULTICAST_DA 0xff//多播接收地址

#define RND_L 15//小于网络长度

#define PERIOD_80MS_TIMER 40//收话音送至上层的80ms定时
#define PERIOD_40MS_TIMER 20//收话音送至上层的40ms定时



typedef struct _rt_items//路由项
{
	unsigned short dest_next_metric;//目的地址,下一跳,跳数
	unsigned short seq;//序号
	unsigned long ip;//IP地址
}Rt_Items;



typedef struct _rti_sub_frm//SRF子帧
{
	unsigned short flag_cnt_pb;//子帧标记b15-b9: 1100001，携带标记b0,序号
	unsigned short seq;//路由项个数
	unsigned long nbr_info;//邻节点信息，从低位到高位，每个节点一个bit
	unsigned long ip;
	Rt_Items rt_items[NODE_MAX_CNT];//路由项
}RTI_Sub_Frm;



typedef struct _net_frm//网络帧
{
	unsigned short pri_sa_da_mul_sn;//源地址,目的地址,序号
	unsigned short sn_len;//序号,长度
	unsigned short net_paload[NET_PAYLOAD_SIZE];//净荷  750-2
}Net_Frm;

#define APP_TX_MALLOC_CNT_MAX 3//应用层发送数据malloc个数
#define READ_FM_MALLOC_CNT_MAX 8//读铁电发送数据malloc个数
#define RELAY_TX_MALLOC_CNT_MAX 10//读铁电发送数据malloc个数

//////////////////////////////////////////////////测试信息/////////////////////////////////////////////////////////////////
extern void AppRxInd(unsigned short *net_frm,unsigned short net_frm_len,unsigned short ta,unsigned short uc_bc_falg,unsigned short pri);//for test
extern void AppTxReq(unsigned short *dat,unsigned short len,unsigned short da,unsigned short pri,unsigned short hello_flag);
extern void AckTxReq(unsigned short *ack_sgl);//for test
extern void RstTxReq(unsigned short *rst_sgl);//for test
extern void DlcTxRsp(unsigned short * mac_frm);
extern unsigned short dat_tx_req_add_sig_flag; //信号dat_tx_req已经加入队列标记
extern unsigned short relay_tx_req_add_sig_flag;
extern void PrintDat(unsigned char* dat,unsigned short len,unsigned short first_dat);
extern unsigned short printdat_once[20];
extern unsigned short period_80ms_timer ;//周期80ms定时
extern unsigned short period_80ms_timer_flag ;//周期80ms定时
extern unsigned short malloc_cnt1;
extern unsigned long HELLO_SEND_TIMER ;//hello包发送周期
extern unsigned short voice_busy_flag ;
extern	unsigned short add_sig_vtx_req_flag;//已经添加信号标记
#endif

