#ifndef NOTELIST_H
#define NOTELIST_H

typedef struct Note
{
  Int16 note;
  Int16 dur;
  Int16 vel;
  Int16 pause;
} NoteType;
typedef struct Note * NotePtr;

typedef struct NoteList
{
  MemHandle bufH;
  UInt16    num;
  RectangleType rect;
  Int16     firstDisplaying;
  Int16     selected;
} NoteListType;
typedef NoteListType * NoteListPtr;

extern void notelist_init(NoteListPtr nl, FormGadgetType *gadget);
extern void notelist_clear(NoteListPtr nl);
extern int  notelist_append(NoteListPtr nl, const NotePtr note);
extern void notelist_draw(NoteListPtr nl);
extern void notelist_tapped(NoteListPtr nl, Int16 tap_x, Int16 tap_y);
extern void notelist_free(NoteListPtr nl);
#endif
