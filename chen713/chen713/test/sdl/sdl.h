#ifndef SDL_H
#define SDL_H

typedef struct _signal
{
	struct _signal* next;	//nextָ�룬�г��źŶ���������
	void* sdlc;				//sdl���й���ṹ��ָ��
	int (*func)(struct _signal*);//�źŵĺ���ָ��
	void *dst;					//�źŵ�Ŀ��ģ��ָ��
	void *param;				//�źŵĲ���ָ��
	void *src;					//�źŵ�Դģ��ָ��
	unsigned short pri;			//���ȼ�
	unsigned short enabled;		//ʹ��
	
}Signal;

typedef struct _timer
{
	struct _timer* next;
	void* sdlc;
	int (*func)(struct _signal*);
	void *dst;
	unsigned long long duration;	//����ʱ��
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

	Timer* tmr_que_head;			//��ʱ������ͷ
	Signal* sig_que_head[SDL_PRI_MAX];	//�źŶ���ͷָ������
	Signal* sig_que_rear[SDL_PRI_MAX];	//�źŶ���βָ������

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
