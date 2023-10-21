#include	<sysid.h>
#define memchk(p)	{if(p >= eomem) brk(eomem =+ BUFLEN);}
#define not		~		/* so that I can see not on Infotons */
#define loop		for(;;)		/* generates no extraneous instructions
					 * (as good as a goto)
					 */
#define true		1
#define false		0

#define BUFLEN		512
#define STRINGLEN	64

#define PROT		0600		/* our standard file protection */

#define cnt		not 0140 &	/* will generate a control character */
#define IOERR		-1	/* indicates an I/O error (see getchar()) */
#define HUPFILE	"sndmsg-hup"	/* body of message after hangup */
#define SH	"/bin/sh"	/* great shell in the sky */
#define EDIT	"/bin/edit"	/* unix editor */
#define NROFF	"/bin/nroff"	/* unix formatter */
#define OFFICE	"/usr/office/"	/* central distrubtion lists */
#define del		0177	/* the delete or rub-out character */
#define abortchar	034	/* FS char (control-shift-L or control-|) */

#define SOH		""	/* octal 01, the ascii code for control-a */

/*
*	The following are most of the global variables of the program.
*	Modified by Walter Lazear, April 1977
*		Changed timezone to Eastern (from Central).
*/

char	*cc, *to, *subject, *header, *eomem, *bomem;
char	*pwptr;
int	pwcount;
int	oldmode, oldsignal,
	uid, gid, myuid;
int	ttys[3];
int	netmsgs;
char	bigbuf[BUFLEN], pwbuf[BUFLEN];
char	from[STRINGLEN], mailbox[STRINGLEN];
struct systemid sid;
char	sysname[8];
char	*putptr;
char	*htempfile, *btempfile, b2tempfile[15];
char	dbuf[]		"dd mmm yyyy at hhmm-EST";
char	netfile[]	"/usr/lpd/netmail/\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char	hostname[]	"/dev/net/\0\0\0\0\0\0\0\0\0\0\0\0\0\0"; /* 14 character maximum on filenames */
char	*ptend, *lttr;
int	receipt, receiptall;	/* for tagging messages for receipt generation when read */
int	repeat		1,
	aflag		0,
	qflag		0,
	fdy,
	fdx,
	npipe[2];


struct status		/*	used by stat system call	*/
{
	char	s_minor;
	char	s_major;
	int	s_number;
	int	s_mode;
	char	s_nlinks;
	char	s_uid;
	char	s_gid;
	char	s_size0;
	int	s_size1;
	int	s_addr[8];
	int	s_actime[2];
	int	s_modtime[2];
}
statb;

struct struct1
{
	char	pw, th, tb;
}
fd;

struct struct2
{
	char	erase, unsent;
}
flag;

/*	The following are arrays which will not be altered by the program.
 *	They are global so that they can be initialized and may be used in
 *	several places.
 */
char	nl[]			"\n"	;
char	unsent_mail[80]	  ;/* file where unsent mail is put */
char	colon_lit[]		":"	;/* an attempt to save literal space */
