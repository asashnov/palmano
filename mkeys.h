#ifndef MIDIKEYS_H
#define MIDIKEYS_H 1

/* Get settings from config, setup global variables */
extern void midikeys_init ();
   
/* midi_keyboard draw all function */
// TODO: It need do through call back function
// extern midi_keyboard_draw_gadget (struct FormGadgetType *gadgetP);

/* midi_keyboard gadget callback function.
   NOTE: it calls by programm, not by system
   becose it facility from PalmOS 3.5 but custom alarm sound from PalmOS3.0
 */
extern void midikeys_gadget_cb (struct FormGadgetType *gadgetP, UInt16 cmd, void *paramP);

/* invert note */
extern void midikeys_invert_note (struct FormGadgetType *gadgetP, Int16 note);

#endif
