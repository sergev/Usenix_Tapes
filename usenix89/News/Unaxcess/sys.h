/*
 *	@(#)sys.h	1.1 (TDI) 7/18/86 21:01:55
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:01:55 by brandon
#     Converted to SCCS 07/18/86 21:01:55
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:01:55 by brandon
#     Converted to SCCS 07/18/86 21:01:55
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:01:55 by brandon
#     Converted to SCCS 07/18/86 21:01:55
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

struct sys {
	char ua_home[50];	/* UNaXcess lives here */
	char ua_roc;	/* read-only conference flag */
	char ua_xrc;	/* x-rated conference flag */
	char ua_edit[50];	/* the default editor */
	char ua_shell[50];	/* the default shell */
	char ua_env;	/* read environment for SHELL, EDITOR */
	char ua_bbs[32];	/* name of BBS login */
	char ua_tlimit;	/* minutes until logout */
	char ua_sysop[32];	/* name of the sysop login */
	char ua_pm;	/* allow private messages? */
	char ua_log;	/* keep a log? */
	char ua_bnr[50];	/* path of banner file, EOS = internal */
	char ua_login[256];	/* login message, EOS = internal */
	char ua_hco;	/* hard-copy-output mode enable */
	char ua_nla;	/* number of attempts to login allowed */
	char ua_auc[256];	/* ascii upload command */
	char ua_adc[256];	/* ascii download command */
	char ua_xuc[256];	/* Xmodem upload command */
	char ua_xdc[256];	/* Xmodem download command */
	char ua_kuc[256];	/* Kermit upload command */
	char ua_kdc[256];	/* Kermit download command */
};

extern struct sys parms;
