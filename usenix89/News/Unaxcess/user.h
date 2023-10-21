/*
 *	@(#)user.h	1.1 (TDI) 7/18/86 21:02:06
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:02:06 by brandon
#     Converted to SCCS 07/18/86 21:02:06
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:02:06 by brandon
#     Converted to SCCS 07/18/86 21:02:06
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:02:06 by brandon
#     Converted to SCCS 07/18/86 21:02:06
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

struct user
    {
    char u_name[33];			/* user name */
    char u_pass[12];			/* password */
    short u_access;			/* can killmsg() or shell() ? */
    char u_login[9];			/* Intended for system login name */
    short u_llen;			/* line length, or default u_access */
    short u_nbull;			/* highest numbered bulletin read */
    char u_lconf[33];			/* login conference (dft = general) */
    }
    ;					/* current user */

extern struct user user;

/* access modes, in u_access */

#define A_NONE		0		/* invalidated login */
#define A_GUEST		1		/* read-only access */
#define A_USER		2		/* standard access */
#define A_FILES		3		/* can udl() */
#define A_SYSTEM	4		/* can system() */
#define A_WITNESS	5		/* Fairwitness */
#define A_MKUSER	6		/* Special: user creator */

struct _himsg
    {
    char hi_conf[33];			/* conference this record refers to */
    char hi_uns;			/* unsubscribed to this conference */
    short hi_num;			/* high message in this conference */
    struct _himsg *hi_next;
    }
    ;

extern struct _himsg *hicnts;

#define HI_SUBSCR	0
#define HI_UNSUB	1
