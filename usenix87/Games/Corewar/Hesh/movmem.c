/*
** movmem.c	-
**
static	char	rcsid[] = "$Header: movmem.c,v 7.0 85/12/28 14:35:55 ccohesh Dist $";
**
*/
#include "cw.h"

/*
** movmem()	- move contents of one location in `memory' to another
*/
void	movmem (m1, m2)
reg	memword	*m1, *m2;
{
	m1->op = m2->op;
	m1->moda = m2->moda;
	m1->modb = m2->modb;
	m1->arga = m2->arga;
	m1->argb = m2->argb;
}
