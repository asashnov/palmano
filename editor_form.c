#include <PalmOS.h>
#include "resource.h"
#include "editor_form.h"
#include "notelist.h"

/* Current song */
SndMidiListItemType EditorMidi;   

/* 
SndMidiListItemType:
  Char	EditorName[sndMidiNameLength];
  LocalID	EditorDbID;
  UInt32	EditorUniqueRecID;
  UInt16	EditorCardNo;  */

/* Note list. Functions for operate list in midi_util.c */
static MemHandle EditorNoteBufH = NULL; /* note buffer */
static UInt16 EditorNumNotes;	/* num notes in current use */

static UInt16 EditorNoteListFirst;	// first displaying note


void DrawOneNoteInList (Int16 itemNum,
			RectangleType * bounds, Char ** itemsText);


void
PlayNote (const NoteType *n)
{
  SndCommandType cmd;

  cmd.cmd = sndCmdNoteOn;
  cmd.param1 = n->note;		// the midi note to play
  cmd.param2 = n->dur;		// milliseconds to play the note
  cmd.param3 = 127;		// play at max. amplitude

  // play the sound asynchronously
  SndDoCmd (0, &cmd, true);
}




//  ErrDisplay("editor form: All OK!!!!");



/*****************************************
          Editor Form
*****************************************/

void
EditorFormOpen (void)
{
  ListType *list;
  FormGadgetType *mkGadget;
  FormType *form = FrmGetActiveForm ();
  mkGadget = GetObjectFromActiveForm (ID_EditorMidiGadget);

  // set the name field
  SetFieldTextFromStr (ID_EditorNameField, &EditorMidi.name[0]);

  // Alloc note buffer
  if (EditorNoteBufH != NULL)
    MemHandleFree (EditorNoteBufH);
  EditorNoteBufH = MemHandleNew (sizeof (NoteType));

  // init editor variables
  EditorNumNotes = 0;
  EditorNoteListFirst = 0;

  // setup note list
  list = GetObjectFromActiveForm (ID_EditorNoteList);
  LstSetDrawFunction (list, DrawOneNoteInList);
  LstSetListChoices (list, NULL, 0);

  // load MIDI in note editor if it not new MIDI
  if (EditorMidi.dbID != 0)
    {
// TODO: call midi_load_notes () function from midi_util
    }

  midi_keyboard_init ();

  FrmDrawForm (form);

  /* Draw midi keyboard */
  midikeyb_gadget_cb (mkGadget, formGadgetDrawCmd, NULL);
}


void
EditorFormClose (void)
{
  // current edited midi handle
  if (EditorNoteBufH != NULL)
    MemHandleFree (EditorNoteBufH);
}


static void
EditorDropButtonClick (void)
{
// TODO: alert 
// Do you want exit without save this song?
// [ ] Don't ask in future
// [Yes] [No] [Cancel]

  FrmGotoForm (ID_MainForm);
}


static void
EditorSaveButtonClick (void)
{
// TODO:

  // create palmano.pdb if not exist

  // open palmano.pdb

  if (EditorMidi.dbID == 0)
    {
      // create new record in palnamo.pdb
    }

  // open appropriate record (by number)

  // save MIDI name

  // save MIDI notes from editor buffer (by calling midi_save_notes() from midi_utils)


// Do this from following code:

#if 0
  Char name[sndMidiNameLength];
  UInt32 uniqueRecID;
  LocalID dbID;
  UInt16 cardNo;

  Err err;

  cardNo = 0;

  err =
    DmCreateDatabase (cardNo, "palmano.pdb", "Pmno", "smfr",
		      1 /*Boolean resDB */ )
    if (err == errNone || err == dmErrAlreadyExists)
    {
      // not finded, try to create it
      if ((err = DmGetLastErr ()) != dmErrCantFind)
	{
	  ErrDisplay ("Can't open database palmano.pdb");
	  return;
	}
    }
  else
    {
      ErrDisplay ("Can't create database palmano.pdb");
      return;
    }

  dbID = DmFindDatabase (cardNo, "palmano.pdb"));

  // create in it database new (empty) record


  // copy data in this record
  MemMove (&EditorName[0], &name[0], sndMidiNameLength);
  EditorCardNo = cardNo;
  EditorDbID = dbID;
  EditorUniqueRecID = uniqueRecID;

#endif

  // go to List form in last
  FrmGotoForm (ID_MainForm);
}


void EditorAddNoteToBuffer (const NoteType *note)
{
  MemHandle nl; /* new note list handle */

  nl = midiutil_AppendNoteToList (EditorNoteBufH, &EditorNumNotes, note);
  
  if (nl != NULL)
  EditorNoteBufH = nl;
}


static void EditorRefreshNoteList (void)
{
  ListPtr list = GetObjectFromActiveForm (ID_EditorNoteList);
    LstSetListChoices (list, NULL, EditorNumNotes);
    LstDrawList (list);
}


void EditorNoteButtonPressed (int note_pressed)
{
  NoteType note;

  note.note  = note_pressed;
  note.dur   = 100;
  note.vel   = 40;
  note.pause = 20;

  EditorAddNoteToBuffer (&note);
  EditorRefreshNoteList ();

  midiutil_PlayNote (&note);
}

/* draw note in list callback function */
void
  DrawOneNoteInList (Int16 itemNum, RectangleType * bounds, Char ** itemsText)
{
  int note, octave;
  int base;
  static char *note_name[12] =
  {
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
  NoteType *n;
  UInt8 *p;
  char buf[30];

  p = MemHandleLock (EditorNoteBufH);
  n = (NoteType*) p + (itemNum * sizeof (NoteType));

  note = n->note;

  if (note < 0)
    {
      StrPrintF (buf, "---");
      return;
    }

  octave = note / 12;
  base = note % 12;

  StrPrintF (buf, "%s%d %3d", note_name[base], octave, n->dur);

  DrawCharsToFitWidth (buf, bounds);

  MemPtrUnlock (p);
}


/* Editor Form event handler */
Boolean EditorFormEventHandler (EventType * e)
{
  FormGadgetType *mkGadget;
  /*  my be need in future 
FormType *form = FrmGetActiveForm ();  */
  mkGadget = GetObjectFromActiveForm (ID_EditorMidiGadget);

  switch (e->eType)
    {

    case frmOpenEvent:
      EditorFormOpen ();
      return true;

    case frmCloseEvent:
      EditorFormClose ();
      return true;

    case ctlSelectEvent:

      switch (e->data.ctlSelect.controlID)
	{
	case ID_EditorDropButton:
	  EditorDropButtonClick ();
	  return true;

	case ID_EditorSaveButton:
	  EditorSaveButtonClick ();
	  return true;
	}

    case penDownEvent:
      {
	FormPtr frm = FrmGetActiveForm ();
	UInt16 gadgetIndex = FrmGetObjectIndex (frm, ID_EditorMidiGadget);
	RectangleType bounds;

	FrmGetObjectBounds (frm, gadgetIndex, &bounds);
	if (RctPtInRectangle (e->screenX, e->screenY, &bounds))
	  {
	    midikeyb_gadget_cb (mkGadget, formGadgetHandleEventCmd, e);
	    return true;
	  }
      }
      break;
    default:
      return false;
    }

  return false;
}
