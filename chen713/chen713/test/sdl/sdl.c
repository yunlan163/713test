 #include "sdl.h"

#define NULL	0

unsigned long long SysGetTime()
{
	return 1;
}

//static void DisIrq(void)
//{
//	__asm ("swi 0");
//}

//static void EnIrq(void)
//{
//	__asm ("swi 1");
//}
static void TmrInit(Sdl_Core* sdlc)
{
	sdlc->tmr_que_head = 0;	
	return;
}
extern unsigned long long SysGetTime(void);

unsigned long long GetTime(Sdl_Core* sdlc)//ms
{//
	//return 0;// SysSimGetTime()/10;//
	return SysGetTime();
	//return 0;
}
static void SigQueInit(Sdl_Core* sdlc)
{
	int i;
	for(i=0;i<SDL_PRI_MAX;i++)
	{
		sdlc->sig_que_head[i]=0;
		sdlc->sig_que_rear[i]=0;
	}	
	return;
}

int AddSignalIrq(Signal *sig)
{
	Sdl_Core* sdlc =(Sdl_Core*)sig->sdlc;
	Signal *tmp_sig;
	unsigned short pri = sig->pri;
//	unsigned char printdat_once[] = "  AddSignal ERROR ";

	if((!sig->func) ||(pri>=SDL_PRI_MAX))
		return 0;
	sig->next = 0;
	sig->enabled = 1;

	tmp_sig = sdlc->sig_que_head[0];
	while(tmp_sig != NULL)
	{
		if(tmp_sig == sig)
		{
			//PrintDat(printdat_once,sizeof(printdat_once),1);
			break;
		}
		tmp_sig = tmp_sig->next;
	}





	if(sdlc->sig_que_rear[pri])
	{
		sdlc->sig_que_rear[pri]->next = sig;
	}
	else
	{
		sdlc->sig_que_head[pri]=sig;
	}
	sdlc->sig_que_rear[pri] =  sig;

	//if((sdlc->sig_que[pri].rear+1)==sdlc->sig_que[pri].front)
	//	return 0;
	//	
	//sdlc->sig_que[pri].que[sdlc->sig_que[pri].rear++] = sig;
	//sig->enabled = 1;
	return 1;
}

int AddSignal(Signal *sig)
{	
	int ret;
	//DisIrq();
	ret = AddSignalIrq(sig);
	//EnIrq();
	return ret;
}

void ExecSignal(Signal *sig)
{
	sig->next = 0;
	if(sig->enabled)
	{
		sig->enabled = 0;
		if(sig->func(sig))//返回0说明信号正确执行
		{
			AddSignal(sig);	
		}
	}
}

int CheckSignalPri(Sdl_Core* sdlc,unsigned short pri)
{
	Signal *sig = 0;
	
	//DisIrq();
	if(sdlc->sig_que_head[pri])
	{
		sig = sdlc->sig_que_head[pri];
		if(sdlc->sig_que_head[pri] == sdlc->sig_que_rear[pri])
		{
			sdlc->sig_que_head[pri] =0;
			sdlc->sig_que_rear[pri] =0;
  		}
		else
		{
			sdlc->sig_que_head[pri] = sig->next;
		}
	}
	//EnIrq();
	if(sig)
	{
		ExecSignal(sig);
	}
	return (int)sig;
}
int CheckSignal(Sdl_Core* sdlc)
{
	int i;
	for(i=0;i<SDL_PRI_MAX;i++)
	{
		if(CheckSignalPri(sdlc,i))
		{
			break;
		}
	}
	return SDL_PRI_MAX-i;
}
int SetTimer(Timer *tmr,unsigned int duration)
{
	Timer * p_tmr,*c_tmr;
	Sdl_Core* sdlc = (Sdl_Core*)tmr->sdlc;

	tmr->duration = GetTime(sdlc)+duration;
//	tmr->enabled = 1;

	//DisIrq();

	p_tmr = 0;

	c_tmr = sdlc->tmr_que_head;
	while(c_tmr)
	{
		if(c_tmr->duration <= tmr->duration)
		{
			p_tmr = c_tmr;
			c_tmr = c_tmr->next;
		}
		else
		{
			break;
		}
	}
	if(p_tmr)
	{
		p_tmr->next = tmr;		
	}
	else
	{
		sdlc->tmr_que_head = tmr;		
	}
	tmr->next = c_tmr;
	//EnIrq();
	return 1;
}

void CheckTimer(Sdl_Core* sdlc)
{
	Timer * c_tmr;
	unsigned long long c_time = GetTime(sdlc);

	while(sdlc->tmr_que_head)
	{
		if(sdlc->tmr_que_head->duration <= c_time)
		{
			c_tmr = sdlc->tmr_que_head;
			sdlc->tmr_que_head = c_tmr->next;
			if(c_tmr->enabled)
			{
				ExecSignal((Signal*)c_tmr);
				//AddSignalIrq((Signal*)c_tmr);
			}
		}
		else
		{
			break;
		}
		
	}
}

void CancelTimer(Timer* tmr)
{
	Timer * p_tmr,*c_tmr;
	Sdl_Core* sdlc = (Sdl_Core*)tmr->sdlc;
	
	p_tmr = 0;
	c_tmr = sdlc->tmr_que_head;
	while(c_tmr)
	{
		if(c_tmr != tmr)
		{
			p_tmr = c_tmr;
			c_tmr = c_tmr->next;
		}
		else
		{
			if(p_tmr)
			{
				p_tmr->next = c_tmr->next;
			}
			else
			{
				sdlc->tmr_que_head = c_tmr->next;
			}

			break;
		}
	}
}

void SdlCoreInit(Sdl_Core* sdlc)
{		
	SigQueInit(sdlc);
	TmrInit(sdlc);
}

void SdlCoreRun(Sdl_Core* sdlc)
{
	while(1)
	{		
		CheckSignal(sdlc);
		CheckTimer(sdlc); 
	}
}

 

