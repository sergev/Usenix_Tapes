/*
 *	@(#)mkconf.c	1.1 (TDI) 7/18/86 21:10:32
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:10:32 by brandon
 *     Converted to SCCS 07/18/86 21:10:31
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:10:32 by brandon
 *     Converted to SCCS 07/18/86 21:10:31
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:10:32 by brandon
 *     Converted to SCCS 07/18/86 21:10:31
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)mkconf.c	1.1 (TDI) 7/18/86 21:10:32";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

char line[256];

#ifdef SYS5
#  define SYS3
#endif SYS5

#ifdef SYS3
#  define RIndex	strrchr
#else
#  ifdef XENIX3
#    define RIndex	strrchr
#  else
#    ifdef XENIX5
#      define RIndex	strrchr
#    else
#      define RIndex	rindex
#    endif XENIX5
#  endif XENIX3
#endif SYS3

char *RIndex();

main(argc, argv)
    char **argv;
    {
    if (argc != 3)
	exit(1);
    mknod(argv[1], S_IFDIR|0755, 0);
    chown(argv[1], atoi(argv[2]), 50);
    sprintf(line, "%s/.", argv[1]);
    link(argv[1], line);
    *RIndex(argv[1], '/') = '\0';
    strcat(line, ".");
    link(argv[1], line);
    }
