#include <PalmOS.h>
#include "util.h"

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
  StrCopy (MemHandleLock (txtH), strP);

  // unlock the string handle.
  MemHandleUnlock (txtH);

  // set the field to the handle
  return SetFieldTextFromHandle (fieldID, txtH);
}
