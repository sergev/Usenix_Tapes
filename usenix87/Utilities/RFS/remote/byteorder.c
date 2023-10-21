/*
 * Copyright 1985, Todd Brunhoff.
 *
 * This software was written at Tektronix Computer Research Laboratories
 * as partial fulfillment of a Master's degree at the University of Denver.
 * This is not Tektronix proprietary software and should not be
 * confused with any software product sold by Tektronix.  No warranty is
 * expressed or implied on the reliability of this software; the author,
 * the University of Denver, and Tektronix, inc. accept no liability for
 * any damage done directly or indirectly by this software.  This software
 * may be copied, modified or used in any way, without fee, provided this
 * notice remains an unaltered part of the software.
 *
 * $Log:	byteorder.c,v $
 * Revision 2.0  85/12/07  18:20:50  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: byteorder.c,v 2.0 85/12/07 18:20:50 toddb Rel $";
long	data = 0x03020100;

main()
{
	char	*p = (char *)&data;

	printf("bytes order=%d,%d,%d,%d\n", p[0],p[1],p[2],p[3]);
}
