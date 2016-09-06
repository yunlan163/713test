#ifndef _COMMON_H
#define _COMMON_H


#define NODE_MAX_CNT 32
//#define NODE_SIM_CNT 2

#define CHK_CNT_PER_TIME 300

//MAC�㷢�ͽ���ģ��
#define MAC_FRM_LEN_234          15 //mac֡�ܳ���234bit 14����10bit   15�� 
#define MAC_FRM_LEN_1002         63 //mac֡�ܳ���1002bit 62����10bit  63��
#define MAC_FRM_COMMOM_HEAD_LEN  1 //mac����ͷ����,��ʵ��10bit
#define MAC_FRM_PAYLOAD_1002     62//(MAC_FRM_LEN_1002 - MAC_FRM_COMMOM_HEAD_LEN) //mac������ͷ���������ֳ���
#define MAC_FRM_PAYLOAD_234      14//(MAC_FRM_LEN_234 - MAC_FRM_COMMOM_HEAD_LEN) //mac������ͷ���������ֳ���


#define DATA_MODE_0_DATA_LEN   15   //��������0��mac֡�ܳ���234bit,14����10bit
//#define DATA_MODE_1_MAC_FRM_LEN 63//��������1,mac֡�ܳ���1002bit 62����10bit
#define DATA_MODE_2_DATA_LEN   126//��������2,mac֡�ܳ���2004bit,125����4
#define DATA_MODE_3_DATA_LEN   251//��������3,mac֡�ܳ���4008bit,250����8
#define DATA_MODE_2_MAC_FRM_CNT   2//��������2��MAC֡����
#define DATA_MODE_3_MAC_FRM_CNT   4//��������3��MAC֡����

#define DATA_MODE_0_TIME_LEN    800   //��������0��mac֡234bitʱ��0.8ms 
//#define DATA_MODE_1_TIME_LEN 1300//��������1,mac֡1002bitʱ��1.3ms
#define DATA_MODE_2_TIME_LEN    1800  //��������2,2��mac֡1002bitʱ��1.8ms
#define DATA_MODE_3_TIME_LEN    2900  //��������3,4mac֡1002bitʱ��2.9ms

#define DATA_MODE_0_MICRO_CNT    2   //��������0��mac֡234bit΢ʱ϶����5
//#define DATA_MODE_1_MICRO_CNT  3//��������1��mac֡1002bit΢ʱ϶����7
#define DATA_MODE_2_MICRO_CNT    4   //��������2��2��mac֡1002bit΢ʱ϶����10
#define DATA_MODE_3_MICRO_CNT    6  //��������3��4mac֡1002bit΢ʱ϶����15

#define DATA_MODE_0 0//��������ģʽ���Ͷ���
#define DATA_MODE_1 1
#define DATA_MODE_2 2
#define DATA_MODE_3 3

#ifndef NULL
#define NULL 0
#endif


//��֡����
#define SUB_FRM_TYPE_RTS 0xc000//��8λ��Ч
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
#define SUB_FRM_TYPE_DATA 0x0000//��1λ��Ч
#define SUB_FRM_TYPE_VOICE 0x8000//��2λ��Ч

//��֡������
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
#define DATA_SUB_FRM_HEAD_LEN 1 //������֡֡ͷ����
#define RTI_SUB_FRM_HEAD_LEN 6//rti��֡ͷ����
#define VOICE_SUB_FRM_HEAD_LEN 1 //������֡֡ͷ����

//vtx_req�źŲ���
#define RELAY_LOCAL_OFF 0
#define LOCAL_ON 1
#define RELAY_ON 2

#define MAX_VOICE_ROAD_CNT 3//�������3·����
#define VOICE_DATA_LEN 40//�������ɳ���

#define RSV_VOICE_BSN_NUM 0//ԤԼҵ��Ż���
#define RSV_DATA_BSN_NUM 1 //ԤԼҵ�������(CTS RTS)

typedef struct _data_sub_frm//������֡
{
	unsigned short seg_len_sn_rst;
	unsigned short data[MAC_FRM_PAYLOAD_1002 - DATA_SUB_FRM_HEAD_LEN]; //62-1
}Data_Sub_Frm;

typedef struct _ack_sub_frm//ack��֡
{
	unsigned short flag_ra_rst;//֡��־:b15-b9��1101000��,���սڵ�b8-b4,arq���ñ��b3
	unsigned short sn_sts;//arq���:b15-b12,���ձ�־b11-b5
}Ack_Sub_Frm;

typedef struct _rst_sub_frm//arq������֡
{
	unsigned short flag_ra;//֡��־:b15-b9��1101010��,���սڵ�b8-b4
}Rst_Sub_Frm;

typedef struct _nsi_sub_frm//NSI��֡
{
	unsigned short flag_sn;//��֡���b15-b9
	unsigned short group_level_parent;
	unsigned short fn;
}NSI_Sub_Frm;

typedef struct _mac_frm_234//MAC֡�ṹ
{
	unsigned short payload[MAC_FRM_PAYLOAD_234];  //14
	unsigned short common_head;
}Mac_Frm_234;

typedef struct _mac_frm_1002//MAC֡�ṹ
{
	unsigned short payload[MAC_FRM_PAYLOAD_1002];  //62
	unsigned short common_head;
}Mac_Frm_1002;

typedef struct _voice_sub_frm
{
	unsigned short flag_sa_sn;
	unsigned short voice_payload[VOICE_DATA_LEN];

}Voice_Sub_Frm;


////////////////////////////////DLC��궨��////////////////////////////////////
#define ARQ_SN_SIZE 16//���������ܴ�С
#define ARQ_WIN_SIZE 8//���ڴ�С
#define SEND_FAIL_TYPE_RESEND 0 //����ʧ�����ͣ��ط�ʧ��
#define SEND_FAIL_TYPE_RST 1 //����ʧ�����ͣ��յ�arq��������ʧ��
#define MAX_RESEND_CNT 20//arq_tx����ط�����
#define TIMER_WAIT_ACK_MAX 500//�ȵ�ack��ʱ,��ʱ��Ϊ�ܴ��ֵ�����ڷ�ֹDlcTxCfm�ж϶�



//�ֶο�ʼ������־����
#define FIRST_SEG_FLAG 0x4000
#define MID_SEG_FLAG   0x0000
#define LAST_SEG_FLAG  0x2000  


//������֡����
#define DATA_SUB_FRM_PALOAD_LEN_1002  (MAC_FRM_PAYLOAD_1002 - DATA_SUB_FRM_HEAD_LEN)//1002bitMAC֡��������֡����   62-1
#define DATA_SUB_FRM_PALOAD_LEN_234   (MAC_FRM_PAYLOAD_234 - DATA_SUB_FRM_HEAD_LEN)//234bitMAC֡��������֡����
#define DLC_SEG_LEN_MAX                DATA_SUB_FRM_PALOAD_LEN_1002//���DLC�ֶγ���


///////////////////////////////////NET�㶨��//////////////////////////////////////


#define NET_FRM_HEAD_LEN 2//����֡ͷ����
#define NET_FRM_LEN_MAX 750//�������֡����,��
#define NET_PAYLOAD_SIZE NET_FRM_LEN_MAX - NET_FRM_HEAD_LEN //����֡������󳤶�  750-2

#define UC_NET_FRM_FLAG 0//����
#define BC_NET_FRM_FLAG 1//�㲥
#define MC_NET_FRM_FLAG 2//�ಥ

#define HELLO_PKT_PTI 0//hello�����ȼ�
#define MULTICAST_DA 0xff//�ಥ���յ�ַ

#define RND_L 15//С�����糤��

#define PERIOD_80MS_TIMER 40//�ջ��������ϲ��80ms��ʱ
#define PERIOD_40MS_TIMER 20//�ջ��������ϲ��40ms��ʱ



typedef struct _rt_items//·����
{
	unsigned short dest_next_metric;//Ŀ�ĵ�ַ,��һ��,����
	unsigned short seq;//���
	unsigned long ip;//IP��ַ
}Rt_Items;



typedef struct _rti_sub_frm//SRF��֡
{
	unsigned short flag_cnt_pb;//��֡���b15-b9: 1100001��Я�����b0,���
	unsigned short seq;//·�������
	unsigned long nbr_info;//�ڽڵ���Ϣ���ӵ�λ����λ��ÿ���ڵ�һ��bit
	unsigned long ip;
	Rt_Items rt_items[NODE_MAX_CNT];//·����
}RTI_Sub_Frm;



typedef struct _net_frm//����֡
{
	unsigned short pri_sa_da_mul_sn;//Դ��ַ,Ŀ�ĵ�ַ,���
	unsigned short sn_len;//���,����
	unsigned short net_paload[NET_PAYLOAD_SIZE];//����  750-2
}Net_Frm;

#define APP_TX_MALLOC_CNT_MAX 3//Ӧ�ò㷢������malloc����
#define READ_FM_MALLOC_CNT_MAX 8//�����緢������malloc����
#define RELAY_TX_MALLOC_CNT_MAX 10//�����緢������malloc����

//////////////////////////////////////////////////������Ϣ/////////////////////////////////////////////////////////////////
extern void AppRxInd(unsigned short *net_frm,unsigned short net_frm_len,unsigned short ta,unsigned short uc_bc_falg,unsigned short pri);//for test
extern void AppTxReq(unsigned short *dat,unsigned short len,unsigned short da,unsigned short pri,unsigned short hello_flag);
extern void AckTxReq(unsigned short *ack_sgl);//for test
extern void RstTxReq(unsigned short *rst_sgl);//for test
extern void DlcTxRsp(unsigned short * mac_frm);
extern unsigned short dat_tx_req_add_sig_flag; //�ź�dat_tx_req�Ѿ�������б��
extern unsigned short relay_tx_req_add_sig_flag;
extern void PrintDat(unsigned char* dat,unsigned short len,unsigned short first_dat);
extern unsigned short printdat_once[20];
extern unsigned short period_80ms_timer ;//����80ms��ʱ
extern unsigned short period_80ms_timer_flag ;//����80ms��ʱ
extern unsigned short malloc_cnt1;
extern unsigned long HELLO_SEND_TIMER ;//hello����������
extern unsigned short voice_busy_flag ;
extern	unsigned short add_sig_vtx_req_flag;//�Ѿ�����źű��
#endif

