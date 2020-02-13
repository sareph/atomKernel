#pragma once

#include "atomconfig.h"

#ifndef ATOM_SYSTEM_STACK_SIZE
#define ATOM_SYSTEM_STACK_SIZE 2048
#endif

#ifndef ATOM_SYSTEM_STACK_ATTRIBUTE
#define ATOM_SYSTEM_STACK_ATTRIBUTE 
#endif

#ifndef ATOM_SYSTEM_MAX_THREADS
#define ATOM_SYSTEM_MAX_THREADS 4
#endif

atom_status_t atomOSInit(size_t idleThreadStack, size_t stackChecking);
atom_status_t atomOSCreateThread(size_t threadStack, atom_prio_t priority, _fnAtomThread entryPoint, void *param, ATOM_TCB **pTcb);