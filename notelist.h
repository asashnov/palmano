#ifndef NOTE_LIST_H
#define NOTE_LIST_H 1

typedef struct
{
  Int16 note;
  Int16 dur;
  Int16 vel;
  Int16 pause;
} NoteType;

MemHandle
notelist_AppendNoteToList (MemHandle listH, UInt16 *numnotes, const NoteType *n);

/* TODO: insert note, delete note */

#endif
