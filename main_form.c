#include <PalmOS.h>
#include "resource.h"
#include "main_form.h"
#include "editor_form.h"

static MemHandle MidiListH = NULL;
static UInt16 NumMidi;
static Int16 list_Selected = -1;

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



/*   Main form actions   */

static void 
New ()
{
  MemMove (EditorMidi.name, "Untitled", sndMidiNameLength);
  EditorMidi.cardNo = 0;
  EditorMidi.dbID = 0;
  EditorMidi.uniqueRecID = 0;

  FrmGotoForm (ID_EditorForm);
}

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

  MemMove (&EditorMidi.name[0], &p[list_Selected].name[0], sndMidiNameLength);
  EditorMidi.cardNo = p[list_Selected].cardNo;
  EditorMidi.dbID = p[list_Selected].dbID;
  EditorMidi.uniqueRecID = p[list_Selected].uniqueRecID;

  MemHandleUnlock (MidiListH);

  FrmGotoForm (ID_EditorForm);
}


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
  FormType *form = FrmGetActiveForm ();

  if (MidiListH != NULL)
    {
      MemHandleFree (MidiListH);
      MidiListH = NULL;
    }

  /* get list of system midi */
  SndCreateMidiList (0, true, &NumMidi, &MidiListH);

  list = GetObjectFromActiveForm (ID_MainSongList);
  LstSetDrawFunction (list, cb_midi_list_draw_item);
  LstSetListChoices (list, NULL, NumMidi);

  FrmDrawForm (form);	/* Draw all objects in a form and the frame around the form */
}


void
MainFormClose (void)
{
  if (MidiListH != NULL) {
    MemHandleFree (MidiListH);
    MidiListH = NULL;
  }
}


Boolean
MainFormEventHandler (EventType * e)
{
  //  FormType *form = FrmGetActiveForm ();
  static UInt16 ActionListSelected = 0; /* Open action selected by default */

  switch (e->eType)
    {
    case frmOpenEvent:
      MainFormOpen ();
      return true;

    case frmCloseEvent:
      MainFormClose ();
      return false;

    case lstSelectEvent:
      switch (e->data.ctlSelect.controlID)
	{
	case ID_MainSongList:	/* select song in midi list */
	  {
	    list_Selected = e->data.lstSelect.selection;
	    switch (ActionListSelected)	/* action whis selected midi */
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

    case popSelectEvent:
      ActionListSelected = e->data.popSelect.selection;	/* remember number of selected action in action list */
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

  return false;			/* FrmDispatchEvent now must call FrmHandleEvent*/
}
