#ifndef EDITORFORM_H
#define EDITORFORM_H

extern Boolean EditorFormEventHandler (EventType * e);


/* Currently database edit 
   It can be real or temporary database.
   During form open, it read only.
   Write occure when Save button pressed.
   When 'Drop' button taped it just a drop.
   When it switch between application, it saves in temporary
   database and remember it for next time.

   Is equevalent to following:
  extern Char	EditorName[sndMidiNameLength];
  extern UInt32	EditorUniqueRecID;
  extern LocalID	EditorDbID;
  extern UInt16	EditorCardNo; */

/* MIDI file currently edited */
extern SndMidiListItemType EditorMidi;

#endif
