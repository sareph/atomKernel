/*
 * Copyright (c) 2010, Kelvin Lawson. All rights reserved.
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
extern "C" {
#endif

#include <atomsync.h>
	
typedef struct atom_queue
{
	ATOM_SYNC_OBJECT aso;
	
    ATOM_TCB *  getSuspQ;       /* Queue of threads waiting to receive */
    uint8_t *   buff_ptr;       /* Pointer to queue data area */
    int32_t    unit_size;      /* Size of each message */
    int32_t    max_num_msgs;   /* Max number of storable messages */
    int32_t    insert_index;   /* Next byte index to insert into */
    int32_t    remove_index;   /* Next byte index to remove from */
    int32_t    num_msgs_stored;/* Number of messages stored */
} ATOM_QUEUE;

extern atom_status_t atomQueueInit(ATOM_QUEUE *qptr, uint8_t *buff_ptr, int32_t unit_size, int32_t max_num_msgs);
extern atom_status_t atomQueueDelete(ATOM_QUEUE *qptr);
extern atom_status_t atomQueueGet(ATOM_QUEUE *qptr, int32_t timeout, void *msgptr);
extern atom_status_t atomQueuePut(ATOM_QUEUE *qptr, int32_t timeout, const void *msgptr);

#ifdef __cplusplus
}
#endif
