#include <PalmOS.h>
#include "mkeys.h"
#include "notelist.h"

#define WHITE_WIDTH     13
#define BLACK_HEIGHT    18
#define BLACK_WIDTH (WHITE_WIDTH/2)     

/* Return true if note (by number) is black (note=0 is do in first octave) */
static Boolean
is_black (Int16 note)
{
  ErrFatalDisplayIf (note < (-24), "note < (-24)");
  note = (note + 24) % 12;
  switch(note)
    {
    case 1:
    case 3:
    case 6:
    case 8:
    case 10:
      return true;
    default:
      return false;
    }
  return false;
}

/* set first button. If specified black button, its find first white */
static void
set_first_button (MidiKeysPtr mkeys, Int16 note)
{
  // first must be white for drawing procedure
  if (is_black (note))
    note++;
  mkeys->first = note;
}


void
midikeys_init(MidiKeysPtr mkeys, FormGadgetType *gadget)
{
  RctCopyRectangle(&gadget->rect, &mkeys->rect);
  set_first_button (mkeys, 60 - 3); // magic numbers again :-(((
  mkeys->invert = -1;
}

void
midikeys_draw (MidiKeysPtr mkeys)
{
  int x1, y1, x2, y2;
  int x, note;
  RectangleType blrect;
  
  WinEraseRectangle (&mkeys->rect, 0);

  x1 = mkeys->rect.topLeft.x;
  y1 = mkeys->rect.topLeft.y;
  x2 = x1 + mkeys->rect.extent.x;
  y2 = y1 + mkeys->rect.extent.y;

  /* Draw white buttons */
  WinDrawLine (x1, y1, x2, y1);
  WinDrawLine (x1, y2, x2, y2);
  for (x = x1, note = mkeys->first; x <= x2; x += WHITE_WIDTH)
    WinDrawLine (x, y1, x, y2);

  /* Draw black buttons */
  WinSetPatternType (blackPattern);
  blrect.topLeft.x = x1 + WHITE_WIDTH - (BLACK_WIDTH / 2);
  blrect.topLeft.y = y1;
  blrect.extent.x = BLACK_WIDTH;
  blrect.extent.y = BLACK_HEIGHT;

  note = mkeys->first;
  for (x = x1 ; x <= x2; x += WHITE_WIDTH) {
    if (is_black (++note)) {
      WinDrawRectangle (&blrect, 0);
      note++;
    }
    blrect.topLeft.x += WHITE_WIDTH;
  }
}

void
midikeys_tapped(MidiKeysPtr mkeys, Int16 tap_x, Int16 tap_y)
{
  RectangleType blrect;
  RectangleType whrect;
  EventType event;
  Int16 max_x = mkeys->rect.topLeft.x + mkeys->rect.extent.x;
  Int16 note, tapped_note = -1;

  /* Check white buttons */
  whrect.topLeft.x = mkeys->rect.topLeft.x;
  whrect.topLeft.y = mkeys->rect.topLeft.y;
  whrect.extent.x = WHITE_WIDTH;
  whrect.extent.y = mkeys->rect.extent.y;
  for (note = mkeys->first; whrect.topLeft.x < max_x ; whrect.topLeft.x += WHITE_WIDTH) {
    if (RctPtInRectangle (tap_x, tap_y, &whrect))
      tapped_note = note;
    if(is_black(++note))
      note++;
  }

  /* Check black buttons */
  blrect.topLeft.x = mkeys->rect.topLeft.x + WHITE_WIDTH - (BLACK_WIDTH / 2);
  blrect.topLeft.y = mkeys->rect.topLeft.y;
  blrect.extent.x = BLACK_WIDTH;
  blrect.extent.y = BLACK_HEIGHT;
  for (note = mkeys->first ; blrect.topLeft.x < max_x; blrect.topLeft.x += WHITE_WIDTH) {
    if (is_black (++note)) {
      if (RctPtInRectangle (tap_x, tap_y, &blrect))
	tapped_note = note;
      note++;
    }
  }

  if (tapped_note != -1) {
    event.eType = MKeysNoteTappedEvent;
    event.data.generic.datum[0] = tapped_note;
    EvtAddEventToQueue(&event);
  }
  return;
}

void
midikeys_invert(MidiKeysPtr mkeys, Int16 note)
{
  return;
}
