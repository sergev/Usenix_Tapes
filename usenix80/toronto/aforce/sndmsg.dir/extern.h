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
#define OFFICE	"/usr/office/"	/* central distribution lists */
#define del		0177	/* the delete or rub-out character */
#define abortchar	034	/* FS char (control-shift-L or control-|) */

#define SOH		""	/* octal 01, the ascii code for control-a */

/*
	The following are most of the global variables of the program.
*	Modified by Walter Lazear, April 1977
*		Changed timezone to Eastern (from Central).
*/

extern	char	*cc, *to, *subject, *header, *eomem, *bomem;
extern	char	*pwptr;
extern	int	pwcount;
extern	int	oldmode, oldsignal,
		uid, gid, myuid;
extern	int	ttys[3];
extern	int	netmsgs;
extern	char	bigbuf[BUFLEN], pwbuf[BUFLEN];
extern	char	from[STRINGLEN], mailbox[STRINGLEN];
extern	struct systemid sid;
extern char	sysname[8];
extern char	*putptr;
extern	char	*htempfile, *btempfile, b2tempfile[15];
extern	char	dbuf[];
extern	char	netfile[];
extern	char	hostname[];
extern	char	*ptend, *lttr;
extern	int	receipt, receiptall;	/* for tagging messages for receipt generation when read */
extern	int	repeat,
		aflag,
		qflag,
		fdy,
		fdx,
		npipe[2];


extern	struct status		/*	used by stat system call	*/
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
	} statb;

extern	struct struct1
	{
		char	pw, th, tb;
	} fd;

extern	struct struct2
	{
		char	erase, unsent;
	} flag;

/*	The following are arrays which will not be altered by the program.
 *	They are global so that they can be initialized and may be used in
 *	several places.
 */
extern	char	nl[];
extern	char	unsent_mail[];
extern	char	colon_lit[];
