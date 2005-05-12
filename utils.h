#ifndef UTIL_H
#define UTIL_H

#include "notelist.h"

#ifdef DEBUG
  void debugPrintf(char *fmt, ...);
#else
# define debugPrintf \/\/
#endif

extern void* GetObjectFromActiveForm (UInt16 id);

extern void DrawCharsToFitWidth (const Char *text, RectangleType *bounds);

extern FieldPtr SetFieldTextFromHandle (UInt16 fieldID, MemHandle txtH);

/* Allocates new handle and copies incoming string */
extern FieldPtr SetFieldTextFromStr (UInt16 fieldID, Char *strP);

extern void PlayNote (const NoteType *n);

#endif
