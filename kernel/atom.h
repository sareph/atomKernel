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

#ifndef __ATOM_H
#define __ATOM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "atomtimer.h"
#include "atomport.h"
#include "atomconfig.h"

/* Data types */

/* Forward declaration */
struct atom_tcb;

#include "atomdefs.h"

#ifdef atom_port_assert
#define atom_assert(x,m) atom_port_assert(x,m)
#else
#define atom_assert(x,m)
#endif

typedef atom_status_t (*_fnAtomThread)(void *ptr);
	
typedef struct atom_tcb
{
    /*
     * Thread's current stack pointer. When a thread is scheduled
     * out the architecture port can save its stack pointer here.
     */
    POINTER sp_save_ptr;

    /* Thread's port specific private data */
#if defined(THREAD_PORT_PRIV)
    THREAD_PORT_PRIV;
#endif

    /* Thread entry point and parameter */
	_fnAtomThread entry_point;
    void* entry_param;
	atom_status_t return_value;
	
    /* Queue pointers */
    struct atom_tcb *prev_tcb;    /* Previous TCB in doubly-linked TCB list */
    struct atom_tcb *next_tcb;    /* Next TCB in doubly-linked list */

    /* Suspension data */
    ATOM_TIMER *suspend_timo_cb;  /* Callback registered for suspension timeouts */

    /* Details used if thread stack-checking is required */
#ifdef ATOM_STACK_CHECKING
    POINTER stack_bottom;         /* Pointer to bottom of stack allocation */
    uint32_t stack_size;          /* Size of stack allocation in bytes */
#endif
	uint32_t	suspend_option; /* yujs used for event to save the event flags  get option*/
	uint32_t	suspend_info; /* Remember which event flags we are looking for. requested_flags yujs */
	
	atom_status_t suspend_wake_status; /* Status returned to woken suspend calls */
	atom_prio_t priority; /* Thread priority (-1 - 127) */
#if ATOM_KERNEL_TCB_AGEING
	atom_prio_t base_priority; /* Base thread priority (0 - 127) */
#endif
	int8_t terminated; /* TRUE if task is being terminated (run to completion) */
	int8_t suspended; /* TRUE if task is currently suspended */
	
} ATOM_TCB;


/* Global data */
extern ATOM_TCB *tcbReadyQ;
extern uint8_t atomOSStarted;


/* Constants */
#define TRUE                    1
#define FALSE                   0

/* Error values */

#define ATOM_OK                  0
#define ATOM_ERROR              -1
#define ATOM_TIMEOUT            -2
#define ATOM_WOULDBLOCK         -3
#define ATOM_ERR_CONTEXT        -200
#define ATOM_ERR_PARAM          -201
#define ATOM_ERR_DELETED        -202
#define ATOM_ERR_OVF            -203
#define ATOM_ERR_QUEUE          -204
#define ATOM_ERR_TIMER          -205
#define ATOM_ERR_NOT_FOUND      -206
#define ATOM_ERR_OWNERSHIP      -207
#define ATOM_ERR_RUNNING        -208
#define ATOM_ERR_NO_MEM         -209

/* Idle thread priority (lowest) */
#define IDLE_THREAD_PRIORITY    127
#define IDLE_MINIMUM_PRIORITY   127


/* Function prototypes */
extern atom_status_t atomKernelInit(void *idle_thread_stack_bottom, uint32_t idle_thread_stack_size, void *idleThreadParam);
extern void atomKernelStart (void);

extern void atomSched (uint8_t timer_tick);

extern void atomIntEnter (void);
extern void atomIntExit (uint8_t timer_tick);
	
extern int atomGetIdleTime();

extern uint8_t tcbEnqueuePriority (ATOM_TCB **tcb_queue_ptr, ATOM_TCB *tcb_ptr);
extern ATOM_TCB *tcbDequeueHead (ATOM_TCB **tcb_queue_ptr);
extern ATOM_TCB *tcbDequeueEntry (ATOM_TCB **tcb_queue_ptr, ATOM_TCB *tcb_ptr);
extern ATOM_TCB *tcbDequeuePriority(ATOM_TCB **tcb_queue_ptr, atom_prio_t priority);

extern ATOM_TCB *atomCurrentContext (void);

extern atom_status_t atomThreadCreate(ATOM_TCB *tcb_ptr, atom_prio_t priority, _fnAtomThread entry_point, void* entry_param, void *stack_bottom, uint32_t stack_size);
extern atom_status_t atomThreadDestroy(ATOM_TCB *tcb_ptr, int *ret_value);
extern atom_status_t atomThreadStackCheck(ATOM_TCB *tcb_ptr, uint32_t *used_bytes, uint32_t *free_bytes);

extern void archContextSwitch (ATOM_TCB *old_tcb_ptr, ATOM_TCB *new_tcb_ptr);
extern void archThreadContextInit(ATOM_TCB *tcb_ptr, void *stack_top, _fnAtomThread entry_point, void* entry_param);
extern void archFirstThreadRestore(ATOM_TCB *new_tcb_ptr);

extern atom_status_t atomSleep(uint32_t ms);
extern void atomTimerTick (void);

#ifdef __cplusplus
}
#endif

#endif /* __ATOM_H */
