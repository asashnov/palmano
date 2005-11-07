#include "smfutils.h"
#include "utils.h"

// Useful structure field offset macro
#define prvFieldOffset(type, field) ((UInt32)(&((type*)0)->field))

// A standard header for MIDI format 0 sounds, with one track.
const UInt8 MidiHeader[] = {
  0x4D, 0x54, 0x68, 0x64,	// MThd
  0x00, 0x00, 0x00, 0x06,	// header data is 6 bytes long
  0x00, 0x00, 0x00, 0x01,       // format 0, 1 track,
  0x01, 0x90,	                // 0190 tempo (microseconds per quarter note)
  0x4D, 0x54, 0x72, 0x6B,	// MTrk
  0x00, 0x00, 0x00, 0x02,	// length in bytes of following data
  0x00, 0x90			// start playing a sound...
// then things like:
//             0x5C, 0x40,    // note 5c at velocity 40
// 0x81, 0x60, 0x5C, 0x00,    // after 160 ticks, turn off 5c
// (repeat above two lines ad nauseum)
// 0xFF, 0x2F, 0x00     // shut everything down
};
const UInt32 MidiHeaderLength = 24;

static MemHandle
smf_StartSMF (const NoteListPtr nl)
{
  MemHandle retH;

  retH = MemHandleNew(MidiHeaderLength);
	
  if (retH) {
    UInt8 *buf = MemHandleLock(retH);
    // copy standard MIDI header from read-only location
    MemMove(buf, MidiHeader, MidiHeaderLength);
    // set tempo
    *((UInt16*)(buf+12)) = nl->tempo;
    MemHandleUnlock(retH);
  } else
    ErrDisplay ("Can't alloc midi header");

  return retH;
}

static MemHandle
smf_AppendNote (MemHandle bufH, int note, int dur, int vel, int pause)
{
  UInt8 *buf;
  UInt32 bufsize, neededSize;
  int pos; // , startpos;
	
  if (!bufH) return 0;		// make error-handling easy for our caller
	
  // how big is our buffer?
  bufsize = MemHandleSize(bufH);
  buf = MemHandleLock(bufH);
  pos = (*((UInt32 *)(&buf[18]))) + MidiHeaderLength -2;	// position of next spot to put data
  MemHandleUnlock(bufH);
	
  // allocate more memory if necessary
  neededSize = 9 + pos; // 9 is maximum number of bytes we'd use in here
  if (bufsize < neededSize)
    if (MemHandleResize(bufH, neededSize))
      return 0;
	
  buf = MemHandleLock(bufH);
	
  // start the note
  buf[pos++] = note;	// frequency
  if (vel >= 128)		// velocity i.e. volume
    buf[pos++] = 0x80 + ((vel & 0x0780) >> 7);
  buf[pos++] = (vel & 0x07f);

  // wait for the specified duration
  if (dur >= 128)
    buf[pos++] = 0x80 + ((dur & 0x7f80) >> 7);
  buf[pos++] = (dur & 0x07f);

  // stop playing it
  buf[pos++] = note;
  buf[pos++] = 0;

  // pause afterwards
  if (pause >= 128)
    buf[pos++] = 0x80 + ((pause & 0x7f80) >> 7);
  buf[pos++] = (pause & 0x07f);
	
  // update header to show how much data is there
  *((UInt32 *)(&buf[18])) = pos - (MidiHeaderLength -2);

  MemHandleUnlock(bufH);
  return bufH;
}

static MemHandle
smf_FinishSMF (MemHandle bufH)
{
  UInt8 *buf;
  UInt32 bufsize, neededSize;
  int pos;

  if (!bufH) return 0; // make error-handling easy for our caller

  // how big is our buffer?
  bufsize = MemHandleSize(bufH);
  buf = MemHandleLock(bufH);
  pos = (*((UInt32 *)(&buf[18]))) + MidiHeaderLength -2; // position of next spot to put data
  MemHandleUnlock(bufH);
	
  // allocate more memory if necessary
  neededSize = 3 + pos; // 3 more bytes to terminate things
  if (bufsize < neededSize)
    if (MemHandleResize(bufH, neededSize))
      return 0;
	
  buf = MemHandleLock(bufH);

  // now stop everything
  buf[pos++] = 0xff;
  buf[pos++] = 0x2f;
  buf[pos++] = 0x00;

  // update header to show how much data is there
  *((UInt32 *)(&buf[18])) = pos - (MidiHeaderLength -2);

  MemHandleUnlock(bufH);
  return bufH;
}


static MemPtr
smf_GetBeginNoteData (MemPtr smf_stream)
{
  /*  return smf_stream + MidiHeaderLength; */
  
  /* Check that this MIDI */
  if ( StrNCompare(smf_stream, "MThd", 4) != 0 ) {
    ErrDisplay ("It's not MThd header");
    return NULL;
  }

  return smf_stream + MidiHeaderLength;
}


/* Return 'true' if pointer on end of SMF data */
static Boolean
smf_isEndOfNoteData (MemPtr smf_stream)
{
  UInt8 *p = (UInt8 *) smf_stream;

  if (*(p + 0) != 0xff)
    return false;
  if (*(p + 1) != 0x2f)
    return false;
  if (*(p + 2) != 0x00)
    return false;

  return true;
}

/* Read one note from stream.
 * Return value: pointer to next note in stream.
 */
static MemPtr 
smf_ReadNote (MemPtr smf_stream, NoteType * n)
{
  UInt8 *p = smf_stream;

  n->note = *p++;

  if (*p >= 0x80)
    n->vel = (*p++ - 0x80) << 7;
  else
    n->vel = 0;
  n->vel += *p++;

  if (*p >= 0x80)
    n->dur = (*p++ - 0x80) << 7;
  else
    n->dur = 0;
  n->dur += *p++;
  
  /* TODO: May be not check that??  Then do simple P+=2; */
  /* Check that note off command follow */
  if (*p++ != n->note) { ErrDisplay("There is no note off command"); }
  if (*p++ != 0)       { ErrDisplay("There is no 0x00 code after note off command"); }
  
  if (*p >= 0x80)
    n->pause = (*p++ - 0x80) << 7;
  else
    n->pause = 0;
  n->pause += *p++;

  debugPrintf("smf_ReadNote: note %x vel=%d dur=%d pause=%d\n",
	      n->note, n->vel, n->dur, n->pause);

  return p;
}

void
smfutils_load(MemHandle midiH, NoteListPtr dstList)
{
  SndMidiRecHdrType *midiHdrP;
  UInt8 *midiStreamP;
  MemPtr p;
  NoteType note;
  UInt8 midihead[24];

  // get Palm MIDI record header (for skip name of track)
  midiHdrP = MemHandleLock (midiH);
  midiStreamP = (UInt8 *) midiHdrP + midiHdrP->bDataOffset;

  // copy standard MIDI header
  MemMove(&midihead, midiStreamP, MidiHeaderLength);

  dstList->tempo = *((UInt16*)midihead+12);

  if ((p = smf_GetBeginNoteData(midiStreamP)) != NULL) {
    notelist_clear(dstList);
    while (!smf_isEndOfNoteData(p)) {
      p = smf_ReadNote(p, &note);
      notelist_append(dstList, &note);
    }
  }
  MemPtrUnlock (midiHdrP);
}


/* Generate smf from note list */
MemHandle smfutils_create(const NoteListPtr srcList)
{
  MemHandle smfH;
  NoteType *notes;
  Int16 i;

  smfH = smf_StartSMF(srcList);
  debugPrintf("smfutils_create(): smfH started, now is %lx\n", smfH);
  notes = MemHandleLock(srcList->bufH);
  for (i = 0; i < srcList->num; i++)
      smf_AppendNote(smfH, notes[i].note, notes[i].dur, notes[i].vel, notes[i].pause);
  debugPrintf("Notes added\n");
  MemHandleUnlock(srcList->bufH);
  smf_FinishSMF(smfH);
  return smfH;
}

int
smfutils_save(MemHandle saveToH, const Char *midiname, const NoteListPtr srcList)
{
  Err err;
  UInt8  MidiOffset;
  UInt32 SmfSize;
  SndMidiRecHdrType recHdr;
  MemHandle smfH;
  UInt8 *saveToP;

  // get standard MIDI record from note list
  smfH = smfutils_create(srcList);

  debugPrintf("smfutils_save(): smf handle=%lx\n", smfH);

  /* save SMF */
  MidiOffset = sizeof(SndMidiRecHdrType) + StrLen(midiname) + 1;
  SmfSize = MemHandleSize(smfH); /* size of all MIDI section (without PalmOS header) */
  recHdr.signature = sndMidiRecSignature;
  recHdr.reserved = 0;
  recHdr.bDataOffset = MidiOffset;

  /* resize record in DB for insert PalmOS header */
  if (MemHandleResize(saveToH, MidiOffset + SmfSize))
    ErrFatalDisplay("Can't resize handle");

  /* Lock down the source SMF and target record and copy the data */
  saveToP = MemHandleLock(saveToH);

  err = DmWrite(saveToP, 0, &recHdr, sizeof(recHdr));
  if (!err) err = DmStrCopy(saveToP, prvFieldOffset(SndMidiRecType, name), midiname);
  if (!err) {
    UInt8 *smfP = MemHandleLock(smfH);
    err = DmWrite(saveToP, MidiOffset, smfP, SmfSize);
    MemHandleUnlock(smfH);    
  }

  if(err) {
    ErrAlert(err);
  }


  MemHandleUnlock(saveToH);
  return err;
}

int smfutils_playHandle(MemHandle smfH)
{
  SndMidiRecHdrType *midiHdrP;
  UInt8 *midiStreamP;
  SndSmfOptionsType smfOpt;
  Err ret;

  midiHdrP = MemHandleLock (smfH);
  midiStreamP = (UInt8 *) midiHdrP + midiHdrP->bDataOffset;
  smfOpt.dwStartMilliSec = 0;
  smfOpt.dwEndMilliSec = sndSmfPlayAllMilliSec;
  smfOpt.amplitude = (UInt16) PrefGetPreference (prefGameSoundVolume);
  smfOpt.interruptible = true; /* The sound can be interrupted by a key/digitizer event */
  smfOpt.reserved = 0;
  ret = SndPlaySmf (NULL, sndSmfCmdPlay, midiStreamP, &smfOpt, NULL, NULL, false);
  MemPtrUnlock (midiHdrP);

  return ret;
}

int
smfutils_play(SndMidiListItemType *midi)
{
  Err err = false;
  MemHandle midiH;
  DmOpenRef dbP = NULL;
  UInt16 recIndex;


  dbP = DmOpenDatabase (midi->cardNo, midi->dbID, dmModeReadOnly);
  if (!dbP)
    err = true;

  if (!err)
    err = DmFindRecordByID (dbP, midi->uniqueRecID, &recIndex);

  if (!err) {
    midiH = DmQueryRecord (dbP, recIndex);
    err = smfutils_playHandle(midiH);
  }

  if (dbP)
    DmCloseDatabase (dbP);

  if (err)
    ErrDisplay ("smfPlay(): error occure in function.");
  return true;
}
