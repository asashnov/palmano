#ifndef _SMFUTILS_H
#define _SMFUTILS_H

#include <PalmOS.h>
#include "notelist.h"

#define  pmnoCreatorDB   'pmno'

extern void smfutils_load(MemHandle smfH, NoteListPtr dstList);
extern MemHandle smfutils_create(const NoteListPtr srcList);
extern int  smfutils_save(MemHandle smfH, const Char *name, const NoteListPtr srcList);
extern int  smfutils_play(SndMidiListItemType *midi);
extern int  smfutils_playHandle(MemHandle smfH);

#endif
