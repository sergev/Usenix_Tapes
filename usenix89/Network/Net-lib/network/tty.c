#ifndef lint
static char *RCSid = "$Header: tty.c,v 1.1 87/04/15 21:29:36 josh Exp $";
#endif

/*
 *------------------------------------------------------------------
 * Copyright 1987, Josh Siegel
 * All rights reserved.
 *
 * Josh Siegel (siegel@hc.dspo.gov)
 * Dept of Electrical and Computer Engineering,
 * University of New Mexico,
 * Albuquerque , New Mexico
 * (505) 277-2497
 *
 *------------------------------------------------------------------
 *
 * $Source: /usr/pcb/josh/other/network/RCS/tty.c,v $
 * $Revision: 1.1 $
 * $Date: 87/04/15 21:29:36 $
 * $State: Exp $
 * $Author: josh $
 * $Locker: josh $
 *
 *------------------------------------------------------------------
 *
 * $Log:	tty.c,v $
 * Revision 1.1  87/04/15  21:29:36  josh
 * Initial revision
 * 
 *
 *------------------------------------------------------------------
 */


#include "netw.h"

_stty(object, change, status)
	TCOND          *object;
	int             change, status;
{
	switch (change) {
	case COOKED:		/* 0x0 */
		object->sgttyb.sg_flags &= ~RAW;
		object->sgttyb.sg_flags &= ~CBREAK;
		break;
	case CBREAK:		/* 0x2 */
		object->sgttyb.sg_flags |= CBREAK;
		break;
	case LCASE:		/* 0x4 */
		if (status)
			object->sgttyb.sg_flags |= LCASE;
		else
			object->sgttyb.sg_flags &= ~LCASE;
		break;
	case RAW:		/* 0x20 */
		object->sgttyb.sg_flags |= RAW;
		break;
	case ECHO:		/* 0x8 */
		if (status)
			object->sgttyb.sg_flags |= ECHO;
		else
			object->sgttyb.sg_flags &= ~ECHO;
		break;
	case BAUD:		/* 0x1 */
		object->sgttyb.sg_ispeed = status;
		object->sgttyb.sg_ospeed = status;
		break;
	default:
		return(-1);
		break;
	}
}
