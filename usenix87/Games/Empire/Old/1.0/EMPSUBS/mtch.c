#define D_NATSTAT
#define D_NATSTR
#define D_COMSTR
#define D_FILES
#include "empdef.h"

stmtch(sp, strp, strlen)
char	*sp, **strp;
int	strlen;
{
	register char	*cp, **stp;
	register	retval;
	int	i, j;

	retval = -1;
	stp = strp;
	cp = sp;
	i = 0;
X34:	
	if( (j = mineq(cp, *stp)) != 0 ) {
		if( j == 2 ) return(i);
		if( retval != -1 ) return(-2);
		retval = i;
	}
	i++;
	stp = (char **)((char *)stp + strlen);
	if( *stp != 0 ) goto X34;
	return(retval);
}

comtch(comp)
char	*comp;
{
	register char	*cp;
	register struct	comstr	*sp;
	int	retval, i, j;

	retval = -1;
	cp = comp;
	for( i=0; (sp= &coms[i]) != 0; i++ ) {
		if( sp->c_form == '\0' ) break;
		if( sp->c_permit > ncomstat  ) continue;
		if( (j = mineq(cp, sp->c_form)) == 0 ) continue;
		if( j == 2 ) return(i);
		if( retval != -1 ) return(-2);
		retval = i;
	}
	return(retval);
}

mineq(s1, s2)
char	*s1, *s2;
{
	register char	*cp1, *cp2;

	cp1 = s1;
	cp2 = s2;
X14:
	if( *cp1++ != *cp2++ ) return(0);
	if( *cp2 == ' ' ) goto X60;
	if( *cp1 != '\0' ) goto X14;
X60:
	if( *cp1 != '\0' ) return(1);
	if( *cp2 == ' ' ) return(2);
	if( *cp2 == '\0' ) return(2);
	return(1);
}
