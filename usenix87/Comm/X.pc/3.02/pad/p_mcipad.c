/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		   MCI Edit/Echo routines
		     filename = p_mcipad.c

   This file contains the edit processor for the MCI PAD connection to X.PC.
   The edit processor does two things:
   1) local echo for the user application;
   2) implements the editing characteristics specified by MCI.  To wit,
      the user is allowed to type backspace, 'line erase', and 'line
      redisplay' characters.  These are specified in QBit packets from the
      MCI PAD, or they may be specified by the application program.
   -------------------------------------------------------------------------
     date   chg#  who   what
   04/02/85  00   curt  Original generation of code.
   07/31/85  01   curt  Completion of MCI echo code
   09/26/86  02   Ed M  Reformat and recomment
   ------------------------------------------------------------------------- */
#include "driver.h"
#include "pad.h"

#define  NULLCHR  0x00          /* initial values.  */
#define  BELL     0x07
#define  CRETURN  0x0D
#define  LINEFEED 0x0A
#define  BACKSPAC 0x08
#define  SPACE    0x20
#define  offclc(offset,index) ((unsigned) (offset) + (unsigned) (index))




/* -------------------------------------------------------------------------
   Initialize the control variables for the edit session, clear buffer out.
   ------------------------------------------------------------------------- */
void edit_init (ptr_2chcb, backsp_chr, linedel_chr, linagin_chr)
CHCB_PTR ptr_2chcb;
char    backsp_chr,
	linedel_chr,
	linagin_chr;
{
    extern void zero_mem();
    extern int  get_ds();

    ptr_2chcb -> tc.editmci_on = TRUE;
    ptr_2chcb -> tc.backspc = backsp_chr;
    ptr_2chcb -> tc.linedel = linedel_chr;
    ptr_2chcb -> tc.linagin = linagin_chr;
    zero_mem (dcb.ed_buff, EDBUF_MAX);
    dcb.ed_pos = 0;
    dcb.ed_active++;
    dcb.ed_DSbuff = (char *) get_ds ();
}





void edit_end (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    ptr_2chcb -> tc.editmci_on = FALSE;
    ptr_2chcb -> tc.backspc = ptr_2chcb -> tc.linedel =
	ptr_2chcb -> tc.linagin = '\0';
    while (dcb.ed_pos > 0)
	edit_writ (ptr_2chcb);
    --dcb.ed_active;
    dcb.ed_pos = 0;
}


/* -------------------------------------------------------------------------
   Take all of the data out of the edit buffer (that can be taken), and put
   into a packet for transmission.  Turn the packet over to packet layer.
   ------------------------------------------------------------------------- */
void edit_writ (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void mvbyt_to(),
		writ_wrpkt();
    extern int  strt_wrpkt(),
		mv_buf2pkt();
    int     cnt = 0;

    while (cnt < dcb.ed_pos) {
	if (ptr_2chcb -> apwr_tot >= DEF_pktlim)
	    writ_wrpkt (ptr_2chcb);
	if (ptr_2chcb -> apwr_que == NULL)
	    if (strt_wrpkt (ptr_2chcb) == FALSE)
		return;         /* no resources, abort       */
	cnt += mv_buf2pkt (ptr_2chcb, dcb.ed_DSbuff,
		offclc (&dcb.ed_buff[0], cnt), (dcb.ed_pos - cnt));
    }
    if (ptr_2chcb -> apwr_tot > 0)
	writ_wrpkt (ptr_2chcb);
    if (cnt < dcb.ed_pos) {
	dcb.ed_pos -= cnt;
	mvbyt_to (dcb.ed_DSbuff, &dcb.ed_buff[0], dcb.ed_buff[cnt], dcb.ed_pos);
    }
    else
	dcb.ed_pos = 0;
}



/* -------------------------------------------------------------------------
   Handle characters being transmitted by USER, under MCI echo/edit rules.
   If just doing echo, just echo characters.  If doing edit also, check for
   one of special characters, process if find, else echo character.
   ------------------------------------------------------------------------- */
int     echo_mci (ptr_2chcb, max_iobuff)
	CHCB_PTR ptr_2chcb;
int     max_iobuff;
{
    extern void rcv_echchr(),
		frwd_timr ();

    char    echar;
    int     pos_iobuff = 0;

    while (pos_iobuff < max_iobuff) {
	echar = dcb.io_buff[pos_iobuff++];
	if (!MCI_EDIT (ptr_2chcb))
	    rcv_echchr (ptr_2chcb, echar);
	else if (echar == ptr_2chcb -> tc.backspc)
	    bk_space (ptr_2chcb);
	else if (echar == ptr_2chcb -> tc.linedel)
	    line_erase (ptr_2chcb);
	else if (echar == ptr_2chcb -> tc.linagin)
	    line_rstrt (ptr_2chcb);
	else if (dcb.ed_pos >= EDBUF_MAX)
	    rcv_echchr (ptr_2chcb, BELL);
	else {
	    dcb.ed_buff[dcb.ed_pos++] = echar;
	    rcv_echchr (ptr_2chcb, echar);
	}
	if ((echar == CRETURN) && (ptr_2chcb -> tc.ech_lfoncr)) {
	    rcv_echchr (ptr_2chcb, LINEFEED);
	    if (ptr_2chcb -> tc.forwrd_chr == LINEFEED)
		edit_writ (ptr_2chcb);
	}
	if (echar == ptr_2chcb -> tc.forwrd_chr)
	    edit_writ (ptr_2chcb);
    }
    return (pos_iobuff);
}

/* -------------------------------------------------------------------------
   If not at left margin, back up/erase/back up to erase preceding character.
   ------------------------------------------------------------------------- */
void bk_space (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void rcv_echchr();

    if (dcb.ed_pos <= 0) {
	dcb.ed_pos = 0;
	rcv_echchr (ptr_2chcb, BELL);
    }
    else {
	--dcb.ed_pos;
	rcv_echchr (ptr_2chcb, BACKSPAC);
	rcv_echchr (ptr_2chcb, SPACE);
	rcv_echchr (ptr_2chcb, BACKSPAC);
    }
}








/* -------------------------------------------------------------------------
   Erase all of current line, and begin entry on the next line of display.
   ------------------------------------------------------------------------- */
void line_erase (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void rcv_echchr();

    dcb.ed_pos = 0;
    rcv_echchr (ptr_2chcb, CRETURN);
    rcv_echchr (ptr_2chcb, LINEFEED);
}









/* -------------------------------------------------------------------------
   rcv_echchr all of current line on the next line of display.
   ------------------------------------------------------------------------- */
void line_rstrt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void rcv_echchr();

    int     i;
    static char next[] = "XXX";
    char   *ptr_nextline;

    ptr_nextline = next;
    while (*ptr_nextline != '\0')
	rcv_echchr (ptr_2chcb, *ptr_nextline++);
    rcv_echchr (ptr_2chcb, CRETURN);
    rcv_echchr (ptr_2chcb, LINEFEED);

    for (i = 0; i < dcb.ed_pos; i++)
	rcv_echchr (ptr_2chcb, dcb.ed_buff[i]);

}




/* -------------------------------------------------------------------------
   Force the MCI pad parameters for the interactive user, or the batch user.
   Appends the packets to the read que, as if were in-bound commands.
   ------------------------------------------------------------------------- */

#define  debugmci  0
#if debugmci
#include <p_qbit.d>

void frcon_mci (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void io_active ();
    extern int  end_time();

    if (GREEN_OUT (ptr_2chcb))
	snd_qbitpkt (ptr_2chcb, QRED_BALL);
    else if (!(RED_OUT (ptr_2chcb)))
	if (TYMDFR_ECHO (ptr_2chcb)) {
	    int_off ();
	    end_time (&io_active, ptr_2chcb);
	    int_on ();
	}
	else
	    if (TYMLCL_ECHO (ptr_2chcb)) {
		snd_qbitpkt (ptr_2chcb, QENT_DEFECH);
		echo_end (ptr_2chcb);
	    }

    ptr_2chcb -> tc.def_echo = ptr_2chcb -> tc.echo_enb = FALSE;
    echo_init (ptr_2chcb);
   /*
   ^x linedel, ^r redisplay
   */
    edit_init (ptr_2chcb, BACKSPAC, 0x18, 0x12);
    ptr_2chcb -> tc.forwrd_chr = CRETURN;
    ptr_2chcb -> tc.echomci_on = TRUE;
}



void frcof_mci (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    edit_end (ptr_2chcb);
    echo_end (ptr_2chcb);
    ptr_2chcb -> tc.echomci_on = FALSE;
}
#else                           /* not debugging mci */
void frcon_mci (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
}

void frcof_mci (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
}
#endif                          /* debugmci */
