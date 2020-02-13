#pragma once

#define ATOM_NEWLIB_REENT         1
#define ATOM_NEWLIB_MALLOC_HOOKS  1
#define ATOM_KERNEL_TCB_AGEING    1
#define ATOM_STACK_CHECKING       0

#define ATOM_SYSTEM_MAX_THREADS   8 // needed only if wrapper is used
#define ATOM_SYSTEM_STACK_SIZE    (1024 * 16)
#define ATOM_SYSTEM_STACK_ATTRIBUTE __attribute((section(".ramd1")))