#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "atom.h"
#include "atomdefs.h"
#include "atomsystem.h"

static size_t lMasterStackSize;
static uint8_t lMasterStack[ATOM_SYSTEM_STACK_SIZE] ATOM_SYSTEM_STACK_ATTRIBUTE;
static ATOM_TCB lTCBs[ATOM_SYSTEM_MAX_THREADS] ATOM_SYSTEM_STACK_ATTRIBUTE;
static size_t lCurrentTcb;

atom_status_t atomOSInit(size_t idleThreadStack)
{
	atom_status_t ret;
	
	if ((ret = atomPortInit()) != ATOM_OK)
	{
		return ret;
	}
	
	lMasterStackSize = 0;
	lCurrentTcb = 0;
	
	if ((ret = atomKernelInit((void*)&lMasterStack[lMasterStackSize], idleThreadStack)) != ATOM_OK)
	{
		return ret;
	}
	
	lMasterStackSize += idleThreadStack;
	return ret;
}

atom_status_t atomOSCreateThread(size_t threadStack, atom_prio_t priority, _fnAtomThread entryPoint, void *param, ATOM_TCB **pTcb)
{
	atom_status_t ret;
	
	if ((lMasterStackSize + threadStack) > ATOM_SYSTEM_STACK_SIZE)
	{
		return ATOM_ERR_NO_MEM;
	}

	if ((ret = atomThreadCreate(&lTCBs[lCurrentTcb], priority, entryPoint, param, (void*)&lMasterStack[lMasterStackSize], threadStack)) != ATOM_OK)
	{
		return ret;
	}
	
	ret = lCurrentTcb;
	lMasterStackSize += threadStack;
	
	if (pTcb)
	{
		*pTcb = &lTCBs[lCurrentTcb];
	}
	
	lCurrentTcb++;
	return ret;
}