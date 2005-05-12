#ifndef _SMFUTILS_H
#define _SMFUTILS_H

#include <PalmOS.h>
#include "notelist.h"

extern int smfLoad(SndMidiListItemType srcMidi, NoteListPtr dstList);
extern int smfSave(SndMidiListItemType dstMidi, const NoteListPtr srcList);
extern int smfPlay(SndMidiListItemType *midi);

#endif
