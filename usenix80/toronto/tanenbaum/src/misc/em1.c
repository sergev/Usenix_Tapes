#include "../local.h"
/*
 * pick an interpreter, any interpreter.
 */

char tflg ,pflg ,fflg ,cflg ,xflg ,rflg ;
char tchg,pchg,fchg,cchg;

char *codefile;
char *intname,*shint;
int fildes;
struct header {
	int magic;
	int flags;
	int unresolved;
	int unused[5];
} header;

struct {
	char lobyte;
	char hibyte;
};

int sysflag;
int testflag;
char *endcore,*curcore;

main(argc,argv) char **argv; {
	register char *argp;
	register value;
	extern int fout[];

endcore = curcore = sbrk(0);
	fout[0] = 2;
	argv[argc] = 0;	/* for use in later execv */
	while (argc>1 && (argv[1][0] == '-' || argv[1][0] == '+')) {
		value = (argv[1][0] == '+');
		argp = (*++argv);
		argp++;
		argc--;
		while (*argp) switch(*argp++) {
		case 't':	tchg = 1;	tflg = value;	break;
		case 'p':	pchg = 1;	pflg = value;	break;
		case 'f':	fchg = 1;	fflg = value;	break;
		case 'c':	cchg = 1;	cflg = value;	break;
		case 'x':	xflg = value;	break;
		case 'r':	rflg = value;	break;
		case 'i':	sysflag = 1; break;
		case 'X':	testflag = 1;break;
		default:	printf("unknown flag '%c'\n",argp[-1]);
		}
	}

	if (!sysflag) {
		if (argc == 1)
			codefile = "e.out";
		else {
			codefile = *++argv;
			argc--;
		}
		fildes = open(codefile,0);
		if (fildes != 3) {
			if (fildes<0)
				fatal("can't open %s",codefile);
			fatal("file descriptor 3 unavailable");
		}
		if (read(fildes,&header,sizeof header)!= sizeof header)
			fatal("read error in %s",codefile);
		switch(header.magic) {
		default:	fatal("file %s is no binary em1 program",codefile);
		case (172|(14<<8)):	/* ok */	break;
		}
		if (header.unresolved) 
			fatal("%d unresolved references",header.unresolved);

		if (!tchg)
			tflg = ((header.flags&01)!=0);
		if (!pchg)
			pflg = ((header.flags&02)!=0);
		if (!fchg)
			fflg = ((header.flags&04)!=0);
		if (!cchg)
			cflg = ((header.flags&010)!=0);
		xflg = ((header.flags&020)!=0);
		rflg = ((header.flags&040)!=0);
		/*
		 * we have a valid codefile now, all flags are set
		 * this is the time to hunt for the right interpreter
		 */

		*argv = codefile;	/* argv[] is set up right now */
	}
	argp = intname = concat(INT_DIR,"/em1:------");
	while (*argp!='-')
		argp++;
	*argp++ =  ( rflg ? 'r' : '-' );
	*argp++ =  ( xflg ? 'x' : '-' );
	*argp++ =  ( cflg ? 'c' : '-' );
	*argp++ =  ( fflg ? 'f' : '-' );
	*argp++ =  ( pflg ? 'p' : '-' );
	*argp++ =  ( tflg ? 't' : '-' );

	/*
	 * If the interpreter we asked for is available this will be
	 * the end of our work.
	 * check in system and current directory
	 */

	if (!(testflag || sysflag))
		execv(intname,argv);

	/*
	 * not in system directory
	 */

	while (*--argp!= '/')
		;
	shint = argp+1;
	if (!sysflag)
		execv(shint,argv);

	/*
	 * interpreter is not found, so make it
	 */

	makeint();

	if (sysflag)
		exit(0);
	execv(shint,argv);

	fatal("Interpreter not available");
}

makeint() {
	char *vec[11];
	int status;
	register pid;
	int ttbuf[3];

	if (!sysflag &&gtty(1,ttbuf)>=0) {
		printf("Special interpreter has to be constructed\n");
		printf("Sorry for the delay\n");
	}
	vec[0] = "as";
	vec[1] = "-o";
	vec[2] = shint;
	vec[3] = concat(INT_DIR,"/x-");
	last(vec[3],tflg,'t');
	vec[4] = concat(INT_DIR,"/x-");
	last(vec[4],pflg,'p');
	vec[5] = concat(INT_DIR,"/x-");
	last(vec[5],fflg,'f');
	vec[6] = concat(INT_DIR,"/x-");
	last(vec[6],cflg,'c');
	vec[7] = concat(INT_DIR,"/x-");
	last(vec[7],xflg,'x');
	vec[8] = concat(INT_DIR,"/x-");
	last(vec[8],rflg,'r');
	vec[9] = concat(INT_DIR,"/em.s");
	vec[10] = 0;
	while ((pid=fork())<0)
		sleep(5);
	if (pid == 0) {
		execv(AS_PATH,vec);
		fatal("%s: Not found",AS_PATH);
	} else {
		while (wait(&status)!=pid)
			;
	}
	if (status.hibyte)
		fatal("assembler error");
	vec[0] = "ld";
	vec[1] = testflag ? "-n" : "-s";
	vec[2] = "-n";
	vec[3] = "-o";
	vec[4] = shint;
	vec[5] = shint;
	vec[6] = "-lc";
	vec[7] = "-l";
	vec[8] = 0;
	while ((pid=fork())<0)
		sleep(5);
	if (pid == 0) {
		execv(LD_PATH,vec);
		fatal("%s: Not found",LD_PATH);
	} else {
		while (wait(&status)!=pid)
			;
	}
	if (status.hibyte)
		fatal("loader error");
	if (sysflag) {
		while ((pid=fork())<0)
			sleep(5);
		if (pid == 0) {
			execl(MV_PATH,"mv",shint,INT_DIR,0);
			fatal("%s: Not found",MV_PATH);
		} else {
			while (wait(&status)!=pid)
				;
		}
	}
}

last(file,flg,ch) char *file; {
	register char *p;

	p = file;
	while (*p++)
		;
	p[-3] = ch;
	p[-2] = flg ? '+' : '-';
}

fatal(s,arg) {

	printf("Em1 error: ");
	printf(s,arg);
	printf("\n");
	exit(-1);
}

char *concat(str1,str2) {
	register char *result,*p1,*p2;

	p1 = result = alloc(length(str1)+length(str2)+1);
	p2 = str1;
	while (*p1++ = *p2++);
	p1--;
	p2 = str2;
	while(*p1++ = *p2++);
	return(result);
}

length(str) char *str; {
	register char *p;

	p = str;
	while (*p++);
	p--;
	return(p-str);
}


alloc(n) {
	register result;

	result = curcore;
	curcore =+ n;
	if (curcore > endcore) {
		brk(endcore+0400);
		endcore =+ 0400;
	}
	return(result);
}
