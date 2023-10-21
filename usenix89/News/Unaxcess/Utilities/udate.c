/*
 *	@(#)udate.c	1.1 (TDI) 7/18/86 21:10:38
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:10:38 by brandon
 *     Converted to SCCS 07/18/86 21:10:37
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:10:38 by brandon
 *     Converted to SCCS 07/18/86 21:10:37
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:10:38 by brandon
 *     Converted to SCCS 07/18/86 21:10:37
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)udate.c	1.1 (TDI) 7/18/86 21:10:38";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

main() {
	puts(longdate());
}
