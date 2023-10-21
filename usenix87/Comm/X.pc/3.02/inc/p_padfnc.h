/* -------------------------------------------------------------------------
		X.PC Driver/PAD Function Call Constants
			 file name = p_xpcfnc.d
   This file contains defines for use by the linkage module or application
   program that wants to ask the driver to do some function.  The FUNCTION
   CODE values are use with the function code parameter, the RETURN CODE
   values will be set into the status parameter.

		     FUNCTION CODE FIELD VALUES:
   ------------------------------------------------------------------------- */
#define  CLEAR_DEV    0 	 /* Clear the device indicated. 	     */
#define  READ_DEVST   1 	 /* Read the status of the device indicated. */
#define  RESET_DEV    2 	 /* Reset the device indicated. 	     */
#define  SET_CHRMOD   3 	 /* Set the device indicated to char. mode.  */
#define  SET_PKTMOD   4 	 /* Set the device indicated to pkt. mode.   */
#define  SET_PORTPRM  5 	 /* Set the parameters of the port indicated.*/
#define  REP_PORTPRM  6 	 /* Read the parameters of the port indicated*/
#define  SET_DTR_ON   7 	 /* Set the DTR signal ON.		     */
#define  RESET_DTR    8 	 /* Set the DTR signal OFF.		     */
#define  SET_RTS_ON   9 	 /* Set the RTS signal ON.		     */
#define  RESET_RTS   10 	 /* Set the RTS signal OFF.		     */
#define  TEST_DSR    11 	 /* Report the DSR signal: ON or OFF.	     */
#define  TEST_DCD    12 	 /* Report the Carrier Signal: ON or OFF.    */
#define  TEST_RING   13 	 /* Report the Ring Indicator: ON or OFF.    */
#define  SEND_BREAK  14 	 /* Send the appropriate BREAK for channel.  */
#define  REP_IOSTAT  15 	 /* Report the I/O status for the channel.   */
#define  READ_DATA   16 	 /* Read data from the channel indicated.    */
#define  WRITE_DATA  17 	 /* Write data to the channel indicated.     */
#define  REP_CHNSTAT 18 	 /* Report the 'call' status of the channel. */
#define  CALLRQ_SND  19 	 /* Send a call request, enter pending accpt.*/
#define  CALLCL_SND  20 	 /* Send a call clear, enter pending confirm.*/
#define  SET_CALLANS 21 	 /* Set call answer, wait for call request.  */
#define  CALLAC_SND  22 	 /* Send a call accept, enter connected.     */
#define  READ_CALDAT 23 	 /* Read the data associated with call req.  */
#define  SET_INTRP   24 	 /* Enable invocation of appl. code on event.*/
#define  RESET_INTRP 25 	 /* Disable invocation of appl. code on event*/
#define  SET_UPDT    26 	 /* Enable auto-update of flag on event.     */
#define  RESET_UPDT  27 	 /* Disable auto-update of flag on event.    */
#define  REP_PADPARM 28 	 /* Read the PAD parameter indicated, return.*/
#define  SET_PADPARM 29 	 /* Set the PAD parameter indicated to value.*/
#define  INPUT_FLUSH 30 	 /* throws away all un-read input on a chan. */
#define  OUTPT_FLUSH 31 	 /* throws away all un-written output on chan*/
/*efine  REP_LNKSTAT 32 	  * report link stats (# errors, chars, etc.)*/

/* -------------------------------------------------------------------------
			  DEVICE STATES:
   ------------------------------------------------------------------------- */
#define  WAIT_RESET	1	 /* Device is in the 'wait reset' state.     */
#define  CHARACTER	2	 /* Device is in character state.	     */
#define  PACKET 	3	 /* Device is in packet state.		     */

/* -------------------------------------------------------------------------
		       PACKET CHANNEL STATES:
   ------------------------------------------------------------------------- */
#define  DISCONNECT	0	 /* Channel is disconnected.		     */
#define  CALLACC_PND	1	 /* Channel is pending call accept packet.   */
#define  CALL_ACCEPT	2	 /* Channel has received call accept packet. */
#define  CONNECTED	3	 /* Channel is connected.		     */
#define  CLEARCNF_PND	4	 /* Channel is pending clear confirm packet. */
#define  INCOMCAL_PND	5	 /* Channel is pending an incoming call pkt. */
#define  INCOME_CALL	6	 /* Channel has received incoming call pkt.  */
#define  ACCEPT_PND	7	 /* Channel is pending acceptance of call.   */
#define  CALL_CLEARED	8	 /* Channel call was cleared by remote.      */


/* -------------------------------------------------------------------------
		      CALL CLEARING CAUSE CODES:
   ------------------------------------------------------------------------- */
#define  CLR_NORMAL	0	 /* normal end of session, no errors.	     */
#define  CLR_NO_PORTS	1	 /* no ports available for login.	     */
#define  CLR_UNAVAIL	2	 /* host unavailable.			     */
#define  CLR_NO_ACCESS	3	 /* no access allowed to host.		     */
#define  CLR_NETUNAVAIL 4	 /* network unavailable.		     */
#define  CLR_FORM_ERR	5	 /* format error in call request data.	     */
#define  CLR_NAME_ERR	6	 /* username is in error in call request.    */
#define  CLR_ADDR_ERR	7	 /* address is in error in call request.     */
#define  CLR_PSWD_ERR	8	 /* password is in error in call request.    */
#define  CLR_TIMEOUT	9	 /* timeout waiting for clear confirm.	     */
#define  CLR_RESET     10	 /* packet level reset of a channel	     */
#define  CLR_RESTART   11	 /* packet level restart, all chans lost     */
#define  CLR_CRESTART  12	 /* packet level restart, return to char mode*/
#define  CLR_PKTLOST   13	 /* packet level timeout, remote XPC lost    */
#define  CLR_MDMLOST   14	 /* packet level modem, DSR or CD lost	     */


/* -------------------------------------------------------------------------
		       EVENT CODE VALUES:
   ------------------------------------------------------------------------- */
#define  TIMER_DONE	1	 /* update field when timer expires.	     */
#define  MODEM_CHG	2	 /* update field when modem signals change.  */
#define  CHECKPOINT	3	 /* update field when no output data queued. */

#define  EVENT_PSEC	6	 /* for timer, multiply # seconds times this.*/


/* -------------------------------------------------------------------------
		PAD PARAMETER SETTING VALUES:
   ------------------------------------------------------------------------- */
#define  NO		0	 /* use to turn OFF Boolean parameters.      */
#define  YES		1	 /* use to turn ON Boolean parameters.	     */

#define  ECHO_ENABLE	1	 /* YES, echo processing takes place.	     */
#define  CTRL_I_ECHO	2	 /* If ECHO_ENABLE and YES, echo ^I locally. */
#define  CTRL_H_ECHO	3	 /* If ECHO_ENABLE and YES, echo ^H locally. */
#define  ESC_ECHO	4	 /* If ECHO_ENABLE and YES, echo ESC locally.*/
#define  LFONCR_ECHO	5	 /* If ECHO_ENABLE and YES, free LF for CR.  */
#define  CRONLF_ECHO	6	 /* If ECHO_ENABLE and YES, free CR for LF.  */
#define  FRWRD_CHAR	7	 /* If ECHO_ENABLE, send held chars on char. */
#define  FRWRD_TIME	8	 /* if ECHO_ENABLE, will send if chars held. */
#define  PARITY_ENABLE 16	 /* If YES, treat data as 7 bits, I/O 8-bit. */


/* -------------------------------------------------------------------------
		     PORT PARAMETER SETTING VALUES:
   ------------------------------------------------------------------------- */
#define  BAUD_9600	7	    /* BAUD RATE SETTINGS...		     */
#define  BAUD_4800	6
#define  BAUD_2400	5
#define  BAUD_1200	4
#define  BAUD_600	3
#define  BAUD_300	2
#define  BAUD_150	1
#define  BAUD_110	0

#define  NO_PARITY	0	    /* PARITY SETTINGS...		     */
#define  ODD_PARITY	1
#define  EVEN_PARITY	2
#define  MARK_PARITY	3
#define  SPAC_PARITY	4

#define  DATA_5BITS	5	    /* NUMBER OF DATA BITS SETTINGS...	     */
#define  DATA_6BITS	6
#define  DATA_7BITS	7
#define  DATA_8BITS	8
#define  STOP_1BIT	0	    /* NUMBER OF STOP BITS SETTINGS...	     */
#define  STOP_15BIT	1
#define  STOP_2BIT	2

#define  PORT_DCE	0	    /* DXE SETTINGS...			     */
#define  PORT_DTE	1

/* -------------------------------------------------------------------------
		 RETURN CODE (STATUS) FIELD VALUES:
   ------------------------------------------------------------------------- */
#define  FUNC_COMPLETE	 0	 /* Function completed successfully, no error*/
#define  ILL_FUNCTION	 1	 /* Function code not recognized, no action. */
#define  ILL_DEVICE	 2	 /* Device Number of request not valid.      */
#define  ILL_CHANNEL	 3	 /* Channel Number of request not valid.     */
#define  ILL_WAITRQ	 4	 /* Illegal request, given wait reset state. */
#define  ILL_CHARRQ	 5	 /* Illegal request, given char. mode state. */
#define  ILL_PACKRQ	 6	 /* Illegal request, given packet mode state.*/
#define  ILL_PORTNUM	 7	 /* Port Address of request not valid.	     */
#define  ILL_BAUD	 8	 /* Baud rate of request not valid.	     */
#define  ILL_PARITY	 9	 /* Parity of request not valid.	     */
#define  ILL_BTDATA	10	 /* # of data bits of request not valid.     */
#define  ILL_BTSTOP	11	 /* # of stop bits of request not valid.     */
#define  ILL_DTEDCE	12	 /* Must specify either DTE or DCE config.   */
#define  PORT_ACTIVE	13	 /* Tried to reset port(n), without reset(0).*/
#define  IO_OVERFLOW	14	 /* Tried to write > 256 bytes this call.    */
#define  IO_BLOCKED	15	 /* Tried to write while flow control active.*/
#define  ILL_DATATYP	16	 /* Type of data written was not valid.      */
#define  ILL_CALFORM	17	 /* Invalid format for Call request function.*/
#define  ILL_CLRCODE	18	 /* Invalid Call Clearing cause code.	     */
#define  ILL_CALSTAT	19	 /* Illegal request, given call state.	     */
#define  NO_CHANSAVAIL	20	 /* No channels of type requested available. */
#define  ILL_PADPARAM	21	 /* Invalid Pad Parameter specified.	     */
#define  PCHANS_OPEN	22	 /* Can't leave Packet mode: channels open.  */
#define  ILL_EVENT	23	 /* Bad event-code, not implemented.	     */
#define  TIMER_RUNNING	24	 /* A timer is already running (int/upd).    */
#define  NO_RESOURCES	25	 /* No way to start a timer, no que entry.   */
#define  CHKPNT_RUNNING 26	 /* A checkpoint is already running (int/upd)*/
