#include <PalmOS.h>
#include "midi_keys.h"
#include "notes.h"

#if 0


FormGadgetType *bounds
  | --------------------------------------------|<--||1 2 3 4 5 | ||||Y_OFFSET
  | --------------------------------------------|<--||keyboard |
  |||||||--------------------------------------------|
#endif
#define Y_OFFSET               10
#define WHITE_WIDTH     13
#define BLACK_HEIGHT    14
#define BLACK_WIDTH     6
#define OCT_SIZE_X 8
#define OCT_SIZE_Y 8
/* first note displayed number */
static Int16 first_note;


/* Return true if note (by number) is black */
static Boolean
is_black (int note)
{
  // two negative octaves possible only
  ErrFatalDisplayIf (note < (-24), "note < (-24)");

  // shift to interval 0...11 (one ocnave)
  note = (note + 24) % 12;

  // black buttons are 1, 3, 6, 8, 10
  if (note == 1 || note == 3 || note == 6 || note == 8 || note == 10)
    return true;

  return false;
}


/* set first button. If specified black button, its find first white */
static void
set_first_button (int note)
{
  first_note = note;

  // first must be white
  if (is_black (first_note))
    first_note++;
}




/* Get controls bounds */
static void
get_control_bounds (FormGadgetType * gadgetP, RectanglePtr rect)
{
  // upper part of gadget for control
  rect->topLeft.x = gadgetP->rect.topLeft.x;
  rect->topLeft.y = gadgetP->rect.topLeft.y;
  rect->extent.x = gadgetP->rect.extent.x;
  rect->extent.y = Y_OFFSET;
}


/* Get midi keyboard bounds */
static void
get_mkeyboard_bounds (struct FormGadgetType *gadgetP, RectanglePtr rect)
{
  // bottom part for keyboard
  rect->topLeft.x = gadgetP->rect.topLeft.x;
  rect->topLeft.y = gadgetP->rect.topLeft.y + Y_OFFSET;
  rect->extent.x = gadgetP->rect.extent.x;
  rect->extent.y = gadgetP->rect.extent.y - Y_OFFSET;

  /* strip for whole buttons */
  rect->extent.x -= gadgetP->rect.extent.x % WHITE_WIDTH;
}


/* draw MIDI keyboard 
   Note with specified in 'invert' draw inverted
*/
static void
draw_midi_keyboard (FormGadgetType * gadgetP, Int16 invert)
{
  RectangleType bounds;
  RectangleType rect;
  int x1, y1, x2, y2;
  int x, note;

  get_mkeyboard_bounds (gadgetP, &bounds);
  WinEraseRectangle (&bounds, 0);

  x1 = bounds.topLeft.x;
  y1 = bounds.topLeft.y;
  x2 = x1 + rect.extent.x;
  y2 = y1 + rect.extent.y;

  /* Draw white buttons */
  WinDrawLine (x1, y1, x2, y1);
  WinDrawLine (x1, y2, x2, y2);

  for (x = x1, note = first_note; x <= x2; x += WHITE_WIDTH)
    {
      WinDrawLine (x, y1, x, y2);
    }

  /* Draw black buttons */
  WinSetPatternType (blackPattern);

  rect.topLeft.x = x1 + WHITE_WIDTH - (BLACK_WIDTH / 2);
  rect.topLeft.y = y1;
  rect.extent.x = BLACK_WIDTH;
  rect.extent.y = BLACK_HEIGHT;

  /* x is not usage in this */
  for (x = x1, note = first_note; x <= x2; x += WHITE_WIDTH)
    {
      note++;			// possible black note
      if (is_black (note))
	{
	  WinDrawRectangle (&rect, 0);
	  note++;		/* go to next note */

	  rect.topLeft.x += WHITE_WIDTH;
	}
    }
}


/* draw control for octave switcher */
static void
draw_controls (FormGadgetType * gadgetP)
{
  int i;
  int x1, y1, x2, y2;
  RectangleType bounds;

  get_control_bounds (gadgetP, &bounds);

  for (i = 0; i < 5; i++)
    {
      // corners
      x1 = bounds.topLeft.x + OCT_SIZE_X * i + 1;
      y1 = bounds.topLeft.y + OCT_SIZE_Y * i + 1;
      x2 = x1 + OCT_SIZE_X;
      y2 = y1 + OCT_SIZE_Y;

      // number
      WinPaintChar ('0' + i, x1 + 2, y1 + 2);

      // border
      WinDrawLine (x1, y1, x2, y1);
      WinDrawLine (x1, y1, x1, y2);
      WinDrawLine (x1, y2, x2, y2);
      WinDrawLine (x2, y1, x2, y2);
    }
}


static UInt16
tap_note (FormGadgetType * gadgetP, UInt16 tap_x, UInt16 tap_y)
{
  RectangleType rect;
  RectangleType bounds;
  int x1, y1, x2, y2;
  int note, x, x_note, x_note_tap;
  int ret;

  get_mkeyboard_bounds (gadgetP, &bounds);

  ret = -1;

  if (!RctPtInRectangle (tap_x, tap_y, &bounds))
    return ret;

  x1 = bounds.topLeft.x;
  y1 = bounds.topLeft.y;
  x2 = x1 + rect.extent.x;
  y2 = y1 + rect.extent.y;

  note = first_note;
  x_note = 0;

  /* tapped note number by x */
  x_note_tap = (tap_x - bounds.topLeft.x) / WHITE_WIDTH;

  /* first black note rectangle */
  rect.topLeft.x = x1 + WHITE_WIDTH - (BLACK_WIDTH / 2);
  rect.topLeft.y = y1;
  rect.extent.x = BLACK_WIDTH;
  rect.extent.y = BLACK_HEIGHT;

  for (x = x1, note = first_note; x <= x2; x += WHITE_WIDTH)
    {

      if (x_note == x_note_tap)
	ret = x_note;

      note++;			/* go to (possible) black note */

      /* Check black note */
      if (is_black (note))
	{
	  if (RctPtInRectangle (tap_x, tap_y, &rect))
	    {
	      ret = note;
	      return ret;
	    }
	  note++;		/* go to next (white) button */
	}

      x_note++;

      /* if already check previos */
      if (x_note > x_note + 1)
	return ret;

      rect.topLeft.x += WHITE_WIDTH;
    }

  return ret;
}


/* handle pen tap on gadget */
static void
handle_tap (FormGadgetType * gadgetP)
{
  Int16 x, y;
  RectangleType rect;
  Boolean penDown;
  NoteType note;

  PenGetPoint (&x, &y, &penDown);
  get_control_bounds (gadgetP, &rect);

  if (RctPtInRectangle (x, y, &rect))
    {
      /* Control tap */
      // TODO: calculate number of octave tapped
      // and first_note = ??? + 12 * oct
    }
  else
    {
      /* Midi button tap */

      /* determine note number and her rectangle */
      note.note = tap_note (gadgetP, x, y);

      /* draw this note inverted */
      draw_midi_keyboard (gadgetP, note.note);

      /* play appropriate sound */
      note.dur   = 500;
      note.vel   = 0;
      note.pause = 0;

No Play!!!
generate event to active form instead with 
note number pressed (or unpressed)

  //      midiutil_PlayNote (&note);

      /* re-draw midi keyboard */
      draw_midi_keyboard (gadgetP, -1);  -1 ?!!! replace this magic number!!!
    }
}


/* midi_keyboard draw gadget function */
static void
draw_all (struct FormGadgetType *gadgetP)
{
  draw_controls (gadgetP);
  draw_midi_keyboard (gadgetP, -1); need to replace magic number
}




/*********************************************************

  New emplementation

*********************************************************/


/* Get settings from config, setup global variables */
extern void
midi_keyboard_init ()
{
  set_first_button (60 - 3);  magic numbers again :-(((
}


/* midi_keyboard gadget callback function.
   NOTE: it calls by programm, not by system
   becose it facility from PalmOS 3.5 but custom alarm sound from PalmOS3.0
 */


extern void
midikeyb_gadget_cb (struct FormGadgetType *gadgetP, UInt16 cmd, void *paramP)
{
  EventType *e;

  switch (cmd)
    {
    case formGadgetDrawCmd:
      draw_all (gadgetP);
      break;
    case formGadgetHandleEventCmd:
      e = (EventPtr) paramP;
      switch (e->eType)
	{
	case penDownEvent:
	  handle_tap (gadgetP);
	  break;
	}
    default:
      break;
    }
}


/* invert note */
extern void
midi_keyboard_invert_note (struct FormGadgetType *gadgetP, Int16 note)
{
  draw_midi_keyboard (gadgetP, note);
}
