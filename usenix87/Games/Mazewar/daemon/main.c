#ifndef lint
static char rcsid[] = "$Header: main.c,v 1.1 84/08/25 17:04:54 lai Exp $";
#endif

#include <sys/types.h>

#ifdef PLUS5
#include "../berk/btypes.h"
#include "../berk/uio.h"
#include "../berk/socket.h"
#include "../berk/in.h"
#else
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/data.h"

struct sadr_in sins[MAXPLAYER];		/* sockets of players */
long lastup[MAXPLAYER];				/* last update from user */

/*ARGSUSED*/
main(argc, argv)
	char *argv[];
{

	init(argc, argv);

	work();

	/*NOTREACHED*/
}
