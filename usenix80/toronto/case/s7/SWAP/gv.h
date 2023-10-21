#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	last mod:	3/23/79						*/
/*	Module:		gv.h						*/
/*	Description:	This file contains global variable declarations.*/
/*									*/
/************************************************************************/

#include	"df.h"

#include	"/usr/sys/param.h"
#include	"/usr/sys/proc.h"
#include	"/usr/sys/text.h"

/************************************************************************/
/*	The cursor control and erase functions can support 5 types of	*/
/*	terminals. The variable ttype indicates which terminal is used.	*/
/************************************************************************/

int		ttype;

/************************************************************************/
/*	The buffered writes use the following variables.		*/
/************************************************************************/

char	queue[3000];
int	qlen;

/************************************************************************/
/*	There are several flags which indicate which mode SS is in.	*/
/*	The default values are initialized here.			*/
/************************************************************************/

int		modeflg,	/* mode, coremap, process table or stats */
		intflg,		/* interrupt has occured */
		delta;		/* toggle for long or short term stats */

/************************************************************************/
/*	Several functions need to read core, and the swap device.	*/
/*	The following global variables are set up by initialize.	*/
/************************************************************************/

int		mem,		/* file descriptor for /dev/mem */
		swap;		/* file descriptor for /dev/rk0 (cwru) */

/*************************************************************************/
/*	The coremap routine needs the following global structures.	*/
/************************************************************************/

struct
{
	char	csize[180];	/* The chars that show core size */
	char	cstat[180];	/* The chars that show current proc stat */
	int	ppid[180];	/* The pid of the process in a slot */
}
	screen;
struct
{
	char	name[6];	/* Users name */
	int	uuid;		/* Users id */
}
	uname[MAXNAME + 1];

/************************************************************************/
/*	The following struct is used by wh.c				*/
/************************************************************************/

struct
{
	int	pid;	/* Pid of process in screen slot */
	char	flags;	/* Status of information on screen */
	char	unknown;/* Set TRUE, if the process name is '-' */
	char	used;	/* Status of slot */
}
	cproc[SLOTS];

int	swplo,
	nswap;

int	swapmap[SMAPSIZ];

struct map{
	unsigned	m_size;
	unsigned	m_addr;
};
