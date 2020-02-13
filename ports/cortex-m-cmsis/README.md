# ARM Cortex-M Port

Author: Tido Klaassen <tido@4gh.eu>
Author: Kaworu <kaworu-gat@k2t.eu>

License: BSD Revised. 

## Summary and Prerequisites
This port should run on any Cortex-M0/3/4/4F/M7 (M0+ not tested). 
It uses RedHat's [newlib](https://sourceware.org/newlib/). 
You will also need an ARM compiler toolchain. 
If you want to flash or debug your target, [openocd](http://openocd.org)
is also a must. For STM32 targets [stlink](https://github.com/texane/stlink)
might be helpful.

On Debian systems, compiler, newlib and openocd can easily be installed by the
package manager (untested, not going to set up a new system just to test this):

``` 
apt-get install gcc-arm-none-eabi binutils-arm-none-eabi
apt-get install libnewlib-arm-none-eabi libnewlib-dev
apt-get install openocd
```
**N.B.** Usually you will want to compile and link against the size optimised
"nano" version of newlib. This is done by default. If your version
of newlib does not support this (Debian's libnewlib package version before
2.1.0+git20141201.db59ff3-2) you will have to comment out the line
`USE_NANO := true` in the Makefile or pass `USE_NANO=` as
a command line option to make.

**N.B.** Debian's libnewlib-arm-none-eabi version 2.2.0+git20150830.5a3d536-1
ships with broken nano support. To enable necessary workarounds, uncomment
the line `#FIX_DEBIAN := true` in the Makefile or pass `FIX_DEBIAN=true`
as a command line option to make.
If you are using this fix, be advised that when switching between nano
and regular builds, you will have to do a `make realclean` first.

## Code Layout
The "classic" port components (code needed for task set-up and context
switching and the atomport{-private}.h headers) are residing in the
top level port directory.

There are additional subdirectories:

* **common** contains code needed by multiple boards, such as
stub functions for newlib or routines shared by multiple boards using the
same family of target MCUs.

* **build** this is a temporary directory where all object files end up in and
which will be removed by `make clean`.

## Build Preparation
Unless you decide to use an external installation of libopencm3, you will have
to set up the libopencm3 sub-module:
```
git submodule init
git submodule update
```

## Port Internals
The Cortex-M port is different from the other architectures insofar as it makes
use of two particular features. First, it uses separate stacks for thread and
exception context. When the core enters exception mode, it first pushes xPSR,
PC, LR, r0-r3 and r12 on the currently active stack (probably the thread stack
in PSP) and then switches to the main stack stored in MSP. It also stores a
special EXC_RETURN code in LR which, when loaded into the PC, will determine
if on return program execution continues to use the MSP or switches over to 
the PSP and, on cores with an FPU, whether FPU registers need to be restored.
The Cortex-M also implements a nested vectored interrupt controller (NVIC),
which means that a running ISR may be pre-empted by an exception of higher
priority.

The use of separate stacks for thread and exception context has the nice
implication that you do not have to reserve space on every task's stack for 
possible use by ISRs. But it also means that it breaks atomthreads concept 
of simply swapping task stacks, regardless of if atomSched() was called from
thread or interrupt context. We would have to implement different 
archContextSwitch() functions called from thread or exception context and
also do messy stack manipulations depending on whether the task to be
scheduled in was scheduled out in in the same context or not. Yuck!
And don't get me started on nested exceptions calling atomIntExit()...   

This is where the second feature comes handy, the PendSV mechanism.
PendSV is an asynchronous exception with the lowest possible priority, which
means that, when triggered, it will be called by the NVIC if there are no
other exceptions pending or running. We use it by having archContextSwitch()
set up a pointer to the TCB that should be scheduled in and then trigger the
PendSv exception. As soon as program flow leaves the critical section or 
performs the outermost exception return, the pend_sv_handler() will be called
and the thread context switch takes place.
