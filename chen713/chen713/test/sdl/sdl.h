#ifndef SDL_H
#define SDL_H

typedef struct _signal
{
	struct _signal* next;	//next指针，行成信号队列链表用
	void* sdlc;				//sdl队列管理结构体指针
	int (*func)(struct _signal*);//信号的函数指针
	void *dst;					//信号的目的模块指针
	void *param;				//信号的参数指针
	void *src;					//信号的源模块指针
	unsigned short pri;			//优先级
	unsigned short enabled;		//使能
	
}Signal;

typedef struct _timer
{
	struct _timer* next;
	void* sdlc;
	int (*func)(struct _signal*);
	void *dst;
	unsigned long long duration;	//到达时刻
	unsigned short pri;
	unsigned short enabled;
	
}Timer;

#define SDL_PRI_MAX 4
#define SDL_PRI_NORM 2
#define SDL_PRI_LOW  3
#define SDL_PRI_HIGH 1
#define SDL_PRI_URG  0

typedef struct
{

	Timer* tmr_que_head;			//定时器队列头
	Signal* sig_que_head[SDL_PRI_MAX];	//信号队列头指针数组
	Signal* sig_que_rear[SDL_PRI_MAX];	//信号队列尾指针数组

}Sdl_Core;

extern unsigned long long GetTime(Sdl_Core* sdlc);

extern int AddSignal(Signal *sig);
extern int AddSignalIrq(Signal *sig);
extern void ExecSignal(Signal *sig);
//extern int SetTimer(Timer *tmr,unsigned int duration);
extern void CancelTimer(Timer *tmr);
extern void SdlCoreInit(Sdl_Core* sdlc);
extern void SdlCoreRun(Sdl_Core* sdlc);
extern int CheckSignal(Sdl_Core* sdlc);
extern void CheckTimer(Sdl_Core* sdlc);

#endif
