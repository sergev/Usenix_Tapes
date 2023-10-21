#
/*
 *      lpr -- off line print
 *              also known as print
 *
 *      1.1 mike  5/77 - all files copied to spooler space
 *    	(see note below)
 *
 *      1.2 dlm 26 Sep 1977
 *      fix for allowing variable indents (-i flag)
 *
 *      1.3 dlm/njl 12 Dec 1977
 *      fix for open pipe when spawning progess (affects lpd)
 *
 *      1.4 dlm 24 Feb 1978
 *      test for data files; add -C option for classification
 *      and change format of $L card; reversed 1.1 on copies
 *
 *      1.5 dlm 27 Mar 1978
 *      add job name and -h option
 *
 *	2.0 was 21 Dec 1978
 *	-h option changed to -J
 *	added -H option to cause header page to be
 *	  printed, default is no header page
 *	added -p option to cause files to be printed
 *	  using pr
 *	default for classification changed
 *
 *	2.1 was 14 Mar 1979
 *	multiple printer logic changed,
 *	printer number sent to daemon.
 */

char lpr_id[] "~|^`lpr.c:\t2.1\t14 Mar 1979\n";

#include "filemagic.h"
#define FILMOD  0644
#define	DAEMUID	1

char    tfname[]        "/case/lpd0/tfaXXXXX";
char    cfname[]        "/case/lpd0/cfaXXXXX";
char    lfname[]        "/case/lpd0/lfaXXXXX";
char    dfname[]        "/case/lpd0/dfaXXXXX";
int	nact;
int	tff;
int     mailflg, jflag,pflag, prflag;
char	person[10];
int	inchar;
int     maxrec  1000;   /* maximum size in block of a print file */
int     ncopies,bflg,iflag, indent;
char	*printr = "0";	/* /dev/lp0 is default */
int	hdr;
int     uid;
char	class[15] "Case Ecmp Unix";
char    *jobname;

main(argc, argv)
int argc;
char *argv[];
{
	register char *arg;
	int i;
	int c, f, flag;
	int out();
struct inode {
	char    i_minor;  /* +0: minor device of i-node */
	char    i_major;  /* +1: major device */
	int     i_inumber;        /* +2 */
	int     i_flags;  /* +4: see below */
	char    i_nlinks; /* +6: number of links to file */
	char    i_uid;    /* +7: user ID of owner */
	char    i_gid;    /* +8: group ID of owner */
	char    i_size0;  /* +9: high byte of 24-bit size */
	int     i_size1;  /* +10: low word of 24-bit size */
	int     i_addr[8];        /* +12: block numbers or device number */
	int     i_actime[2];      /* +28: time of last access */
	int     i_modtime[2];     /* +32: time of last modification */
} sbuf;
	char *cp, *sp;

	uid = getuid()&0377;
/*      setuid(DAEMUID);        */
	pidfn();
	if((signal(1, 1) & 01) == 0)
		signal(1, out);
	if((signal(2, 1) & 01) == 0)
		signal(2, out);
	if((signal(3, 1) & 01) == 0)
		signal(3, out);
	flag = 0;
	ncopies =1;

	while (argc>1 && (arg = argv[1])[0]=='-') {
		switch (arg[1]) {

		case 'c':
			flag = '+';
			break;

		case 'C':
			if (arg[2])
				i = 2;
			else if (argc >= 2) {
				++argv;
				arg = argv[1];
				argc--;
				i = 0;
			} else break;
			for(cp = class, sp = &arg[i];cp< class+14&& *sp;)
				*cp++ = *sp++;
			*cp = '\0';
			hdr = 1;
			break;

		case 'r':
			flag = '-';
			break;

		case 'm':
			mailflg = 1;
			break;
		case 'J':
			jflag = 1;
			hdr = 1;
			if ( arg[2]) {
				jobname = &arg[2];
				break;
			}
			if (argc>=2) {
				++argv;
				arg = argv[1];
				jobname = &arg[0];
				argc--;
			}
			break;
		case 'b':       /* binary format */
			bflg = 1;
			break;
		case 'i':       /* indent option implies */
			iflag = 1;
			if (arg[2])
				indent = atoi(&arg[2]);
			else
				indent = 8;	/* default indent */
			break;
		case 'p':	/* use pr to print files */
			prflag = 1;
			break;
		case 'P':       /* specifiy remote print */
			printr = &arg[2];
			tfname[9] = cfname[9] = lfname[9] = dfname[9] = *printr;
			break;
		case 'H':	/* we want a header page */
			hdr = 1;
			break;
		default:
			/* -n where n is a string of digits means
			 *  n copies
			 */
			if( (c=arg[1]) >='0'|| c<='9') {
				ncopies = atoi(&arg[1]);
				break;  }
		}
		argc--;
		argv++;
	}
	tff = nfile(tfname);
	if ( jflag == 0) {
		if(argc == 1)
			jobname = &dfname[11];
		else
			jobname = argv[1];
	}
	ident();

	if(argc == 1)
		copy(0, " ");

	while(--argc) {
		arg = *++argv;
	/* test file for reasonableness */
		if(test(arg) == -1 ) continue;
#ifdef COPY
/*mike*/        flag = '+';

/*mike: All files will be copied into the spooler space           */
/*      until the new lpd system is installed with su priveleges. */
/*      Users on the same file system as the                      */
/*      spooler got links to there files for printing.            */
/*      Well if the user did not give the daemon permission to    */
/*      read the file, the daemon could not print it.             */
/*      (The lpd runs with setuid daemon - 4755).                 */
/*      Copying all files provides a quick fix.                   */
/*      Note that some files will now be too large, that would    */
/*      have printed before.                                      */
#endif


		if(flag == '+')
			goto cf;
		if (stat(arg, &sbuf) < 0) {
			printf("lpr:");
			perror(arg);
			continue;
		}
		if((sbuf.i_flags & 04) == 0)  goto cf; /* copy file */
		if(*arg == '/' && flag != '-') {
			for(i=0;i<ncopies;i++)
				if (bflg)
					card('B', arg);
				else if (!prflag)
					card('F', arg);
				else
					card('R', arg);
			nact++;
			continue;
		}
		if(link(arg, lfname) < 0)
			goto cf;
			for(i=0;i<ncopies;i++)
				if (bflg)
					card('B', lfname);
				else if (!prflag)
					card('F', lfname);
				else {
					card('H', arg);
					card('R', lfname);
				}
		card('U', lfname);
		lfname[inchar]++;
		nact++;
		goto df;

	cf:
		f = open(arg, 0);
		if(f < 0) {
			printf("Cannot open %s\n", arg);
			continue;
		}
		copy(f, arg);
		close(f);

	df:
		if(flag == '-') {
			f = unlink(arg);
			if(f < 0)
				printf("Cannot remove %s\n", arg);
		}
	}

	if(nact) {
		tfname[inchar]--;
		f = link(tfname, dfname);
		if(f < 0) {
			printf("Cannot rename %s\n", dfname);
			tfname[inchar]++;
			out();
		}
		unlink(tfname);
		for( f = 0; f<15; close(f++) );
		open("/dev/tty",0);    /* make standard input okay */
		open("/dev/tty",1);    /* make standard output okay */
		execl("/etc/lpd", "lpd", printr, 0);
		dfname[inchar]++;
	}
	out();
}

copy(f, name)
int f;
char name[];
{
	int ff, i, nr, nc;
	static int buf[256];

	for(i=0;i<ncopies;i++)
		if (bflg)
			card('B', cfname);
		else if (!prflag)
			card('F', cfname);
		else {
			card('H', name);
			card('R', cfname);
		}
	card('U', cfname);
	ff = nfile(cfname);
	nc = 0;
	nr = 0;
	while((i = read(f, buf, 512)) > 0) {
		write(ff, buf, i);
		nc =+ i;
		if(nc >= 512) {
			nc =- 512;
			nr++;
			if(nr > maxrec) {
				printf("Copy file is too large\n");
				break;
			}
		}
	}
	close(ff);
	nact++;
}

card(c, s)
int c;
char s[];
{
	char *p1, *p2;
	static char buf[512];
	int col;

	p1 = buf;
	p2 = s;
	col = 0;
	*p1++ = c;
	while((c = *p2++) != '\0') {
		*p1++ = c;
		col++;
	}
	*p1++ = '\n';
	write(tff, buf, col+2);
}

ident()
{
	int c, n;
	register char *b1p, *pp, *b2p;
	static char b1[100], b2[100];
	extern char *itoa();

	b1p = b1;
	if(getpw(uid, b1p)) {
		b1p = "???::::Unknown User::";
	}
	b2p = b2;
	n = 5;
	while(--n) while(*b1p++ != ':');
	while((*b2p++ = *b1p++) != ':');
	b2p[-1] = ':';
	b1p = b1;
	pp = person;
	while((c = *b1p++) != ':') {
		*b2p++ = c;
		*pp++ = c;
	}
	*b2p++ = 0;
	*pp++ = 0;
#ifdef DEBUG
	printf("J%s\n",jobname);
	printf("C%s\n",class);
	printf("L%s\n",b2);
#endif
	if (hdr) {
		card('J',jobname);
		card('C',class);
		card('L', b2);
	}
	if (iflag)
		card('I', itoa(indent));
	if (mailflg)
		card('M', person);
}

pidfn()
{
	register i, j, c;
	int p;

	p = getpid();
	i = 0;
	while(tfname[i] != 'X')
		i++;
	i =+ 4;
	for(j=0; j<5; j++) {
		c = (p%10) + '0';
		p =/ 10;
		tfname[i] = c;
		cfname[i] = c;
		lfname[i] = c;
		dfname[i] = c;
		i--;
	}
	inchar = i;
}

nfile(name)
char *name;
{
	register f;

	f = creat(name, FILMOD);
	if(f < 0) {
		printf("Cannot create %s\n", name);
		out();
	}
	name[inchar]++;
	return(f);
}

out()
{
	register i;

	signal(1, 1);
	signal(2, 1);
	signal(3, 1);
	i = inchar;
	while(tfname[i] != 'a') {
		tfname[i]--;
		unlink(tfname);
	}
	while(cfname[i] != 'a') {
		cfname[i]--;
		unlink(cfname);
	}
	while(lfname[i] != 'a') {
		lfname[i]--;
		unlink(lfname);
	}
	while(dfname[i] != 'a') {
		dfname[i]--;
		unlink(dfname);
	}
	exit();
}

test(file)
char *file;
{
char buf[20];
int  mbuf[20];
int *wd {
	&buf[0]};
	int fd, in;

	if(stat(file, mbuf) < 0) {
		printf("cannot stat %s\n",file);
		return (-1);
	}
	switch(mbuf[2]&060000) {

	case 040000:
		printf("lpr:%s is a directory\n",file);
		return(-1);
	default: ;
	}

	fd = open(file, 0);
	if(fd < 0) {
		printf("lpr:%s:cannot open\n",file);
		return(-1);
	}
	in = read(fd, &buf, 20);
	switch(*wd) {

	case VMAGIC:
	case FMAGIC:
	case NMAGIC:
	case IMAGIC:
	printf("lpr:%s is an executable program and is not printable\n",file);
		goto error1;
	case ARCMAGIC:
	case ARMAGIC:
	case NARMAGIC:
	case NETMAGIC:
	printf("lpr:%s is an archive file and is not printable\n",file);
		goto error1;
	}

/* test for SCCS files under Programmer's Workbench */
	if (buf[0] == 0 && buf[1] == 0142 && buf[2] == 007 )
		{goto error; }

out:
	close(fd);
	return(0);
error:
	printf("lpr:%s is not an ascii file and is unprintable\n",file);
error1:
	close(fd);
	return(-1);
}

/*
 *	itoa - integer to string conversion
 */

char *itoa(i)
register int i;
{
	static char b[10] {"########\0"};
	register char *p;

	p = &b[8];
	do
		*p-- = i%10 + '0';
	while (i /= 10);
	return(++p);
}
