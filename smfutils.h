/******************************************************************************
 *
 * Copyright (c) 1994-1999 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: MakeSMF.h
 *
 * Description:
 *             	routines to create a SMF in memory
 *
 *****************************************************************************/
#ifndef _MAKESMF_H
#define _MAKESMF_H

#include "midi_util.h"

extern MemHandle smf_StartSMF();
extern MemHandle smf_AppendNote(MemHandle bufH, int note, int dur, int vel, int pause);
extern MemHandle smf_FinishSMF(MemHandle bufH);

extern MemPtr    smf_GetBeginNoteData (MemPtr smf_stream);
extern Boolean   smf_isEndOfNoteData (MemPtr smf_stream);
extern MemPtr    smf_ReadNote (MemPtr smf_stream, NoteType * n);

#endif

