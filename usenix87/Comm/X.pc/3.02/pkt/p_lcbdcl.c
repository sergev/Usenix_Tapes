/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
	     Link and Channel Control Block Alloc. and Init. Routines
			      filename = x_lcbdcl.c

   This module defines the storage areas needed for the link control block
   block and any channel control blocks.  The module also contains a routine
   to initialize the link control block and a routine to initialize a channel
   control block, when given a ccb pointer.

   zero_pkt    Initializes the entire packet layer, zeroing all fields, and
	       destroying all channel/timer/link information.

   ccb_init    Accepts a pointer to a ccb, and initializes the data fields.

   ccb_zero    Zeroes Virt Call or Perm Virt Ciruit part of CCB

   get_ccbavl  Returns a pointer to a ccb, if there are any ccbs in the
	       'available ccb' array.  It returns NULL when none are avail.

   rel_ccbavl  Accepts a pointer to a ccb, and returns that ccb to the
	       'available ccb' array.
   -------------------------------------------------------------------------
	Date  Change  By      Reason
      06/04/84 01    curt     Initial generation of code, convert for PAD.
      09/26/86 02    Ed M     Reformat and recomment
   ------------------------------------------------------------------------- */


#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"

/* -------------------------------------------------------------------------
   These statements allocate the storage areas for the LCB, and enough CCB_STR
   entries in array for a character channel ccb & a ccb for all packet chans.
   ------------------------------------------------------------------------- */
#define  TOT_CHANS  (CNT_INC_CHAN+CNT_TWOC_CHAN+CNT_OUTC_CHAN+CNT_PVC_CHAN+1)

LCB_STR lcb;

CCB_STR ccbarr[TOT_CHANS];
int     ccbavl[TOT_CHANS];

static int  ccbi;               /* for private use only in get/rel ccb */

/* -------------------------------------------------------------------------
   This routine initializes fields in the LCB control structure.  It must be
   called before any attempts are made to process X.PC application calls, or,
   before any of the communication oriented (LINK) functions may be invoked.
   Set every field in the entire structure to value 0, set the ccb available
   array to TRUE, and set each ccb array element to initialized values.
   ------------------------------------------------------------------------- */
void zero_pkt ()
{
    extern void zero_mem();

    zero_mem ((char *) & lcb, sizeof (LCB_STR));

    for (ccbi = 0; ccbi < TOT_CHANS; ccbi++) {
	ccbavl[ccbi] = TRUE;
	ccb_init (&ccbarr[ccbi]);
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine sets the fields in a Channel Control Block to default values.
   Use structure for size of control block.  As for setting up protocol values:
   reset protocol control fields to defaults as specified by X.PC document.
   ------------------------------------------------------------------------- */
void ccb_init (accb)
CCB_PTR accb;
{
    extern void zero_mem();

    zero_mem ((char *) accb, sizeof (CCB_STR));

    accb -> early_rep = 2;      /* RR reply after 2 packets recvd */
    accb -> pkt_size = MAX_PKTSZ;
    accb -> dat_window = DAT_WNDWSZ;
    accb -> pkt_window = MAX_WNDWSZ;
    accb -> vrh = accb -> vsh = accb -> pkt_window - 1;
    return;
}



/* -------------------------------------------------------------------------
   This routine accepts a ccb pointer, to zero all ccb fields corresponding to
   a Virtual Call or a Perm Virt Circuit control fields in the CCB structure.
   ------------------------------------------------------------------------- */
void ccb_zero (accb)
CCB_PTR accb;
{
    extern void zero_mem();

    char   *low,
	   *high;

    low = (char *) & accb -> recv_state;
    high = (char *) & accb -> retry;
    zero_mem (low, (high - low));
    return;
}



/* -------------------------------------------------------------------------
   This routine returns a pointer to a ccb array element, or NULL.  It returns
   NULL when there are no more ccb array elements available for use as a ccb.
   ------------------------------------------------------------------------- */
CCB_PTR get_ccbavl ()
{
    CCB_PTR accb;

    accb = NULL;
    for (ccbi = 0; ccbi < TOT_CHANS; ccbi++)
	if (ccbavl[ccbi]) {
	    ccbavl[ccbi] = FALSE;
	    accb = &ccbarr[ccbi];
	    break;
	}
    return (accb);
}



/* -------------------------------------------------------------------------
   This routine accepts a ccb pointer, and returns the ccb array element
   pointed at to the available list by setting the available flag to TRUE.
   ------------------------------------------------------------------------- */
void rel_ccbavl (accb)
CCB_PTR accb;
{
    for (ccbi = 0; ccbi < TOT_CHANS; ccbi++)
	if (&ccbarr[ccbi] == accb)
	    ccbavl[ccbi] = TRUE;
    return;
}
