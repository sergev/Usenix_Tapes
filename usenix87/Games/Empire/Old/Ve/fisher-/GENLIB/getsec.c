/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	<stdio.h>
#include	"emp.h"

extern	FILE	*fdsec;

getsec(buf, n)
struct	sector	*buf;
int	n;
{
	if( fseek(fdsec, ((long)n)*sizeof(struct sector), 0) != 0 ) return(ERROR);
	if( fread(buf, sizeof(struct sector), 1, fdsec) != 1) return(ERROR);
}

putsec(buf, n)
struct	sector	*buf;
int	n;
{
	if( fseek(fdsec, ((long)n)*sizeof(struct sector), 0) != 0 ) return(ERROR);
	if( fwrite(buf, sizeof(struct sector), 1, fdsec) != 1 ) return(ERROR);
}
