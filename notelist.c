#include <PalmOS.h>
#include "notes.h"

/* Dynamic array of notes functions */

/* AppendNoteToList */
MemHandle
notes_AppendNoteToList (MemHandle listH, UInt16 *numnotes, const NoteType *n)
{
  NoteType *newnote;
  UInt8 *p;

  /* try to increase buffer up to one note */
  if (MemHandleResize (listH, (*numnotes + 1) * sizeof(NoteType)) != 0)
    {
      ErrDisplay ("Can't add note- out of memory!");
      return NULL;
    }

  /* put note in end */
  p = MemHandleLock (listH);
  newnote = (NoteType*) (p + *numnotes * sizeof(NoteType));
  *numnotes = *numnotes++;

  /* fill data */
  newnote->note = n->note;
  newnote->dur = n->dur;
  newnote->vel = n->vel;
  newnote->pause = n->pause;

  MemPtrUnlock (p);

  return listH;
}
