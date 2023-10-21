/* comnd.h	Common (user and library) definitions for the
		COMND services.

	Copyright (C) 1984, 1985 Mark E. Mallett

	Permission is hereby granted to distribute this file indiscriminately.

Edit history

When	Who	What
------	---	--------------------------------
84xxxx	MEM	Create file.



*/


#include "mem.h"			/* Include my standard names */

/* Various constants, etc. */


	/* CFB command function codes */

#define	_CMINI	0x0000			/* Initialize the parse */
#define	_CMKEY	0x0001			/* Keyword parse */
#define	_CMNUM	0x0002			/* Number */
#define	_CMNOI	0x0003			/* Noise words (guide string) */
#define	_CMCFM	0x0004			/* Confirm */
#define	_CMGSK	0x0005			/* General Storage Keyword */
#define	_CMSWI	0x0006			/* Switch */
#define	_CMTXT	0x0007			/* Text to end of line */
#define	_CMTOK	0x0008			/* Token */
#define	_CMUQS	0x0009			/* Unquoted string */
#define	_CMDAT	0x000A			/* Date and/or time */

#define	_CMMAX	0x000A			/* Maximum function code */


	/* COMND result codes */

#define	_CROK	0x0000			/* OK completion */
#define	_CRNOP	0x0001			/* No-parse (no matching input) */
#define	_CRRPT	0x0002			/* Reparse required */
#define	_CRIFC	0x0003			/* Invalid function code */
#define	_CRBOF	0x0004			/* Buffer overflow */
#define	_CRBAS	0x0005			/* Invalid radix */
#define	_CRAGN	0x0006			/* Try-again (for support mode only) */



	/* Flags in the CSB_RFL element of the command state block */

#define	_CFNOP	0x0001			/* No Parse */
#define	_CFESC	0x0002			/* Terminated by escape */
#define _CFEOC	0x0004			/* Terminated by CR */
#define	_CFRPT	0x0008			/* Reparse required */
#define	_CFSWT	0x0010			/* Switch ended with colon */
#define	_CFPFE	0x0020			/* Previous field term. with esc */


	/* Flags in the CSB_PFL element of the command state block */

#define	_CFRAI	0x0001			/* Raise lowercase to uppercase */
#define	_CFNEC	0x0002			/* No echo if this is set */


	/* Flags in the CFB_FLG element of the command function block */

#define	_CFHPP	0x0001			/* User-supplied help string */
#define	_CFDPP	0x0002			/* User-supplied default */
#define	_CFSDH	0x0004			/* Suppress default help */
#define	_CFCC	0x0008			/* CC table supplied */
#define	_CFDTD	0x0040			/* Parse date (for _CMDAT) */
#define	_CFDTT	0x0080			/* Parse time (for _CMDAT) */
/*

*//*	Structures	*/

	/* CSB - The Command State Block */

typedef
  struct CSBs
    {
    BYTE	CSB_PFL;		/* Passed flags (from caller) */
    BYTE	CSB_RFL;		/* Returned flags (to caller) */
    struct setjmp *CSB_RSB;		/* Reparse SETJMP buffer */
    int		(*CSB_INP)();		/* Addr of input-char routine */
    int		(*CSB_OUT)();		/* Addr of output-char routine */
    BYTE	*CSB_PMT;		/* Prompt */
    BYTE	*CSB_BUF;		/* Buffer address */
    int		CSB_BSZ;		/* Buffer size */
    BYTE	*CSB_ABF;		/* Atom buffer */
    int		CSB_ASZ;		/* Atom buffer size */
    int		CSB_PRS;		/* Parse index */
    int		CSB_FLN;		/* Filled length (# chars filled) */
    int		CSB_RCD;		/* Result code. */
    union	{			/* Returned value */
	int	_INT;			/*  --for int value--  */
	char	*_ADR;			/*  --for address value--  */
	}	CSB_RVL;		/* (returned value) */
    struct CFBs *CSB_CFB;		/* Addr of matching CFB, if any */
    }
  CSB;


	/* CFB - the Command Function Block */

typedef
  struct CFBs
    {
    BYTE	CFB_FNC;		/* Function code */
    BYTE	CFB_FLG;		/* Flags */
    struct CFB  *CFB_CFB;		/* Addr of next CFB for parse
					   resolution */
    AITYPE	CFB_DAT;		/* Data, if any */
    BYTE	*CFB_HLP;		/* Help string */
    BYTE	*CFB_DEF;		/* Default string */
    int		*CFB_CC;		/* Char characteristics table addr */
    }
  CFB;


	/* CGK - Command General Keyword parsing block */

	/*	CGK is pointed to by CFB type _CMGSK.  It specifies
		a routine to call to fetch each new candidate keyword. */

typedef
  struct CGKs
    {
    char	*CGK_BAS;		/* Base address to give to kfr */
    char	**(*CGK_KFR)();		/* Keyword fetch routine address */
    }
  CGK;
