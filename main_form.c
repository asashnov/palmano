#include <PalmOS.h>
#include "resource.h"
#include "main_form.h"
#include "editor_form.h"
#include "utils.h"
#include "smfutils.h"

static MemHandle MidiListH = NULL;
static UInt16 NumMidi;
static Int16 selected_song = -1;

/*
static Char * tips[] = {
  "You can setup volume for playing in Pref  application, sound volume for games",
  "You may put license key to memo in Unified category with title Palmano serial",
  NULL
};
*/

static void Open();
static void Play();
static void Copy();
static void Delete();
static void Info();
static void Beam();

typedef void (*action_t)(void);
static action_t actions[] = { Open, Play, Copy, Delete, Info, Beam };

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
  MemMove(EditorMidi.name, "Untitled", sndMidiNameLength);
  EditorMidi.cardNo = 0;
  EditorMidi.dbID = 0;
  EditorMidi.uniqueRecID = 0;
  FrmGotoForm (ID_EditorForm);
}

static void 
Play ()
{
  SndMidiListItemType *p;

  p = MemHandleLock (MidiListH);
  if (p == NULL) {
    ErrNonFatalDisplay ("Can't lock MidiListH");
    return;
  }
  smfutils_play(p + selected_song);
  MemHandleUnlock (MidiListH);
}


static void 
Open ()   /* TODO: Open(midi), get pointer in main function. */
{
  SndMidiListItemType *p;

  if((p = MemHandleLock (MidiListH)) == NULL) {
    ErrFatalDisplay ("Can't lock MidiListH");
    return;
  }
  MemMove (&EditorMidi.name[0], &p[selected_song].name[0], sndMidiNameLength);
  EditorMidi.cardNo = p[selected_song].cardNo;
  EditorMidi.dbID = p[selected_song].dbID;
  EditorMidi.uniqueRecID = p[selected_song].uniqueRecID;
  MemHandleUnlock (MidiListH);
  FrmGotoForm (ID_EditorForm);
}

static void 
Copy ()
{
  FrmAlert (ID_NotImplAlert);
}


static void 
Delete ()
{
  FrmAlert (ID_NotImplAlert);
}


static void 
Info ()
{
  FrmAlert (ID_NotImplAlert);
}


static void
Beam ()
{
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
  selected_song = -1;
  NumMidi = 0;
}



Boolean
MainFormEventHandler (EventType * e)
{
  static UInt16 selected_action = 0; /* by default Open() action */

  switch (e->eType)
    {
    case frmOpenEvent:
      MainFormOpen ();
      return true;

    case frmCloseEvent:
      MainFormClose ();
      return false;

    case lstSelectEvent:
      selected_song = e->data.lstSelect.selection;
      if (selected_action < 0 || selected_action > 5) {
	ErrDisplay("Bad action index selected!");
	return false;
      }
      if (selected_song < 0) {
	FrmAlert (ID_MidiNotSelAlert);
	return false;
      }
      actions[selected_action]();      
      return true;
      
    case ctlSelectEvent:
      switch (e->data.ctlSelect.controlID)
	{
	case ID_MainNewButton:
	  New ();
	  return true;
	}
      break;

    case popSelectEvent:
      selected_action = e->data.popSelect.selection;
      return false;

      //    case keyDownEvent:
      // TODO: Hardware button for scrolling handle
      //      struct _KeyDownEventType * k;
      //      k = (_KeyDownEventType *) data;
      //      switch (k->KeyCode)
      // do list scrolling
    }

  return false;			/* FrmDispatchEvent now must call FrmHandleEvent*/
}
