#pragma once

#define ATOM_NEWLIB_REENT          1
#define ATOM_NEWLIB_MALLOC_HOOKS   1
#define ATOM_KERNEL_TCB_AGEING     1
#define ATOM_STACK_CHECKING        1
#define ATOM_IDLE_TIME_CALCULATION 1

#define ATOM_SYSTEM_WRAPPER_ENABLE 1
#define ATOM_SYSTEM_MEM_AIGN       sizeof(int_fast8_t)

#define ATOM_SYSTEM_MEM_SIZE    (1024 * 32)
#define ATOM_SYSTEM_MEM_ATTRIBUTE //__attribute((aligned(4), section(".ramd1")))