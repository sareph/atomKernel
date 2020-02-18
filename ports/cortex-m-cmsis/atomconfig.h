#pragma once

#define ATOM_NEWLIB_REENT          1
#define ATOM_NEWLIB_MALLOC_HOOKS   1
#define ATOM_KERNEL_TCB_AGEING     1
#define ATOM_STACK_CHECKING        0
#define ATOM_IDLE_TIME_CALCULATION 1

#define ATOM_SYSTEM_WRAPPER_ENABLE 1

#define ATOM_SYSTEM_MAX_THREADS   8 // needed only if system wrapper is used
#define ATOM_SYSTEM_STACK_SIZE    (1024 * 32)
#define ATOM_SYSTEM_STACK_ATTRIBUTE __attribute((aligned(4), section(".ramd1")))