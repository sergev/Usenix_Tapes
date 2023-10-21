/*
** print.c	- various printing routines
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
**
static	char	rcsid[] = "$Header: print.c,v 7.0 85/12/28 14:36:14 ccohesh Dist $";
**
*/

#include <stdio.h>
#include "cw.h"

/*
** printit()	- print the given instruction
**		- return non-zero on error
*/
int	printit (fp, i)
FILE	*fp;
reg	memword	*i;
{
	reg	char	*inst, *fmt;
	auto	char	adda, addb;
	static	char	*fmt1 = "%3.3s\t\t%c%d\n",
			*fmt2 = "%3.3s\t%c%d\t%c%d\n";

	switch (i->op)
	{
		when DAT:
			inst = SDAT, fmt = fmt1;
		when MOV:
			inst = SMOV, fmt = fmt2;
		when ADD:
			inst = SADD, fmt = fmt2;
		when SUB:
			inst = SSUB, fmt = fmt2;
		when JMP:
			inst = SJMP, fmt = fmt1;
		when JMZ:
			inst = SJMZ, fmt = fmt2;
		when DJZ:
			inst = SDJZ, fmt = fmt2;
		when CMP:
			inst = SCMP, fmt = fmt2;
		when MUL:
			inst = SMUL, fmt = fmt2;
		when DIV:
			inst = SDIV, fmt = fmt2;
		when RND:
			inst = SRND, fmt = fmt1;
		otherwise:
			return(TRUE);
	}
	switch (i->moda)
	{
		when IMM:
			adda = CIMM;
		when REL:
			adda = CREL;
		when IND:
			adda = CIND;
		otherwise:
			adda = '?';
	}
	switch (i->modb)
	{
		when IMM:
			addb = CIMM;
		when REL:
			addb = CREL;
		when IND:
			addb = CIND;
		otherwise:
			addb = '?';
	}
	if (fmt == fmt1)
		fprintf(fp, fmt, inst, addb, i->argb);
	else
		fprintf(fp, fmt, inst, adda, i->arga, addb, i->argb);
	return(FALSE);
}
