#ifndef MIDIKEYS_H
#define MIDIKEYS_H 1

/* Get settings from config, setup global variables */
extern void midikeys_init ();
   
/* midi_keyboard draw all function */
// TODO: It need do through call back function
// extern midi_keyboard_draw_gadget (struct FormGadgetType *gadgetP);

/* midi_keyboard gadget callback function */
extern void midikeys_gadget_cb (struct FormGadgetType *gadgetP, UInt16 cmd, void *paramP);

/* invert note */
extern void midikeys_invert_note (struct FormGadgetType *gadgetP, Int16 note);

#endif
