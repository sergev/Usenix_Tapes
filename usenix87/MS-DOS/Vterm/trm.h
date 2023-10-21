/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * Terminal capabilities.  These correspond to bits set by trmstart in its
 * parameter flags parameter.
 */

#define HAS_STANDOUT	1	/* Terminal has inverse video or underline */
#define CAN_SCROLL	2	/* Terminal can insert/delete lines */
#define CAN_OPTIMISE	4	/* Terminal can insert/delete characters */
#define CAN_SENSE	8	/* Terminal can send cursor position */

/*
 * Error codes returned by trmstart.
 */

#define TE_OK		0	/* No errors */
#define TE_TWICE	1	/* Trmstart called again */
#define TE_NOTERM	2	/* $TERM not set or empty */
#define TE_BADTERM	3	/* $TERM not found in termcap database */
#define TE_DUMB		4	/* Terminal too dumb */
#define TE_NOTTY	5	/* Stdout not a tty device */
#define TE_NOMEM	6	/* Can't get enough memory */
#define TE_OTHER	7	/* This and higher are reserved errors */
