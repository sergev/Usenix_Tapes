
#include	<stdio.h>

#define	MAXBUF	32760		/* maximum buffer size                */
#define MAXFILES   32		/* maximum number of files on tape    */
#define BLKSIZE    80           /* label size and default blocksize   */

/* error codes */
#define VIDERR	0
#define SECERR	1
#define REDERR  2
#define CMDERR  3
#define OPNERR  4
#define FLGERR  5
#define EOVERR  6
#define FPRERR	7
#define CLOERR	8
#define TECERR	9  
#define FCLERR  10
#define FOPERR  11
#define VOLERR  12
#define WRTERR  13
#define FMTERR	14
#define BLKERR  15
#define MAXERR  16
#define RECERR  17
#define FNMERR  18
#define RFMERR  19

/* labels to check */
# define	L_VOL1		1
# define	L_HDR1		2
# define	L_HDR2		3
# define	L_EOF1		4
# define	L_EOF2		5
# define	L_EOV1		6
# define	L_UTL		7
# define	L_UHL		8

/* record types */
# define	F	0
# define	FB	1
# define	V	2
# define	VB	3

/* miscellaneous constants */
#define SPACE   ' '
#define SPACES  "  "
#define ZERO    '0'
#define VOL1    "VOL1"
#define HDR1    "HDR1"
#define HDR2    "HDR2"
#define EOF1    "EOF1"
#define EOF2    "EOF2"
#define SYSCD   "OS36000000000" 
#define DEVRMT  "/dev/rmt"
#define DEVNRMT "/dev/nrmt"

#define MIN(x, y)	(((x) > (y)) ? (y) : (x))

/* miscellaneous variables */
char	inptbuf[MAXBUF+1];	/* input-output buffer		     */	
char	fname[18];		/* file name for tape labels         */
char	ufname[257];		/* UNIX file name		     */
char	vid[7];			/* volume serial number		     */
int	reclen;			/* logical record length             */
int	recode;			/* record format for writing	     */
int	blklen;			/* block length			     */
int	blkct;			/* block count for a single data set */
int	vblen;			/* variable block length	     */
int	tfd;			/* mag tape file descriptor  	     */
char	*tpdr;			/* rewinding tape drive              */
char	ntpdr[12];		/* non-rewinding tape                */
int	filectr; 		/* count of files processed	     */
extern  int   errno;		/* return code for errors	     */

/* flags */
int	ansi;			/* ansi label tape                   */
int	stdl;			/* stdl tape               	     */
int	flags;			/* required parameters     	     */
int	files;			/* files to be read       	     */
int	wrtflag;		/* write parameters        	     */
int	eov;			/* count of tapemarks for eov        */
int	p;			/* padding or no padding  	     */
int	keep;			/* keeping same file names 	     */
int	firstrec;		/* set until 1st record read   	     */
int	ok_vol_label;		/* set for valid volume label  	     */
int	complete;		/* for write parameters	    	     */
int	devflag;		/* for tape device		     */
