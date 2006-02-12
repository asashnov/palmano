#include <PalmOS.h>
#include "resource.h"
#include "editor_form.h"
#include "mkeys.h"
#include "notelist.h"
#include "utils.h"
#include "smfutils.h"

/* TODO: make menu item "add to system sounds".

   All SMF records in the System MIDI Sounds database are available to
   the user. Developers can add their own alarm SMFs to this database
   as a way to add variety and personalization to their devices. You
   can use the sysFileTMidi file type and sysFileCSystem creator to
   open this database.

   I.e. open datadase as upper and add new record- current song.
DmOpenDatabaseByTypeCreator
Example: file:///mnt/win_e/alex/PalmDev/devdoc/SystemFeatures.html#925370
*/


SndMidiListItemType EditorMidi;	/* current song */

static NoteListType notelist;
static MidiKeysType midikeys;

typedef enum {SCL_BEGIN, SCL_END } scl_seek_t;

static void
SeekScrollBar(scl_seek_t to)
{
  static Int16 pageSize = -1;
  Int16 scroll_max;

  if (pageSize == -1)
    pageSize = notelist.rect.extent.y/FntCharHeight();

  scroll_max = (notelist.num > pageSize)? notelist.num - pageSize : 0;

  notelist.firstDisplaying = (to == SCL_BEGIN)? 0 : scroll_max;

  SclSetScrollBar (GetObjectFromActiveForm(ID_EditorNoteScrollBar),
		   notelist.firstDisplaying /*pos*/, 0 /*min*/,
		   scroll_max, pageSize);
}


/* Get current selected note and fill out note properties elements.
   If selected == -1, clear and disable this fields. */
static void
UpdateNoteProperties()
{
  FormPtr frm = FrmGetActiveForm ();

  if (notelist.selected == -1) {
    FrmHideObject(frm, FrmGetObjectIndex(frm, ID_EditorDuration));
    FrmHideObject(frm, FrmGetObjectIndex(frm, ID_EditorVelocity));
    FrmHideObject(frm, FrmGetObjectIndex(frm, ID_EditorPause));
  }
  else {
    NotePtr notes = MemHandleLock(notelist.bufH);
    NotePtr note = notes + notelist.selected;

    ErrFatalDisplayIf(notelist.selected > notelist.num, "Invalid note index!");

    SetFieldTextFromNumber(ID_EditorDuration, note->dur);
    SetFieldTextFromNumber(ID_EditorVelocity, note->vel);
    SetFieldTextFromNumber(ID_EditorPause, note->pause);

    FrmShowObject(frm, FrmGetObjectIndex(frm, ID_EditorDuration));
    FrmShowObject(frm, FrmGetObjectIndex(frm, ID_EditorVelocity));
    FrmShowObject(frm, FrmGetObjectIndex(frm, ID_EditorPause));

    MemPtrUnlock(notes);
  }
}

static void 
FormUpdate()
{
  FormPtr frm = FrmGetActiveForm ();
  FrmDrawForm (frm);
  midikeys_draw(&midikeys);
  notelist_draw(&notelist);
  UpdateNoteProperties();
}

static Int16
LoadSMF(SndMidiListItemType midi, NoteListPtr list)
{
  Err err = false;
  DmOpenRef dbP;
  UInt16 recIndex;
  MemHandle midiH;

  debugPrintf("LoadSMF: open db cardNo=%d dbID=%d for readOnly\n",
	      midi.cardNo, midi.dbID);

  dbP = DmOpenDatabase (midi.cardNo, midi.dbID, dmModeReadOnly);
  if (!dbP)
    err = true;

  if (!err)
    err = DmFindRecordByID(dbP, midi.uniqueRecID, &recIndex);

  debugPrintf("LoadSMF: find record with uniqueRecID=%ld\n",
	      midi.uniqueRecID);

  if (!err) {
    midiH = DmQueryRecord (dbP, recIndex);
    if (!midiH)
      err = true;
  }

  debugPrintf("LoadSMF: midiH=%lx size=%ld\n",
	      midiH, MemHandleSize(midiH));

  if (!err)
    smfutils_load(midiH, list);

  if (dbP)
    DmCloseDatabase (dbP);

  if (err)
    ErrDisplay ("LoadSMF(): error occure in function.");
  return true;
}

static void
FormOpen (void)
{
  FormGadgetType *notelistGadget
    = GetObjectFromActiveForm (ID_EditorNoteListGadget);

  FormGadgetType *midekeysGadget
    = GetObjectFromActiveForm (ID_EditorMidiKeysGadget);

  notelist_init(&notelist, notelistGadget);
  midikeys_init(&midikeys, midekeysGadget);

  SetFieldTextFromStr (ID_EditorNameField, &EditorMidi.name[0]);

  if (EditorMidi.dbID != 0)
   LoadSMF(EditorMidi, &notelist);
  
  SeekScrollBar(SCL_BEGIN);

  FormUpdate();
}

static Boolean
FormPenDownEvent(EventType * e)
{
  FormPtr frm = FrmGetActiveForm ();
  UInt16 objIndex;
  RectangleType r;
  Boolean res = false;

  objIndex = FrmGetObjectIndex (frm, ID_EditorMidiKeysGadget);
  FrmGetObjectBounds (frm, objIndex, &r);
  if (RctPtInRectangle (e->screenX, e->screenY, &r)) {
    midikeys_tapped(&midikeys, e->screenX, e->screenY);
    res = true;
  }

  objIndex = FrmGetObjectIndex (frm, ID_EditorNoteListGadget);
  FrmGetObjectBounds (frm, objIndex, &r);
  if (RctPtInRectangle (e->screenX, e->screenY, &r)) {
    notelist_tapped(&notelist, e->screenX, e->screenY);
    res = true;
  }

  UpdateNoteProperties();

  return res;
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
    debugPrintf("getPalmanoDatabase(): Can't open database, code %d\n",
		DmGetLastErr());
    error = DmCreateDatabase (0, "palmano", pmnoCreatorDB, sysFileTMidi, false);

    if (error) {
      debugPrintf("getPalmanoDatabase(): DmCreateDatabase exit code %d\n", error);
      ErrAlert(error);
      return error;
    }
    else
      debugPrintf("getPalmanoDatabase(): create DB ok\n");

    dbP = DmOpenDatabaseByTypeCreator(sysFileTMidi, pmnoCreatorDB, mode);
    if (!dbP) {
      debugPrintf("getPalmanoDatabase(): Can't open database after create, code %d\n",
		  DmGetLastErr());
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
  Err err;

  if (EditorMidi.dbID == 0) {

    debugPrintf("dbID==0, create a new record in Palmano DB\n");
    /* create new record in palmano DB */
 
    err = getPalmanoDatabase(&openRef, dmModeReadWrite | dmModeExclusive);
    if (err != 0) {
      ErrAlert(err);
      return;
    }

    /* allocate new record in DB */
    recIndex = dmMaxRecordIndex;
    recH = DmNewRecord(openRef, &recIndex, 5); /* initial size is 5 bytes */
    ErrFatalDisplayIf(!recH, "SaveButtonClick(): can't get new record by index!");
  }
  else {
    /* replace old song in-place */

    openRef = DmOpenDatabase (EditorMidi.cardNo, EditorMidi.dbID,
			      dmModeReadWrite | dmModeExclusive);

    ErrFatalDisplayIf(!openRef, "SaveButtonClick(): "
		      "Can't open old song database for record");

    err = DmFindRecordByID(openRef, EditorMidi.uniqueRecID, &recIndex);

    ErrFatalDisplayIf(err, "Can't find record by ID");

    recH = DmGetRecord(openRef, recIndex);

    ErrFatalDisplayIf(!recH, "SaveButtonClick(): "
		      "Can't get old song record for writing");
  }

  // save midi to recH
  GetFieldTextToStr(EditorMidi.name, ID_EditorNameField, sndMidiNameLength);

  debugPrintf("SaveButtonClick(): smfutils_save(recH=%lx, name=%s\n",
	      recH, EditorMidi.name);
  smfutils_save(recH, EditorMidi.name, &notelist);
  debugPrintf("SaveButtonClick(): return from smfutils_save()\n");

  DmReleaseRecord (openRef, recIndex, 1);
  DmCloseDatabase (openRef);
  
  FrmGotoForm(ID_MainForm);
}


static void
PlayButtonClick (void)
{
  MemHandle smfH;

  smfH = smfutils_create(&notelist);
  smfutils_playHandle(smfH);
  MemHandleFree(smfH);
}


static void 
NoteButtonPressed (Int16 note)
{
  NoteType n = {note, 100, 40, 20};

  /* replace (selected) or append (to end) note */
  if (notelist.selected == -1)
    notelist_append(&notelist, &n);
  else {
    notelist_updateSelected(&notelist, &n);
    if (++notelist.selected >= notelist.num)
      notelist.selected = -1;
  }

  SeekScrollBar(SCL_END);
  notelist_draw(&notelist);
  PlayNote (&n);
}

static void
ScrollbarEvent (struct sclRepeat *data)
{
  if(data->scrollBarID == ID_EditorNoteScrollBar) {
    notelist.firstDisplaying = data->newValue;
    notelist.selected = -1;
    notelist_draw(&notelist);
  }

  UpdateNoteProperties();
}

static void
FormClose (void)
{
  notelist_free(&notelist);
}


/* Editor Form event handler */
Boolean EditorFormEventHandler(EventType * e)
{
  switch ((UInt16)e->eType)
    {
    case frmOpenEvent:
      FormOpen();
      return true;

    case frmUpdateEvent:
      FormUpdate();
      return true;

    case frmCloseEvent:
      FormClose();
      return false; /* return unhandled status for call default system handler (witch free form data),
		       accordin with standard sdk examples */
    case penDownEvent:
      return FormPenDownEvent(e);

    case ctlSelectEvent:
      switch (e->data.ctlSelect.controlID) {
      case ID_EditorDropButton: DropButtonClick(); return true;
      case ID_EditorSaveButton: SaveButtonClick(); return true;
      case ID_EditorPlayButton: PlayButtonClick(); return true;
      }
      break;

    case sclRepeatEvent:
      ScrollbarEvent((struct sclRepeat *)&e->data);
      return false;

    case MKeysNoteTappedEvent:
      NoteButtonPressed(e->data.generic.datum[0]);
      return true;
    }
  return false;
}
