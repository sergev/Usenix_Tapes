#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	Description:	This file contains global variable declarations,*/
/*			which are included in the main driver. A	*/
/*			similar file, gl.h, contains the same variables,*/
/*			declared externally, so that they can be	*/
/*			included in other function files.		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*	Include "defines" (C macros).					*/
/************************************************************************/

#include	"df.h"

#include	"/usr/sys/proc.h"
#include	"/usr/sys/text.h"

/************************************************************************/
/*	The cursor control and erase functions can support 4 types of	*/
/*	terminals. The variable type indicates which terminal is used.	*/
/************************************************************************/

int		type;

/************************************************************************/
/*	The status function, which prints CPU utilization stats, uses	*/
/*	the following global variables.					*/
/************************************************************************/

struct
BEGIN
	char	     name[8];
	int	     stype;
	unsigned     value;
END
	nl[3]  ;

long	sk_cnt[6];
int	mon_out[6];
int	coref;
int	mem;

char	screen[124];
extern char	sc_stat[124];

extern struct
BEGIN
	char	umne[6];
	int	uuid;
END
	uname[30];

extern char	sc_flag[124];
