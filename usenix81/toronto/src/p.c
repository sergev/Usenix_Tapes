#include <stdio.h>
#include <sys/types.h>
#define yes 1
#define no 0
#include <sys/stat.h>
/***************************************************** */
/* Terminal dependent!!!! */
# define deflt ((char *)0)
struct termstuff
	{
	char *termid;
	int groupsize;
	int linesize;
	int backmove;
	char *ff;
	}
	termdata[] =
	{
	"/dev/console", 22, 79, yes, "",
	"/dev/ttyd",	33,	80,	yes,	"",
	deflt,	22,	79,	yes,	""
	};
/* End terminal dependent. */
/**************************************************** */

int grouplength;
int skiplength = 0;
int linelength;
char skipunit = 'g';   /* g, l, or p */
int pagelength = 66;
int backmotion;
int raw = 0;
int fnprint = 1;
char *ffsequence;

int controltty;
char *ttyid;

char usagemsg[] = "Usage: p [-nnn] [-snnn[lpg]] [-b] [-r] [file ...]\n";
#define usagegoof() {fprintf(stderr, usagemsg);exit(1);}
#define setupbuffering(ignore, name)	strcpy(filename, name)
#define read_only 0
#define std_out 1
#define SYSERR	(-1)
#define ENDOFARGS	NULL
#define streq(a,b)	(strcmp(a,b)==0)

FILE * filep;
char filename[128];

#define maxline 512
char line[maxline];
int cursor;
int lineplace;
int groupline = 1;
int pageline = 1;
int backcount;
int overline;

char blanks[] = "\t\t\t\t\t\t\t\t\t";


main(argc,argv)
int argc;
char **argv;
	{
	int argindex;
	char c;
	char *p;
	int skipcount;
	struct termstuff *termscan;

	ttyid = ttyname(fileno(stdout));
	if(ttyid == NULL) {
		fprintf(stderr, "Output not a terminal\n");
		exit(1);
	}

	termscan = termdata;
	for(;;) {
	if( termscan->termid == deflt || streq(termscan->termid, ttyid) )break;
		termscan++;
	}
	grouplength = termscan->groupsize;
	linelength=termscan->linesize;
	backmotion = termscan->backmove;
	ffsequence = termscan->ff;

	argindex = 1;
	for(;;) {
		p = argv[argindex];
		if( p == ENDOFARGS || *p != '-' )break;
		p++;
		c = *p;
		p++;
		if( c == 's' ){
			skipcount = atoi(p);
			for(;;) {
			if( '0' > *p || *p > '9' )break;
				p++;
			}
			if( *p == 'l' ){
				skipunit = 'l';
			}else if( *p == 'p' ){
				skipunit = 'p';
			}else if( *p == 'g' ){
				skipunit = 'g';
			}else if( *p == '\0' ){
				skipunit = 'g';  /* Default */
			}else{
				usagegoof();
			}
		}else if( c == 'b' ){
			backmotion = 0;
		}else if( c == 'r' ){
			raw = yes;
		}else if( '0' <= c && c <= '9' ){
			p--;
			grouplength = atoi(p);
		}else{
			usagegoof();
		}

		argindex++;
	}

	if( skipunit == 'l' ){
		skiplength = skipcount;
	}else if( skipunit == 'g' ){
		skiplength = skipcount*grouplength;
	}else if( skipunit == 'p' ){
		skiplength = skipcount*pagelength;
	}

	erasecrt();

	controltty = open("/dev/tty", read_only);
	if(controltty == SYSERR) {
		fprintf(stderr, "Can't open control teletype\n");
		exit(1);
	}

	doit(&(argv[argindex]));
	};

doit(files)
	char *files[];
	{
	int fileindex;

	if( files[0] == ENDOFARGS ){
		/* */
		fnprint = no;
		dofile(NULL);
		return;
	/*fi*/};

	if( files[1] == ENDOFARGS ){
		/* */
		fnprint = no;
	/*fi*/};

	fileindex = 0;
	for(;;) {
	if( files[fileindex] == ENDOFARGS )break;
		dofile(files[fileindex]);

		fileindex++;
	/*end*/};
	};

dofile(file)
	char *file;
	{
	char c;
	register okflag;
	char *ptr;
	struct stat statbuf;


	okflag = 0;
	if( file == NULL ){
		/* */
		filep = stdin;
		setupbuffering(0, "Standard input");
	}else{
		filep = fopen(file, "r");
		if( filep == NULL) {
			/* file may be there but unreadable */
			if(stat(file, &statbuf) != SYSERR)
			{
				fprintf(stderr, "--- No read permission on %s --------------\n", file);
				return;
			}
			if(ptr=spname(file))
			{
				fprintf(stderr, "\"p %s\"?", ptr);
				read(controltty, &c, 1);
				if(c != 'n') okflag++;
				while(c!= '\n') read(controltty, &c, 1);
				if(!okflag)
				{
					fprintf(stderr, "aborted\n");
					return;
				}
				file = ptr;
				filep = fopen(file, "r");
				if(filep == NULL)
					okflag = 0;
			}
			/* */
	if(okflag == 0)
	{
		fprintf(stderr, "--- Can't open file %s --------------\n", file);
			return;
	}
		/*fi*/};
		setupbuffering(0, file);
	/*fi*/};

	beginfile();

	if( fnprint ){
		/* */
		ptr = "--- ";
		for(;;) {
		if( *ptr == '\0' )break;
			linechar(*ptr);
			ptr++;
		/*end*/};
		ptr = file;
		for(;;) {
		if( *ptr == '\0' )break;
			linechar(*ptr);
			ptr++;
		/*end*/};
		ptr = ": ------------------------";
		for(;;) {
		if( *ptr == '\0' )break;
			linechar(*ptr);
			ptr++;
		/*end*/};
		linechar('\n');
	/*fi*/};

	for(;;) {
		c = getc(filep);
		if(ferror(filep))
			fprintf(stderr, "--- Read error in file %s --------------\n", filename);
	if(feof(filep))	break;
		linechar(c);
	/*end*/};

	endfile();

	fclose(filep);
	};

beginfile()
	{
	beginline();
	};

beginline()
	{
	cursor = 0;
	lineplace=0;
	backcount = 0;
	overline = no;
	};

linechar(c)
	char c;
	{
	if( c == '\014'   /* ff */ ){
		/* */
		outline();
		for(;;) {
		if( pageline == 1 )break;
			outline();
		/*end*/};
	}else if( c == '\b' ){
		/* */
		if( raw || backmotion ){
			/* */
			addchar(c);
		}else{
			backcount++;
		/*fi*/};
	}else if( c == '\r' ){
		/* */
		if( raw || backmotion ){
			/* */
			addchar(c);
		}else{
			overline = yes;
		/*fi*/};
	}else if( c == '\n' ){
		/* */
		outline();
	}else if( ('\0' <= c && c < ' ') && c != '\t' ){
		/* */
		if( raw ){
			/* */
			addchar(c);
		/*fi*/};
	}else{    /* printable or tab */
		if( backcount != 0 ){
			/* */
			backcount--;
		}else if( overline == no ){
			/* */
			addchar(c);
		/*fi*/};
	/*fi*/};
	};

/* this subroutine changed by Tom O'Dell, to handle wraparounds */
/* 28 June 1978 */
addchar(c)
	char c;
	{
	line[cursor++] = c;
	if( c == '\b' ){
		/* */
		lineplace--;
	}else if( c == '\t' ){
		/* */
		lineplace = ((lineplace+8)/8)*8;
		if( lineplace >= linelength){
			/* */
			cursor--;
			outline();
			return;
		/*fi*/}
	}else lineplace++;

	if( lineplace == linelength ){
		/* */
		outline();
	/*fi*/}
	};

outline()
	{
	if( skiplength != 0 ){
		/* */
		skiplength--;
		pageline = ( pageline == pagelength ? 1 : pageline+1 );
		beginline();
		return;
	/*fi*/};
	if( cursor != 0 ){
		/* */
		write(std_out, line, cursor);
	/*fi*/};
	if( groupline == grouplength ){
		/* */
		if( cursor == 0 )
			{
			write(std_out, blanks, strlen(blanks));
			};
		delay();
		erasecrt();
		groupline = 1;
	}else{
		write(std_out, "\n", 1);
		groupline++;
	/*fi*/};
	pageline = ( pageline == pagelength ? 1 : pageline+1 );


	beginline();
	};

delay()
	{
	char c;
	for(;;) {
		read(controltty, &c, 1);
	if( c == '\n' )break;
	/*end*/};
	};

endfile()
	{
	if( cursor != 0 ){
		/* */
		outline();
	/*fi*/};
	};


erasecrt()
	{
	write(std_out, ffsequence, strlen(ffsequence));
	};

#define equal(a,b)	(strcmp(a,b)==0)
/*library spname spelling correction file pathname
 *
 * char *spname(name)
 * char name[];
 * 
 * spname attempts to correct the spelling of the pathname
 * which is its argument.  It returns a pointer to the correctly
 * spelled name, or 0 if no reasonable name can be determined.
 * The corrected name is stored in a static buffer and should be
 * copied elsewhere if necessary before calling the routine a
 * second time.
 */
/* written by Tom Duff, installed in system by Dave Sherman */
spname(name)
register char *name;
{
	register char *p, *q;
	char *new;
	int d, nd, dir;
	static char newname[80], guess[15], best[15];
	static struct{
		int ino;
		char name[15];
	}nbuf;
	new=newname;
	nbuf.name[14]='\0';
	for(;;){
		while(*name=='/')
			*new++ = *name++;
		*new='\0';
		if(*name=='\0')
			return(newname);
		p=guess;
		while(*name!='/' && *name!='\0'){
			if(p!=guess+14)
				*p++ = *name;
			name++;
		}
		*p='\0';
		if((dir=open(newname, 0))<0)
			return(0);
		d=3;
		while(read(dir, &nbuf, 16)==16)
			if(nbuf.ino){
				nd=SPdist(nbuf.name, guess);
				if(nd<d){
					p=best;
					q=nbuf.name;
					do;while(*p++ = *q++);
					d=nd;
					if(d==0)
						break;
				}
			}
		close(dir);
		if(d==3)
			return(0);
		p=best;
		do;while(*new++ = *p++);
		--new;
	}
}
/*
 * very rough spelling metric
 * 0 if the strings are identical
 * 1 if two chars interchanged
 * 2 if one char wrong, added, or deleted.
 * 3 otherwise.
 */
SPdist(s, t)
register char *s, *t;
{
	while(*s++ == *t)
		if(*t++=='\0')
			return(0);
	if(*--s){
		if(*t){
			if(s[1] && t[1] && *s==t[1] && *t==s[1] && equal(s+2, t+2))
				return(1);
			if(equal(s+1, t+1))
				return(2);
		}
		if(equal(s+1, t))
			return(2);
	}
	if(*t && equal(s, t+1))
		return(2);
	return(3);
}
