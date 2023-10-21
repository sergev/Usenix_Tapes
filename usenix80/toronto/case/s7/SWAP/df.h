#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	last mod:	3/23/79						*/
/*	Module:		df.h						*/
/*	Description:	This file contains all defines used by SS.	*/
/*									*/
/************************************************************************/

#define		QUIT		3
#define		INTERRUPT	2
#define		FOREVER		1
#define		TABLE		9
#define		STATS		8
#define		CORE		0
#define		SWAP		7
#define		TRUE		1
#define		FALSE		0
#define		DEFSLEEP	4
#define		NOBODY		99
#define		FD		1
#define		ANNARBOR	5
#define		DATETITLE	1,1
#define		TIMETITLE	65,1
#define		MESGTITLE	31,1
#define		STATTITLE	1,3
#define		TIMEDATA	71,1
#define		DATEDATA	7,1
#define		WHO1TITLE	68,7
#define		ADDR1TITLE	1,11
#define		SIZE1TITLE	68,10
#define		STATUS1TITLE    68,12
#define		WHO2TITLE	68,17
#define		ADDR2TITLE	1,21
#define		SIZE2TITLE	68,20
#define		STATUS2TITLE	68,22
#define		TTYSIZE		150
#define		DIDLEDATA	72,3
#define		DATA		3
#define		TEXT		4
#define		CORE1		4,10
#define		STAT1		4,12
#define		CORE2		4,20
#define		STAT2		4,22
#define		MAXNAME		35
#define		SLOTS		22

/*
 *	Globally defined symbols for swapping display
 */
# define SVTXT	5
# define ROOT	0

# define DTITLE		1,1
# define TTITLE		65,1
# define MTITLE		31,1
# define STITLE		10,3
# define TDATA		71,1
# define DDATA		7,1
# define SUDATA		36,3
# define SADATA		20,3
# define W1TITLE	69,5
# define A1TITLE	1,9
# define S1TITLE	69,8
# define W2TITLE	69,12
# define A2TITLE	1,16
# define S2TITLE	69,15
# define W3TITLE	69,19
# define A3TITLE	1,23
# define S3TITLE	69,22

# define MNAME		5
# define GRANULARITY	5

# define convert(x)	(x)/GRANULARITY
