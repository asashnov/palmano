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
}

int
notelist_append(NoteListPtr nl, const NotePtr newnote)
{
  NoteType *notes;
  
  debugPrintf("notelist_append(): entry: num of notes %d\n", nl->num);

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
  debugPrintf("notelist_append(): notes=%lx\n", notes);

  *(notes + nl->num) = *newnote;
  nl->selected = nl->num;   /* just appended note is selected automaticaly */
  nl->num++;
  MemPtrUnlock (notes);

  debugPrintf("notelist_append(): exit: num of notes %d\n", nl->num);
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
  int x = nl->rect.topLeft.x;
  int y, max_y = nl->rect.topLeft.y + nl->rect.extent.y;
  int i;
  
  if (nl->num > 0)
    notes = (NoteType*) MemHandleLock(nl->bufH);

  debugPrintf("notelist_draw: num of notes %d\n", nl->num);

  /* draw must carry for visibiliti of selected note */
  if (nl->selected != -1) {
    if (nl->selected < nl->firstDisplaying)
      nl->firstDisplaying = nl->selected;
    else {
      Int16 total_displaying = nl->rect.extent.y / h;
      Int16 last_displaying = nl->firstDisplaying + total_displaying - 1;
      if (nl->selected > last_displaying)
	nl->firstDisplaying = nl->selected - total_displaying + 1;
   }
  }

  i = nl->firstDisplaying;
  for (y = nl->rect.topLeft.y; y < max_y ; y += h, i++) {
    debugPrintf("y=%d i=%d  num=%d\n", y, i, nl->num);
    if (i >= nl->num) {
      StrPrintF (buf, "                ");
    } else if (notes[i].note < 0) {
      StrPrintF (buf, " --- %3d %3d %3d", notes[i].dur, notes[i].vel, notes[i].pause);
    } else {
      StrPrintF (buf, " %s%d %3d %3d %3d",
		 names[notes[i].note % 12], /* note number in octave */
		 notes[i].note / 12, /* octave */
		 notes[i].dur, notes[i].vel, notes[i].pause);
    }
    /* paint result in any way */
    if(i == nl->selected)
      buf[0] = '*';
    WinPaintChars(buf, StrLen (buf), x, y);	
  }
  if (notes != NULL)
    MemPtrUnlock (notes);
}

void
notelist_tapped(NoteListPtr nl, Int16 tap_x, Int16 tap_y)
{
  int h = FntCharHeight();
  nl->selected = (tap_y - nl->rect.topLeft.y) / h + nl->firstDisplaying;
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
