#include <PalmOS.h>

#include "notelist.h"
#include "utils.h"

void
notelist_init(NoteListPtr nl, FormGadgetType *gadget)
{
  notelist_clear(nl);
  RctCopyRectangle(&gadget->rect, &nl->rect);
}

void
notelist_clear(NoteListPtr nl)
{
  notelist_free(nl);
  nl->firstDisplaying = 0;
  nl->selected = -1;
  nl->tempo = 0x0190;		/* standard default value (from sdk3.5 rockmusic sample) */
}

int
notelist_append(NoteListPtr nl, const NotePtr newnote)
{
  NoteType *notes;
  
  if (nl->bufH == NULL) {
    nl->bufH = MemHandleNew(sizeof(NoteType));
  } else if (MemHandleResize (nl->bufH, ((nl->num + 1) * sizeof(NoteType))) != 0) {
    ErrDisplay ("Can't add note- out of memory!");
    return 0;
  }

  if ((notes = (NoteType*) MemHandleLock(nl->bufH)) == NULL) {
    ErrDisplay("notelist_append(): can't lock bufH");
    return false;
  }
  *(notes + nl->num) = *newnote;
  //nl->selected = nl->num;   /* just appended note is selected automaticaly */
  nl->num++;
  MemPtrUnlock (notes);
  return true;
}

int notelist_updateSelected(NoteListPtr nl, const NotePtr note)
{
  NoteType *notes = (NoteType*) MemHandleLock(nl->bufH);
  if (notes == NULL) {
    ErrDisplay("notelist_updateSelected(): can't lock bufH");
    return false;
  }
  *(notes + nl->selected) = *note;  
  MemPtrUnlock (notes);
  return true;
}

void
notelist_draw (NoteListPtr nl)
{
  static char *names[12] = {
    "C-", "C#", "D-", "D#", "E-", 
    "F-", "F#", "G-", "G#", "A-", 
    "A#", "B-" };
  int h = FntCharHeight();
  NoteType *notes = NULL;
  char buf[30];
  int i;
  int x = nl->rect.topLeft.x;
  int y = nl->rect.topLeft.y;
  int max_y = nl->rect.topLeft.y + nl->rect.extent.y - 1;

  if (nl->num > 0)
    notes = (NoteType*) MemHandleLock(nl->bufH);

  i = nl->firstDisplaying;
  while (y < max_y) {
    if (i >= nl->num) {
      StrPrintF (buf, "                ");
    } else if (notes[i].note < 0) {
      StrPrintF (buf, "--- %3d %3d %3d", notes[i].dur, notes[i].vel, notes[i].pause);
    } else {
      StrPrintF (buf, "%s%d %3d %3d %3d",
		 names[notes[i].note % 12], /* note number in octave */
		 notes[i].note / 12, /* octave */
		 notes[i].dur, notes[i].vel, notes[i].pause);
    }

    if(1) {
      RectangleType r = {{x,y}, {nl->rect.extent.x, h}};
      WinEraseRectangle(&r, 0);
      WinPaintChars(buf, StrLen(buf), x, y);
      if(i == nl->selected)
	WinInvertRectangle(&r, 0);
    }

    y += h, i++;
  }
  if (notes != NULL)
    MemPtrUnlock (notes);
}

void
notelist_tapped(NoteListPtr nl, Int16 tap_x, Int16 tap_y)
{
  Int16 select;
  int h = FntCharHeight();
  select = (tap_y - nl->rect.topLeft.y) / h + nl->firstDisplaying;
  if (select >= nl->num)
    return;
  if (select == nl->selected)
    nl->selected = -1;		/* unselect */
  else
    nl->selected = select;
  notelist_draw(nl);
}

void
notelist_free(NoteListPtr nl)
{
  if (nl->bufH != NULL) {
    MemHandleFree(nl->bufH);
    nl->bufH = NULL;
  }
  nl->num = 0;
}
