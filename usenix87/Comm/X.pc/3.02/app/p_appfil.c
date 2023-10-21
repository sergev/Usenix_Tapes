/* -------------------------------------------------------------------------
   This file contains the Input/Output routines as used by the 'P_APPL.EXE'.
		  filename = 'p_applio.c'
   ------------------------------------------------------------------------- */
#include <ctype.h>
#include <stdio.h>
#undef	 NULL			/* avoids warning of redefinition from stdio.h */
#undef	 max			/* avoids warning of redefinition from stdio.h */
#undef	 min			/* avoids warning of redefinition from stdio.h */
#include <p_padfnc.h>		/*  This the only include useful to the PAD invoker. */
#include <app.h>

FILE * infile, *fopen ();



/* -------------------------------------------------------------------------
   Infile is input, mapfile is output, fopen is predeclared function.
   ------------------------------------------------------------------------- */
void file_send (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  sc_beep();
    extern int  yenterd();
    extern int  inchar();
    extern int  key_hit();
    extern void wn_setsz();
    extern void wn_clear();
    extern void wn_dstrg();
    extern void wn_setdf();
    extern void wnd_char();
    extern void get_str();

    char   *ret_line;

    int     length,
            i,
            maxlen,
            fmt,
            wrcnt,
            rdcnt,
            tot_sent,
            done,
            lcl_echo,
            lf_added,
            data_qued,
            event,
           *where,		/* for checkpoint address */
            in_stat,
            out_stat,
            in_cnt;

    sc_clrlin (1, 7, 7);
    cprintf ("TEXT file name (CR:exit) :");
    length = get_str (CR_CHR, &ca[achan].in_arr[0], 30);

    if (length > 0) {		/* open the filename specified */
	ca[achan].in_arr[length] = '\0';
	if ((infile = fopen (&ca[achan].in_arr[0], "r")) == NULL) {
	    cprintf ("\n\rfile not found...");
	    inchar (NO_SHOW);
	    sc_clrlin (1, 7, 23);
	    return;
	}			/* if file not found */
	else {
	    lcl_echo = yenterd ("   local echo");
	    lf_added = yenterd ("   add LFs");
	    wn_setsz (1, 8, 80, 23);
	    wn_clear ();
	    done = FALSE;
	}			/* else file found */
    }				/* if length > 0 */
    else {
	sc_clrlin (1, 7, 23);
	return;
    }				/* else length = 0 */
    while ((done == FALSE)	/* until EOF read or keystroke, send */
	    &&((ret_line = fgets (&ca[achan].in_arr[0], 130, infile)) != NULL)) {
	tot_sent = 0;
	cnv_tabs (&ca[achan].in_arr[0], &ca[achan].out_arr[0], 130);/* expand */
	length = 0;
	while ((length < 130) && (ca[achan].out_arr[length] != LF_CHR))
	    ++length;
	ca[achan].out_arr[length++] = CR_CHR;/* send carriage return too */
	if (lf_added)
	    ca[achan].out_arr[length++] = LF_CHR;
	while (tot_sent < length) {
	    get_iostat (achan);
	    if (ca[achan].xmit_status == TRUE) {
		fmt = wrcnt = 0;
		maxlen = length - tot_sent;
		setp_xpc (achan, (int *) & ca[achan].out_arr[tot_sent],
			&maxlen, &wrcnt, &fmt);
		call_xpc (achan, WRITE_DATA);
		if (lcl_echo)	/* display what is sent */
		    for (i = tot_sent; i < (wrcnt + tot_sent); i++)
			wnd_char (ca[achan].out_arr[i]);
		tot_sent = tot_sent + wrcnt;
	    }			/* if xmit status */

	    get_iostat (achan);
	    if (ca[achan].recv_status) {
		maxlen = INARA_LN - (MAX_PKTSZ + 1);
		setp_xpc (achan, (int *) & ca[achan].in_arr[0], &maxlen, &rdcnt, &fmt);
		call_xpc (achan, READ_DATA);
		if (REQST_DONE (achan))
		    if (lcl_echo != TRUE)/* display what is recvd */
			for (i = 0; i < rdcnt; i++)
			    wnd_char (ca[achan].in_arr[i]);
	    }			/* receive status */

	    if (key_hit ()) {
		done = TRUE;
		break;
	    }
	    else {
		call_state (achan, NO_SHOW, NO_WAIT);
		if (ca[achan].call_state != CONNECTED) {
		    wn_dstrg ("\n\rCall lost...");
		    goto LOST_CALL;
		}		/* if connected */
	    }			/* else key not hit */
	}			/* while this line not sent out */
    }				/* while sending file loop */
    data_qued = TRUE;
    if (achan == SYS_CH) {	/* by definition character mode */
	setp_xpc (achan, &in_cnt, &in_stat, &out_stat, 0);
	call_xpc (achan, REP_IOSTAT);
	if (out_stat == 0)
	    data_qued = FALSE;
    }				/* if sys chan (0) */
    else {			/* packet channel 1-F */
	event = CHECKPOINT;	/* send Yellow Ball asks empty buff? */
	where = &data_qued;	/* recv Orange Ball when buff empty! */
	setp_xpc (achan, &event, &where, 0, 0);
	call_xpc (achan, SET_UPDT);
    }				/* else data chan (>0) */

    wn_dstrg ("\n\rwaiting for all data sent...");
    while (data_qued) {		/* packet channel 1-F */
	setp_xpc (achan, &in_cnt, &in_stat, &out_stat, 0);
	call_xpc (achan, REP_IOSTAT);
	if (out_stat == 0)
	    data_qued = FALSE;

	if (in_cnt > 0) {	/* if any input, read it */
	    maxlen = LINE_LEN - 1;
	    setp_xpc (achan, (int *) & ca[achan].in_arr[0], &maxlen, &rdcnt, &fmt);
	    call_xpc (achan, READ_DATA);
	    if (REQST_DONE (achan))
		if (!lcl_echo)
		    for (maxlen = 0; maxlen < rdcnt; maxlen++)
			wnd_char (ca[achan].in_arr[maxlen]);
	}			/* receive status */

	if (key_hit ())
	    break;
	else {
	    call_state (achan, NO_SHOW, NO_WAIT);
	    if (ca[achan].call_state != CONNECTED) {
		wn_dstrg ("\n\rCall lost...");
		goto LOST_CALL;
	    }
	}			/* else not key hit */
    }				/* while data_qued */

    if (achan != SYS_CH) {
	event = CHECKPOINT;	/* send Yellow Ball asks empty buff? */
	setp_xpc (achan, &event, 0, 0, 0);
	call_xpc (achan, RESET_UPDT);
    }

    sc_beep ();
    wn_dstrg ("\n\rfile sent");

LOST_CALL: ;
    fclose (infile);
    inchar (NO_SHOW);
    wn_clear ();
    wn_setdf ();
    return;
}				/* file_send */



/*---------------------------------------------------------------
  This function converts a line with tabs to a line with spaces.
----------------------------------------------------------------*/
cnv_tabs (in_line, out_line, max_cnv_cnt)
char    in_line[],
       *out_line;
int     max_cnv_cnt;		/* max space available in out_line */
{
    int     tabs,
            in_pos,
            out_cnt = 0;

    for (in_pos = 0; out_cnt <= max_cnv_cnt; ++in_pos)
	if (in_line[in_pos] == '\t')
	/* how_many returns the number of spaces to replace a tab with. */
	    for (tabs = how_many (out_cnt); tabs > 0; --tabs)
		*(out_line + (out_cnt++)) = ' ';
	else			/* after the end of line is a null character, stop search for tabs.   */
	/* Remember that a NULL is treated as a FALSE, if NOT null, continue. */
	    if (!(*(out_line + (out_cnt++)) = in_line[in_pos]))
		break;
    return;
}				/* cnv_tabs function */



int     stf_chan (achan)
int     achan;
{
#define  BELL 0x07
#define  BLANK ' '
    extern int  sc_clrlin();
    extern int  get_int();
    extern int  yenterd();
    extern int  key_hit();
    extern void wn_setsz();
    extern void wn_clear();
    extern void wn_setdf();
    extern void wnd_char();
    extern void sh_chanstat();

    int     endstuf,
            stflen,
            maxlen,
            rdcnt,
            wrcnt,
            fmt,
            i,
            j,
            del_count,
            serial,
            line_cnt,
            input_cnt;
    char    expect;
    sc_clrlin (1, 7, 23);
    cprintf ("continuous output (while receiving) of the following data:");
    sc_clrlin (1, 8, 8);
 /* stflen = get_str (EOT_CHR, &ca[achan].out_arr[0], STUF_LEN); */
    stflen = port.DTE_port ? 79 : 81;/* 79 or 81 chars */
    j = expect = BLANK;		/* first char should be blank */
    for (i = 0; i < stflen; i++, j++) {
	ca[achan].out_arr[i] = j;
	cprintf ("%c", j);
    }				/* for */
    input_cnt = 0;
    endstuf = FALSE;
    del_count = get_int (1, 10, "delay between writes", 0, 25, 2);
    serial = yenterd ("   serialize lines");
    line_cnt = 0;
    wn_setsz (1, 9, 80, 23);
    wn_clear ();
    while (!endstuf) {
	get_iostat (achan);
	if (ca[achan].xmit_status) {
	    wrcnt = fmt = 0;
	    if (serial) {
		ca[achan].out_arr[0] = BLANK;
		ca[achan].out_arr[1] = (line_cnt / 10) + '0';
		ca[achan].out_arr[2] = (line_cnt % 10) + '0';/* mod 10 */
		ca[achan].out_arr[3] = ':';
		if (++line_cnt > 99)
		    line_cnt = 0;
	    }			/* if serial */
	    setp_xpc (achan, (int *) & ca[achan].out_arr[0], &stflen, &wrcnt, &fmt);
	    call_xpc (achan, WRITE_DATA);
	    if (!REQST_DONE (achan))
		sh_chanstat (achan);
	}			/* if smit status */
    /* 3-time loop removed here, to let output run as fast as input */
	get_iostat (achan);
	if (ca[achan].recv_status) {
	    maxlen = LINE_LEN - 1;
	    setp_xpc (achan, (int *) & ca[achan].in_arr[0], &maxlen, &rdcnt, &fmt);
	    call_xpc (achan, READ_DATA);
	    if (REQST_DONE (achan))
		for (j = 0; j < rdcnt; j++) {
		    i = ca[achan].in_arr[j];
		    if (i != expect) {/* accept blank in col 80 or 82 */
			if (input_cnt >= 79 && i == BLANK) {
			    input_cnt = 0;/* start new line */
			    expect = BLANK;
			}	/* if blank in col 80 or 82 */
			else {	/* not col 80 or 82 with blank */
			    if (input_cnt < 4) {/* allow seq numb [nn] */
				if (i == ':')
				    expect = '#';
			    }	/* else if */
			    else {/* not seq numb [nn] */
				wnd_char (BELL);
				expect = i;/* accept next char */
			    }	/* else not seq numb [nn] */
			}	/* else not col 80 or 82 with blank */
		    }		/* if not expected char */
		    expect++;
		    input_cnt++;
		    wnd_char (i);
		}		/* for # char recvd */
	    else
		sh_chanstat (achan);
	}			/* if recv status */
    /* else break;    * 3-time loop removed, so delete this * * 3-time loop removed */
	if (key_hit ())
	    endstuf = TRUE;
	else {
	    call_state (achan, NO_SHOW, NO_WAIT);
	    if (ca[achan].call_state != CONNECTED)
		endstuf = TRUE;
	    else
		for (i = 1; i < del_count; i++)
		    for (j = 0; j < 1000; j++);
	}			/* else not key hit */
    }				/* while not endstuf */
    wn_clear ();
    wn_setdf ();
    return;
}				/* stf_chan */
