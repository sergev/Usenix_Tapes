#ifndef lint
static char rcsid[] = "$Header: setkeyok.c,v 1.1 84/08/25 17:11:31 chris Exp $";
#endif

#include <stdio.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

/*
 * Decrement the keyok variable.  If this variable is <= 0 then we allow
 * new shot.
 */
setkeyok()
{
	keyok--;
}
