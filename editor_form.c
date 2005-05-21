#include <PalmOS.h>
#include "resource.h"
#include "editor_form.h"
#include "mkeys.h"
#include "notelist.h"
#include "utils.h"
#include "smfutils.h"

SndMidiListItemType EditorMidi;	/* current song */

static NoteListType notelist;
static MidiKeysType midikeys;

static void 
FormUpdate()
{
  FormPtr frm = FrmGetActiveForm ();
  FrmDrawForm (frm);
  midikeys_draw(&midikeys);
  notelist_draw(&notelist);
}

static Int16
LoadSMF(SndMidiListItemType midi, NoteListPtr list)
{
  Err err = false;
  DmOpenRef dbP;
  UInt16 recIndex;
  MemHandle midiH;

  dbP = DmOpenDatabase (midi.cardNo, midi.dbID, dmModeReadOnly);
  if (!dbP)
    err = true;

  if (!err)
    err = DmFindRecordByID(dbP, midi.uniqueRecID, &recIndex);

  if (!err) {
    midiH = DmQueryRecord (dbP, recIndex); 
    smfutils_load(midiH, list);
  }

  if (dbP)
    DmCloseDatabase (dbP);

  if (err)
    ErrDisplay ("LoadSMF(): error occure in function.");
  return true;
}

static void
FormOpen (void)
{
  FormGadgetType *notelistGadget = GetObjectFromActiveForm (ID_EditorNoteListGadget);
  FormGadgetType *midekeysGadget = GetObjectFromActiveForm (ID_EditorMidiKeysGadget);

  notelist_init(&notelist, notelistGadget);
  midikeys_init(&midikeys, midekeysGadget);

  SetFieldTextFromStr (ID_EditorNameField, &EditorMidi.name[0]);

  if (EditorMidi.dbID != 0)
   LoadSMF(EditorMidi, &notelist);

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

/** Returns reference to opened Palmano database, create it if not exit */
static DmOpenRef
getPalmanoDB()
{
  DmOpenRef dbP;


  dbP = DmOpenDatabaseByTypeCreator(sysFileTMidi, sysFileCSystem, dmModeReadWrite | dmModeExclusive);
  if (!dbP)
    ErrFatalDisplay("Can't open system MIDI database!");
  
  return dbP;
}


static void
SaveButtonClick (void)
{
  DmOpenRef openRef;
  UInt16 recIndex;
  MemHandle recH;

  if (EditorMidi.dbID == 0) {
    debugPrintf("SaveButtonClick(): new midi create required.\n");

    /* New song- create new record in DB */
    openRef = getPalmanoDB();
    debugPrintf("SaveButtonClick(): getPalmanoDB returns %x\n", openRef);
    ErrFatalDisplayIf(openRef == 0, "Can't open System MIDI Sounds database");

    recIndex = dmMaxRecordIndex;
    recH = DmNewRecord(openRef, &recIndex, 5); /* initial size is 5 bytes */
    if (recH == 0) {
      Err err = DmGetLastErr();
      ErrAlert(err);
      ErrFatalDisplay("Can't create new record in palmano DB");
    }
  } else {
    /* replace old song in-place */
    openRef = DmOpenDatabase (EditorMidi.cardNo, EditorMidi.dbID, dmModeReadWrite | dmModeExclusive);
    ErrFatalDisplayIf(!openRef, "Can't open old song database for record");
    recIndex = EditorMidi.uniqueRecID;
    recH = DmGetRecord(openRef, recIndex);
    ErrFatalDisplayIf(!recH, "Can't get old song record for writing");
  }
    
  smfutils_save(recH, EditorMidi.name, &notelist);
  DmReleaseRecord (openRef, recIndex, 1);
  DmCloseDatabase (openRef);
  
  FrmGotoForm(ID_MainForm);
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
      return false;		/* return unhandled status for call default system handler (witch free form data),
				   accordin with standard sdk examples */
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
