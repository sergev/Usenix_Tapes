/********************************************************************
*********************************************************************
 
Module Name:		btree.fe.c
============
 
Function:		Front end for btree code ...
=========		

Description:
============
	Implements a front-end program for the btree code

	Libraries:
		stdio.h
		btree.fe.h
		btree.c	(-> btree.h)
		bree.prt.h

****************************************************************************
****************************************************************************/

static char btreefrontendc[] = "@(#)btree.fe.c	1.1 7/16/86";

#include <stdio.h>
#include "btree.c"
#include "btree.fe.h"
#include "btree.prt.h"


/*
** MAIN PROGRAM
** ============
**
** Purpose:	Front panel type thing for btree code - allows interactive
**		manipulation of tree
**
** Parameters:	none
**
** Returns:	none
**
*/

main()
{
	frontend();
}
