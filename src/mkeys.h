/** This module is screen MIDI keyboard with handle tapp event
 *
*/
#ifndef MIDIKEYS_H
#define MIDIKEYS_H

enum {
  MKeysNoteTappedEvent = firstUserEvent
};

typedef struct MidiKeys
{
  RectangleType rect;
  Int16 first;			/* first note in screen MIDI keyboard */
  Int16 invert;
} MidiKeysType;
typedef MidiKeysType *MidiKeysPtr;

extern void midikeys_init(MidiKeysPtr mkeys, FormGadgetType *gadget);
extern void midikeys_draw(MidiKeysPtr mykeys);
extern void midikeys_tapped(MidiKeysPtr mykeys, Int16 tap_x, Int16 tap_y);

#endif
