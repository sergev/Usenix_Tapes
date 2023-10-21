62,64c
	0, &nosys,			/* 49 = reserved for USG */
	0, &nosys,			/* 50 = reserved for USG */
	0, &nosys,			/* 51 = turn acct off/on */
.
52,53c
	0, &nosys,			/* 39 = setpgrp (not in yet) */
	0, &tell,			/* 40 = tell */
.
46c
	0, &nosys,			/* 33 = access */
.
42c
	0, &nosys,			/* 29 = pause */
.
40c
	0, &nosys,			/* 27 = alarm */
.
3,5c
 * sysent.c - system call entry vector
 *
 *	modified 03-Jun-1980 by D A Gwyn:
 *	1) added "tell".
 *
.
w
q
