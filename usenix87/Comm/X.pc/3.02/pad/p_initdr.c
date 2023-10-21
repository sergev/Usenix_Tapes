/* -------------------------------------------------------------------------
	    Driver Initialization and Startup Routines
		     filename = p_initdr.c

   The routines in this file are used to get the driver to an initialized
   state.  They zero data structures, initialize the Asynchronous port,
   and set up data structures with the default values specified in the X.PC
   PAD/Driver Specification.

   drv_init    Called by the Assembler routine that is invoked when the DOS
	       device driver is loading the driver.  Initializes all data
	       structures, and displays the version number for the user.

   init_device Sets up the fields of the Device Control Block, as required
	       by the X.PC Driver Interface Specification (from p_config.d).

   init_port   Sets up the fields of the Port Control Block, as required
	       by the X.PC Driver Interface Specification (from p_config.d).
   -------------------------------------------------------------------------
	 Date    Change   By     Reason
      05/26/84     01   curt     Initial Generation of Code.
      08/27/84     02   curt     Altered default Port Parameters (init_port).
      11/13/84     03   curt     Re-did pool building logic.
      09/26/86     04   Ed M     Reformat and recomment
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "pad.h"

/*                   0----+----1----+----2----+----3----+----4----+----5 */
char ver_string[] = "X.PC Driver, 1986, version v.rr\n\r$";
#define  ZER0 '0'
#define  vers_offset 27
#define  rev1_offset vers_offset+2
#define  rev2_offset rev1_offset+1



/* -------------------------------------------------------------------------
   This routine is called by the loader of DOS device drivers.  It has to
   call the routine that initializes the DOS interrupt vectors for applications
   to use to access the driver.  The 'init_device' routines sets driver's state
   to 'wait reset', meaning that the driver is loaded and ready for a reset.
   ------------------------------------------------------------------------- */
void drv_init ()
{
    extern void disp_str(),
		mak_quepool(),
		mak_bufpool();

    int_off ();
    load_drv ();
    zero_mem ((char *) & dcb, sizeof (DCB_STR));
    zero_pkt ();
    mak_quepool ();
    mak_bufpool ();
    init_port ();
    init_device ();
    int_on ();
 /*
  This code puts the version/revision values into postions in a string, then
  displays that string on the screen, providing the info on version to user.
  */
    *(ver_string + vers_offset) = (char) (dcb.device.version + ZER0);
    *(ver_string + rev1_offset) = (char) ((dcb.device.revision / 10) + ZER0);
    *(ver_string + rev2_offset) = (char) ((dcb.device.revision % 10) + ZER0);
    disp_str (&ver_string[0]);
    return;
}



/* -------------------------------------------------------------------------
   Using values from 'p_config.d', initialize the values of the fields of the
   Device Control Block's device status structure.
   ------------------------------------------------------------------------- */
void init_device ()
{
    dcb.device.version = DEV_VERSION;
    dcb.device.revision = DEV_REVISION;
    dcb.device.pad_avail = PAD_TYPE;
    dcb.device.port_numb = 0;
    dcb.device.PVC_channels = CNT_PVC_CHAN;
    dcb.device.INC_channels = CNT_INC_CHAN;
    dcb.device.TWOC_channels = CNT_TWOC_CHAN;
    dcb.device.OUTC_channels = CNT_OUTC_CHAN;
    dcb.device.state = WAIT_RESET;
    return;
}



/* -------------------------------------------------------------------------
   Initialize the port parameters, as per X.PC Driver Interface Specification.
   ------------------------------------------------------------------------- */
void init_port ()
{
    dcb.port.baud = BAUD_1200;
    dcb.port.data_bits = DATA_8BITS;
    dcb.port.parity = NO_PARITY;
    dcb.port.stop_bits = STOP_1BIT;
    dcb.port.xon_xoff = TRUE;
    dcb.port.DTE_port = PORT_DTE;
    return;
}

void zero_dcb () {
 /*
	no process - RNC 24MAR86 7:32pm
 This name came from this mod and has externs (only) for it.
	*/
}
