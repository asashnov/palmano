#include <PalmOS.h>
#include "resource.h"
#include "main_form.h"
#include "editor_form.h"

static MemHandle MidiListH = NULL;	/* handle to list */
static UInt16 NumMidi;		/* number of sond in list */
static Int16 list_Selected = -1;	/* -1 nothing selected */


/* --- From PalmOS 3.5 SDK documentation

 SndMidiListItemType

When the SndCreateMidiList function returns true, its entHP parameter holds a
handle to a memory chunk containing an array of SndMidiListItemType structures.

typedef struct SndMidiListItemType{

Char name[sndMidiNameLength];
UInt32 uniqueRecID;
LocalID dbID;
UInt16 cardNo;
} SndMidiListItemType;

Field Descriptions

name MIDI name including NULL terminator.
uniqueRecID Unique ID of the record containing the MIDI file.
dbID Database (file) ID.
cardNo Number of the memory card on which the MIDI file resides.


*/


//  ErrDisplay("main form: All OK!!!!");



/* draw one midi name in the list */
static void
cb_midi_list_draw_item (Int16 itemNum,
			RectangleType * bounds, Char ** itemsText)
{
  const char *toDraw = "";
  SndMidiListItemType *p;

  p = MemHandleLock (MidiListH);

  if (p == NULL)
    ErrFatalDisplay ("Can't lock MidiListH");

  toDraw = p[itemNum].name;

  DrawCharsToFitWidth (toDraw, bounds);

  MemHandleUnlock (MidiListH);
}


/* new midi */
static void
New ()
{
  // open editor for new MIDI
  MemMove (&EditorMidi.name[0], "Untitled", sndMidiNameLength);
  EditorMidi.cardNo = 0;
  EditorMidi.dbID = 0;
  EditorMidi.uniqueRecID = 0;

  FrmGotoForm (ID_EditorForm);
}


/* play midi */
static void
Play ()
{
  SndMidiListItemType *p;

  if (list_Selected < 0)
    {
      FrmAlert (ID_MidiNotSelAlert);
      return;
    }

  p = MemHandleLock (MidiListH);

  if (p == NULL)
    ErrFatalDisplay ("Can't lock MidiListH");

  midiutil_PlayMidi (p[list_Selected].cardNo, p[list_Selected].dbID,
		     p[list_Selected].uniqueRecID);

  MemHandleUnlock (MidiListH);
}


/* open (edit) midi */
static void
Edit ()
{
  SndMidiListItemType *p;

  if (list_Selected < 0)
    {
      FrmAlert (ID_MidiNotSelAlert);
      return;
    }

  p = MemHandleLock (MidiListH);

  if (p == NULL)
    ErrFatalDisplay ("Can't lock MidiListH");

  /* copy current selected song info in editor */
  MemMove (&EditorMidi.name[0], &p[list_Selected].name[0], sndMidiNameLength);
  EditorMidi.cardNo = p[list_Selected].cardNo;
  EditorMidi.dbID = p[list_Selected].dbID;
  EditorMidi.uniqueRecID = p[list_Selected].uniqueRecID;

  MemHandleUnlock (MidiListH);

  FrmGotoForm (ID_EditorForm);
}


/* copy action */
static void
Copy ()
{
  if (list_Selected < 0)
    {
      FrmAlert (ID_MidiNotSelAlert);
      return;
    }

  FrmAlert (ID_NotImplAlert);
}


/* delete action */
static void
Delete ()
{
  if (list_Selected < 0)
    {
      FrmAlert (ID_MidiNotSelAlert);
      return;
    }

  FrmAlert (ID_NotImplAlert);
}


/* info action */
static void
Info ()
{
  if (list_Selected < 0)
    {
      FrmAlert (ID_MidiNotSelAlert);
      return;
    }

  FrmAlert (ID_NotImplAlert);
}


/* beam action */
static void
Beam ()
{
  if (list_Selected < 0)
    {
      FrmAlert (ID_MidiNotSelAlert);
      return;
    }

  FrmAlert (ID_NotImplAlert);
}


void
MainFormOpen (void)
{
  ListPtr list;

  /* free old list */
  if (MidiListH != NULL)
    {
      MemHandleFree (MidiListH);
      MidiListH = NULL;
    }

  /* get list of system midi */
  SndCreateMidiList (0, true, &NumMidi, &MidiListH);

  /* setup list callback function and data pointer */
  list = GetObjectFromActiveForm (ID_MainSongList);
  LstSetDrawFunction (list, cb_midi_list_draw_item);
  LstSetListChoices (list, NULL, NumMidi);
}


void
MainFormClose (void)
{
  /* system midi list handle */
  if (MidiListH != NULL)
    MemHandleFree (MidiListH);
}

/* Main form event handler */
Boolean
MainFormEventHandler (EventType * e)
{
  FormType *form = FrmGetActiveForm ();
  /* Action drop-down list */
  static UInt16 ActionListSelected = 0;	/* Open action selected default */

  switch (e->eType)
    {
    case frmOpenEvent:
      ActionListSelected = 0;
      MainFormOpen ();
      FrmDrawForm (form);
      return true;

    case frmCloseEvent:
      MainFormClose ();
      return true;

    case lstSelectEvent:
      switch (e->data.ctlSelect.controlID)
	{
	  /* select song in midi list */
	case ID_MainSongList:
	  {
	    list_Selected = e->data.lstSelect.selection;

	    /* action whis selected midi */
	    switch (ActionListSelected)
	      {
	      case 0:		/* open action */
		Edit ();
		break;
	      case 1:		/* play action */
		Play ();
		break;
	      case 2:		/* copy action */
		Copy ();
		break;
	      case 3:		/* delete action */
		Delete ();
		break;
	      case 4:		/* info action */
		Info ();
		break;
	      case 5:		/* beam action */
		Beam ();
	      }

	    return true;
	  }
	}
      break;

    case ctlSelectEvent:
      switch (e->data.ctlSelect.controlID)
	{
	case ID_MainNewButton:
	  New ();
	  return true;
	}
      break;

      /* remember number of selected action in action list */
    case popSelectEvent:
      ActionListSelected = e->data.popSelect.selection;
      return false;

//    case keyDownEvent:
//      struct _KeyDownEventType * k;
//      k = (_KeyDownEventType *) data;
//      switch (k->KeyCode)
// do list scrolling

      break;

    default:
      break;
    }

  return false;
}
