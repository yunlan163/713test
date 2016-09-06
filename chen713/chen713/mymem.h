#ifndef _MEM_H
#define _MEM_H
//#include "entity/common.h"
#include "test/common.h"

//网络包公用区malloc
#define MEM_NET_MAX_NUM  15
#define EACK_MEM_NET_BLK_SIZE NET_FRM_LEN_MAX
typedef struct _mem_net
{
	unsigned short mem_dat[EACK_MEM_NET_BLK_SIZE];
	struct _mem_net *  next;
}Mem_Net;

typedef struct _mem_ctrl_net
{
	Mem_Net* idle_mem_head;
	Mem_Net* idle_mem_rear;
	Mem_Net mem_net[MEM_NET_MAX_NUM];

}Mem_Ctrl_Net;

//MAC包公用区malloc
#define MEM_MAC_MAX_NUM  80
#define EACK_MEM_MAC_BLK_SIZE MAC_FRM_LEN_1002
typedef struct _mem_mac
{
	unsigned short mem_dat[EACK_MEM_MAC_BLK_SIZE];  //63字
	struct _mem_mac *  next;
}Mem_Mac;

typedef struct _mem_ctrl_mac
{
	Mem_Mac* idle_mem_head;  //头
	Mem_Mac* idle_mem_rear;   //尾
	Mem_Mac  mem_mac[MEM_MAC_MAX_NUM];  //80个

}Mem_Ctrl_Mac;

extern unsigned short* MemAllocMac();
extern void MemFreeMac(unsigned short* p_dat);
extern void MemInitNet();
extern void MemInitMac();
extern unsigned short* MemAllocNet();
extern void MemFreeNet(unsigned short* p_dat);
#endif

