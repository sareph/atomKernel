/*
 * Copyright (c) 2015, Tido Klaassen. All rights reserved.
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

#ifndef __ATOM_PORT_H
#define __ATOM_PORT_H

#include "arch/cpu.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "atomdefs.h"
#include "atomconfig.h"

/* Required number of system ticks per second (normally 100 for 10ms tick) */
#define SYSTEM_TICKS_PER_SEC            1000
#define SYSTEM_MS_PER_TICK				(1000 / SYSTEM_TICKS_PER_SEC)

/* Size of each stack entry / stack alignment size (4 bytes on Cortex-M without FPU) */
#define STACK_ALIGN_SIZE    sizeof(uint32_t)

#define ALIGN(x, a)         ((x + (typeof(x))(a) - 1) & ~((__typeof__(x))(a) - 1))
#define PTR_ALIGN(p, a)     ((__typeof__(p))ALIGN((uint32_t)(p), (a)))
#define STACK_ALIGN(p, a)   (__typeof__(p))((__typeof__(a))(p) & ~((a) - 1))

#define POINTER             void *
#define UINT32              uint32_t

#define likely(x)           __builtin_expect(!!(x), 1)
#define unlikely(x)         __builtin_expect(!!(x), 0)
#define __maybe_unused      __attribute__((unused))

#define assert_static(e) \
   do { \
      enum { assert_static__ = 1 / (e) } ; \
   } while (0)

/**
 * Critical region protection: this should disable interrupts
 * to protect OS data structures during modification. It must
 * allow nested calls, which means that interrupts should only
 * be re-enabled when the outer CRITICAL_END() is reached.
 */
#define CRITICAL_STORE      int __irq_flags
#define CRITICAL_START()    __irq_flags = __get_PRIMASK(); __disable_irq();
#define CRITICAL_END()      __set_PRIMASK(__irq_flags);

/**
 * When using newlib, define port private field in atom_tcb to be a
 * struct _reent.
 */
#if defined(__NEWLIB__) && ATOM_NEWLIB_REENT
struct cortex_port_priv {
    struct _reent reent;
};

#define THREAD_PORT_PRIV    struct cortex_port_priv port_priv
#endif

void atomOSSysTickHander(void);
atom_status_t atomPortInit();

/* Uncomment to enable stack-checking */
/* #define ATOM_STACK_CHECKING */

#endif /* __ATOM_PORT_H */
