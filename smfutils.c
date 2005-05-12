#include "smfutils.h"

#if 0

  The SndMidiRecHdrType structure defines the fixed - size portion of a
  Palm OS MIDI record.(See SndCallbackInfoType.)

     typedef struct SndMidiRecHdrType
     {
       UInt32 signature;	//  Set to sndMidiRecSignature.
       UInt8 bDataOffset;	//  Offset from the beginning of the record to the 
                                //  standard MIDI File data stream.
       UInt8 reserved;		//  Set to zero.
     }
SndMidiRecHdrType;

     typedef struct SndMidiRecType
     {
       SndMidiRecHdrType hdr;
       Char name[1];		// Track name: 1 or more characters including 
//        NULL terminator. The length of name, including NULL
//        terminator, must not be greater than sndMidiNameLength.
     }
SndMidiRecType

     Each record in the database is a single SMF,
       with a header structure containing the user -
       visible name.The record includes a song header, then a track header,
       followed by any number of events.
#endif

// Useful structure field offset macro
#define prvFieldOffset(type, field) ((UInt32)(&((type*)0)->field))


// A standard header for MIDI format 0 sounds, with one track.
const UInt8 MidiHeader[] = {
  0x4D, 0x54, 0x68, 0x64,	// MThd
  0x00, 0x00, 0x00, 0x06,	// header data is 6 bytes long
  0x00, 0x00, 0x00, 0x01, 0x01, 0x90,	// format 0, 1 track, 0190 tempo (microseconds per quarter note)

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
smf_StartSMF ()
{
  UInt8 *buf;
  MemHandle bufH;

  bufH = MemHandleNew (MidiHeaderLength);

  if (bufH)
    {
      buf = MemHandleLock (bufH);
      MemMove (buf, (void *) MidiHeader, MidiHeaderLength);
      MemHandleUnlock (bufH);
    }

  return bufH;
}

static MemHandle
smf_AppendNote (MemHandle bufH, int note, int dur, int vel, int pause)
{
  UInt8 *buf;
  UInt32 bufsize, neededSize;
  int pos;			// , startpos;

  if (!bufH)
    return 0;			// make error-handling easy for our caller

  // how big is our buffer?
  bufsize = MemHandleSize (bufH);
  buf = MemHandleLock (bufH);
  pos = (*((UInt32 *) (&buf[18]))) + MidiHeaderLength - 2;	// position of next spot to put data
  MemHandleUnlock (bufH);

  // allocate more memory if necessary
  neededSize = 9 + pos;		// 9 is maximum number of bytes we'd use in here
  if (bufsize < neededSize)
    if (MemHandleResize (bufH, neededSize))
      return 0;

  buf = MemHandleLock (bufH);

  // start the note
  buf[pos++] = note;		// frequency
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
  *((UInt32 *) (&buf[18])) = pos - (MidiHeaderLength - 2);

  MemHandleUnlock (bufH);
  return bufH;
}

static MemHandle
smf_FinishSMF (MemHandle bufH)
{
  UInt8 *buf;
  UInt32 bufsize, neededSize;
  int pos;

  if (!bufH)
    return 0;			// make error-handling easy for our caller

  // how big is our buffer?
  bufsize = MemHandleSize (bufH);
  buf = MemHandleLock (bufH);
  pos = (*((UInt32 *) (&buf[18]))) + MidiHeaderLength - 2;	// position of next spot to put data
  MemHandleUnlock (bufH);

  // allocate more memory if necessary
  neededSize = 3 + pos;		// 3 more bytes to terminate things
  if (bufsize < neededSize)
    if (MemHandleResize (bufH, neededSize))
      return 0;

  buf = MemHandleLock (bufH);

  // now stop everything
  buf[pos++] = 0xff;
  buf[pos++] = 0x2f;
  buf[pos++] = 0x00;

  // update header to show how much data is there
  *((UInt32 *) (&buf[18])) = pos - (MidiHeaderLength - 2);

  MemHandleUnlock (bufH);
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


static MemPtr 
smf_ReadNote (MemPtr smf_stream, NoteType * n)
{
  UInt8 *p;

  p = smf_stream;
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

  return p;
}

int
smfLoad(SndMidiListItemType srcMidi, NoteListPtr dstList)
{
  Err err = false;
  MemHandle midiH;
  SndMidiRecHdrType *midiHdrP;
  UInt8 *midiStreamP;
  DmOpenRef dbP = NULL;
  UInt16 recIndex;
  MemPtr p;
  NoteType note;

  dbP = DmOpenDatabase (srcMidi.cardNo, srcMidi.dbID, dmModeReadOnly);
  if (!dbP)
    err = true;

  if (!err)
    err = DmFindRecordByID (dbP, srcMidi.uniqueRecID, &recIndex);

  if (!err) {
    midiH = DmQueryRecord (dbP, recIndex);
    midiHdrP = MemHandleLock (midiH);
    midiStreamP = (UInt8 *) midiHdrP + midiHdrP->bDataOffset;

    if ((p = smf_GetBeginNoteData(midiStreamP)) != NULL) {
      notelist_clear(dstList);
      while (!smf_isEndOfNoteData(p)) {
	p = smf_ReadNote(p, &note);
	notelist_append(dstList, &note);
      }
    }
    MemPtrUnlock (midiHdrP);
  }

  if (dbP)
    DmCloseDatabase (dbP);

  if (err)
    ErrDisplay ("smfLoad(): error occure in function.");
  return true;
}

int
smfSave(SndMidiListItemType dstMidi, const NoteListPtr srcList)
{
  Err err = 0;
  DmOpenRef dbP;
  UInt16 recIndex;
  MemHandle recH;
  UInt8 *recP;
  UInt8 *smfP;
  UInt8 				bMidiOffset;
  UInt32				dwSmfSize;
  SndMidiRecHdrType recHdr;
  MemHandle smfH;
  NoteType *notes;
  Int16 i;

  /* generate SMF from NoteList */
  smfH = smf_StartSMF ();
  notes = MemHandleLock(srcList->bufH);
  for (i = 0; i < srcList->num; i++)
      smf_AppendNote (smfH, notes[i].note, notes[i].dur, notes[i].vel, notes[i].pause);
  MemPtrUnlock(notes);
  smf_FinishSMF (smfH);

  /* save SMF */
  bMidiOffset = sizeof(SndMidiRecHdrType) + StrLen(dstMidi.name) + 1;
  dwSmfSize = MemHandleSize(smfH);
  recHdr.signature = sndMidiRecSignature;
  recHdr.reserved = 0;
  recHdr.bDataOffset = bMidiOffset;
  dbP = DmOpenDatabaseByTypeCreator (sysFileTMidi, sysFileCSystem, dmModeReadWrite | dmModeExclusive);
  //  dbP = DmOpenDatabaseByTypeCreator(dstMidi.);
  if (!dbP)
    return 1;

  /* Allocate a new record for the midi resource */
  recIndex = dmMaxRecordIndex;
  recH = DmNewRecord (dbP, &recIndex, bMidiOffset + dwSmfSize);
  if (!recH)
    return 2;

  /* Lock down the source SMF and target record and copy the data */
  smfP = MemHandleLock (smfH);
  recP = MemHandleLock (recH);

  err = DmWrite (recP, 0 /* offset */, &recHdr, sizeof (recHdr));
  if (!err)
    err = DmStrCopy (recP, prvFieldOffset(SndMidiRecType, name) /* offset of field name */, dstMidi.name);
  if (!err)
    err = DmWrite (recP, bMidiOffset, smfP, dwSmfSize);

  /* Unlock the pointers */
  MemHandleUnlock (smfH);
  MemHandleUnlock (recH);

  /*Because DmNewRecord marks the new record as busy,
    we must call DmReleaseRecord before closing the database */
  DmReleaseRecord (dbP, recIndex, 1);
  DmCloseDatabase (dbP);
  return err;
}

int
smfPlay(SndMidiListItemType *midi)
{
  Err err = false;
  MemHandle midiH;
  SndMidiRecHdrType *midiHdrP;
  UInt8 *midiStreamP;
  DmOpenRef dbP = NULL;
  UInt16 recIndex;
  SndSmfOptionsType smfOpt;

  dbP = DmOpenDatabase (midi->cardNo, midi->dbID, dmModeReadOnly);
  if (!dbP)
    err = true;

  if (!err)
    err = DmFindRecordByID (dbP, midi->uniqueRecID, &recIndex);

  if (!err) {
    midiH = DmQueryRecord (dbP, recIndex);
    midiHdrP = MemHandleLock (midiH);
    midiStreamP = (UInt8 *) midiHdrP + midiHdrP->bDataOffset;
    smfOpt.dwStartMilliSec = 0;
    smfOpt.dwEndMilliSec = sndSmfPlayAllMilliSec;
    smfOpt.amplitude = (UInt16) PrefGetPreference (prefGameSoundVolume);
    smfOpt.interruptible = true; /* The sound can be interrupted by a key/digitizer event */
    smfOpt.reserved = 0;
    err = SndPlaySmf (NULL, sndSmfCmdPlay, midiStreamP, &smfOpt, NULL, NULL, false);
    MemPtrUnlock (midiHdrP);
  }

  if (dbP)
    DmCloseDatabase (dbP);

  if (err)
    ErrDisplay ("smfPlay(): error occure in function.");
  return true;
}
