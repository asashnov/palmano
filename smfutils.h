#ifndef _SMFUTILS_H
#define _SMFUTILS_H

#include <PalmOS.h>
#include "notelist.h"

#define  pmnoCreatorDB   'pmno'

extern void smfutils_load(MemHandle smfH, NoteListPtr dstList);
extern int  smfutils_save(MemHandle smfH, const Char *name, const NoteListPtr srcList);
extern int  smfutils_play(SndMidiListItemType *midi);

#endif
