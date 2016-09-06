
#ifndef _TEST_UNFO_H
#define _TEST_UNFO_H

typedef struct _test_info //for test
{
	unsigned long snd_net_pkt_bc;//发送广播
	unsigned long snd_net_pkt_uc;//发送单播
	unsigned long snd_net_pkt_gc;//发送单播
	unsigned long snd_fail_net_pkt_uc;//发送失败uc

	unsigned long rcv_net_pkt_uc;//接收	
	unsigned long rcv_net_pkt_bc;//接收	
	unsigned long rcv_net_pkt_gc;//接收

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
