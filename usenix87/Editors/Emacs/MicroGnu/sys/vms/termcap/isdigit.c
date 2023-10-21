/************************************************************************
 *									*
 *			Copyright (c) 1982, Fred Fish			*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released for public	*
 *	distribution for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose.							*
 *									*
 ************************************************************************
 */




/*
 *  LIBRARY FUNCTION
 *
 *	isdigit    test character for numeric property
 *
 *  SYNOPSIS
 *
 *	int isdigit(ch)
 *	char ch;
 *
 *  DESCRIPTION
 *
 *	Returns TRUE or FALSE depending upon whether the specified
 *	character is a numeric character or not.
 *
 *  BUGS
 *
 *	May fail on machines in which native character set is not ASCII.
 *
 */

#include <stdio.h>

#define TRUE 1
#define FALSE 0

int isdigit(ch)
char ch;
{
    if (ch > '9' || ch < '0') {
	return(FALSE);
    } else {
	return(TRUE);
    }
}
