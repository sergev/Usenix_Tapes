#
/*
 * pip -- read/write DEC/DOS format mag tape files.
 *
 * pip command unit [options] [files...]
 *
 *	command:	get:	read from tape --> unix file
 *			in:	 "    "    "        "    "
 *			put:	write files onto tape
 *			out:	  "     "     "    "
 *			cat:	print catalog of the tape
 * 			dir:	  "      "    
 * 			ls:	  "      "    
 *
 * options:
 *
 *	-v:	verbose -- print each name as it is copied out
 *
 *	-a:	(input only) read ALL files on the tape.
 *		(File names listed on the command line are then
 *		ignored.)
 *
 *	-u:	convert output chars to upper case.
 *
 *	-l:	convert output chars to lower case.
 *
 *	-t:	trim trailing nulls from each input record.
 *
 * 	unit:	the single digit unit number of the tape drive to use
 *
 *	files:	a list of unix files.
 */

/*
 * our TM11 tape driver recognizes
 * the following to write an end of
 * file on the tape.
 */

#define WEOF(fd)	write(fd,0,0)

/*
 * function codes:
 */
#define CAT 01
#define OUT 02
#define IN 03


struct {
	int fname[2];	/* rad50 */
	int ext;	/* rad50 */
	int uid;	/* [prog,ppn] */
	char prot;
	char junk;
	int date;
	int unused;
} dosh, dosh2;

int vflg;
int tflg;
int uflg;
int lflg;
int aflg;
int tape;
int op;

char tname[] "/dev/rmt?";

struct {
	char *com;
	int code;
} coms[] {
	"get",	IN,
	"in",	IN,
	"put",	OUT,
	"out",	OUT,
	"cat",	CAT,
	"ls",	CAT,
	"di",	CAT,
	0,0
};

main(argc, argp)
char **argp;
{
	register i;

	argp++;
	if(argc <= 1){
		help();
	} else {
		argc--;
		op = 0;
		for(i=0; coms[i].com; i++){
			if(strcmp(coms[i].com, *argp) == 0){
				op = coms[i].code;
				break;
			}
		}
		if(op == 0){
			printf("Illegal command: %s\n", *argp);
			help();
		}
		argp++;
		tname[8] = **argp;
		argp++;
		argc--;
	}
	while(argc > 1 && **argp == '-'){
		switch(argp[0][1]){
		default:
			printf("Bad flag: %s\n", *argp);
			exit();

		case 'v': vflg++; break;
		case 'u': uflg++; break;
		case 'l': lflg++; break;
		case 't': tflg++; break;
		case 'a': aflg++; break;
		}
		argp++;
		argc--;
	}
	if(tname[8] == '?'){
		printf("Must specify tape unit number\n");
		printf("[Type '-0' '-1' or '-2']\n");
		exit();
	}

	tape = opn(tname, (op == OUT? 1 : 0));
	switch(op){
	case OUT:
		while(argc-- > 1)
			pipout(*argp++);
		WEOF(tape);
		break;

	case CAT:
		pipcat();
		break;

	case IN:
		pipin(argp, argc - 1);
		break;
	}
	exit();
}

pipout(name)
char name[];
{
	register i;
	register cnt, ifd;
	char buf[512];

	write(tape, bdosh(name), 14);	/* DOS tape file header */
	ifd = opn(name, 0);
	if(vflg)
		printf("%s\n", name);
	while((cnt = read(ifd, buf, 512)) > 0){
		for(i=cnt; i<512; i++)
			buf[i] = 0;
		fwrite(tape, buf, 512);
	}
	WEOF(tape);
	close(ifd);
}

pipcat()
{
	register nfiles,nrec,cnt;
	int nblocks;
	char buf[512];
	int t[2];

	printf("Directory of %s\n", tname);
	time(t);
	printf("\n%s\n", ctime(t));
	printf("Name  .ext\t[uid,gid]  prot\t# blocks\n\n");
	nfiles = nblocks = 0;
	while(1){
		cnt = read(tape, &dosh, 14);
		switch(cnt){
		case 0:
			printf("\nTotal blocks: %6l\n", nblocks);
			printf("Total files:  %6l\n\n", nfiles);
			return;
		case -1:
			printf("mag-tape read error\n");
			return;
		default:
			printf("bad header on tape\n");
			return;
		case 14:
			break; /* ok */
		}
		printf("%s\t[%3o,%3o] <%3o>\t",
			gfn(&dosh), (dosh.uid>>8)&0377,
			dosh.uid&0377, dosh.prot&0377);
		nrec = 0;
		while(cnt = read(tape, buf, 512))
			nrec++;
		printf("%6l\n", nrec);
		nfiles++;
		nblocks =+ nrec;
	}
}

pipin(names, nums)
char *names[];
{
	int j;
	register cnt, fd, nfiles;
	char buf[512];
	char name2[20];
	char *name;
	int more_to_go;

	more_to_go = nums;
	while(aflg || more_to_go){
		cnt = read(tape, &dosh2, 14);
		switch(cnt){
		case 0:
			if(!aflg)
				printf("%s not found\n", name);
			exit();
		case -1:
			printf("mag-tape read error\n");
			exit();
		default:
			printf("bad header on file %d\n", nfiles);
			exit();
		case 14:
			break; /* ok */
		}
		nfiles++;
		if(aflg)
			name = gfn(&dosh2);	/* always use the one that's there */
		else
			name = 0;
		for(cnt = 0; cnt < nums; cnt++){
			bdosh(names[cnt]);
			if(dosh.fname[0] == dosh2.fname[0]
			&& dosh.fname[1] == dosh2.fname[1]
			&& dosh.ext == dosh2.ext){
				if(aflg)
					name = 0;	/* ignore this one */
				else
					name = names[cnt]; /* use this one */
				break;
			}
		}
		if(name){
			/* copy this file */
			/* zip out blanks */
			delspace(name,name2);
			if((fd = creat(name2, 0644)) < 0){
				printf("Can't create %s\n", name2);
				exit();
			}
			if(vflg)
				printf("Copying   %s\n", gfn(&dosh2));
			while((cnt = read(tape, buf, 512)) > 0)
				fwrite(fd, buf, cnt);
			if(cnt < 0)
				printf("Read error on file %s\n", name2);
			close(fd);
			more_to_go--;
		} else {
			/* skip this file */
			if(vflg)
				printf("Skipping  %s\n", gfn(&dosh2));
			while(cnt = read(tape, buf, 512)){}
		}
	}
}

bdosh(fn)
char fn[];
{
	char f[6], e[3];
	register i,j;

	for(i=j=0; i<6; i++)
		if(fn[j] && fn[j] != '.')
			f[i] = fn[j++];
		else
			f[i] = ' ';
	if(fn[j] == '.')
		j++;
	for(i=0; i<3; i++)
		if(fn[j])
			if(fn[j] == '.')
				continue;
			else
				e[i] = fn[j++];
		else
			e[i] = ' ';
	dosh.fname[0] = rad50(f);
	dosh.fname[1] = rad50(f+3);
	dosh.ext = rad50(e);
	dosh.uid = 257;		/* always use [1,1] */
	dosh.prot = 0233;	/* default */
	dosh.date = 0;		/* epoch */
	dosh.junk = dosh.unused = 0;
	return(&dosh);
}

/*
 * Pack 3 chars into 1 word -- .rad50 format
 */

rad50(x)
char x[3];
{
	register i;

	for(i=0; i<3; i++)
		x[i] = rad50a(x[i]);
	return(((x[0]*050) + x[1])*050 + x[2]);
}

rad50a(c)
char c;
{
	c =& 0177;
	if(c >= 'a' && c <= 'z')
		c =+ 'A' - 'a';
	if(c >= 'A' && c <= 'Z')
		return(c - 'A' + 1);
	if(c >= '0' && c <= '9')
		return(c - '0' + 036);
	switch(c){
	case '.':	return(034);
	case '$':	return(033);
	default:	return(0);
	}
}
gfn(ap)
int *ap;
{
	static char buf[12];
	register char *q;
	register int *p;
	int i,j,n;
	extern ldivr;

	p = ap;
	q = buf;
	for(i=0; i<3; i++){
		n = *p++;
		if(i == 2)
			*q++ = '.';
		q[0] = ldiv(0,n, 050*050);
		q[1] = ldiv(0,ldivr,050);
		q[2] = ldivr;
		for(j=0; j<3; j++){
			*q = urad50(*q);
			q++;
		}
	}
	buf[11] = buf[10] = 0;
	return(buf);
}

urad50(c)
char c;
{
	if(c < 0)
		c = -c;
	switch(c){
	case 034:
		return('.');
	case 033:
		return('$');
	case 0:
		return(' ');
	default:
		if(c < 033)
			return(c - 1 + 'a');
		else
			return(c - 036 + '0');
	}
}

opn(f,m)
char *f;
{
	register fd;

	if((fd = open(f,m)) > 0)
		return(fd);
	printf("%s: cannot open\n",f);
	exit();
}

help()
{
	printf("Syntax: pip <com> [-unit] [files ...]\n");
	printf("<com>:	get | put | cat | ls | dis\n");
	exit();
}

strcmp(a,b)
char *a, *b;
{
	while(*a == *b++)
		if(*a++ == 0)
			return(0);
	return(*a - *--b);
}

/*
 * fwrite -- formatted write
 *
 * (I/O options handled here)
 */
fwrite(fd, buf, cnt)
char *buf;
int fd, cnt;
{
	register i;

	if(tflg){
		for(i=cnt-1; i>0; i--)
			if(buf[i] == 0)
				cnt--;
	}
	if(uflg)
		for(i=0; i<cnt; i++)
			buf[i] = upper(buf[i]);
	if(lflg)
		for(i=0; i<cnt; i++)
			buf[i] = lower(buf[i]);
	if(cnt)
		write(fd, buf, cnt);
}

upper(c)
char c;
{
	if(c >= 'a' && c <= 'z')
		return(c - 'a' + 'A');
	else
		return(c);
}

lower(c)
char c;
{
	if(c >= 'A' && c <= 'Z')
		return(c - 'A' + 'a');
	else
		return(c);
}

delspace(p1,p2)
char *p1, *p2;
{

	register char *rp1, *rp2;

	rp1 = p1; rp2 = p2;
	while (*rp2 = *rp1++)  {
		if (*rp2 == '.' && *rp1 == ' ')
			*rp2 = 0; /* no . if null ext */
		if (*rp2 != ' ')
			rp2++;
	}
}
