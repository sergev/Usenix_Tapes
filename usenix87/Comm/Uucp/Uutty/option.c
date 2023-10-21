#include "uutty.h"
/* 
** Process an option string.  Note that we get
** a pointer to the initial char, usually '-'.
*/
extern int baudmask;	/* CBAUD mask, if baud rate specified */
extern int baudrate;	/* Actual baud rate */

option(op)
	char*op;
{	int  n;

	D4("option(\"%s\")",op);
	switch (op[1]) {	/* Which option? */
	default:
		sprintf(stderr,"Unknown option \"%s\" ignored.\n",op);
		break;
	case 'B':		/* Set baud rate */
	case 'b':
		n = sscanf(op+2,"%u",&baudrate);
		if (n < 1) baudrate = 1;
		D3("baudrate=%u\n",baudrate);
		switch (baudrate) {
		case 12: case 1200: baudmask = B1200; break;
		case 24: case 2400: baudmask = B2400; break;
		case 48: case 4800: baudmask = B4800; break;
		case 96: case 9600: baudmask = B9600; break;
		default: E("Can't handle baud rate of %d",baudrate);
			baudrate = baudmask = 0;
		}
		break;
	case 'C':	/* Count between I/O operations */
	case 'c':
		n = sscanf(op+2,"%u",&count);
		if (n < 1) count = 1;
		D3("count=%u\n",count);
		slowfl = count || slow;
		break;
	case 'D':	/* Set debug level */
	case 'd':
	case 'X':	/* Set debug level */
	case 'x':
		n = sscanf(op+2,"%u",&debug);
		if (n < 1) debug = 1;
		D3("debug=%d\n",debug);
		break;
	case 'E':
	case 'e':		/* Exit message */
		if (op[2]) {	/* -e"msg" is exit message */
			m_exit = op + 2;
			D3("m_exit=\"%s\"",m_exit);
		} else {		/* -e turns on echoing */
			echoing = echofl = 1;
			D3("echofl=%d",echofl);
		}
		break;
	case 'F':
	case 'f':       /* Fork subprocesses for a shells */
		forkfl++;
		D3("forkfl=%d",forkfl);
		break;
	case 'H':
	case 'h':	/* Display "help" messages */
		help();
		break;
	case 'I':
	case 'i':       /* Initialization message */
		m_init = op + 2;
		D3("m_init=\"%s\"",m_init);
		break;
	case 'L':
	case 'l':       /* Create lockfile on login */
		lockfl++;
		forkfl++;
		D3("forkfl=%d lockfl=%d",forkfl,lockfl);
		break;
	case 'N':
	case 'n':       /* Nudge message */
		m_nudge = op + 2;
		D3("m_nudge=\"%s\"",m_nudge);
		break;
	case 'P':
	case 'p':       /* Port name */
		device = op + 2;
		D3("device=\"%s\"",device);
		break;
	case 'R':
	case 'r':       /* Raw I/O [default=TRUE] */
		n = sscanf(op+2,"%d",&raw);
		if (n < 1) raw = 1;
		D3("raw=%d\n",raw);
		break;
	case 'S':
	case 's':       /* Slow output, sleep(slow) between buffers */
		n = sscanf(op+2,"%d",&slow);
		if (n < 1) slow = 1;
		D3("slow=%d\n",slow);
		break;
	}
	return 0;
}
