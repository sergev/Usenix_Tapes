/* comndi.h	Internal (for implementation only) definitions for the
		COMND services.

	Copyright (C) 1984, 1985 Mark E. Mallett

	Permission is hereby granted to distribute this file indiscriminately.

Edit history

When	Who	What
------	---	--------------------------------
84xxxx	MEM	Create file.



*/



/* Various constants, etc. */


	/* Parse result codes, to be returned by function code parsers */

#define	_CPSUCC	0x0000			/* Success, causes immediate successful
					   return to COMND caller. */
#define	_CPABT	0x0001			/* Some severe error, causes immediate
					   unsuccessful return to caller */
#define	_CPNOP	0x0002			/* No parse... does not match.  May
					   cause unsuccessful return to COMND
					   caller if there are no better
					   results in the CFB chain. */
#define	_CPGVH	0x0003			/* Gave help... input might have
					   parsed if it was completed. */
#define	_CPAGN	0x0004			/* Try again.  Might match if it were
					   complete. */
#define	_CPCPE	0x0005			/* Completed with escape. */


	/* Character codes passed around by COMND support routines */

#define	_CCHLP	0xFFFF			/* Help request */
#define	_CCCMP	0xFFFE			/* Command complete, please? */
#define	_CCINC	0xFFFD			/* Incomplete (get more) */
#define	_CCEND	0xFFFC			/* End of input (CR found) */
#define	_CCINV	0xFFFB			/* Invalid character in atom */
