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
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <atomdefs.h>
#include "atom.h"

#define ATOM_SYNC_TYPE_INVALID 0x0000
#define ATOM_SYNC_TYPE_SEM 0x0001
#define ATOM_SYNC_TYPE_MUTEX 0x0002
#define ATOM_SYNC_TYPE_QUEUE 0x0003
#define ATOM_SYNC_TYPE_EVENT 0x0004
	
typedef struct atom_sycn_object
{
	uint_fast8_t type;
	union
	{
		ATOM_TCB *  suspQ; /* Queue of threads suspended on this object */
		ATOM_TCB *  putSuspQ; /* Queue of threads waiting to send */
	};
} ATOM_SYNC_OBJECT;

extern atom_status_t atomLock(ATOM_SYNC_OBJECT *pSo, int_fast8_t tineout);
extern atom_status_t atomUnlock(ATOM_SYNC_OBJECT *pSo, int_fast8_t timeout);

#ifdef __cplusplus
}
#endif

