/* $Header: /usr/sys/vaxuba/RCS/dmfo.c,v 1.1 84/09/12 01:31:20 james Exp $ */

#include "dmf.h"
#if NDMF > 0
#include "dmfo.h"
#if NDMFO > 0
/*
 * DMF-11 ``outgoing mode'' driver
 *
 * Requires hooks in dmf.c
 */
#include "../h/types.h"

/*
 * Open a DMF32 line in outgoing mode
 */
/*ARGSUSED*/
dmfoopen(dev, flag)
	dev_t dev;
{
	return (dmfopen(dev, -1));
}

/*
 * Close a DMF32 outgoing mode line
 */
/*ARGSUSED*/
dmfoclose(dev, flag)
	dev_t dev;
{
	dmfclose(dev, -1);
}
#endif NDMFO > 0
#endif NDMF > 0
