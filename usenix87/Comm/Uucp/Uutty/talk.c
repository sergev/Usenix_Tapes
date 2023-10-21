#include "uutty.h"
/* 
** This routine assumes that it is talking to a port connected
** to a VMEbus device with an on-board debugger.  It exchanges
** some pleasantries to verify this fact, then procedes to ask
** the debugger to show it the requested chunk of its memory.
*/
talk()
{	int  c, i, n, r;
	int  baddies, lowers, uppers;
	int  messages;

	D4("talk()");
	r = 0;
	ss = S_INIT;
	D4("State %d=%s",ss,gestate());
	lockwait();		/* Try to avoid collisions */
	if (device) target = device;
/*
** We have a long list of initialization strings here.
** This has come in handy in a few cases, but usually
** they are mostly null.
*/
	if (m_init )  Awrite(m_init);
	if (m_init1) {Awrite(m_init1); sleep(SLEEP1);}
	if (m_init2) {Awrite(m_init2); sleep(SLEEP2);}
	if (m_init3) {Awrite(m_init3); sleep(SLEEP3);}
	ss = S_INIT;
	D4("State %d=%s",ss,gestate());
	if (m_login)  Awrite(m_login);
	ss = S_LOGIN;
	D4("State %d=%s",ss,gestate());
	Response;
nudge:			/* Try to elicit a response */
	D3("Nudge.");
	if (++nudges > Nudges) {
		E("Too many nudges.");
		r = 1;
		Dead;
	}
	lockwait();		/* Try to avoid interference */
	Resync;
	if (m_nudge)		/* Is a nudge message defined? */
		Awrite(m_nudge);	/* If so, send it */
idle:					/* We will now accept anything */
	D3("Idle.");
	ss = S_IDLE;		/* Note we're awaiting a response */
	D4("State %d=%s",ss,gestate());
	messages = 0;		/* Counter to trigger nudges */
response:				/* Read a response */
	D4("Response: state=%d=%s",ss,gestate());
	if (debug > 0 && target != oldtarg) {
		if (debug >= 3) P("%s Talking to %s.",getime(),target);
		oldtarg = target;
	}
	/*
	** Check for any of a list of known debugger prompts.
	** If we don't get one of then, try to nudge the debugger,
	** and try again, giving up after some number of tries.
	*/
	errno = 0;
	D4("talk:l_tries=%d",l_tries);
	sleep(1);		/* Sheer laziness */
	lockwait();
	if (ss != S_PASSWD)	/* If not expecting password, do echoing if requested */
		echoing = echofl;
	while ((n = Pread(eol0,rsp,rspmax)) <= 0) {
		D3("No response.");
		/*
		** Note this program loops forever waiting for something to come down the line.
		** You might want to do something else here if there's no input.
		*/
	}
	D4("talk:Pread()=%d",n);
	rsp[n] = 0;
	++messages;
	if (debug) {
		dbgtimep = getime();
		if (debug >= 2) Ascdnm(rsp,n,"Got:");
		if (debug >= 4) Hexdnm(rsp,n,"Got:");
	}
	/*
	** Next, we do some preliminary checking for sanity of the input.
	** Since this program's duty is to do login interviews, the only
	** input we really want is something that might be a login id or
	** a password.  Another likely input is a nudge from a counterpart
	** on the other end, to which we respond with a prompt.
	*/
	if (n < 3) {		/* Just "\r", "\n", or "\r\n" is a nudge */
		D4("Short response, %d chars.",n);
		switch (c = rsp[0]) {
		case 0x03:		/* ^C = ETX (usually means "die") */
			if (debug) P("%s: ^C in input, quitting.",getime());
			die(c);
		case '\n':		/* ^J = LF */
		case '\r':		/* ^M = CR */
		case 0x04:		/* ^D = EOT */
		case 0x02:		/* ^B = STX */
		case 0x01:		/* ^A = SOH */
			ss = S_IDLE;		/* Flush garbage from buffer */
			D4("State %d=%s",ss,gestate());
			lockwait();
			if (ibfa < ibfz) {	/* Is there input waiting? */
				D3("Nudge plus garbage received, ignored.");
				Resync;			/* Flush garbage from buffer */
			} else {			/* No input waiting */
				D3("Nudged; send login prompt \"%s\"",m_login);
				Awrite(m_login);	/* Send them a login prompt */
				ss = S_LOGIN;		/* Note that we did it */
				D4("State %d=%s",ss,gestate());
				messages = 0;
			}
			Response;			/* See if there's a response */
		case 0x10:			/* This is a uucp Start-of-message */
			D3("We seem to be talking to a UUCP demon.");
			makeraw(dev);		/* Paranoia! */
			Awrite("OOOOOOOO\r");	/* Try to get it to stop */
			Resync;			/* Discard the rest of the message */
			if (m_init) Awrite(m_init);
			ss = S_INIT;		/* Try to get back to idle state */
			D4("State %d=%s",ss,gestate());
			Response;
		}
	}
	baddies = lowers = uppers = 0;/* These are for counting letters */
	D4("Scan id for char classes...");
	for (i=0; i<n; i++) {		/* Examine the response for nasty stuff */
		c = rsp[i];			/* One byte at a time */
		if (islower(c)) {
			++lowers;			/* Lower-case letters are desirable */
			D6("c=%02X-'%c' lowers=%d",c,dsp(c),lowers);
		} else
		if (isupper(c)) {
			++uppers;			/* Upper-case letters are acceptable */
			D6("c=%02X-'%c' uppers=%d",c,dsp(c),uppers);
		} else			/* Everything else is dubious */
		switch (c) {
		case 0x03:			/* Special goody to let others kill us */
		case 0x02:
		case 0x01:
			if (debug) P("%s: %02X in input, quitting [id]",getime(),c);
			die(c);
		case '#':			/* These are likely shell prompts */
		case '$':			/* Default Bourne shell prompt */
		case '%':			/* Default C- shell prompt */
		case '>':			/* Popular prompt in some circles */
			if (i >= n-2) {		/* Likely only if at end of input */
				shprompt(rsp);		/* Can we handle it? */
				Response;
			}				/* If earlier in string, reject it */
		case 0x00:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09:            case 0x0B:
		case 0x0C:            case 0x0E: case 0x0F:
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		case ' ' : case ':' :	/* None of these are acceptable in ids or passwords */
			D3("Invalid char %02x='%c' in input.",c,dsp(c));
			++baddies;
			break;
		}
	}
	if (baddies || ((lowers+uppers) == 0)) {	/* Is it acceptable? */
		D4("Unacceptable; baddies=%d lowers=%d uppers=%d.",baddies,lowers,uppers);
		Resync;			/* No; drop buffered input */
		lockwait();
		if (baddies && messages > THRESH) {
			makeraw(dev);		/* Others may have munged the driver */
			if (m_init) {
				Awrite(m_init);
				ss = S_INIT;
				D4("State %d=%s",ss,gestate());
			} else {
				Awrite(m_login);
				ss = S_LOGIN;
				D4("State %d=%s",ss,gestate());
			}
			messages = 0;
		}
		ss = S_IDLE;
		D4("State %d=%s",ss,gestate());
		Response;
	}
	switch (ss) {		/* Special actions depending on state */
	case S_EXIT:		/* We are trying to exit */
		if (m_exit) {	/* Is there an exit command? */
			Awrite(m_exit);	/* If so, send it */
			ss = S_EXIT;	/* Note again that we're trying to quit */
			D4("State %d=%s",ss,gestate());
		}
		Response;
	case S_IDLE:		/* We're waiting for the other end to act */
		D4("Got id while idle.");
		c = rsp[0];		/* First char of response is sometimes special */
		goto maybeid;
	case S_LOGIN:		/* We just sent a login prompt */
		D4("Got response to login prompt.");
	maybeid:		/* We may have a login id */
		D4("Got possible login id.");
		if (lowers == 0) {	/* No lower-case letters; it's not an id */
			lockwait();	/* Make sure we're not interfering */
			makeraw(dev);	/* Paranoia! */
			if (m_init) {	/* Do we have a special init string? */
				Awrite(m_init);
				ss = S_INIT;
				D4("State %d=%s",ss,gestate());
			}
			Response;		/* Go wait for next input */
		}
		i = checkid(rsp);	/* Put it through final tests */
		if (i <= 0) {	/* If it fails... */
			if (debug) {
				dbgtimep = getime();
				if (debug >= 2) Ascdnm(rsp,n,"Bad id:");
				if (debug >= 4) Hexdnm(rsp,n,"Bad id:");
			}
			Resync;		/* Get port into known state */
			sleep(5);		/* Extra delay for safety's sake */
			ss = S_IDLE;
			D4("State %d=%s",ss,gestate());
			Response;		/* Go wait for next input */
		}
		/* It sure looks like an id */
		D3("Send password prompt \"%s\"",m_passwd);
		echoing = 0;
		Awrite(m_passwd);
		ss = S_PASSWD;
		D4("State %d=%s",ss,gestate());
		Response;
	case S_PASSWD:		/* We just sent a password prompt */
		D4("Got response to password prompt.");
		i = pswd(rsp);	/* Check it out to see if it's a good guy */
		if (i <= 0) {	/* Successful call won't return */
			D3("Unacceptable password \"%s\"",rsp);
			Response;
		}
		E("pswd(\"%s\")=%d Shouldn't happen.",rsp,i);
		Response;
	default:
		D4("ss=%d not special",ss);
	}
	if (debug) {
		dbgtimep = getime();
		if (debug >= 2) Ascdnm(rsp,n,"Ignore:");
		if (debug >= 4) Hexdnm(rsp,n,"Ignore:");
	}
	Response;
dead:  P("Giving up; %s seems to be dead.",target);
	return r;
}
