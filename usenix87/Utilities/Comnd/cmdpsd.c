/*	cmdpsd.c	COMND module; Stub for date/time function parse.

	Copyright (C) 1985 Mark E. Mallett

	Permission is hereby granted to distribute this file indiscriminately.

	This file is a stub for the date/time parsing function, and
is to be included in the link when date/time functions are not used
(in order to save memory on small machines like mine).

Edit history

When	Who	What
------	---	--------------------------------
850527	MEM	Create file.

	Routines included:

		CFPdat		Parse date and/or time
		CMDpsd		fake routine to drag in this module.

*/

#include "stdio.h"			/* Standard system defs */
#include "comnd.h"			/* COMND interface definitions */
#include "comndi.h"			/* COMND internal definitions */



/* External routines */


/* External data */


/* Internal (public) routines */



/* Internal (public) data */


/* Local (static) data */


/*

*//* CMDpsd()

	Parse stub for date/time

If the COMND files are stored in an object library (for the linker), you
can put this module first and call this routine directly in order to
drag in this module and satisfy the references with the stub(s) in this
routine.  Or you can keep this module separate and link it in before
referencing the library.


*/

CMDpsd()

{}
/*

*//* CFPdat (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMDAT, date and/or time

This is a stub for the _CMDAT function processor.  It will indicate
an invalid function call.

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (N/A here)


Returns :

	<value>		Parse status of _CPABT.

*/

CFPdat (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
CSBptr -> CSB_RCD = _CRIFC;		/* Set invalid function code */
return (_CPABT);			/* Abort, right now */
}
