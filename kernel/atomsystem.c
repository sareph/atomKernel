/*
 * Copyright (c) 2020, Tomek Nagisa. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. No personal names or organizations' names associated with the
 *    Atomthreads project may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ATOMTHREADS PROJECT AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "atom.h"
#include "atomdefs.h"
#include "atomsystem.h"
#include <sys/debug.h>

#if ATOM_SYSTEM_WRAPPER_ENABLE

#if ATOM_STACK_CHECKING
#include <string.h>
static ATOM_TCB *lTcbPos[16];
static size_t lCurrentTcb;
extern ATOM_TCB *atomGetIdleTcb();
#endif

static size_t lKernelMemPos;
static uint8_t lKernelMem[ATOM_SYSTEM_MEM_SIZE] ATOM_SYSTEM_MEM_ATTRIBUTE;

/**
 * \b atomOSInit
 *
 * Initializes main stack & tcb's.
 *
 * @param[in] idleThreadStack Idle thread stack size
 * @param[in] idleThreadParam Idle thread hook param
 *
 * @return None
 */

atom_status_t atomOSInit(size_t idleThreadStack, void *idleThreadParam)
{
	atom_status_t ret;
	
	if ((ret = atomPortInit()) != ATOM_OK)
	{
		atom_assert(0, "Port init failed");
		return ret;
	}
	
	lKernelMemPos = 0;
	lCurrentTcb = 0;
	
#if ATOM_STACK_CHECKING
	memset(&lTcbPos[0], 0, sizeof(ATOM_TCB) * 16);
#endif
	
	if((ret = atomKernelInit((void*)&lKernelMem[lKernelMemPos], idleThreadStack, idleThreadParam)) != ATOM_OK)
	{
		atom_assert(0, "Kernel init falied");
		return ret;
	}
	
	lKernelMemPos += idleThreadStack;
	return ret;
}

/**
 * \b atomOSCreateThread
 *
 * Creates thread using system stack & tcb
 *
 * @param[in] threadStack Idle thread stack size
 * @param[in] priority Idle thread hook param
 * @param[in] threadParam Idle thread hook param
 * @param[in] entryPoint Thread entry point
 * @param[in] param Param for the entry point
 * @param[out] pTcb Pointer to TCB for created thread
 *
 * @retval >=0 Sucess & thread index
 * @retval ATOM_ERR_PARAM Bad parameters
 * @retval ATOM_ERR_NO_MEM System stack exhausted
 * @retval ATOM_ERR_QUEUE Error putting the thread on the ready queue
 */

atom_status_t atomOSCreateThread(size_t threadStack, atom_prio_t priority, _fnAtomThread entryPoint, void *param, ATOM_TCB **pTcb)
{
	atom_status_t ret;
	
	if ((lKernelMemPos + threadStack + sizeof(ATOM_TCB)) > ATOM_SYSTEM_MEM_SIZE)
	{
		atom_assert(0, "No more space in system memory storage area");
		return ATOM_ERR_NO_MEM;
	}

	if (threadStack == 0)
	{
		threadStack = 1024;
	}
	
	if ((ret = atomThreadCreate((ATOM_TCB*)&lKernelMem[lKernelMemPos], priority, entryPoint, param, (void*)&lKernelMem[lKernelMemPos + sizeof(ATOM_TCB)], threadStack)) != ATOM_OK)
	{
		atom_assert(0, "Thread creation falied");
		return ret;
	}
	
	ret = lKernelMemPos;
	
	if (pTcb)
	{
		*pTcb = (ATOM_TCB*)&lKernelMem[lKernelMemPos];
	}

	lTcbPos[lCurrentTcb++] = (ATOM_TCB*)&lKernelMem[lKernelMemPos];
	
	lKernelMemPos += sizeof(ATOM_TCB);
	lKernelMemPos += threadStack;
	
	return ret;
}

/**
 * \b atomOSStart
 *
 * Starts the OS
 *
 * @return None
 */

void atomOSStart(void)
{
	atomKernelStart();
}

#if ATOM_STACK_CHECKING
void atomOSCheckStack()
{
	uint32_t ub, fb;
	debugLog("---------\r\n");
	for (int i = 0; i < lCurrentTcb; ++i)
	{
		atomThreadStackCheck(lTcbPos[i], &ub, &fb);
		debugLog("Thread %02d: %d used, %d free\r\n", i, ub, fb);
	}

	atomThreadStackCheck(atomGetIdleTcb(), &ub, &fb);
	debugLog("Thread %02d: %d used, %d free\r\n", -1, ub, fb);

	debugLog("Total: %d of %d bytes\r\n", lKernelMemPos, ATOM_SYSTEM_MEM_SIZE);
}

#endif
#endif