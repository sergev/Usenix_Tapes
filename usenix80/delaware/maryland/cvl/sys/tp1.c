#include "tp.h"
/*	c-version of tp?.s
 *
 *	M. Ferentz
 *	August 1976
 *
 *      2.1 dlm  14 Dec 1977
 *      Change of format of tv option to allow the printing of
 *      information about large sized files and large block
 *      numbers
 *
 *      2.2 dlm  14 Jun 1978
 *      Allow extracts with directories in pathname and if
 *      not exist, make required directories
 *
 *      2.3 dlm  20 Jun 1978
 *      new flag 'h' use ht instead of mt (1600bpi vice 800bpi)
 */

char	tc[]	"/dev/tapx";
char	mt[]	"/dev/mt0";
char    smt[]   "/dev/smt0";
int     flags   020;    /* default is flu */

main(argc,argv)
char **argv;
{
	register char c, *ptr;
	extern cmd(), cmr(),cmx(), cmt();

	command = &cmr;
	if ((narg = rnarg = argc) < 2)	narg = 2;
	else {
		ptr = argv[1];	/* get first argument */
		parg = &argv[2];	/* pointer to second argument */
		while (c = *ptr++) switch(c)  {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			tc[8] = c;
			mt[7] = c;
			smt[8] = c;
			continue;
		case 'c':
			flags =| flc;  
			continue;
		case 'd':
			setcom(&cmd);  
			continue;
		case 'f':
			flags =| flf;  
			continue;
		case 'i':
			flags =| fli;  
			continue;
		case 'h':   /* means use /dev/htx */
			flags =| flm;  
			mt[5] = 'h';
			continue;
		case 'm':
			flags =| flm;  
			continue;
		case 'r':
			flags =& ~flu;  
			setcom(&cmr);  
			continue;
		case 's':       /* Special magtape (R.C.S.) */
			flags =| fls;
			continue;
		case 't':
			setcom(&cmt);  
			continue;
		case 'u':
			flags =| flu;  
			setcom(&cmr);  
			continue;
		case 'v':
			flags =| flv;  
			continue;
		case 'w':
			flags =| flw;  
			continue;
		case 'x':
			setcom(&cmx);  
			continue;
		default:
			useerr();
		}
	}
	/**/
	optap();
	/* Temporary change
		nameblk = nptr = sbrk(0);		/* establish top */
	nptr = nameblk = &nameb;
	top = &(nameb.nameb2[sizeof nameb.nameb2]);
	(*command)();
}

optap()
{
	extern cmr();

	if ((flags & (flm | fls)) == 0) {       /*  DECTAPE */
		tapsiz = 578;
		ndirent = 192;
		fio =open(tc,2);
	} 
	else {                  /* MAGTAPE (or special MAGTAPE (R.C.S.)) */
		tapsiz = 32767;
		ndirent = MDIRENT;
		if (command == &cmr) {
			if(flags & flm)
				fio = open(mt,1);
			else    /* Special magtape (R.C.S.) */
				fio = open(smt,1);
		}
		else {
			if(flags & flm)
				fio = open(mt,0);
			else    /* Special magtape (R.C.S.) */
				fio = open(smt,0);
		}
	}
	if (fio < 0)  {
		printf("tp:%s or %s:No such device\n",mt,smt);
		done();
	}
	ndentd8 = ndirent>>3;
	edir = &dir[ndirent];
}

setcom(newcom)
{
	extern cmr();

	if (command != &cmr)  	useerr();
	command = newcom;
}

useerr()
{
	printf("Bad usage\n");
	done();
}

/*/* COMMANDS */

cmd()
{
	extern delete();

	if (flags & (flm|fls|flc|flf))  useerr();   /* Special magtape addition (R.C.S.) */
	if (narg <= 2)			useerr();
	rddir();
	gettape(&delete);
	wrdir();
	check();
}

cmr()
{
	if (flags & (flc|flm|fls))      clrdir();   /* Special... */
	else				rddir();
	getfiles();
	update();
	check();
}

cmt()
{
	extern taboc();

	if (flags & (flc|flf|flw))	useerr();
	rddir();
	if (flags & flv)
		printf("   mode    uid gid tapa    size     date    time   name\n");
	gettape(&taboc);
	check();
}

cmx()
{
	extern extract();

	if (flags & (flc|flf))		useerr();
	rddir();
	gettape(&extract);
	done();
}

check()
{
	usage();
	done();
}

done()
{
	printf("End\n");
	exit();
}

encode(pname,dptr)	/* pname points to the pathname
			 * nptr points to next location in nameblk
			 * dptr points to the dir entry		   */
{
	register  char *np, *sp;

	sp = pname;
	dptr->d_namep = np = nptr;
	if (np > top - 32)  {
		printf("Out of core\n");
		done();
	}
	while (*np++ = *sp++);
	if (np <= nptr + 32)  nptr = np;
	else {
		printf("Pathname too long - %s\nFile ignored\n",nptr);
		clrent(dptr);
	}
}

decode(pname,dptr)	/* dptr points to the dir entry
			 * name is placed in pname[] */
{

	register char *p1, *p2;

	p1 = pname;
	p2 = dptr->d_namep;
	while (*p1++ = *p2++);
}
