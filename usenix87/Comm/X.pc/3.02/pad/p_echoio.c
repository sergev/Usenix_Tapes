/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		     PAD Echo   Input/Output Logic routines
				 filename = p_echio.c

   The routines in this file implement the VTPAD echoing logic for X.PC.
   The 'send' routine is called by the Driver to process a buffer of data as
   received from the application during a 'write' when TYMLCL_ECHO is TRUE.
   The routine will place the characters into the output que, and may also
   place some or all of the characters in the input que.

   echo_init	accepts a chan ^, sets up echo flags/que/buf for that channel
		to do either MCI or TYMNET local echoing.

   echo_end	accepts a chan ^, disables local echo capabilities.

   tym_echo	accepts a chan ^, uses buffer for IO that is controlled through
		the 'device' structure.  The routine sends what it can, and
		may generate multiple packets for the single buffer of output.
		Upon return, IO control variables indicate amount of data sent.

   rcv_echchr	accepts a chan ^, and a character.  Decides where to append the
		character onto the channels read que (aprd_que/inque/packetlyr).
		Routine is called by 'snd_echo' to handle characters that need
		need to be echoed back to the application, and it calls inq_apnd
		or rdq_apnd depending upon where the echoed character belongs.

   pad_param	accepts a chan ^, and a flag indicating whether the PAD param
		should be reported or set for the caller.  Most of the PAD
		parameters are echo controlling values.

   frwd_timr	accepts a chan ^, is run on a timer.  The routine will output
		any data that is queued for output, but not yet sent.  Started
		by routines when finished processing input, with forward timer.
   -------------------------------------------------------------------------
	Date	Change	By	Reason
      05/17/84	00	curt	Initial Generation of code.
      05/31/84	01	curt	Completed first phase of echoing logic.
      08/02/84	02	curt	What a bunch of crap the first version was!!!
      10/08/84	03	curt	Moved 'pad_param' routine in from the driver,
				added forward-timer processing.
      04/03/85	04	curt	Made forward timer processor generic function.
      05/07/85	05	curt	Rewrite to incorporate MCI echo, perm resources.
      07/31/85	06	curt	Completion of MCI echo code
      09/26/86	07	Ed M	Reformat and recomment
   ------------------------------------------------------------------------- */
#include "driver.h"
#include "pkt.h"
#include "pad.h"

extern  io_active ();

/* -------------------------------------------------------------------------
   These define some names for the special characters used in processing.
   ------------------------------------------------------------------------- */
#define	BSPAC	0x08
#define	HT	0x09
#define	LF	0x0A
#define	CR	0x0D
#define	ESC	0x1B
#define	ALTMD	0x7D	/* Right Brace }, 0x7D, is same char. */
#define	TILDE	0x7E
#define	RUB	0x7F

/* -------------------------------------------------------------------------
   This routine is declared first so that the following routines will know
   its address when they call 'set_time' and 'end_time'.  The routine forces
   output of any queued characters held, because a forward timout is active.
   ------------------------------------------------------------------------- */
void frwd_timr (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void writ_wrpkt(),
		edit_writ();
    extern int  set_time();

    if (ptr_2chcb -> tc.forwrd_tim > 0) {
	if (MCI_EDIT (ptr_2chcb))
	    edit_writ (ptr_2chcb);
	if (ptr_2chcb -> apwr_tot > 0)
	    writ_wrpkt (ptr_2chcb);

	int_off ();
	set_time (ptr_2chcb -> tc.forwrd_tim, &frwd_timr, ptr_2chcb);
	int_on ();
    }
    return;
}


/* -------------------------------------------------------------------------
   Setup for echoing.  Get Permanent Echo que/buf set up and available.
   initialize edit control chars.
   ------------------------------------------------------------------------- */
void echo_init (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern QUE_PTR get_que();

    int_off ();
    ptr_2chcb -> perm_echque = get_que (YES_BUFF);
    int_on ();
    ptr_2chcb -> curr_echque = NULL;
    ptr_2chcb -> curr_echbuf = NULL;
}



/* -------------------------------------------------------------------------
   Echo ending, release permanent echo que/buf.  Since caused by receipt of
   q-bit packet, was first packet in input que.  Any following may be echoed
   characters, echoed after receipt of q-bit echo ender.  If permanent echo
   que/buf in use for echoed chars, let read logic release perm when emptied.
   ------------------------------------------------------------------------- */
void echo_end (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void rel_que();
    int     in_input = FALSE;
    QUE_PTR curr_que;

    if (ptr_2chcb -> perm_echque != NULL) {
	if ((in_input = (ptr_2chcb -> aprd_que == ptr_2chcb -> perm_echque)) == FALSE) {
	    curr_que = ptr_2chcb -> in_que.first;
	    while (!in_input && curr_que) {
		if (curr_que == ptr_2chcb -> perm_echque)
		    in_input = TRUE;
		curr_que = curr_que -> qnext;
	    }
	}

	if (!in_input) {
	    int_off ();
	    rel_que (ptr_2chcb -> perm_echque, YES_BUFF);
	    int_on ();
	    ptr_2chcb -> perm_echque = NULL;
	}
    }
    ptr_2chcb -> curr_echque = NULL;
    ptr_2chcb -> curr_echbuf = NULL;
}



/* -------------------------------------------------------------------------
   This routine needs the buffer of data (NORMAL DATA ONLY) to be set up by
   the caller, for a 'pad_write' function call when echoing is enabled.  The
   caller must move the application buffer into the common 'dcb.io_buff'
   buffer, and pass in the control values.  This routine moves bytes of data
   out of the buffer until:
      1) all bytes have been moved (pos_iobuff will == max_iobuff);
      2) No que or bufferlet entries are available... major error condition.
   ------------------------------------------------------------------------- */
void echo_tym (ptr_2chcb, max_iobuff)
CHCB_PTR ptr_2chcb;
int	 max_iobuff;
{
    extern void snd_qbitpkt(),
		writ_wrpkt();
    extern int  strt_wrpkt(),
		next_wrbuf();

    int	    sendchar,	/* actual character to send, as found in buffer.     */
	    workchar,	/* character with parity set as needed to work with. */
	    send_EDM;	/* May need to leave local echo, this will do it.    */
    int     pos_iobuff = 0;

    while (pos_iobuff < max_iobuff) {
	/*
	 Need two versions of char: one to send as found, the other to work
	 with.
	 */
	sendchar = dcb.io_buff[pos_iobuff++];
	if (ptr_2chcb -> tc.parity_enb == TRUE)
	    workchar = sendchar & 0x7F;
	else
	    workchar = sendchar;
	/*
	 If still in local echo, decision depends upon the character we input.
	 */
	if (TYMLCL_ECHO (ptr_2chcb)) {
	    send_EDM = FALSE;
	    switch (workchar) {
		case RUB:
		case ALTMD:	/* also "}" RTCRL character */
		case TILDE:
		    send_EDM = TRUE;
		    break;
		case ESC:
		    if (ptr_2chcb -> tc.ech_escp == TRUE)
			rcv_echchr (ptr_2chcb, workchar);
		    else
			send_EDM = TRUE;
		    break;
		case BSPAC:
		    if (ptr_2chcb -> tc.ech_ctlh == TRUE)
			rcv_echchr (ptr_2chcb, workchar);
		    else
			send_EDM = TRUE;
		    break;
		case CR:
		    rcv_echchr (ptr_2chcb, workchar);
		    if (ptr_2chcb -> tc.ech_lfoncr == TRUE)
			rcv_echchr (ptr_2chcb, LF);
		    break;
		case LF:
		    rcv_echchr (ptr_2chcb, workchar);
		    if (ptr_2chcb -> tc.ech_cronlf == TRUE) {
			rcv_echchr (ptr_2chcb, CR);
			rcv_echchr (ptr_2chcb, RUB);
		    }
		    break;
		case HT:
		    if (ptr_2chcb -> tc.ech_ctli == TRUE)
			rcv_echchr (ptr_2chcb, workchar);
		    else
			send_EDM = TRUE;
		    break;
		default:
		    /*
		     Handling of all others depends upon
		     printable/non-printable.  When get normal char, echo
		     locally.  When get a non-printable, EDEM.
		     */
		    if (workchar >= 0x20)
			rcv_echchr (ptr_2chcb, workchar);
		    else
			send_EDM = TRUE;
		    break;
	    }
	    if (send_EDM) {
		snd_qbitpkt (ptr_2chcb, QENT_DEFECH);
		ptr_2chcb -> tc.def_echo = TRUE;
	    }
	}

    /*
     In all cases, enque for output!!!  Some cases may have sent Q-bit packets
     ahead of the char. Some may have echoed, some may have not echoed.
     If the currently usable packet has reached the maximum data size, send it.
     If there is not packet currently being built, start a fresh one.
     If the current bufferlet fills up, chain a new bufferlet to the end.
     */
	if (ptr_2chcb -> apwr_tot >= DEF_pktlim)
	    writ_wrpkt (ptr_2chcb);

	if (ptr_2chcb -> apwr_que == NULL)
	    if (strt_wrpkt (ptr_2chcb) == FALSE)
		break;		/* no resources, abort! exit while loop */

	if (ptr_2chcb -> apwr_buf -> num_bytes >= BFLT_DATA_SIZE)
	    if (next_wrbuf (ptr_2chcb) == FALSE)
		break;	/* no resources, abort! exit while loop */

	ptr_2chcb -> apwr_buf -> data[ptr_2chcb -> apwr_buf -> num_bytes++] = sendchar;
	ptr_2chcb -> apwr_tot++;
	if ((ptr_2chcb -> tc.forwrd_chr == workchar) || (ptr_2chcb -> tc.forwrd_chr == '\0'))
	    writ_wrpkt (ptr_2chcb);
    }

 /*
  If no longer local echo, flush out remains of buffer to the output que.
  Don't flush out remains if the user has a forwarding timeout.
  */
    if ((!TYMLCL_ECHO (ptr_2chcb)) && (ptr_2chcb -> tc.forwrd_tim == 0))
	if (ptr_2chcb -> apwr_tot > 0)
	    writ_wrpkt (ptr_2chcb);
    return (pos_iobuff);
}

/* -------------------------------------------------------------------------
   This routine will echo a character back into the user's input que.  The
   character goes onto the 'curr_echque' entry for the chan (current packet
   echoed into).  IF there is not 'current' entry a packet will be created,
   using 'perm_echque/buf' rather than get one from pool if possible.

   - If the aprd_que is not NULL, and the in_que is empty, the current que/buf
       is the only packet: add the char to the end of the aprd_que/buf.
   - If the aprd_que is NULL, and the in_que is empty, create a 'packet' for
       the char, enque onto the in_que list (there are no packets preceding).
   - If the aprd_que is not NULL, and the in_que is NOT empty, add the char
       to the end of the last packet in the in_que, or new last packet. This
       also covers: If the aprd_que is NULL, and the in_que is NOT empty.
       NOTE that in this case curr_ech and aprd_que are no longer equal.
      ---------------------------------------------------------------------- */
void rcv_echchr (ptr_2chcb, echchar)
CHCB_PTR ptr_2chcb;
int	 echchar;
{
    extern BUF_PTR get_buff();
    extern QUE_PTR get_que();

    if (ptr_2chcb -> curr_echque == NULL) {
	if (ptr_2chcb -> in_que.ent_cnt == 0) {
	    ptr_2chcb -> curr_echque = ptr_2chcb -> aprd_que;
	}
	else
	    ptr_2chcb -> curr_echque = ptr_2chcb -> in_que.last;

	if (ptr_2chcb -> curr_echque != NULL) {
	    ptr_2chcb -> curr_echbuf = ptr_2chcb -> curr_echque -> u.cmd.bfirst;
	    while (ptr_2chcb -> curr_echbuf -> bnext != NULL)
		ptr_2chcb -> curr_echbuf = ptr_2chcb -> curr_echbuf -> bnext;
	}
    }

    if ((ptr_2chcb -> curr_echque == NULL) ||
	    (ptr_2chcb -> curr_echque -> DATLEN_FLD >= DEF_pktlim)) {
	/*
	 If have full packet, need new next: permanent is already in use. If no
	 current and permanent, use permanent que/buf for echo.  If no current
	 and no permanent, need new next:  no packets queued.
	 */
	if ((ptr_2chcb -> curr_echque == NULL) && (ptr_2chcb -> perm_echque != NULL))
	    ptr_2chcb -> curr_echque = ptr_2chcb -> perm_echque;
	else {
	    int_off ();
	    ptr_2chcb -> curr_echque = get_que (YES_BUFF);
	    int_on ();
	    if (ptr_2chcb -> curr_echque == NULL)
		return;		/* abandon all hope. */
	    if (ptr_2chcb -> perm_echque == NULL)
		ptr_2chcb -> perm_echque = ptr_2chcb -> curr_echque;
	}

	ptr_2chcb -> curr_echbuf = ptr_2chcb -> curr_echque -> u.cmd.bfirst;
	ptr_2chcb -> curr_echbuf -> num_bytes = B_BYTE0;
	ptr_2chcb -> curr_echque -> u.cmd.p_gfilci = ptr_2chcb -> pkt_chan;
	ptr_2chcb -> curr_echque -> u.cmd.p_id = NORMAL_DATA;
    }

    if (ptr_2chcb -> curr_echbuf -> num_bytes >= BFLT_DATA_SIZE) {
	int_off ();
	ptr_2chcb -> curr_echbuf -> bnext = get_buff ();
	int_on ();
	if (ptr_2chcb -> curr_echbuf -> bnext == NULL)
	    return;
	else {
	    ptr_2chcb -> curr_echbuf = ptr_2chcb -> curr_echbuf -> bnext;
	    ptr_2chcb -> curr_echbuf -> num_bytes = 0;
	}
    }

     /*
      To get here either had a buffer available, or created one.  Store echoed.
      */
    ptr_2chcb -> curr_echbuf -> data[ptr_2chcb -> curr_echbuf -> num_bytes++] = echchar;
    ptr_2chcb -> in_bytecnt++;
    return;
}


/* -------------------------------------------------------------------------
   This routine accepts a flag that indicates what to do with the pad parameter
   in the 'param1' location of the callers' area.  If 'set_rep' is equal to
   REP, the current value of that parameter is returned to the 'param2'
   location.  If 'set_rep' is equal to SET, the value found at the 'param2'
   location is assigned to the pad parameter indicated.  The define below sets
   the field, or reports the field depending on 'set_rep' parameter (leave off
   the semicolon on the define so that use looks like normal C line of code).
   ------------------------------------------------------------------------- */
void pad_param (ptr_2chcb, set_rep)
CHCB_PTR ptr_2chcb;
int	 set_rep;
{
    extern int ret_code;	/* declared in p_driver.c */

#define	REP 1			/* as found in p_driver.c call to routine.*/
#define	SET 2
#define	doit(fld) if (set_rep==SET) fld=new_value; else new_value=fld

    extern void mvwrd_to(),
		snd_qbitpkt(),
		ball_timr();
    extern int  gtwrd_fr(),
		end_time();
    int     new_value,
	    which_prm;

    if (dcb.device.state == WAIT_RESET)
	ret_code = ILL_WAITRQ;
    else if (dcb.device.state == CHARACTER)
	ret_code = ILL_CHARRQ;
    else if (ptr_2chcb -> call_state != CONNECTED)
	ret_code = ILL_CALSTAT;

    if (ret_code == FUNC_COMPLETE) {
	which_prm = gtwrd_fr (params.param_seg, params.param1);
	if (set_rep == SET) {
	    new_value = gtwrd_fr (params.param_seg, params.param2);
	    if ((which_prm >= CTRL_I_ECHO) && (which_prm <= FRWRD_TIME))
		if (ptr_2chcb -> tc.echo_ok == FALSE)
		    which_prm = 99;
	}
	/*
	 Now got some preliminary testing done, take care of set/report
	 functions.  The ECHO_ENABLE function requires special processing if
	 echo is currently enabled and in effect, and the application is
	 turning it off.
	 */
	switch (which_prm) {
	    case ECHO_ENABLE:
		if (set_rep == SET) {
		    if ((ptr_2chcb -> tc.echo_ok == YES) && (new_value == NO)) {
			int_off ();
			end_time (&ball_timr, ptr_2chcb);
			end_time (&io_active, ptr_2chcb);
			int_on ();
			if (TYMLCL_ECHO (ptr_2chcb))
			    snd_qbitpkt (ptr_2chcb, QENT_DEFECH);
			ptr_2chcb -> tc.def_echo = ptr_2chcb -> tc.echo_enb = FALSE;
		    }
		}
		doit (ptr_2chcb -> tc.echo_ok);
		break;
	    case CTRL_I_ECHO:
		doit (ptr_2chcb -> tc.ech_ctli);
		break;
	    case CTRL_H_ECHO:
		doit (ptr_2chcb -> tc.ech_ctlh);
		break;
	    case ESC_ECHO:
		doit (ptr_2chcb -> tc.ech_escp);
		break;
	    case LFONCR_ECHO:
		doit (ptr_2chcb -> tc.ech_lfoncr);
		break;
	    case CRONLF_ECHO:
		doit (ptr_2chcb -> tc.ech_cronlf);
		break;
	    case FRWRD_CHAR:
		doit (ptr_2chcb -> tc.forwrd_chr);
		break;
	    case FRWRD_TIME:
		if ((set_rep == SET) && (ptr_2chcb -> tc.forwrd_tim > 0)) {
		    int_off ();
		    end_time (&frwd_timr, ptr_2chcb);
		    int_on ();
		}
		doit (ptr_2chcb -> tc.forwrd_tim);
		break;
	    case PARITY_ENABLE:
		doit (ptr_2chcb -> tc.parity_enb);
		break;
	    default:
		ret_code = ILL_PADPARAM;
		break;
	}
	if ((ret_code == FUNC_COMPLETE) && (set_rep == REP))
	    mvwrd_to (params.param_seg, params.param2, new_value);
    }
    return;
}
