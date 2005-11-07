#include <PalmOS.h>
#include "utils.h"

#ifdef DEBUG

#include <stdarg.h>

#define HOSTLOGNAME "palmano.log"

void debugPrintf(char *fmt, ...)
{
  Char text[256];
  static HostFILE  *log = NULL;
  va_list args;
  va_start(args, fmt);

  if (log == NULL)
    log=HostFOpen(HOSTLOGNAME,"w");
  StrVPrintF(text, fmt, args);
  HostFPutS(text, log);
  HostFFlush(log);
  va_end(args);
}
#endif

/*
Formats samples:

  %lx  - MemHandles


*/



void *
GetObjectFromActiveForm (UInt16 id)
{
  UInt16 index;

  FormPtr frm = FrmGetActiveForm ();
  index = FrmGetObjectIndex (frm, id);
  return FrmGetObjectPtr (frm, index);
}


void
DrawCharsToFitWidth (const Char * text, RectangleType * bounds)
{
  WinPaintChars (text, StrLen (text), bounds->topLeft.x, bounds->topLeft.y);
}

FieldPtr
SetFieldTextFromHandle (UInt16 fieldID, MemHandle txtH)
{
  MemHandle oldTxtH;
  FormPtr frm = FrmGetActiveForm ();
  FieldPtr fldP;

  // get the field and the field's current text handle.
  fldP = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, fieldID));
  ErrNonFatalDisplayIf (!fldP, "missing field");
  oldTxtH = FldGetTextHandle (fldP);

  // set the field's text to the new text.
  FldSetTextHandle (fldP, txtH);
  FldDrawField (fldP);

  // free the handle AFTER we call FldSetTextHandle().
  if (oldTxtH)
    MemHandleFree (oldTxtH);

  return fldP;
}

// Allocates new handle and copies incoming string
FieldPtr
SetFieldTextFromStr (UInt16 fieldID, Char * strP)
{
  MemHandle txtH;

  // get some space in which to stash the string.
  txtH = MemHandleNew (StrLen (strP) + 1);
  if (!txtH)
    return NULL;

  // copy the string to the locked handle.
  StrCopy (MemHandleLock(txtH), strP);

  // unlock the string handle.
  MemHandleUnlock (txtH);

  // set the field to the handle
  return SetFieldTextFromHandle (fieldID, txtH);
}

void GetFieldTextToStr (char *to, const UInt16 fieldId, int max)
{
  FieldPtr fldP = GetObjectFromActiveForm(fieldId);
  Char *s = FldGetTextPtr(fldP);
  if (s == NULL)		/* field is empty */
    to[0] = '\0';
  else {
    if(StrLen(s) > max)
      ErrDisplay("String to long!");
    else
      StrCopy(to,s);
    MemPtrUnlock(s);
  }
}

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
