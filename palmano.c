#include <PalmOS.h>
#include <SysEvtMgr.h>
#include <Rect.h>

#include "resource.h"
#include "main_form.h"
#include "editor_form.h"
#include "option_form.h"

static UInt32 rom_version;




//  ErrDisplay("All OK!!!!");




void
StopApplication (void)
{
  FrmCloseAllForms ();		/* Send a frmCloseEvent to all open forms. */
}


static Boolean
AppHandleKeyDown (EventType * event)
{
  return false;
}

static Boolean
ApplicationHandleEvent (EventType * event)
{
  UInt16 formId;
  FormPtr frm;

  if (event->eType == frmLoadEvent)
    {
      /* Load the form resource. */
      formId = event->data.frmLoad.formID;
      frm = FrmInitForm (formId); /* Load and initialize a form resource. */
      FrmSetActiveForm (frm);	/* Set the active form. All input (key and pen) is directed 
				   to the active form and all drawing occurs there */

      /* Registers the event handler callback routine for the specified form. */
      switch (formId)
	{
	case ID_MainForm:
	  FrmSetEventHandler (frm, MainFormEventHandler);
	  break;

	case ID_EditorForm:
	  FrmSetEventHandler (frm, EditorFormEventHandler);
	  break;

	default:
	  ErrNonFatalDisplay ("Invalid Form Load Event");
	  break;
	}
      return (true);
    }
  return (false);
}


#define UNUSED  __attribute__((__unused__))

UInt32
PilotMain (UInt16 cmd, void *cmdPBP UNUSED, UInt16 launchFlags UNUSED)
{
  UInt16 error;
  EventType event;

  if (cmd == sysAppLaunchCmdNormalLaunch)
    {
      if (FtrGet (sysFtrCreator, sysFtrNumROMVersion, &rom_version) != 0)
	rom_version = 0;	/* Set to very low version if FtrGet failed.  */

      FrmGotoForm (ID_MainForm);

      do
	{
	  EvtGetEvent (&event, evtWaitForever);

	  if (!SysHandleEvent (&event))

	    if (!AppHandleKeyDown (&event))

	      if (!MenuHandleEvent (0, &event, &error))

		if (!ApplicationHandleEvent (&event)) /* only FrmLoadEvent */

		  FrmDispatchEvent (&event);

	}
      while (event.eType != appStopEvent);

      StopApplication ();
    }

  return 0;
}