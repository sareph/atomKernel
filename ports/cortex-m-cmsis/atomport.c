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

#include <string.h>

#include "sys/debug.h"
#include "atom.h"
#include "atomport.h"
#include "atomport-private.h"
#include "asm_offsets.h"

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#define THUMB_2
#endif

#if defined(__VFP_FP__) && !defined(__SOFTFP__)
#define WITH_FPU
#endif

#if ATOM_NEWLIB_MALLOC_HOOKS
#include <malloc.h>

static int32_t lMallocLocks = 0;
static uint32_t lPrimask;

void __malloc_lock(struct _reent *reent)
{
	if (lMallocLocks == 0)
	{
		lPrimask = __get_PRIMASK();
		__disable_irq();
	}
	++lMallocLocks;
}

void __malloc_unlock(struct _reent *reent)
{
	--lMallocLocks;
	if (lMallocLocks == 0)
	{
		__set_PRIMASK(lPrimask);
	}
}
#endif

static void thread_shell(void);

struct task_switch_info ctx_switch_info =// asm("CTX_SW_NFO") =
{
    .running_tcb = NULL,
    .next_tcb    = NULL,
};

void archFirstThreadRestore(ATOM_TCB *new_tcb_ptr)
{
#if defined(__NEWLIB__) && ATOM_NEWLIB_REENT
    ctx_switch_info.reent = &(new_tcb_ptr->port_priv.reent);
    __DSB();
#endif

	__disable_irq();
	
	ctx_switch_info.next_tcb = new_tcb_ptr;
	ctx_switch_info.running_tcb = new_tcb_ptr;
	
    //_archFirstThreadRestore(new_tcb_ptr);
	__asm volatile 
	(	
    /**
     * Reset main stack pointer to initial value, which is the first entry
     * in the vector table.
     */
    "ldr     r1,         = g_pfnVectors			\n"
    "ldr     r1,         [r1, #0]				\n"
    "msr     MSP,        r1						\n"
    
    /* Load ctx_switch_info */
	"ldr     r1,         = ctx_switch_info		\n"

#if defined(__NEWLIB__) && ATOM_NEWLIB_REENT
    /**
     * Store the thread's reentry context address in _impure_ptr. This
     * will have been stored in ctx_switch_info.reent.
     */ 
    "ldr     r2,         [r1, #8]				\n"
    "ldr     r3,         = _impure_ptr			\n"
    "str     r2,         [r3, #0]				\n"
#endif

    /* Get thread stack pointer from tcb. Conveniently the first element */
    "ldr     r1,         [r0, #0]				\n"
    "msr     PSP,        r1						\n"

    /**
     * Set bit #1 in CONTROL. Causes switch to PSP, so we can work directly
     * with SP now and use pop/push.
     */
    "movs    r1,         #2						\n"
    "mrs     r2,         CONTROL				\n"
    "orrs    r2,         r2,     r1				\n"
    "msr     CONTROL,    r2						\n"

    /**
     * Initialise thread's register context from its stack frame. Since this
     * function gets called only once at system start up, execution time is
     * not critical. We can get away with using only Thumb-1 instructions that
     * will work on all Cortex-M devices.
     *
     * Initial stack looks like this:
     * xPSR
     * PC
     * lr
     * r12
     * r3
     * r2
     * r1
     * r0
     * exc_ret <- ignored here
     * r11
     * r10
     * r9
     * r8
     * r7
     * r6
     * r5
     * r4 <- thread's saved_sp points here
     */

    /**
     *
     * Move SP to position of r8 and restore high registers by loading
     * them to r4-r7 before moving them to r8-r11
     */
    "add     SP,         #16					\n"
    "pop     {r4-r7}							\n"
    "mov     r8,         r4						\n"
    "mov     r9,         r5						\n"
    "mov     r10,        r6						\n"
    "mov     r11,        r7						\n"

    /* move SP back to top of stack and load r4-r7 */
    "sub     SP,         #32					\n"
    "pop     {r4-r7}							\n"

    /*load r12, lr, pc and xpsr to r0-r3 and restore r12 and lr */
    "add     SP,         #36					\n"
    "pop     {r0-r3}							\n"
    "mov     r12,        r0						\n"
    "mov     lr,         r1						\n"

    /**
     * r2 contains the PC and r3 APSR, SP is now at the bottom of the stack. We
     * can't initialise APSR now because we will have to do a movs later when
     * enabling interrupts, so r3 must not be touched. We also need an extra
     * register holding the value that will be moved to PRIMASK. To do this,
     * we build a new stack containing only the initial values of r2, r3
     * and pc. In the end this will be directly popped into the registers,
     * finishing the thread restore and branching to the thread's entry point.
     */

    /* Save PC value */
    "push    {r2}								\n"

    /* Move values for r2 and r3 to lie directly below value for pc */
    "sub     SP,         #20					\n"
    "pop     {r1-r2}							\n"
    "add     SP,         #12					\n"
    "push    {r1-r2}							\n"

    /* Load values for r0 and r1 from stack */
    "sub     SP,         #20					\n"
    "pop     {r0-r1}							\n"

    /* Move SP to start of our new r2,r3,pc mini stack */
    "add     SP,         #12					\n"

    /* Restore xPSR and enable interrupts */
    "msr     APSR_nzcvq, r3						\n"
	"cpsie	 i									\n"
	/* If any interrupts are pending */
	"isb										\n"
    
    /* Pop r2,r3,pc from stack, thereby jumping to thread entry point */
    "pop     {r2,r3,pc}							\n"
	);
}

/**
 * We do not perform the context switch directly. Instead we mark the new tcb
 * as should-be-running in ctx_switch_info and trigger a PendSv-interrupt.
 * The pend_sv_handler will be called when all other pending exceptions have
 * returned and perform the actual context switch.
 * This way we do not have to worry if we are being called from task or
 * interrupt context, which would mean messing with either main or thread
 * stack format.
 *
 * One difference to the other architectures is that execution flow will
 * actually continue in the old thread context until interrupts are enabled
 * again. From a thread context this should make no difference, as the context
 * switch will be performed as soon as the execution flow would return to the
 * calling thread. Unless, of course, the thread called atomSched() with
 * disabled interrupts, which it should not do anyways...
 */
void __attribute__((noinline))
archContextSwitch(ATOM_TCB *old_tcb_ptr __maybe_unused, ATOM_TCB *new_tcb_ptr)
{
    if(likely(ctx_switch_info.running_tcb != NULL)){
        ctx_switch_info.next_tcb = new_tcb_ptr;
#if defined(__NEWLIB__) && ATOM_NEWLIB_REENT
        ctx_switch_info.reent = &(new_tcb_ptr->port_priv.reent);
#endif
		__DSB();

        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    }
}

void sys_tick_handler(void)
{
    /* Call the interrupt entry routine */
    atomIntEnter();

    /* Call the OS system tick handler */
    atomTimerTick();

    /* Call the interrupt exit routine */
    atomIntExit(TRUE);
}

void __attribute__((naked)) PendSV_Handler()
{
	__disable_irq();
	SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;
	
	if (ctx_switch_info.running_tcb != ctx_switch_info.next_tcb)
	{
		__asm volatile 
		(
			"ldr     r0,         = ctx_switch_info	\n"
			"ldr     r1,         [r0, #0]			\n"
			"ldr     r2,         [r0, #4]			\n"

			//"cmp     r1,         r2					\n"
			//"beq     no_switch						\n"

    /**
     * Copy running thread's process stack pointer to r3 and use it to push
     * the thread's register context on its stack
     */
			"mrs     r3,         PSP				\n"

#if defined(THUMB_2)
    /**
     * Save old thread's context on Cortex-M[34]
     */
	 
#if defined(WITH_FPU)
    /* Check if FPU was used by thread and store registers if necessary */
			"tst     lr,         #1 << 4			\n"
			"it      eq								\n"
			"vstmdbeq  r3!,      {s16-s31}			\n"
	
    /**
     * TODO: Defer stacking FPU context by disabling FPU and using a
     * fault handler to store the FPU registers if another thread
     * tries using it
     */
#endif // WITH_FPU
    
    /* Push running thread's remaining registers on stack */
			"stmdb   r3!,        {r4-r11, lr}		\n"

#else // !THUMB2

    /**
     * Save old thread's register context on Cortex-M0.
     * Push running thread's remaining registers on stack.
     * Thumb-1 can use stm only on low registers, so we
     * have to do this in two steps.
     */

    /* Reserve space for r8-r11 + exc_return before storing r4-r7 */
			"subs    r3,         r3,     #36		\n"
			"stmia   r3!,        {r4-r7}			\n"

    /**
     * Move r8-r11 to low registers and use store multiple with automatic
     * post-increment to push them on the stack
     */
			"mov     r4,         r8					\n"
			"mov     r5,         r9					\n"
			"mov     r6,         r10				\n"
			"mov     r7,         r11				\n"
			"stmia   r3!,        {r4-r7}			\n"

    /**
     * Move lr (contains the exc_return code) to low registers and store it
     * on the stack.
     */
			"mov     r4,         lr					\n"
			"str     r4,         [r3, #0]			\n"

    /* Re-adjust r3 to point at top of stack */
			"subs    r3,         r3, #32			\n"
#endif // !THUMB_2
    /**
     * Address of running TCB still in r1. Store thread's current stack top
     * into its sp_save_ptr, which is the struct's first element.
     */
			"str     r3,         [r1, #0]			\n"
    
    /**
     * ctx_switch_info.next_tcb is going to become ctx_switch_info.running_tcb,
     * so we update the pointer.
     */
			"str     r2,         [r0, #0]			\n"
    
#if defined(__NEWLIB__) && ATOM_NEWLIB_REENT
    /**
     * Store the thread's reentry context address in _impure_ptr. This
     * will have been stored in ctx_switch_info.reent.
     */ 
			"ldr     r4,         [r0, #8]	\n"
			"ldr     r3,         = _impure_ptr		\n"
			"str     r4,         [r3, #0]			\n"
#endif

    /**
     * Fetch next thread's stack pointer from its TCB's sp_save_ptr and restore
     * the thread's register context.
     */
			"ldr     r3,         [r2, #0]			\n"

#if defined(THUMB_2)

    /* Cortex-M[34], restore thread's task stack frame */
			"ldmia   r3!,        {r4-r11, lr}		\n"

#if defined(WITH_FPU)
    /**
     * Check if FPU was used by new thread and restore registers if necessary.
     */
			"tst     lr,         #1 << 4			\n"
			"it      eq								\n"
			"vldmiaeq  r3!,      {s16-s31}			\n"

    /**
     * TODO: only restore FPU registers if FPU was used by another thread
     * between this thread being scheduled out and now.
     */
#endif // WITH_FPU
#else // !THUMB_2

    /**
     * Thread restore for Cortex-M0
     * Restore thread's task stack frame. Because thumb 1 only supports
     * load multiple on low register, we have to do it in two steps and
     * adjust the stack pointer manually.
     */

    /* Restore high registers */
			"adds    r3,         r3, #16			\n"
			"ldmia   r3!,        {r4-r7}			\n"
			"mov     r8,         r4					\n"
			"mov     r9,         r5					\n"
			"mov     r10,        r6					\n"
			"mov     r11,        r7					\n"

    /* Restore lr */
			"ldr     r4,         [r3, #0]			\n"
			"mov     lr,         r4					\n"
			"subs    r3,         r3, #32			\n"

    /**
     * Restore r4-r7 and adjust r3 to point at the top of the exception
     * stack frame.
     */
			"ldmia   r3!,        {r4-r7}			\n"
			"adds    r3,         r3, #20			\n"
			"dsb									\n"
			"isb									\n"

#endif // !THUMB_2

    /* Set process stack pointer to new thread's stack*/
			"msr     PSP,        r3					\n"
		);
	}
	
	__ISB();
	__DMB();
	__DSB();
	
	__asm volatile
	(
			"no_switch:								\n"

	/* Re-enable interrupts */
			"cpsie		i							\n"
			"isb									\n"
    
    /* Return to new thread */
			"bx      lr								\n"
			".ltorg									\n"
	);
}

/**
 * This function is called when a new thread is scheduled in for the first
 * time. It will simply call the threads entry point function.
 */
static void thread_shell(void)
{
    ATOM_TCB *task_ptr;

    /**
     * We "return" to here after being scheduled in by the pend_sv_handler.
     * We get a pointer to our TCB from atomCurrentContext()
     */
    task_ptr = atomCurrentContext();

    /**
     * Our thread entry point and parameter are stored in the TCB.
     * Call it if it is valid
     */
    if(task_ptr && task_ptr->entry_point) {
        task_ptr->return_value = task_ptr->entry_point(task_ptr->entry_param);
    }

	/**
	 * If thread returned, store it's rv in TCB and mark asa termminated
	 * so it wont be scheduled again
	 */
	
	task_ptr->terminated = TRUE;
	atomSched(FALSE); 
}

/**
 * Initialise a threads stack so it can be scheduled in by
 * archFirstThreadRestore or the pend_sv_handler.
 */
void archThreadContextInit(ATOM_TCB *tcb_ptr, void *stack_top,
	_fnAtomThread entry_point, void* entry_param)
{
    struct isr_stack *isr_ctx;
    struct task_stack *tsk_ctx;

    /**
     * Do compile time verification for offsets used in _archFirstThreadRestore
     * and pend_sv_handler. If compilation aborts here, you will have to adjust
     * the offsets for struct task_switch_info's members in asm-offsets.h
     */
    assert_static(offsetof(struct task_switch_info, running_tcb) == CTX_RUN_OFF);
    assert_static(offsetof(struct task_switch_info, next_tcb) == CTX_NEXT_OFF);
#if defined(__NEWLIB__) && ATOM_NEWLIB_REENT
    assert_static(offsetof(struct task_switch_info, reent) == CTX_REENT_OFF);
#endif

    /**
     * Enforce initial stack alignment
     */
    stack_top = STACK_ALIGN(stack_top, STACK_ALIGN_SIZE);

    /**
     * New threads will be scheduled from an exception handler, so we have to
     * set up an exception stack frame as well as task stack frame
     */
    isr_ctx = stack_top - sizeof(*isr_ctx);
    tsk_ctx = stack_top - sizeof(*isr_ctx) - sizeof(*tsk_ctx);

#if 0
    printf("[%s] tcb_ptr: %p stack_top: %p isr_ctx: %p tsk_ctx: %p entry_point: %p, entry_param: 0x%x\n",
            __func__, tcb_ptr, stack_top, isr_ctx, tsk_ctx, entry_point, entry_param);
    printf("[%s] isr_ctx->r0: %p isr_ctx->psr: %p tsk_ctx->r4: %p tsk_ctx->lr: %p\n",
            __func__, &isr_ctx->r0, &isr_ctx->psr, &tsk_ctx->r4, &tsk_ctx->lr);
#endif
    /**
     * We use the exception return mechanism to jump to our thread_shell()
     * function and initialise the PSR to the default value (thumb state
     * flag set and nothing else)
     */
    isr_ctx->psr = 0x01000000;
    isr_ctx->pc  = (uint32_t) thread_shell;

    /* initialise unused registers to silly value */
    isr_ctx->lr  = 0xEEEEEEEE;
    isr_ctx->r12 = 0xCCCCCCCC;
    isr_ctx->r3  = 0x33333333;
    isr_ctx->r2  = 0x22222222;
    isr_ctx->r1  = 0x11111111;
    isr_ctx->r0  = 0x00000000;

    /**
     * We use this special EXC_RETURN code to switch from main stack to our
     * thread stack on exception return
     */
    tsk_ctx->exc_ret = 0xFFFFFFFD;

    /* initialise unused registers to silly value */
    tsk_ctx->r11 = 0xBBBBBBBB;
    tsk_ctx->r10 = 0xAAAAAAAA;
    tsk_ctx->r9  = 0x99999999;
    tsk_ctx->r8  = 0x88888888;
    tsk_ctx->r7  = 0x77777777;
    tsk_ctx->r6  = 0x66666666;
    tsk_ctx->r5  = 0x55555555;
    tsk_ctx->r4  = 0x44444444;

    /**
     * Stack frames have been initialised, save it to the TCB. Also set
     * the thread's real entry point and param, so the thread shell knows
     * what function to call.
     */
    tcb_ptr->sp_save_ptr = tsk_ctx;
    tcb_ptr->entry_point = entry_point;
    tcb_ptr->entry_param = entry_param;

#if defined(__NEWLIB__) && ATOM_NEWLIB_REENT
    /**
     * Initialise thread's reentry context for newlib
     */ 
    _REENT_INIT_PTR(&(tcb_ptr->port_priv.reent));
#endif
}

