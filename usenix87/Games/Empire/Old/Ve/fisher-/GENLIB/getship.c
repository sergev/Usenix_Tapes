/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	<stdio.h>
#include	"emp.h"
#include	"ship.h"

extern	FILE	*fdship;

getship(buf, num)
struct	shpstr	*buf;
int	num;
{
	if( fseek(fdship, ((long)num)*sizeof(struct shpstr), 0 ) != 0 ) return(ERROR);
	if( fread(buf, sizeof(struct shpstr), 1, fdship) != 1 ) return(ERROR);
	return(OK);
}

putship(buf, num)
struct	shpstr	*buf;
int	num;
{
	if( fseek(fdship, ((long)num)*sizeof(struct shpstr), 0 ) != 0 ) return(ERROR);
	if( fwrite(buf, sizeof(struct shpstr), 1, fdship) != 1 ) return(ERROR);
	return(OK);
}
