#include <stdio.h>

#define	DFLTRECSIZE	5120
#define DFLTOFFSET	2048
#define MAXRECSIZE (5120*4)
#define SLEEPEVERY 10

char	buf[MAXRECSIZE];
char	pattern[MAXRECSIZE];
int	Recsize = DFLTRECSIZE;
struct flags {
	int slow;
	int verbose;
} Flags;
int Tapefd, Seed;
char *drivename = "/dev/rmt0";	/* default drive name */


genpat(seed)
{
	register int i = Recsize;
	register char *patp = pattern;
	register char *p = buf;

	seed += Seed;
	while(--i) {
		*patp++ = seed++ /* & 0377  */  ;
		*p++    = 0;
	}
	patp = pattern;
	strncpy(patp,"tptest",6);
}

check()
{
	register int i = Recsize;
	register char *patp = pattern;
	register char *p = buf;

	while(--i)
		if (*patp++ != *p++)
			return (-1);
	return (0);
}


main(argc,argv)
int argc;
char **argv;
{
	int nrecords;
	int offset = DFLTOFFSET;
	long time();


	if (getuid()) nice(20);
	close(2);
	dup(1);
	setbuf(stdout, NULL);	/* both stdout and stderr go to same place
				   and are unbuffered */

	printf("pid %d\n",getpid());
	while (argc > 1 && argv[1][0]=='-') {
		switch(argv[1][1]) {
		case 'n':
			Flags.slow++;
			break;
		case 'v':
			Flags.verbose++;
			break;
		case 'r':
			Recsize = atoi(&argv[1][2]);
			if (Recsize <= 0 || Recsize > MAXRECSIZE) {
				printf("recsize %d too large\n", Recsize);
				exit(1);
			}
			break;
		case 'o':
			offset = atoi(&argv[1][2]);
			if (offset <= 0 || offset > MAXRECSIZE) {
				printf("offset %d too large\n",	offset);
				exit(1);
			}
			if (offset > DFLTRECSIZE)
				printf("offset %d is rather large\n", offset);

			break;
		case 'f':
			drivename = &argv[1][2];
			break;
		}
		argv++, argc--;
	}


	printf("slow=%d, Recsize=%d, offset for pass2 = %d\n",
		Flags.slow, Recsize, offset);

	printf("Pass 1:\n");
	Seed = (int)time((char *)0);
	if (Flags.verbose)
		printf("Seed=%d\n", Seed);
	tapeopen(1, 0);
	nrecords = tapewrite(0);	/* test with an offset of 0 */
	tapeopen(0, 10);
	taperead(0, nrecords);
	printf("Finished pass 1\n");

	tapeopen(1, 10);
	printf("Beginning pass 2\n");
	Seed = (int)time((char *)0);
	if (Flags.verbose)
		printf("Seed=%d\n", Seed);
	nrecords = tapewrite(offset);	
	tapeopen(0, 10);
	taperead(offset, nrecords);
	printf("finished pass 2\nDone, no errors\n");
	newseqnumber();
	exit(0);

}

tapewrite(offset)
{
	int nrecords, n;
	long tplength = 0;

	printf("Writing...\n");
	if (Flags.verbose) printf("offset=%d\n", offset);
	if (offset)
		write(Tapefd, buf, offset); /* who cares what buf contains? */

	for(nrecords=0;;nrecords++) {
		genpat(nrecords);
		if ((n=write (Tapefd,pattern,Recsize))!=Recsize) {
			if (n == -1)
				perror("Tape write error (presumably EOT)");
				/* this isn't fatal, really...*/
			break;
		}
		if (Flags.verbose)
			printf("w");
		tplength += Recsize;
		if (Flags.slow && ((nrecords%SLEEPEVERY) == 0)) {
			sleep(Flags.slow);
			if (Flags.verbose) printf("s");
		}
	}

	printf ("%d records, unformatted tape length = %ld bytes\n",
			nrecords, tplength);
	printf("Finished writing; Rewinding...\n");
	close (Tapefd);
	return(nrecords);
}

taperead(offset, nrecords)
{
	register int r2, n;
	printf ("Reading ...\n");
	if (Flags.verbose)
		printf("offset=%d, nrecords=%d\n", offset, nrecords);
	
	if (offset)
		read(Tapefd, buf, offset);
	for (r2=0;r2<nrecords;r2++) {
		genpat(r2);
		if ((n=read(Tapefd,buf,Recsize)) != Recsize) {
			if (n == -1)
				perror("Read error");
			else
				printf ("recsiz error, record %d;", r2),
				printf ("actual=%d, expected=%d\n", n, Recsize);
			exit(3);
		}
		if (Flags.verbose) printf("r");
		if (check()<0) {
			printf ("data error: record %d\n", r2);
			exit(4);
		}
		if (Flags.slow && ((r2%SLEEPEVERY) == 0)) {
			sleep(Flags.slow);
			if(Flags.verbose) printf("s");
		}
	}

	printf("Done reading; Rewinding...\n");
	close(Tapefd);
}

newseqnumber()
{
	FILE *seqfd;
	int seqnum;

	if ((seqfd=fopen ("/usr/lib/tapeseq","r+")) == NULL) {
		printf ("warning: creating new sequence file\n");
		umask(000);
		close(creat ("/usr/lib/tapeseq",0666));
		seqfd = fopen ("/usr/lib/tapeseq","r+");
		fprintf(seqfd, "0");
		rewind(seqfd);
	}

	fscanf (seqfd,"%d",&seqnum);
	rewind(seqfd);
	fprintf(seqfd,"%d\n",seqnum+1);
	printf("Sequence number = %d\n", seqnum+1);

	fclose (seqfd);
}

tapeopen(mode, sleeptime)
{
	if (Flags.verbose)
		printf("mode=%d, sleeptime=%d\n", mode, sleeptime);

	if (sleeptime==0) {	/* means die if can't open immediately */
		Tapefd = open (drivename, mode);
		if (Tapefd == -1) {
			perror("Can't open tape drive");
			exit(2);
		} else
			return;
	}

	while ((Tapefd=open(drivename, mode)) < 0) {
		if(Flags.verbose) printf("s");
		sleep(sleeptime);
	}

	return;
}
