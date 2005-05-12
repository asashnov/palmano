#include <PalmOS.h>
#include "resource.h"
#include "editor_form.h"
#include "mkeys.h"
#include "notelist.h"
#include "utils.h"

/* 
SndMidiListItemType:
  Char	EditorName[sndMidiNameLength];
  LocalID	EditorDbID;
  UInt32	EditorUniqueRecID;
  UInt16	EditorCardNo;  */
SndMidiListItemType EditorMidi;	/* current song */

static NoteListType notelist;
static MidiKeysType midikeys;

//  ErrDisplay("editor form: All OK!!!!");

static void 
FormUpdate()
{
  FormPtr frm = FrmGetActiveForm ();
  FrmDrawForm (frm);
  midikeys_draw(&midikeys);
  notelist_draw(&notelist);
}

static void
FormOpen (void)
{
  FormGadgetType *notelistGadget = GetObjectFromActiveForm (ID_EditorNoteListGadget);
  FormGadgetType *midekeysGadget = GetObjectFromActiveForm (ID_EditorMidiKeysGadget);

  notelist_init(&notelist, notelistGadget);
  midikeys_init(&midikeys, midekeysGadget);

  // set the name field
  SetFieldTextFromStr (ID_EditorNameField, &EditorMidi.name[0]);

  if (EditorMidi.dbID != 0) {
    // load MIDI in note editor if it not new MIDI

    // TODO: call midi_load_notes () function from midi_util
  }
  FormUpdate();
}

static Boolean
FormPenDownEvent(EventType * e)
{
  FormPtr frm = FrmGetActiveForm ();
  UInt16 objIndex;
  RectangleType r;

  objIndex = FrmGetObjectIndex (frm, ID_EditorMidiKeysGadget);
  FrmGetObjectBounds (frm, objIndex, &r);
  if (RctPtInRectangle (e->screenX, e->screenY, &r)) {
    midikeys_tapped(&midikeys, e->screenX, e->screenY);
    return true;
  }
  objIndex = FrmGetObjectIndex (frm, ID_EditorNoteListGadget);
  FrmGetObjectBounds (frm, objIndex, &r);
  if (RctPtInRectangle (e->screenX, e->screenY, &r)) {
    notelist_tapped(&notelist, e->screenX, e->screenY);
    return true;
  }
  return false;
}

static void
DropButtonClick (void)
{
  // TODO: alert 
  // Do you want exit without save this song?
  // [ ] Don't ask in future
  // [Yes] [No] [Cancel]

  FrmGotoForm (ID_MainForm);
}

static void
SaveButtonClick (void)
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

  dbID = DmFindDatabase (cardNo, "palmano.pdb");
  
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

static void 
NoteButtonPressed (Int16 note)
{
  NoteType n = 
    { note, 100, 40, 20 };
  debugPrintf("NoteButtonPressed(): note %d is pressed\n", note);
  notelist_append(&notelist, &n);
  notelist_draw(&notelist);
  PlayNote (&n);
}

static void
FormClose (void)
{
  notelist_free(&notelist);
}


/* Editor Form event handler */
Boolean EditorFormEventHandler (EventType * e)
{
  switch ((UInt16)e->eType)
    {
    case frmOpenEvent:
      FormOpen ();
      return true;

    case frmUpdateEvent:
      FormUpdate();
      return true;

    case frmCloseEvent:
      FormClose ();
      return false;		/* return unhandled status for call default system handler (witch free form data). Accordin with standard sdk examples */

    case penDownEvent:
      return FormPenDownEvent(e);

    case ctlSelectEvent:
      switch (e->data.ctlSelect.controlID)
	{
	case ID_EditorDropButton:
	  DropButtonClick ();
	  return true;

	case ID_EditorSaveButton:
	  SaveButtonClick ();
	  return true;
	}
      break;

    case MKeysNoteTappedEvent:
      NoteButtonPressed(e->data.generic.datum[0]);
      return true;
    }
  return false;
}
