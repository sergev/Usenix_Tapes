/*  acucntrl - turn around tty line between dialin and dialout
 * 
 * Usage:	acucntrl {enable,disable} /dev/ttydX
 *
 * History:
 *	First written by Allan Wilkes (fisher!allan)
 *
 *	Modified June 8,1983 by W.Sebok (astrovax!wls) to poke kernel rather
 * 	than use kernel hack to turn on/off modem control, using subroutine
 *	stolen from program written by Tsutomu Shimomura
 *	{astrovax,escher}!tsutomu
 *
 *	Worked over many times by W.Sebok (i.e. hacked to death)
 *
 *	$Header: acucntrl.c,v 1.3 85/03/29 13:21:59 root Exp $
 *	$Log:	acucntrl.c,v $
 * Revision 1.3  85/03/29  13:21:59  root
 * added parentheses to test for /dev/ttydX.  The operator prececence was
 * not what I thought it was.
 * 
 * Revision 1.2  85/03/17  13:49:44  root
 * 1) fixed infinite loop when a line already disabled was disabled again.
 * 2) fixed data types of modem control flags.  In particular the in the
 * dz driver it was writing out a short when it should have been writing out
 * an int.  Also, it was addressing dmfsoftCAR as a short array when it
 * should have been addressing it as a char array.
 * 3) did various cleanups to enable it to pass lint.
 * 
 * Revision 1.1  85/02/18  13:50:03  root
 * Initial revision
 * 
 *
 * Operation:
 *   disable (i.e. setup for dialing out)
 *	(1) check input arguments
 *	(2) look in /etc/utmp to check that the line is not in use by another
 *	(3) disable modem control on terminal
 *	(4) check for carrier on device
 *	(5) change owner of device to real id
 *	(6) edit /etc/ttys,  changing the first character of the appropriate
 *	    line to 0
 *	(7) send a hangup to process 1 to poke init to disable getty
 *	(8) post uid name in capitals in /etc/utmp to let world know device has
 *	    been grabbed
 *	(9) make sure that DTR is on
 *
 *   enable (i.e.) restore for dialin
 *	(1) check input arguments
 *	(2) look in /etc/utmp to check that the line is not in use by another
 *	(3) make sure modem control on terminal is disabled
 *	(4) turn off DTR to make sure line is hung up
 *	(5) condition line: clear exclusive use and set hangup on close modes
 *	(6) turn on modem control
 *	(7) edit /etc/ttys,  changing the first character of the appropriate
 *	    line to 1
 *	(8) send a hangup to process 1 to poke init to enable getty
 *	(9) clear uid name for /etc/utmp
 */

/* #define SENSECARRIER */

#include <sys/param.h>
#include <sys/buf.h>
#include <signal.h>
#ifdef SIGIO
#	define	BSD4_2		/* how else am I to know? */
#endif
#include <sys/conf.h>
#ifdef BSD4_2
#include "/sys/vaxuba/ubavar.h"
#else
#include <sys/ubavar.h>
#endif
#include <sys/stat.h>
#include <nlist.h>
#include <sgtty.h>
#include <utmp.h>
#include <pwd.h>
#include <stdio.h>

#define NDZLINE	8	/* lines/dz */
#define NDHLINE	16	/* lines/dh */
#define NDMFLINE 8	/* lines/dmf */

#define DZ11	1
#define DH11	2
#define DMF	3

#define NLVALUE(val)	(nl[val].n_value)

struct nlist nl[] = {
#define CDEVSW	0
	{ "_cdevsw" },

#define DZOPEN	1
	{ "_dzopen" },
#define DZINFO	2
	{ "_dzinfo" },
#define NDZ11	3
	{ "_dz_cnt" },
#define DZSCAR	4
	{ "_dzsoftCAR" },

#define DHOPEN	5
	{ "_dhopen" },
#define DHINFO	6
	{ "_dhinfo" },
#define NDH11	7
	{ "_ndh11" },
#define DHSCAR	8
	{ "_dhsoftCAR" },

#define DMFOPEN	9
	{ "_dmfopen" },
#define DMFINFO	10
	{ "_dmfinfo" },
#define NDMF	11
	{ "_ndmf" },
#define DMFSCAR	12
	{ "_dmfsoftCAR" },

	{ "\0" }
};

#define ENABLE	1
#define DISABLE	0

char Etcutmp[] = "/etc/utmp";
char Etcttys[] = "/etc/ttys";
char Devhome[] = "/dev";

char usage[] = "Usage: acucntrl {dis|en}able ttydX\n";

struct utmp utmp;
char resettty, resetmodem;
int etcutmp;
off_t utmploc;
off_t ttyslnbeg;

#define NAMSIZ	sizeof(utmp.ut_name)
#define	LINSIZ	sizeof(utmp.ut_line)

main(argc, argv)
int argc; char *argv[];
{
	register char *p;
	register int i;
	char uname[NAMSIZ], Uname[NAMSIZ];
	int enable ;
	char *device;
	int devfile;
	int uid, gid;
	off_t lseek();
	struct passwd *getpwuid();
	char *rindex();
	extern int errno;
	extern char *sys_errlist[];

	/* check input arguments */
	if (argc!=3) {
		fprintf(stderr, usage);
		exit(1);
	}

	/* interpret command type */
	if (prefix(argv[1], "disable")  || strcmp(argv[1],"dialout")==0)
		enable = 0;
	else if (prefix(argv[1], "enable")  || strcmp(argv[1],"dialin")==0)
		enable = 1;
	else {
		fprintf(stderr, usage);
		exit(1);
	}

	device = rindex(argv[2],'/');
	device = (device == NULL) ? argv[2]: device+1;

	/* only recognize devices of the form ttydX */
	if (strncmp(device,"ttyd",4)!=0) {
		fprintf(stderr,"Bad Device Name %s",device);
		exit(1);
	}

	opnttys(device);

	/* Get nlist info */
	nlist("/vmunix",nl);

	/* Chdir to /dev */
	if(chdir(Devhome) < 0) {
		fprintf(stderr, "Cannot chdir to %s: %s\r\n",
			Devhome, sys_errlist[errno]);
		exit(1);
	}

	/* Get uid information */
	uid = getuid();
	gid = getgid();

	p = getpwuid(uid)->pw_name;
	if (p==NULL) {
		fprintf(stderr,"cannot get uid name\n");
		exit(1);
	}

	/*  to upper case */
	i = 0;
	do {
		uname[i] = *p;
		Uname[i] = (*p>='a' && *p<='z') ? (*p - ('a'-'A')) : *p;
		i++; p++;
	} while (*p && i<NAMSIZ);


	/* check to see if line is being used */
	if( (etcutmp = open(Etcutmp, 2)) < 0) {
		fprintf(stderr,"On open %s open: %s\n",
			Etcutmp,sys_errlist[errno]);
		exit(1);
	}

	(void)lseek(etcutmp,utmploc, 0);

	i = read(etcutmp,(char *)&utmp,sizeof(struct utmp));

	if(
		i == sizeof(struct utmp) &&
		utmp.ut_line[0] != '\0'  &&
		utmp.ut_name[0] != '\0'  &&
		(
			!upcase(utmp.ut_name,NAMSIZ) ||
			(
				uid != 0 &&
				strncmp(utmp.ut_name,Uname,NAMSIZ) != 0
			)
		)
	) {
		fprintf(stderr, "%s in use by %s\n", device, utmp.ut_name);
		exit(2);
	}

	/* Disable modem control */
	if (setmodem(device,DISABLE)<0) {
		fprintf(stderr,"Unable to disable modem control\n");
		exit(1);
	}

	if (enable) {
		if((devfile = open(device, 1)) < 0) {
			fprintf(stderr,"On open of %s: %s\n",
				device, sys_errlist[errno]);
			(void)setmodem(device,resetmodem);
			exit(1);
		}
		/* Try one last time to hang up */
		if (ioctl(devfile,(int)TIOCCDTR,(char *)0) < 0)
			fprintf(stderr,"On TIOCCDTR ioctl: %s\n",
				sys_errlist[errno]);

		if (ioctl(devfile, (int)TIOCNXCL,(char *)0) < 0)
			fprintf(stderr,
			    "Cannot clear Exclusive Use on %s: %s\n",
				device, sys_errlist[errno]);

		if (ioctl(devfile, (int)TIOCHPCL,(char *)0) < 0)
			fprintf(stderr,
			    "Cannot set hangup on close on %s: %s\n",
				device, sys_errlist[errno]);

		i = resetmodem;

		if (setmodem(device,ENABLE) < 0) {
			fprintf(stderr,"Cannot Enable modem control\n");
			(void)setmodem(device,i);
			exit(1);
		}
		resetmodem=i;

		if (settys(ENABLE)) {
			fprintf(stderr,"%s already enabled\n",device);
		} else {
			pokeinit(device,Uname,enable);
		}
		post(device,"");

	} else {
#if defined(TIOCMGET) && defined(SENSECARRIER)
		if (uid!=0) {
			int linestat = 0;

			/* check for presence of carrier */
			sleep(2); /* need time after modem control turnoff */

			if((devfile = open(device, 1)) < 0) {
				fprintf(stderr,"On open of %s: %s\n",
					device, sys_errlist[errno]);
				(void)setmodem(device,resetmodem);
				exit(1);
			}

			(void)ioctl(devfile,TIOCMGET,&linestat);

			if (linestat&TIOCM_CAR) {
				fprintf(stderr,"%s is in use (Carrier On)\n",
					device);
				(void)setmodem(device,resetmodem);
				exit(2);
			}
			(void)close(devfile);
		}
#endif TIOCMGET
		/* chown device */
		if(chown(device, uid, gid) < 0)
			fprintf(stderr, "Cannot chown %s: %s\n",
				device, sys_errlist[errno]);


		/* poke init */
		if(settys(DISABLE)) {
			fprintf(stderr,"%s already disabled\n",device);
		} else {
			pokeinit(device,Uname,enable);
		}
		post(device,Uname);
		if((devfile = open(device, 1)) < 0) {
			fprintf(stderr, "On %s open: %s\n",
				device, sys_errlist[errno]);
		} else {
			if(ioctl(devfile, (int)TIOCSDTR, (char *)0) < 0)
				fprintf(stderr,
				    "Cannot set DTR on %s: %s\n",
					device, sys_errlist[errno]);
		}
	}

	exit(0);
}

/* return true if no lower case */
upcase(str,len)
register char *str;
register int len;
{
	for (; *str, --len >= 0 ; str++)
		if (*str>='a' && *str<='z')
			return(0);
	return(1);
}

/* Post name to public */
post(device,name)
char *device, *name;
{
	(void)time((time_t *)&utmp.ut_time);
	strcpyn(utmp.ut_line, device, LINSIZ);
	strcpyn(utmp.ut_name, name,  NAMSIZ);
	if (lseek(etcutmp, utmploc, 0)<0)
		fprintf(stderr,"on lseek in /etc/utmp: %s",
			sys_errlist[errno]);
	if (write(etcutmp, (char *)&utmp, sizeof(utmp))<0)
		fprintf(stderr,"on write in /etc/utmp: %s",
			sys_errlist[errno]);
}
	
/* poke process 1 and wait for it to do its thing */
pokeinit(device,uname,enable)
char *uname, *device; int enable;
{
	struct utmp utmp;
	register int i;

	post(device, uname);

	/* poke init */
	if (kill(1, SIGHUP)) {
		fprintf(stderr,
		    "Cannot send hangup to init process: %s\n",
			sys_errlist[errno]);
		(void)settys(resettty);
		(void)setmodem(device,resetmodem);
		exit(1);
	}

	if (enable)
		return;

	/* wait till init has responded, clearing the utmp entry */
	i=100;
	do {
		sleep(1);
		if (lseek(etcutmp,utmploc,0)<0)
			fprintf(stderr,"On lseek in /etc/utmp: %s",
				sys_errlist[errno]);
		if (read(etcutmp,(char *)&utmp,sizeof utmp)<0)
			fprintf(stderr,"On read from /etc/utmp: %s",
				sys_errlist[errno]);
	} while (utmp.ut_name[0] != '\0' && --i > 0);
}

/* identify terminal line in ttys */
opnttys(device)
char *device;
{
	register FILE *ttysfile;
	register int  ndevice, lnsiz; 
	register char *p;
	char *index();
	char linebuf[100];

	ttysfile = fopen(Etcttys, "r");
	if(ttysfile == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", Etcttys,
			sys_errlist[errno]);
		exit(1);
	}

	ndevice = strlen(device);
	ttyslnbeg = 0;
	utmploc = 0;

	while(fgets(linebuf, sizeof(linebuf) - 1, ttysfile) != NULL) {
		utmploc += sizeof(utmp);
		lnsiz = strlen(linebuf);
		if ((p = index(linebuf,'\n')) != NULL)
			*p = '\0';
		if(strncmp(device, &linebuf[2], ndevice) == 0) {
			(void)fclose(ttysfile);
			return;
		}
		ttyslnbeg += lnsiz;
	}
	fprintf(stderr, "%s not found in %s\n", device, Etcttys);
	exit(1);
}

/* modify appropriate line in /etc/ttys to turn on/off the device */
settys(enable)
int enable;
{
	int ittysfil;
	char out,in;

	ittysfil = open(Etcttys, 2);
	if(ittysfil == NULL) {
		fprintf(stderr, "Cannot open %s for output: %s\n",
			Etcttys, sys_errlist[errno]);
		exit(1);
	}
	(void)lseek(ittysfil,ttyslnbeg,0);
	if(read(ittysfil,&in,1)<0) {
		fprintf(stderr,"On %s write: %s\n",
			Etcttys, sys_errlist[errno]);
		exit(1);
	}
	resettty = (in == '1');
	out = enable ? '1' : '0';
	(void)lseek(ittysfil,ttyslnbeg,0);
	if(write(ittysfil,&out,1)<0) {
		fprintf(stderr,"On %s write: %s\n",
			Etcttys, sys_errlist[errno]);
		exit(1);
	}
	(void)close(ittysfil);
	return(in==out);
}

/*
 * Excerpted from (June 8,1983 W.Sebok)
 * > ttymodem.c - enable/disable modem control for tty lines.
 * >
 * > Knows about DZ11s and DH11/DM11s.
 * > 23.3.83 - TS
 * > modified to know about DMF's  (hasn't been tested) Nov 8,1984 - WLS
 */


setmodem(ttyline,enable)
char *ttyline; int enable;
{
	dev_t dev;
	int kmem;
	int unit,line,nlines,addr,tflags;
	int devtype=0;
	char cflags; short sflags;
#ifdef BSD4_2
	int flags;
#else
	short flags;
#endif
	struct uba_device *ubinfo;
	struct stat statb;
	struct cdevsw cdevsw;

	if(nl[CDEVSW].n_type == 0) {
		fprintf(stderr,"No namelist.\n");
		return(-1);
	}

	if((kmem = open("/dev/kmem", 2)) < 0) {
		fprintf(stderr,"/dev/kmem open: %s\n", sys_errlist[errno]);
		return(-1);
	}

	if(stat(ttyline,&statb) < 0) {
		fprintf(stderr,"%s stat: %s\n", ttyline, sys_errlist[errno]);
		return(-1);
	}

	if((statb.st_mode&S_IFMT) != S_IFCHR) {
		fprintf(stderr,"%s is not a character device.\n",ttyline);
		return(-1);
	}

	dev = statb.st_rdev;
	(void)lseek(kmem,
		(off_t) &(((struct cdevsw *)NLVALUE(CDEVSW))[major(dev)]),0);
	(void)read(kmem,(char *) &cdevsw,sizeof cdevsw);

	if((int)(cdevsw.d_open) == NLVALUE(DZOPEN)) {
		devtype = DZ11;
		unit = minor(dev) / NDZLINE;
		line = minor(dev) % NDZLINE;
		addr = (int) &(((int *)NLVALUE(DZINFO))[unit]);
		(void)lseek(kmem,(off_t) NLVALUE(NDZ11),0);
	} else if((int)(cdevsw.d_open) == NLVALUE(DHOPEN)) {
		devtype = DH11;
		unit = minor(dev) / NDHLINE;
		line = minor(dev) % NDHLINE;
		addr = (int) &(((int *)NLVALUE(DHINFO))[unit]);
		(void)lseek(kmem,(off_t) NLVALUE(NDH11),0);
	} else if((int)(cdevsw.d_open) == NLVALUE(DMFOPEN)) {
		devtype = DMF;
		unit = minor(dev) / NDMFLINE;
		line = minor(dev) % NDMFLINE;
		addr = (int) &(((int *)NLVALUE(DMFINFO))[unit]);
		(void)lseek(kmem,(off_t) NLVALUE(NDMF),0);
	} else {
		fprintf(stderr,"Device %s (%d/%d) unknown.\n",ttyline,
		    major(dev),minor(dev));
		return(-1);
	}

	(void)read(kmem,(char *) &nlines,sizeof nlines);
	if(minor(dev) >= nlines) {
		fprintf(stderr,"Sub-device %d does not exist (only %d).\n",
		    minor(dev),nlines);
		return(-1);
	}

	(void)lseek(kmem,(off_t)addr,0);
	(void)read(kmem,(char *) &ubinfo,sizeof ubinfo);
	(void)lseek(kmem,(off_t) &(ubinfo->ui_flags),0);
	(void)read(kmem,(char *) &flags,sizeof flags);

	tflags = 1<<line;
	resetmodem = ((flags&tflags) == 0);
	flags = enable ? (flags & ~tflags) : (flags | tflags);
	(void)lseek(kmem,(off_t) &(ubinfo->ui_flags),0);
	(void)write(kmem,(char *) &flags, sizeof flags);
	switch(devtype) {
		case DZ11:
			if((addr = NLVALUE(DZSCAR)) == 0) {
				fprintf(stderr,"No dzsoftCAR.\n");
				return(-1);
			}
			cflags = flags;
			(void)lseek(kmem,(off_t) &(((char *)addr)[unit]),0);
			(void)write(kmem,(char *) &cflags, sizeof cflags);
			break;
		case DH11:
			if((addr = NLVALUE(DHSCAR)) == 0) {
				fprintf(stderr,"No dhsoftCAR.\n");
				return(-1);
			}
			sflags = flags;
			(void)lseek(kmem,(off_t) &(((short *)addr)[unit]),0);
			(void)write(kmem,(char *) &sflags, sizeof sflags);
			break;
		case DMF:
			if((addr = NLVALUE(DMFSCAR)) == 0) {
				fprintf(stderr,"No dmfsoftCAR.\n");
				return(-1);
			}
			cflags = flags;
			(void)lseek(kmem,(off_t) &(((char *)addr)[unit]),0);
			(void)write(kmem,(char *) &flags, sizeof cflags);
			break;
		default:
			fprintf(stderr,"Unknown device type\n");
			return(-1);
	}
	return(0);
}

prefix(s1, s2)
	register char *s1, *s2;
{
	register char c;

	while ((c = *s1++) == *s2++)
		if (c == '\0')
			return (1);
	return (c == '\0');
}
