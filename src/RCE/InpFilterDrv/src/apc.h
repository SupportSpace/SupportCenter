/*++
  Copyright (c) 2002 CPN Group, University of Aarhus

  Module Name:

  apc.h

Abstract:

This file provides prototypes for some functions concerning APC.
These functions are exportted by the libraries, but the prototypes
are not available.

Environment:

kernel mode only

Notes:
Talk to Michael (mw@daimi) for information


Revision History:


--*/

#ifndef _apc_h
#define _apc_h

void KeInitializeApc(PKAPC Apc,
		PKTHREAD Thread,
		CCHAR ApcStateIndex,
		PKKERNEL_ROUTINE KernelRoutine,
		PKRUNDOWN_ROUTINE RundownRoutine,
		PKNORMAL_ROUTINE NormalRoutine,
		KPROCESSOR_MODE ApcMode,
		PVOID NormalContext);

void KeInsertQueueApc(PKAPC Apc,
		PVOID SystemArgument1,
		PVOID SystemArgument2,
		UCHAR unknown);

#endif
