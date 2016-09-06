#include "mymem.h"
#include <string.h>
/////////////////////网络报公用区malloc////////////////////////////////

Mem_Ctrl_Net mem_ctrl_net;
unsigned short malloc_cnt1 = 0;  //yun:申请内存计数
unsigned short net_dat_space_busy = 0;//5509与ARM流控,标记网络层空间已占满，不允许arm再发数据

void MemInitNet()
{
	unsigned short i;
	memset(&mem_ctrl_net,0,sizeof(Mem_Ctrl_Net));
	mem_ctrl_net.idle_mem_head=&mem_ctrl_net.mem_net[0];
	for(i=0;i<(MEM_NET_MAX_NUM-1);i++)
	{
		mem_ctrl_net.mem_net[i].next= & mem_ctrl_net.mem_net[i+1];

	}
	mem_ctrl_net.idle_mem_rear=&mem_ctrl_net.mem_net[MEM_NET_MAX_NUM-1];
}
unsigned short* MemAllocNet()
{
	Mem_Net* p_mem;
	if(!mem_ctrl_net.idle_mem_head)
	{
		//ARMRx("error! no memory available!",40,0);
		return (unsigned short*)0;
	}
	p_mem=mem_ctrl_net.idle_mem_head;
	mem_ctrl_net.idle_mem_head = mem_ctrl_net.idle_mem_head->next;
	if(!mem_ctrl_net.idle_mem_head)
	{
		mem_ctrl_net.idle_mem_rear=0;
	}
	p_mem->next=0;
	memset(p_mem->mem_dat,0,EACK_MEM_NET_BLK_SIZE);
	malloc_cnt1 ++;
	if(malloc_cnt1 > MEM_NET_MAX_NUM -5)//5509与ARM流控
	{
		net_dat_space_busy = 1;
	}
	else
	{
		net_dat_space_busy = 0;
	}
	return (unsigned short*)(p_mem);
}

void MemFreeNet(unsigned short* p_dat)
{
	unsigned char printdat_once[] = "  MemFreeNet ERROR ";
	Mem_Net* p_mem=(Mem_Net*)p_dat;
	malloc_cnt1--;
	p_mem->next=0;
	if(!p_mem->mem_dat)
	{
		//PrintDat(printdat_once,sizeof(printdat_once),1);
	}
	memset(p_mem->mem_dat,0,EACK_MEM_NET_BLK_SIZE);
	if(!mem_ctrl_net.idle_mem_head)
	{
		mem_ctrl_net.idle_mem_head=p_mem;
		mem_ctrl_net.idle_mem_rear=p_mem;
		return;
	}
	else
	{
		mem_ctrl_net.idle_mem_rear->next=p_mem;
		mem_ctrl_net.idle_mem_rear=p_mem;
		return;
	}
}

//////////////////////////////MAC包公用区malloc///////////////////////////////////////

Mem_Ctrl_Mac mem_ctrl_mac;
volatile unsigned short malloc_cnt = 0;
unsigned short error_flag = 0;

void MemInitMac()  //yun:队列 链表
{
	unsigned short i;
	memset(&mem_ctrl_mac,0,sizeof(Mem_Ctrl_Mac));
	mem_ctrl_mac.idle_mem_head=&mem_ctrl_mac.mem_mac[0];
	for(i=0;i<(MEM_MAC_MAX_NUM-1);i++)
	{
		mem_ctrl_mac.mem_mac[i].next= & mem_ctrl_mac.mem_mac[i+1];

	}
	mem_ctrl_mac.idle_mem_rear=&mem_ctrl_mac.mem_mac[MEM_MAC_MAX_NUM-1];
}
unsigned short* MemAllocMac()
{
	Mem_Mac* p_mem;
	if(!mem_ctrl_mac.idle_mem_head)  //头为空
	{
		//ARMRx("error! no memory available!",40,0);
		return (unsigned short*)0;
	}
	p_mem=mem_ctrl_mac.idle_mem_head;
	mem_ctrl_mac.idle_mem_head = mem_ctrl_mac.idle_mem_head->next;
	if(!mem_ctrl_mac.idle_mem_head)
	{
		mem_ctrl_mac.idle_mem_rear=0;
	}
	p_mem->next=0;
	memset(p_mem->mem_dat,0,EACK_MEM_MAC_BLK_SIZE);
	malloc_cnt++;
	return (unsigned short*)(p_mem);
}
//yun：内存释放
void MemFreeMac(unsigned short* p_dat)
{
	Mem_Mac* p_mem=(Mem_Mac*)p_dat;
	unsigned char printdat_once[] = "  MemFreeMac ERROR ";
	malloc_cnt --;
	p_mem->next=0;
	if(!p_mem->mem_dat)
	{
		//PrintDat(printdat_once,sizeof(printdat_once),1);
	}
	memset(p_mem->mem_dat,0,EACK_MEM_MAC_BLK_SIZE);
	if(!mem_ctrl_mac.idle_mem_head)
	{
		mem_ctrl_mac.idle_mem_head=p_mem;
		mem_ctrl_mac.idle_mem_rear=p_mem;
		error_flag = 1;
		return;
	}
	else
	{
		mem_ctrl_mac.idle_mem_rear->next=p_mem;
		mem_ctrl_mac.idle_mem_rear=p_mem;
		return;
	}
}
