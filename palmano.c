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
  FrmCloseAllForms ();
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
      // Load the form resource.
      formId = event->data.frmLoad.formID;
      frm = FrmInitForm (formId);
      FrmSetActiveForm (frm);
      // Set the event handler for the form.  The handler of the currently
      // active form is called by FrmHandleEvent each time is receives an
      // event.
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

		if (!ApplicationHandleEvent (&event))

		  FrmDispatchEvent (&event);

	}
      while (event.eType != appStopEvent);

      StopApplication ();
    }

  return 0;
}
