#ifndef _MIB_H
#define _MIB_H

#include "../../test_info.h"
typedef struct _mib
{
	unsigned short local_id;	
}Mib;


extern void MibInit(Mib*);

#endif
