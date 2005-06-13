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


/* DESCRIPTION:  Get the application's database.  Open the database if it
 * exists, create it if neccessary.
 *
 * PARAMETERS:   *dbPP - pointer to a database ref (DmOpenRef) to be set
 *					  mode - how to open the database (dmModeReadWrite)
 *
 * RETURNED:     Err - zero if no error, else the error
 */
static Err getPalmanoDatabase (DmOpenRef *dbPP, UInt16 mode)
{
  Err error = 0;
  DmOpenRef dbP;

  *dbPP = NULL;
  
  // Find the application's data file.  If it doesn't exist create it.
  dbP = DmOpenDatabaseByTypeCreator (sysFileTMidi, pmnoCreatorDB, mode);
  if (!dbP) {
    debugPrintf("getPalmanoDatabase(): Can't open database, code %d\n", DmGetLastErr());
    error = DmCreateDatabase (0, "palmano", pmnoCreatorDB, sysFileTMidi, false);
    if (error) {
      debugPrintf("getPalmanoDatabase(): DmCreateDatabase exit code %d\n", error);
      ErrAlert(error);
      return error;
    } else
      debugPrintf("getPalmanoDatabase(): create DB ok\n");

    dbP = DmOpenDatabaseByTypeCreator(sysFileTMidi, pmnoCreatorDB, mode);
    if (!dbP) {
      debugPrintf("getPalmanoDatabase(): Can't open database after create, code %d\n", DmGetLastErr());
      ErrAlert(DmGetLastErr());      
      return DmGetLastErr();
    } else
      debugPrintf("getPalmanoDatabase(): second open DB ok\n");


    // Set the backup bit.  This is to aid syncs with non Palm software.
    //    ToDoSetDBBackupBit(dbP);
		
  } else
    debugPrintf("getPalmanoDatabase(): open DB ok\n");


  *dbPP = dbP;
  return 0;
}


static void
SaveButtonClick (void)
{
  DmOpenRef openRef;
  UInt16 recIndex;
  MemHandle recH;

  if (EditorMidi.dbID == 0) {
    Err err;

    err = getPalmanoDatabase(&openRef, dmModeReadWrite | dmModeExclusive);
    if (err != 0) {
      ErrAlert(err);
      return;
    }

    /* allocate new record in DB */
    recIndex = dmMaxRecordIndex;
    recH = DmNewRecord(openRef, &recIndex, 5); /* initial size is 5 bytes */
    if (recH == 0) {
      Err err = DmGetLastErr();
      ErrAlert(err);
      ErrFatalDisplay("SaveButtonClick(): Can't create new record in palmano DB");
    } else
      debugPrintf("SaveButtonClick(): new record with index %u is created\n", recIndex);
    //    recH = DmGetRecord(openRef, recIndex);
    // debugPrintf("SaveButtonClick(): new record handle by DmGetRecord is %lx\n", recH);
  } else {
    /* replace old song in-place */
    openRef = DmOpenDatabase (EditorMidi.cardNo, EditorMidi.dbID, dmModeReadWrite | dmModeExclusive);
    ErrFatalDisplayIf(!openRef, "SaveButtonClick(): Can't open old song database for record");
    recIndex = EditorMidi.uniqueRecID;
    recH = DmGetRecord(openRef, recIndex);
    ErrFatalDisplayIf(!recH, "SaveButtonClick(): Can't get old song record for writing");
  }

  debugPrintf("SaveButtonClick(): call smfutils_save for save notelist to handle %lx\n", recH);
  GetFieldTextToStr(EditorMidi.name, ID_EditorNameField, sndMidiNameLength);
  smfutils_save(recH, EditorMidi.name, &notelist);
  debugPrintf("SaveButtonClick(): return from smfutils_save()\n");

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
