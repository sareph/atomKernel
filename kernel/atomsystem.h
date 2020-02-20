#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "atomconfig.h"

#if ATOM_SYSTEM_WRAPPER_ENABLE

#ifndef ATOM_SYSTEM_MEM_SIZE
#define ATOM_SYSTEM_MEM_SIZE 2048
#endif

#ifndef ATOM_SYSTEM_MEM_ATTRIBUTE
#define ATOM_SYSTEM_MEM_ATTRIBUTE 
#endif

atom_status_t atomOSInit(size_t idleThreadStack, void *idleThreadParam);
atom_status_t atomOSCreateThread(size_t threadStack, atom_prio_t priority, _fnAtomThread entryPoint, void *param, ATOM_TCB **pTcb);
void atomOSStart(void);

#endif

#ifdef __cplusplus
}
#endif
