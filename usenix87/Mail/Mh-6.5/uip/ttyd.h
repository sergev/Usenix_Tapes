/* ttyd.h - definitions for ttyd */

/* The Regents of the University of California wish to make it known that:
 *
 *
 *				  DISCLAIMER
 *
 *	"Although each program has been tested by its contributor, no
 *	warranty, express or implied, is made by the contributor or the
 *	University of California, as to the accuracy and functioning of
 *	the program and related program material, nor shall the fact of
 *	distribution constitute any such warranty, and no responsibility
 *	is assumed by the contributor or the University of California in
 *	connection herewith."
 *
 */

/*  */

#define	MAIL	"/bin/mail"

#define	SMLWAIT	60		/* seconds for select() */


#define	NOTOK	(-1)
#define	OK	0
#define	DONE	1

#define	TRUE	1
#define	FALSE	0

#define	serror	\
	(errno > 0 && errno < sys_nerr ? sys_errlist[errno] : "Unknown error")

extern int  errno;
extern int  sys_nerr;
extern char *sys_errlist[];
extern char *sys_siglist[];
