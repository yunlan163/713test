
#ifndef _TEST_UNFO_H
#define _TEST_UNFO_H

typedef struct _test_info //for test
{
	unsigned long snd_net_pkt_bc;//���͹㲥
	unsigned long snd_net_pkt_uc;//���͵���
	unsigned long snd_net_pkt_gc;//���͵���
	unsigned long snd_fail_net_pkt_uc;//����ʧ��uc

	unsigned long rcv_net_pkt_uc;//����	
	unsigned long rcv_net_pkt_bc;//����	
	unsigned long rcv_net_pkt_gc;//����

	unsigned short vrx_road_cnt[10];
	unsigned short vrx_road_cnt_idx;
	//unsigned short vctrl_road_cnt[10];
	//unsigned short vctrl_road_cnt_idx;
	//unsigned short rcv_net_frm_id;
	//unsigned short lost_net_frm_cnt;
	//unsigned short rcv_udp_net_frm_cnt;
	//unsigned short snd_udp_net_frm_cnt;
}Test_Info;

extern Test_Info test_info;
#endif
