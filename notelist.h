#ifndef NOTES_H
#define NOTES_H 1

typedef struct
{
  Int16 note;
  Int16 dur;
  Int16 vel;
  Int16 pause;
} NoteType;

MemHandle
notes_AppendNoteToList (MemHandle listH, UInt16 *numnotes, const NoteType *n);

/* TODO: insert note, delete note */

#endif
