#ifndef UTIL_H
#define UTIL_H

extern void* GetObjectFromActiveForm(UInt16 id);

extern void DrawCharsToFitWidth(const Char *text, RectangleType *bounds);

extern FieldPtr SetFieldTextFromHandle(UInt16 fieldID, MemHandle txtH);

/* Allocates new handle and copies incoming string */
extern FieldPtr SetFieldTextFromStr(UInt16 fieldID, Char *strP);

#endif
