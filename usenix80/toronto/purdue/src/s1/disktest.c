#
/*  this program is a generalized diagnostic for any random access */
/* 	or sequential device running under unix. */
/* 		argument is device name */

/* 		Bill Allen */
/* 		Naval Postgraduate School */
/* 		October, 1975 */

/*
 * The variable buffer allocation will not work with
 * v6 C compiler.. ok on V7.
 * --ghg
 */

#define EVER (;;)

char *dev;	/* device name, argv[1] */
int abort, iflag;	/* iflag; interrupt flag*/
int stim[2],etim[2],tranf;
int upat;	/* user specified pattern */
char	*bn;		/* current block number */
int	r;		/* current test number */
int fflag 0;
int  tfd;
#define NUMPAT 70
int pat[NUMPAT]
	{ 0,-1,0125252,0052525,070707,0107070,
	0165555,0133333,0153333,0177776,0177775,0177773,0177767,0177757,
	0177737,0177677,0177577,0177377,0176777,0175777,0173777,0167777,
	0157777,0137777,0077777,0000001,0000002,0000004,0000010,0000020,
	0000040,0000100,0000200,0000400,0001000,0002000,0004000,0010000,
	0020000,0040000,0100000,0000003,0000007,0000017,0000037,0000077,
	0000177,0000377,0000777,0001777,0003777,0007777,0017777,0037777,
	0077777,0177776,0177774,0177770,0177760,0177740,0177700,0177600,
	0177400,0177000,0176000,0174000,0170000,0160000,0140000,0100000
	};

char line[80];	/* terminal input buffer */
char *lp;

int	lowbn;		/* low address to test */
int	highbn;		/* high address to test */
int repeat	1;		/* test repetition factor */
int	patf	0;		/* pattern flag */
int	outf	1;		/* output flag */
int	seqf	0;		/*  sequential-random test flag */
int	abtf	1;		/* abort test on error */
int	cmpf	0;		/* compare data read */
int	fout;
int blksiz 1, ublksiz;
int ulowbn 0, uhighbn 0;
extern char end;
static int *buffer &end;


int tvec[2];

main(argc,argv)
char  **argv;
{

	int quit();
	int rubout();

	fout = dup(1);
	dev = argv[1];
	signal(3,quit);		/* catch quit signal */
	signal(2,rubout);	/* catch rubout */
	if(argc<2){
		printfb("Specify device to test\n");
		exit();
	}
	tfd = open(dev,2);	/* open test device */
	if(tfd<0){
		printfb("unable to open %s\n",dev);
		exit();
	}
	if (setblk (blksiz) == -1){
		printfb("can't allocate buffer\n");
		exit();
	}

	time(tvec);
	srand(tvec[1]);		/* init random number generator */
	while(1)
		scommand();
}


/* read and execute a command */

scommand()
{
	char *getnum();

	flush();
	lp = &line[0];
	while ((*lp = getchar()) != '\n')
		if (*lp++ == -1)
			exit();

	lp = &line[0];
	abort = 0;	/* abort the test flag */

	switch(*lp++) {

	case 'l':		/* set low address */
		lowbn = ((ulowbn=getnum()) + blksiz-1) / blksiz * blksiz;
		break;

	case 'h':		/* set high address */
		highbn = (((uhighbn=getnum()) + blksiz) / blksiz * blksiz) - 1;
		break;

	case 't':		/* set repetition factor */
		repeat = getnum();
		break;

	case 'p':		/* pattern flag */
		patf = getnum();
		if(patf == 2)
			upat = getnum();
		break;

	case 'o':		/* output flag */
		outf = getnum();
		break;

	case 'c':
		cmpf = getnum();	/* compare flag */
		break;

	case 'a':
		abtf = getnum();	/* abort flag */
		break;

	case 'b':
		if ((ublksiz=getnum()) == 0)
			ublksiz = 1;
		if (setblk(ublksiz) != -1)
			blksiz = ublksiz;
		else
printfb("can't allocate blocks-old block size still in effect\n");
	printfb("block size: %l\n",blksiz);
	printfb("low block number: %l\n",lowbn);
	printfb("high block number: %l\n",highbn);
		break;

	case 'd':		/* display all the modes */
		display();
		break;

	case 'q':		/* quit */
		exit();

	case 'g':		/* execute the test */
		if (fflag)
			if (forks() == -1)
				return;
		time(stim);	/* get time test started */
		for(r=0; r<repeat; r++) {
			writpat();
			if(abort)  return;
			readpat();
			if(abort)  return;
		}
		printfb("test completed on %s\n", dev);
		tranf = (highbn-lowbn)*2;	/* number of disk transfers */
		rptim();					/* test statistics */
		break;

	case 'w':	/* write test */
		if (fflag)
			if (forks() == -1)
				return;
		time(stim);
		for(r=0; r<repeat; r++) {
			writpat();
			if(abort)  return;
		}
		printfb("write test completed on %s\n", dev);
		tranf = highbn - lowbn;
		rptim();
		break;

	case 'r':	/* read test */
		if (fflag)
			if (forks() == -1)
				return;
		time(stim);
		for(r=0; r<repeat; r++) {
			readpat();
			if(abort) return;
		}
		printfb("read test completed on %s\n", dev);
		tranf = highbn - lowbn;
		rptim();
		break;

	case 's':		/* set sequential-random flag */
		seqf = getnum();
		break;

	case 'f':
		fflag = getnum();
		break;
	default:
		printfb("Unrecognized command\n");
		return;

	}
}

/* get an octal or decimal number from input string */
char *getnum()
{

	register mode,num;

	while(*lp == ' ') lp++;	/* skip blanks */
	mode = 10;
	num = 0;
	if(*lp == '0')		/* octal */
		mode = 8;
	while(*lp != '\n' && *lp != ' ') {
		if(*lp<'0' || *lp>((mode==8)?'7':'9')) {
			printfb("invalid numeric argument\n");
			return(0);
		}
		num = num*mode + (*lp++ - '0');
	}
	return(num);
}

/* display all the flags */

display()
{

	printfb("block size: %l\n",blksiz);
	printfb("low block number: %l\n",lowbn);
	printfb("high block number: %l\n",highbn);
	printfb("repeat factor: %l\n",repeat);
	if(seqf)
		printfb("random access\n");
	else
		printfb("sequential access\n");
	if(outf)
		printfb("output normal\n");
	else
		printfb("output errors only\n");
	if (abtf)
		printfb("abort on error\n");
	else
		printfb("continue on error\n");
	if (cmpf)
		printfb("data compare on read\n");
	else
		printfb("no data compare on read\n");
	if (fflag)
		printfb("fork on next test\n");
	else
		printfb("program interactive\n");

	switch(patf) {

	case 0:
		printfb("pattern: default set\n");
		break;

	case 1:
		printfb("pattern: block number\n");
		break;

	case 2:
		printfb("pattern: %.1o\n",upat);
		break;

	default:
		printfb("invalid pattern flag\n");
		exit();
	}
}

/*  do the test */
writpat()
{

	register tpat,i,temp;
	int j;

	rewind();
	if(outf) {
		if(patf != 1)
			printfb("%s - write pass %l pattern: %.1o\n",dev,r,getpat());
		else
			printfb("%s - write pass %l pattern: block number\n",dev,r);
	}
	if(seek(tfd,lowbn,3)<0) {
		printfb("unable to seek to block %l\n",lowbn);
		return;
	}
	tpat = getpat();
	for(i=0; i<256*blksiz; i++)
		buffer[i] = tpat;
	for(bn=lowbn; bn!=(highbn+1); bn =+ blksiz) {
		if(abort) return;
		if(write(tfd,buffer,512*blksiz) != 512*blksiz){
			printfb("%s - write error block %l pattern %.1o\n",dev,bn,tpat);
			if (abtf)
				quit();
		}
	}
}

readpat()
{

	register tpat,i,temp;
	int eflag, ecount, eword;
	int j;

	rewind();
	if(outf) {
		if(patf != 1)
			printfb("%s - read pass %l pattern: %.1o\n",dev,r,getpat());
		else
			printfb("%s - read pass %l pattern: block number\n",dev,r);
	}
	if(seek(tfd,lowbn,3)<0) {
		printfb("unable to seek to block %l\n",lowbn);
		return;
	}
	for(j=lowbn; j!=(highbn+1); j =+ blksiz) {
		if(abort)  return;
		if(seqf) {	/* random access */
again:
			temp = rand();
			bn = lowbn + (temp%(highbn-lowbn));
			if (bn+blksiz-1 > highbn)
				goto again;
			if(seek(tfd,bn,3)<0) {
				printfb("seek error block %l\n",bn);
				continue;
			}
		}
		else
			bn = j;
		tpat = getpat();
		if(read(tfd,buffer,512*blksiz) != 512*blksiz) {
			printfb("%s - read error block %l\n",dev,bn);
			if (abtf)
				quit();
			continue;
		}
		ecount=1; eflag=0;
		if (cmpf) {
			for(i=0; i<256*blksiz; i++) {
				if(buffer[i] != tpat) {
					if(eflag==0) {
						if(ecount>1)
							printfb("%s - compare error count %l\n",dev,ecount);
	newerr:
						eflag++;
						ecount = 1;
						eword = buffer[i];
						printfb("%s - compare error block %l word %l pattern %.1o bad %.1o\n",
						dev,bn,i,tpat,buffer[i]);
					}
					else {
						if(eword==buffer[i])
							ecount++;
						else
							goto newerr;
					}
				}
			}	/* endfor */
		}
		if(ecount>1)
			printfb("%s - compare error count %l\n",dev,ecount);
		if (eflag && abtf)
			quit();
	}
	/* seek to low block and read in so write routine won't */
	/* fill up the system buffer pool while device is rewinding */
	/* ghg 6/14/77 */
/*
	seek(tfd,lowbn,3);
	read(tfd,buffer,512*blksiz);
*/
/* .................... */
}

/*  get a pattern for the current block */
getpat()
{

	switch(patf) {

	case 0:		/* default set */
		return(pat[r%NUMPAT]);

	case 1:		/* block number */
		return(bn);

	case 2:
		return(upat);

	}
}

quit()
{
	signal(3,quit);
	printfb("test aborted on %s\n", dev);
	abort++;
	iflag++;
}

rubout()
{
	signal(2,1);
	printfb("%s - pass %l block %l pattern %.1o\n",dev,r,bn,getpat());
	signal(2,rubout);
	iflag++;
	return;
}

rptim()
{

	time(etim);		/* test end time */
	printfb("%s - %l passes with %l transfers per pass in %l seconds\n",
		dev,repeat,tranf/blksiz+1,etim[1]-stim[1]);
}
printfb(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc)
{

	printf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc);
	flush();
}
getchar()
{
char c;
	for EVER{
		iflag=0;
		if (read(0,&c,1) <= 0){
			if (!iflag)
				return(-1);
			continue;
		}
		return(c);
	}
}
setblk(nb)
/* sets new blksiz, resets lo & high  correspondingly */
{
	register nbrk;
	if ((nbrk = (&end + 512 * nb)) <= &end)
		return(-1);
	if ((nbrk - &end)/512 != nb)
		return(-1);
	if (brk (nbrk) == -1)
		return(-1);

	lowbn = (ulowbn + nb-1) / nb * nb;
	highbn = ((uhighbn + nb) / nb * nb) - 1;
	return(0);
}
forks(){

	switch (fork()){
		case -1:
			printfb("Can't fork\n");
			return(-1);
		case 0:
			close(0);
			return(0);
		default:
			exit();
	}
}
rewind()
{

	close(tfd);	/* rewind magtape */
	tfd = open(dev,2);	/* open test device */
	if(tfd<0){
		printfb("unable to open %s\n",dev);
		exit();
	}
}
