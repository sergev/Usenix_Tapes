# include <stdio.h>
/*
	modcheck -- checks if files were modified since the user last checked

USAGE:
	modcheck

	for each filename, will print corresponding message if the file
	was modified since the user last used the program in this directory
 */
# include <sys/types.h>
# include <sys/stat.h>
# define SYSERR	(-1)


FILE	*checkdesc;
char	checkfile[] = ".modcheck";
char	timecheck[] = ".timecheck";
char	defaultmsg[] = "%s was modified %s\n";
char	usage[] = "USAGE: modcheck (.modcheck and .timecheck are files)";

main(argc,argv)
	int argc;
	char *argv[];
{
	struct stat	fstats;
	register char	*mtime;
	register char	*p;
	char		*msg;
	char		*fname;
	time_t		chktime;
	extern char	*ctime();
	struct	{
		short unsigned us[2];
	};

	if (argc != 1) {
		fprintf(stderr, "%s\n", usage);
		exit(1);
	}

	if((checkdesc=fopen(checkfile, "r")) == NULL) {
		fprintf(stderr, "modcheck: Can't open %s\n", checkfile);
		exit(2);
	}
	if (stat(timecheck,&fstats) == SYSERR) {
		/* first time -- use 1 Jan 1970 as last time */
		chktime = 0;
	} else	{
		chktime = fstats.st_mtime;
	}

	while (newline(&fname,&msg)) {

		if (stat(fname,&fstats) != SYSERR) {
			if(chktime.us[0] < fstats.st_mtime.us[0] ||
			   chktime.us[0] == fstats.st_mtime.us[0] &&
			   chktime.us[1] < fstats.st_mtime.us[1]) {
				if (msg == NULL) msg = defaultmsg;
				mtime = ctime(&chktime);
				for (p = mtime; *p != '\n'; p++);
				*p = '\0';
				fprintf(stdout, msg, fname, mtime);
			}
		} else
			fprintf(stderr,"can't stat %s\n", fname);
	}

	if(freopen(timecheck, "w", stdout) == NULL) {
		fprintf(stderr, "modcheck: can't create %s\n",timecheck);
		exit(3);
	}
	time(&chktime);
	msg = ctime(&chktime);
	fprintf(stdout, "%s",msg);
	return(0);
} /* end of main */


char buffer[128];
newline(fptr,mptr)
	char **fptr, **mptr;
	{/* read the next line of checkdesc */
	/* return 0 on EOF; set fptr (til blank/tab), mptr(til \n,EOF) (nothing->0) */
	register char *p;
	register int c;
	register FILE *iop;

	*mptr = NULL;
	*fptr = p = buffer;
	iop = checkdesc;
	for(;;) {
		switch(c=getc(iop)) {
		case EOF:
			return(0);
		case ' ':
		case '\t':
			if(! *mptr) {
				*p++ = '\0';
				*mptr = p;
				continue;
			}
			break;
		case '\n':
			if(*mptr)
				*p++ = c;
			*p++ = '\0';
			return(1);
			break;
		}
		*p++ = c;
	}
}
