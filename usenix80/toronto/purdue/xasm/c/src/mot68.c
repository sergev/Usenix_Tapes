#define ASVENO 3	/* assembler version number	*/
#define	NINTVAR 12	/* number of interal variables	*/
#define NINCL 10	/* max # nested 'include's - 2 */
#define NUMSYM 500	/* # of symbols in symbol table */
#define NUMMAC 70	/* # of macros allowed		*/
#define	NCIM 1000	/* # of charactors (max) in a macro	*/
/*
*	6800 absolute macro assembler
*
*	Peter D Hallenbeck
*
*	(c) copyright Dec, 1977
*
*	compatible with motorola format
*/
	extern fin,fout;
	int stable[NUMSYM];		/* symbol table */
	char sstable[NUMSYM*7];
	int mbody[NUMMAC];		/* macro body table*/
	char mmbody[NUMMAC*100];
	int mdef[NUMMAC];		/* macro name definition table */
	char mmdef[NUMMAC*10];

	char *MI "Missing operhand";	/* Error messages */

	char *soptab[]{
		"1#4b%6",
		"equ",
		"end",
		"db",
		"ds",
		"dw",
		"org",
		"eval",
		"repeat",
		"fixtable",
		"expunge",
		"dup",
		"defmacro",
		"fixmacro",
		"expunge_macro",
		"endmacro",
		"enddup",
		"fcb",
		"fdb",
		"fcc",
		"rmb",
		"if",
		"else",
		"endif",
		"base",
		"listoff",
		"liston",
		"binoff",
		"binon",
		"cycle_time",
		"include",
		"macoff",
		"macon",
		"message",
		-1
		};



	char *mot[]{		/* machine opcode table */
	"jsr",
	"ldaa",
	"ldx",
	"staa",
	"bne",
	"ldab",
	"bsr",
	"beq",
	"rts",
	"bra",
	"cmpa",
	"clr",
	"inx",
	"jmp",
	"stx",
	"stab",
	"pshb",
	"dex",
	"asla",
	"cmpb",
	"cpx",
	"bmi",
	"pulb",
	"bita",
	"anda",
	"psha",
	"adda",
	"addb",
	"aba",
	"adca",
	"adcb",
	"andb",
	"bitb",
	"clra",
	"clrb",
	"cba",
	"com",
	"coma",
	"comb",
	"neg",
	"nega",
	"negb",
	"daa",
	"dec",
	"deca",
	"decb",
	"eora",
	"eorb",
	"inc",
	"inca",
	"incb",
	"oraa",
	"orab",
	"pula",
	"rol",
	"rola",
	"rolb",
	"ror",
	"rora",
	"rorb",
	"asl",
	"aslb",
	"asr",
	"asra",
	"asrb",
	"lsr",
	"lsra",
	"lsrb",
	"suba",
	"subb",
	"sba",
	"sbca",
	"sbcb",
	"tab",
	"tba",
	"tst",
	"tsta",
	"tstb",
	"des",
	"ins",
	"lds",
	"sts",
	"txs",
	"tsx",
	"bcc",
	"bcs",
	"bge",
	"bgt",
	"bhi",
	"ble",
	"bls",
	"blt",
	"bvc",
	"bvs",
	"bpl",
	"nop",
	"rti",
	"swi",
	"wai",
	"clc",
	"cli",
	"clv",
	"sec",
	"sei",
	"sev",
	"tap",
	"tpa",
	"nba",
	"hcf",
	-1
	};

	int mvt[][6]{	/* machine value table	*/
	0,	0,	173,	189,	0,	0,	/* jsr */
	134,	150,	166,	182,	0,	0,	/* ldaa */
	206,	222,	238,	254,	0,	0,	/* ldx  */
	135,	151,	167,	183,	0,	0,	/* staa */
	0,	0,	0,	0,	0,	38,	/* bne  */
	198,	214,	230,	246,	0,	0,	/* ldab */
	0,	0,	0,	0,	0,	141,	/* bsr  */
	0,	0,	0,	0,	0,	39,	/* beq  */
	0,	0,	0,	0,	57,	0,	/* rts  */
	0,	0,	0,	0,	0,	32,	/* bra  */
	129,	145,	161,	177,	0,	0,	/* cmpa */
	0,	0,	111,	127,	0,	0,	/* clr  */
	0,	0,	0,	0,	8,	0,	/* inx  */
	0,	0,	110,	126,	0,	0,	/* jmp  */
	207,	223,	239,	255,	0,	0,	/* stx  */
	199,	215,	231,	247,	0,	0,	/* stab */
	0,	0,	0,	0,	55,	0,	/* pshb */
	0,	0,	0,	0,	9,	0,	/* dex  */
	0,	0,	0,	0,	72,	0,	/* asla */
	193,	209,	225,	241,	0,	0,	/* cmpb */
	140,	156,	172,	188,	0,	0,	/* cpx  */
	0,	0,	0,	0,	0,	43,	/* bmi  */
	0,	0,	0,	0,	51,	0,	/* pulb */
	133,	149,	165,	181,	0,	0,	/* bita */
	132,	148,	164,	180,	0,	0,	/* anda */
	0,	0,	0,	0,	54,	0,	/* psha */
	139,	155,	171,	187,	0,	0,
	203,	219,	235,	251,	0,	0,
	0,	0,	0,	0,	27,	0,
	137,	153,	169,	185,	0,	0,
	201,	217,	233,	249,	0,	0,
	196,	212,	228,	244,	0,	0,
	197,	213,	229,	245,	0,	0,
	0,	0,	0,	0,	79,	0,
	0,	0,	0,	0,	95,	0,
	0,	0,	0,	0,	17,	0,
	0,	0,	99,	115,	0,	0,
	0,	0,	0,	0,	67,	0,
	0,	0,	0,	0,	83,	0,
	0,	0,	96,	112,	0,	0,
	0,	0,	0,	0,	64,	0,
	0,	0,	0,	0,	80,	0,
	0,	0,	0,	0,	25,	0,
	0,	0,	106,	122,	0,	0,
	0,	0,	0,	0,	74,	0,
	0,	0,	0,	0,	90,	0,
	136,	152,	168,	184,	0,	0,
	200,	216,	232,	248,	0,	0,
	0,	0,	108,	124,	0,	0,
	0,	0,	0,	0,	76,	0,
	0,	0,	0,	0,	92,	0,
	138,	154,	170,	186,	0,	0,
	202,	218,	234,	250,	0,	0,
	0,	0,	0,	0,	50,	0,
	0,	0,	105,	121,	0,	0,
	0,	0,	0,	0,	73,	0,
	0,	0,	0,	0,	89,	0,
	0,	0,	102,	118,	0,	0,
	0,	0,	0,	0,	70,	0,
	0,	0,	0,	0,	86,	0,
	0,	0,	104,	120,	0,	0,
	0,	0,	0,	0,	88,	0,
	0,	0,	103,	119,	0,	0,
	0,	0,	0,	0,	71,	0,
	0,	0,	0,	0,	87,	0,
	0,	0,	100,	116,	0,	0,
	0,	0,	0,	0,	68,	0,
	0,	0,	0,	0,	84,	0,
	128,	144,	160,	176,	0,	0,
	192,	208,	224,	240,	0,	0,
	0,	0,	0,	0,	16,	0,
	130,	146,	162,	178,	0,	0,
	194,	210,	226,	242,	0,	0,
	0,	0,	0,	0,	22,	0,
	0,	0,	0,	0,	23,	0,
	0,	0,	109,	125,	0,	0,
	0,	0,	0,	0,	77,	0,
	0,	0,	0,	0,	93,	0,
	0,	0,	0,	0,	52,	0,
	0,	0,	0,	0,	49,	0,
	142,	158,	174,	190,	0,	0,
	143,	159,	175,	191,	0,	0,
	0,	0,	0,	0,	53,	0,
	0,	0,	0,	0,	48,	0,
	0,	0,	0,	0,	0,	36,
	0,	0,	0,	0,	0,	37,
	0,	0,	0,	0,	0,	44,
	0,	0,	0,	0,	0,	46,
	0,	0,	0,	0,	0,	34,
	0,	0,	0,	0,	0,	47,
	0,	0,	0,	0,	0,	35,
	0,	0,	0,	0,	0,	45,
	0,	0,	0,	0,	0,	40,
	0,	0,	0,	0,	0,	41,
	0,	0,	0,	0,	0,	42,
	0,	0,	0,	0,	1,	0,
	0,	0,	0,	0,	59,	0,
	0,	0,	0,	0,	63,	0,
	0,	0,	0,	0,	62,	0,
	0,	0,	0,	0,	12,	0,
	0,	0,	0,	0,	14,	0,
	0,	0,	0,	0,	10,	0,
	0,	0,	0,	0,	13,	0,
	0,	0,	0,	0,	15,	0,
	0,	0,	0,	0,	11,	0,
	0,	0,	0,	0,	6,	0,
	0,	0,	0,	0,	7,	0,
	0,	0,	0,	0,	20,	0,
	0,	0,	0,	0,	221,	0,
	0,	0,	0,	0,	0,	0
	};


	int pc;		/* program counter	*/
	int pass;	/* pass number		*/
	char rlpass[24];/* nested pass # for macro lable hack	*/
	int lineno;	/* line number		*/
	int passend;	/* end of pass flag	*/
	int stlen;	/* symbol table lenght	*/
	int mtlen;	/* macro table lenght	*/
	int vtable[NUMSYM];/* symbol table value	*/
	int b1,b2,b3,fb2,fb3;
	int fd;		/* file descriptor */
	int pcolpc;	/* last pc	*/
	int pcoch;	/* checksum	*/
	int pcocnt;	/* punch out count */
	int pcob[30];	/* punch buffer */
	int pcobs;	/* p.o. buffer start location	*/
	int pcost;	/* start-up flag */
	int numerrs;	/* number of errors in a pass */
	int nestc;	/* nested stack level counter */
	char *nest[50];	/* nested stack stack */
	int ifcount;	/* nested if counter */
	int curbase;	/* current base for numbers */
	int macnest;	/* nested counter for macro-made varables */
	int macstack[50];/*nested macro variable list stack pointer*/
	int listflag;	/* flag for suppressed listing facility */
	int binflag;	/* flag for suppressed binary facility */
	int macflag;	/* 0 = list expanded macro code	*/
	int cytime;	/* mpu cycle time counter	*/
	char dbuff[513];/* disk object output buffer	*/
	int dbuffc;	/* output charactor counter	*/
	int ifd[NINCL];	/* 'include' file descriptors	*/
	int ifdc;	/* 'include' nested counter	*/


main(argc,argv)
	int argc;
	char **argv;
{
	char line [121];
	register char *k,c;
	register i;
	char arg[22];
	int j,bc,s;
	int ii,jj;
	int **ss;
	char b;
	int *kaboom;
	int tvec[3];

	pass = 1;
	lineno = 0;
	passend = 1;
	vtable[0] = NINTVAR + 1;
	vtable[1] = 0;
	vtable[2] = 0;
	stlen = 1;
	mtlen = 1;
	curbase = 10;
	macnest = 0;
	listflag = 1;
	binflag = 0;	/* 0 means punch binary */
	macflag = 0;	/* enable listing of expanded macro code */
	cytime = 0;
	dbuffc = 0;
	ifdc = 0;

	mixerup(stable,sstable);
	mixerup(mbody,mmbody);
	mixerup(mdef,mmdef);
	pcocnt = 0;
	pcolpc = 0;


	pc = 0;
	numerrs = 0;
	nestc = 0;
	fin = open(argv[1],0);
	if(fin == -1){
	    printf("Can't find %s.\n",argv[1]);
	    return;
	    }

/* Enter various user-type symbols into symbol table	*/
	cement("_NARGS",stable);
	cement("_NEST",stable);
	cement("_SECONDS",stable);
	cement("_MIN",stable);
	cement("_HOUR",stable);
	cement("_DAY",stable);
	cement("_MONTH",stable);
	cement("_YEAR",stable);
	cement("_DOW",stable);
	cement("_DOY",stable);
	cement("_DSTF",stable);
	cement("_ASSVERNO",stable);

	vtable[12] = ASVENO;
	time(tvec);
	kaboom = localtime(tvec);
	i = 3;
	for(ii = 0;ii != 9;ii++)vtable[i++] = *kaboom++;


foobar:	while(passend){		/* in memory of AI Lab	*/
	if(vtable[0] > (NUMSYM -4)){
		panic("Symbol table overflow.  Increase 'NUMSYM'.");
		flush();
		}
	getline(line);
	cline(line);		/* get and process line	*/
	}
	if(pass == 2){
		if(pcocnt)pdump();
		dput('S');	dput('9');	dput('\n');
		fdput();
		close(fd);
		flush();
		fd = creat("m.sym",0604);
		if(fd != -1){
		    dbuffc = 0;	/* reset output buff */
		    i = 1;		/* dump symbols	*/
		    while(stable[i] != -1){
			dput('0');
			pdumb((vtable[i] >> 8) & 0377);
			pdumb(vtable[i] & 0377);
			dput(':');
			k = stable[i++];
			while(*k != '\0')dput(*k++);
			dput('\n');
			}
		    fdput();	close(fd);
		    flush();
		    }
		fout = 1;
		if(numerrs){
		   printf("%d error%s\n",numerrs,numerrs==1?"":"s");
			}
		flush();
		return;
		}
	while(getchar());
	seek(fin,0,0);
	fout = creat("m.lst",0604);

/*			*/
	fd = creat("m.out",0604);

	pass = 2;
	lineno = 0;
	passend = 1;
	pc = 0;
	pcocnt = 0;
	pcost = 1;
	numerrs = 0;
	nestc = 0;
	listflag = 1;
	macflag = 0;
	binflag = 0;
	dput('L');
	dput('\n');
	goto foobar;
}

getline(line)
	char *line;
{
	register char *k;
	register char c;
	register i;
	int p,j;

getlinl: if(nestc){
		c = *nest[nestc]++;
		}
	   else {
		c = gchar();
		if(ifdc == 0)lineno=+ 1;
		}
	k = line;
	i = 0;
	while((c != '\n')&&(c != '\0')&&(i < 119)){
		*k++ = c;
		i++;
		if(nestc)c = *nest[nestc]++;
		   else c = gchar();
		}

	if(i >= 119){
		panic("Warning- input line too long.  Line truncated");
		k--;	*k = '\0';
		while((c != '\n')&&(c != '\0')){
			if(nestc)c = *nest[nestc]++;
			    else c = gchar();
			}
		}
	if(c == '\0'){
		if(ifdc == 0){
			panic("No end statement");
			passend = 0;
			line[0] = '\0';
			}
		    else {
			close(ifd[ifdc--]);
			goto getlinl;
			}
		}
	if(c == '\n'){
		*k++ = ';';
		*k = '\0';
		}
}

gchar()
{
	char data;
	int count;

	if(ifdc){
		count = read(ifd[ifdc],&data,1);
		if(count == 0)data = '\0';
		}
	    else data = getchar();
	return(data);
}

mexp(body,arglist,flag)
	char *body,**arglist;
	int flag;
{
	char line[120];
	char bline[NCIM+100];
	register char *p,*k,c;
	int i;
    if(flag != 0){
	k = bline;
	while(*body != '\0'){
		switch(c = *body++){
			case '&':
			    if(flag){
				c = *body++;
				i = c - '0';
				p = arglist[i];
				while(*k++ = *p++);
				k --;
				}
				else {
				   *k++ = c;
				   }
				break;
			case '\\':
				if((*body == '&')&&flag){
					*k++ = '&';
					body++;
					}
				else *k++ = c;
				break;
			default:
				*k++ = c;
			}
		}
	*k = '\0';
	k = bline;
	}
    else k = body;
	nestc++;
	while(*k != '\0'){
		p = line;
		while((*p++ = *k++)!= '\n');
		*p = '\0';
		nest[nestc] = k;
		cline(line);
		k = nest[nestc];
		}
	nestc--;
}


dumpst()
{
	register *p;
	register i;
	register char *k;
	int j,w;
	alsort(stable,vtable);
	p = stable;
	p++;
	i = 1;
	j = 1;
	while(*p != -1){
		jnum(vtable[i++],16,4,0);
		putchar(' ');
		k = *p++;
		for(w = 0;w != 18;w++){
			if(*k != '\0')putchar(*k++);
			    else putchar(' ');
			}
		if((j++ % 3) == 0)putchar('\n');
		    else printf("     ");
	}
	putchar('\n');
}

commaer(p,j)
	char **p;
	int j;
{
	while(*(*p)++ == ','){
		printf("%%%% ");
		jnum(lineno,10,4,0);
		printf(": Missing operhand before the ");
		ordnp(j++);
		printf(" comma\n");
		}
	(*p)--;
	return(j);
}

pcommer(j)
	int j;
{
	printf("%%%% ");
	jnum(lineno,10,4,0);
	printf(": Missing operhand after the ");
	ordnp(j++);
	printf(" comma\n");
}

panic(s)
	char *s;
{
	putchar('%');	putchar('%');	putchar(' ');
	jnum(lineno,10,4,0);
	printf(": %s\n",s);
	numerrs =+ 1;
}

eqs(c)
	char c;
{
	return(((c == '\n')||(c == ';')));
}

neqs(c)
	char c;
{
	return((c != '\n')&&(c != ';'));
}

prn()
{
    register i;
     if(!ifdc && listflag && !(nestc && macflag)){
	jnum(lineno,10,4,0);
	printf(" ");
	jnum(pc,16,4,0);
	printf(" ");
	jnum(b1,16,2,0);
	putchar(' ');
	if(fb3){
		i = b3;
		i = (b3 & 0377) | (b2 << 8);
		jnum(i,16,4,0);
		}
	else if(fb2){
		i = b2;
		i =& 0377;
		jnum(i,16,2,0);
		printf("  ");
		}
	else printf("    ");
	}
    pbytes();
}
prs()
{
	if(!ifdc && listflag && !(nestc && macflag))
		printf("                 ");
}
prl(line)
	char *line;
{
	register char *k;
	register char delay;
	if(listflag == 0)return;
	if(ifdc)return;
	if(nestc && macflag)return;
	k = line;
	putchar (' ');
	if(*k == '\0')return;
	if(line[1] == '\0')return;
	delay = *k++;
	while(*k != '\0'){
		putchar(delay);
		delay = *k++;
		}
	putchar('\n');
}

prsl(line)
	char *line;
{
	prs();
	prl(line);
}

canlist()
{
	return((listflag && !(nestc && macflag)) && !ifdc);
}
pbytes()
{
	if(binflag)return;
	if(pcost){
		pcost = 0;
		pcolpc = pc;
		}
pbyteq:	if(pcocnt == 0){
		pcolpc = pc;
		pcobs = pc;
		}
	if(pc != pcolpc){
		pdump();
		goto pbyteq;
		}
	pcob[pcocnt++]= b1;
	pcolpc =+ 1;
	if(fb2){
	    pcob[pcocnt++] = b2;	/* enter bytes */
	    pcolpc =+ 1;
		}
	if(fb3){
	    pcob[pcocnt++] = b3;
	    pcolpc =+ 1;
		}
	if(pcocnt > 25){
	    pdump();		/* if line is full */
	    return;
		}
}

pdump()
{
	register i;
	int j;
	dput('S');
	dput('1');
	i = 0;
	pcoch = 0;
	pdumb(pcocnt+3);
	pdumb(pcobs >> 8);
	pdumb(pcobs & 0377);
	while(pcocnt){
		pdumb(pcob[i++]);
		pcocnt--;
		}
	pdumb(~pcoch);
	dput('\n');
}

pdumb(n)
	int n;
{
	int	i;
	register j;
	i = n;
	i = (i & 0360)>>4;
	i =+ 060;
	if(i > '9')i =+ ('A'-':');
	dput(i);
	i = n & 017;
	i =+ 060;
	if(i > '9')i =+ ('A'-':');
	dput(i);
	pcoch =+ n;
}

dput(data)
	char data;
{
	dbuff[dbuffc++] = data;
	if(dbuffc == 512){
		write(fd,&dbuff,512);
		dbuffc = 0;
		}
}

fdput()
{
	if(dbuffc)write(fd,&dbuff,dbuffc);
}

cline(line)
	char *line;
{
	char arg[21];
	char sarg[21];
	char lline[NCIM];
	char llline[120];
	int s;
	char **ss;
	int hlable;
	int hclable;
	int hdclable;
	int ii,jj;
	char dseos;
	int marg[10];
	char mmarg[100];
	int maclab[20];		/* macro lable save table	*/
	int mmaclab[150];
	register char c;
	register i,j;

	hlable = 0;
	hclable = 0;
	hdclable = 1;
	mixerup(marg,mmarg);
	mixerup(maclab,mmaclab);
	s = line;
	ss = &s;
	arg[0] = '\0';
	vtable[2]++;

	if( pass == 1 )  {


   if(islet(**ss)== 1){
	if((i = (sscan(arg,ss)))== ':'){
		(*ss)++;			/* ignor : */
		hclable = 1;
		if(**ss == ':'){
			(*ss)++;
			hdclable = 0;
			}
		}
	if(i == -1){
		panic("Missing Opcode");
		goto eocline;
		}
	if((i = llu(arg,stable))!= -1){
	    if(i > NINTVAR){
		panic("Double defined symbol:");
		printf("%%%%\t%s\n",arg);
		}
	    }
	if(i == -1){
	cement(arg,stable);	/* put into stable */
	vtable[llu(arg,stable)] = pc;
	hlable = 1;
	vtable[0] =+ 1;
	if(macnest && (hdclable || rlpass[macnest] == 1))
		cement(arg,macstack[macnest]);
	}
      }
   else {
	if((**ss == ';')||(**ss == '*'))goto eocline;
	if((**ss != ' ')&&(**ss != '\t')){
		if(**ss == '\n')goto eocline;
		panic("Invalid lable");
		while((**ss != ' ')&&(**ss != '\t')&&(**ss != '\n'))
			(*ss)++;
		if(**ss == '\n')goto eocline;
		}
	}
   i = sscan(sarg,ss);		/* get opcode/psydo-op	*/
   if(sarg[0] == '\0'){
	goto eocline;
	}
   if((**ss == ' ') && ((*(*ss + 1) == 'a') || (*(*ss + 1) == 'b')
	|| (*(*ss + 1) == 'x')) &&
	((*(*ss + 2) == ' ') || (*(*ss + 2) == '\t') || (*(*ss + 2)
	== '\n') || (*(*ss + 2) == ';'))){
	llline[0] = *(*ss + 1);
	llline[1] = '\0';
	cats(sarg,llline);
	(*ss)++;	(*ss)++;
	}
   if((j = llu(sarg,mdef))!= -1){	/* if macro	*/
	p1mac:	ii = 0;
	eatspace(ss);
	hlable = 0;
	while(neqs(**ss)){
	    jj = 0;
	    llline[0] = '\0';
	    while((**ss != ',')&&(neqs(**ss))){
		llline[jj++] = *(*ss)++;
		}
	llline[jj]= '\0';
	if(jj > 80)panic("Too many charactors in macro arg.");
	  else cement(llline,marg);
	if(**ss == ',')(*ss)++;
	eatspace(ss);
	if(hlable++ > 9)panic("Too many macro arguments");
	eatspace(ss);
	}
	vtable[1] = hlable;
	while(hlable++ < 10)cement("",marg);
	macstack[++macnest]= maclab;
	rlpass[macnest] = pass;
	if(pass == 2){
	    pass = 1;
	    i = pc;
	    mexp(mbody[j],marg,1);
	    pass = 2;
	    pc = i;
	    mexp(mbody[j],marg,1);
	    }
	  else mexp(mbody[j],marg,1);
	--macnest;
	while(maclab[1] != -1){
	    if((i = llu(maclab[1],stable)) != -1){
		chisel(i,stable);
		slide(vtable,i,i+1,vtable[0]);
		vtable[0] =- 1;
		if(i < stlen)stlen--;
		}
	    chisel(1,maclab);
	    }
	goto eocline;
	}
  else
   if((j = llu(sarg,soptab))!= -1){		/* pysdo op	*/

     switch(j){

      case 1:					/* equ		*/
  p1equ:  if(arg[0]== '\0'){panic("EQU with no symbol");goto p1equa;}
	if(i == -1){
		panic(MI);
		chisel(arg,stable);
		vtable[0] =- 1;
		goto p1equa;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		chisel(arg,stable);
		vtable[0] =- 1;
		goto p1equa;
		}
	vtable[i = llu(arg,stable)] = evolexp(ss,stable,vtable);
   p1equa:	if((pass == 2) && canlist()){
			printf("     ");
			jnum(vtable[i],16,4,0);
			printf("        ");
			prl(line);
			}
	goto eocline;


     case 2:					/* end */
	passend = 0;
	goto eocline;


     case 3:					/* db */
  p1db: if(i == -1){
		panic(MI);
		pc =+ 1;
		goto eocline;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		pc =+ 1;
		goto eocline;
		}
	j = (commaer(ss,1) -1);
	pc =+ j;
	while(neqs(**ss) && (**ss != ' ') && (**ss != '\t')){
		if(**ss == ','){
			(*ss)++;
			pc =+ 1;
			j =+ 1;
			while(**ss == ','){
				pc =+ 1;
				pcommer(j);
				j =+ 1;
				(*ss)++;
				}
			if(eqs(**ss)||(**ss == ' ')||(**ss == '\t')){
				pcommer(j);
				panic("Zero assumbed\n");
				(*ss)--;
				}
			}
		(*ss)++;
		}
	pc =+ 1;
	goto eocline;


   case 4:					/* ds */
 p1ds: if(i == -1){panic(MI);goto eocline;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		goto eocline;
		}
	j = (commaer(ss,1)-1);
	while(neqs(**ss)&&(**ss != ' ')&&(**ss != '\t')){
		dseos = **ss;
		i = 0;
		(*ss)++;
		while((**ss != dseos)&&(**ss != '\n')){
			if(**ss == '\\')
				(*ss)++;
			(*ss)++;
			i++;
			}
		if(**ss == '\n'){
		   panic("Missing clossing charactor in string");
		   pc =+ i;
		   goto eocline;
		   }
		(*ss)++;
		pc =+ i;
	if(**ss == ','){
		(*ss)++;
		eatspace(ss);
		j =+ 1;
		while(**ss == ','){
			pcommer(j);
			j =+ 1;
			(*ss)++;
			eatspace(ss);
			}
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			putchar('\n');
			goto eocline;
			}
		}
	}
	goto eocline;


   case 5:					/* dw	*/
   p1dw: if(i == -1){panic(MI);
		pc =+ 2;
		goto eocline;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		pc =+ 2;
		goto eocline;
		}
	j = commaer(ss,1) - 1;
	pc =+ j*2;
	while(neqs(**ss)&&(**ss != ' ')&&(**ss != '\t')){
	   if(**ss == ','){
		(*ss)++;
		j =+ 1;
		pc =+ 2;
		while(**ss == ','){
			pc =+ 2;
			pcommer(j);
			j =+ 1;
			(*ss)++;
			}
		if(eqs(**ss)||(**ss == ' ')||(**ss == '\t')){
			(*ss)--;
			pcommer(j);
			printf(" Zero assumed\n");
			}
		}
	   (*ss)++;
	}
	pc =+ 2;
	goto eocline;


   case 6:					/* org  */
   p1org:   if(i == -1){panic(MI);goto p1orga;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		goto p1orga;
		}
	pc = evolexp(ss,stable,vtable);
	eatspace(ss);
	if(**ss == ',')
	  panic("Only on operand in an org statment- first on used");
	if(arg[0] != 0){
		if((j = llu(arg,stable))!= -1)
			vtable[j] = pc;
	}
   p1orga:	if((pass == 2) && canlist()){
			printf("     ");
			jnum(pc,16,4,0);
			printf("        ");
			prl(line);
			}
	goto eocline;


   case 7:					/* eval	*/
	if(i == -1){panic(MI);
		goto eocline;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		goto eocline;
		}
	evolexp(ss,stable,vtable);
	goto eocline;


   case 8:					/* repeat */
  p1rep:  if(i == -1){panic(MI);goto p1repa;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		goto p1repa;
		}
	i = evolexp(ss,stable,vtable);
	if(i < 0){
		panic("Can't repeat a negative number of times");
		i = 1;
		}
	if(pass == 2)prsl(line);
	getline(lline);
	if(i){
		cline(lline);
		i =- 1;
		}
	j = 0;
	while((lline[j]!= ' ')&&(lline[j] != '\t')&&(lline[j] != '\0'))
		lline[j++] = ' ';
	while(i--)cline(lline);
	goto eocline;
  p1repa:  if(pass == 2)prsl(line);
	goto eocline;


   case 9:					/* fixtab */
  p1fixt:  i = 1;
	while(stable[i++] != -1);
	stlen = --i;
	goto eocline;


   case 10:					/* expunge	*/
  p1exps:  if(i != -1){
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';'))i = -1;
		}
	if(i == -1){
		stable[stlen] = -1;
		vtable[0] = stlen + 1;
		goto eocline;
		}
	commaer(ss,1);
	j = 0;
   lop1:   i = sscan(arg,ss);
	if(arg[0] == '\0'){
		pcommer(j);
		putchar('\n');
		}
	 else {
		if((ii = llu(arg,stable))== -1){
			panic("Undefinded Symbol:");
			printf("%%%%\t%s\n",arg);
			}
		   else {
			chisel(ii,stable);
			slide(vtable,ii,ii+1,vtable[0]);
			vtable[0] =- 1;
			if(ii < stlen)stlen--;
		    }
		}
	if(i == -1)goto eocline;
	eatspace(ss);
	if(**ss == ','){
		j =+ 1;
		(*ss)++;
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			goto eocline;
			}
		  else goto lop1;		/* cringe	*/
		}
	if((**ss == '\n')||(**ss == ';'))goto eocline;
	panic("Extranious symbols in operhand field");
	goto eocline;


   case 11:					/* dup	*/

   p1dup:  if(i == -1){panic(MI);goto eocline;}
	eatspace(ss);
	if(eqs(**ss)){panic(MI);goto eocline;}
	i = evolexp(ss,stable,vtable);
	if((pass == 1)&&(i < 0)){
		panic("Can't duplicate by negative number of times");
		i = 1;
		}
	if(i < 0)i = 1;
	jj = i;
	j = 0;
	ii = 0;
	i = 0;
	while(1){		/* Allways wanted to use a while(1) */
	   getline(llline);
	   if(lfs("enddup",llline)){
		if(ii <= 0)goto lop4;
		ii =- 2;
		}
	   if(lfs("endalldup",llline))goto lop4;
	   if(lfs("dup",llline)){
		ii =+ 1;
		}
	   i = 0;
          while(((lline[j] = llline[i++])!= '\0')&&(j < NCIM)){
		j++;
		}
	   lline[j++] = '\n';
	   if(j >= NCIM){
		panic("Too many charactors in 'dup' argument");
		panic("Interanally, adjust 'NCIM'.");
		lline[NCIM-1] = '\n';
		lline[NCIM] = '\0';
		j--;
		}
	   }
  lop4:
	lline[j] = '\0';
    if(jj)mexp(lline,marg,0);
	j = 0;
	jj =- 1;
	while(lline[j] != '\0'){		/* remove symbols */
	    if((lline[j] != ' ')&&(lline[j] != '\t')){
		while((lline[j] != ' ')&&(lline[j] != '\t'))
		    lline[j++] = ' ';
		}
	    while((lline[j] != '\n')&&(lline[j] != '\0'))j++;
	    if(lline[j] == '\n')j++;
	    }
	while(jj--)mexp(lline,marg,0);
	goto eocline;


  case 12:					/* defmacro */

   p1defm:	hlable = 0;
	b1 = 1;
	if(i == -1){panic(MI);goto eocline;}
	eatspace(ss);
	if(eqs(**ss)){panic(MI);goto eocline;}
	sscan(arg,ss);
	if(llu(arg,mdef)!= -1){
		if(pass== 1){
			panic("Double defined macro");
			printf("%%%%\t%s\n",arg);
			b1 = 0;
			}
		}
	  else hlable = 1;
	if(((pass == 1)||(hlable))&& b1 )cement(arg,mdef);
	eatspace(ss);
	if(**ss == ',')
	   panic("Only one operhand in a defmacro statemnt -first one used");
	j = 0;
	lline[0] = '\n';
	ii = 0;
	i = 0;
	while(1){
	   getline(llline);
	   if(lfs("endmacro",llline)){
		if(ii <= 0)goto lop5;
		ii =- 1;
		}
	   if(lfs("endallmacro",llline))goto lop5;
	   if(lfs("defmacro",llline)){
		s = &line;
		ss = &s;
		if((llline[0] != ' ')&&(llline[0] != '\t'))break;
		sscan(arg,ss);
		if(comstr(arg,"defmacro"))ii =+ 1;
		}
	 i = 0;
         while(((lline[j] = llline[i++])!= '\0')&&(j < NCIM)){
		j++;
		}
	   lline[j++] = '\n';
	   if(j >= NCIM){
		panic("Too many charactors in 'defmacro' argument");
		panic("Internally, adjust 'NCIM'");
		lline[NCIM - 1] = '\n';
		lline[NCIM] = '\0';
		j--;
		goto lop5;
		}
	   }
   lop5: lline[j] = '\0';
	if(((pass == 1)||(hlable))&& b1)cement(lline,mbody);
	goto eocline;


  case 13:					/* fixmacro	*/
   p1fixm:  i =1;
	while(stable[i++] != -1);
	mtlen = --i;
	goto eocline;


  case 14:					/* expunge macro */
   p1expm:   if(i != -1){
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';'))i = -1;
		}
	if(i == -1){
		mdef[mtlen] = -1;
		goto eocline;
		}
	commaer(ss,1);
	j = 0;
   lop3: i = sscan(arg,ss);
	if(arg[0] == '\0')pcommer(j);
	  else {
	    if((ii = llu(arg,mdef)) == -1){
		panic("Undefinded macro:");
		printf("%%%%\t%s\n",arg);
		}
	      else {
		  chisel(ii,mdef);
		  chisel(ii,mbody);
		  }
	      }
	if(i == -1)goto eocline;
	eatspace(ss);
	if(**ss == ','){
		j =+ 1;
		(*ss)++;
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			goto eocline;
			}
		  else goto lop3;
		}
	if((**ss == '\n')||(**ss == ';'))goto eocline;
	panic("Extranious charactor in operhand field");
	goto eocline;


   case 15:					/* endmacro */
   p1endm:   panic("Extra 'endmacro'");
	goto eocline;


   case 16:					/* enddup */
   p1endd:   panic("Extra 'enddup'");
	goto eocline;


   case 17:					/* fcb	*/
   p1fcb:	goto p1db;
	goto eocline;

    case 18:					/* fdb	*/
   p1fdb:	goto p1dw;
	goto eocline;

    case 19:					/* fcc */
    p1fcc:	goto p1ds;
	goto eocline;


    case 20:				/* rmb	*/
p1rmb:	if(i == -1)panic(MI);
	eatspace(ss);
	if(eqs(**ss)){
		panic(MI);
		goto eocline;
		}
	j = commaer(ss,1) - 1;
	pc =+ evolexp(ss,stable,vtable);
	while(**ss == ','){
		(*ss)++;
		while(**ss == ','){
			j =+ 1;
			pcommer(j);
			(*ss)++;
			}
		pc =+ evolexp(ss,stable,vtable);
		j =+ 1;
		}
	goto eocline;



   case 21:					/* if	*/
	p1if:	eatspace(ss);
	j =  evolexp(ss,stable,vtable);
	ifcount++;
	if(j)goto eocline;
	j = 1;
	ii = 0;
	while(j){
		getline(llline);
		s = llline;
		ss = &s;
		if(islet(**ss) == 1)sscan(arg,ss);
		if(eqs(**ss)){
			if(passend == 0){
				panic("Broken 'if' statment");
				goto eocline;
				}
			}
		  else {
			sscan(arg,ss);
			if(comstr(arg,"endif")){
				if(ii == 0){j = 0;
				ifcount--;
					}
				   else ii--;
				}
			else if(comstr(arg,"else")){
				if(ii == 0)j = 0;
				}
			else if(comstr(arg,"if"))ii++;
			}
		}
	goto eocline;


   case 22:					/* else */
    p1else:   if(ifcount == 0){
		panic("Extra 'else' statment");
		goto eocline;
		}
	j = 1;
	ii = 0;
	while(j){
		getline(llline);
		s = llline;
		ss = &s;
		if(islet(**ss) == 1)sscan(arg,ss);
		if(eqs(**ss)){
			if(passend == 0){
				panic("Broken if statment");
				goto eocline;
				}
			}
		   else {
			sscan(arg,ss);
			if(comstr(arg,"endif")){
				if(ii == 0){j = 0;
				ifcount--;
					}
				   else ii--;
				}
				else if(comstr(arg,"if"))ii++;
			}
		}
	goto eocline;


   case 23:					/* endif	*/
	p1endif:	if(ifcount == 0){
		panic("extra 'endif'");
		goto eocline;
		}
	ifcount--;
	goto eocline;

    case 24:					/* base 	*/
p1base:	eatspace(ss);
	if(eqs(**ss)){
		panic("Missing Operhand\n");
		goto eocline;
		}
	curbase = 10;
	curbase = evolexp(ss,stable,vtable);
	goto eocline;

    case 25:					/* listoff */
	listflag = 0;
	goto eocline;

    case 26:					/* liston */
	listflag = 1;
	goto eocline;

    case 27:					/* binoff */
	binflag = 1;
	goto eocline;

    case 28:					/* binon */
	binflag = 0;
	goto eocline;

    case 29:					/* cycle_time */
	goto eocline;

    case 30:					/* include */
p1incl:	eatspace(ss);
	if(eqs(**ss)){panic(MI); goto eocline;}
	if(ifdc == NINCL){panic("Too many nested includes"); goto eocline;}
	i = 0;
	while(neqs(**ss))llline[i++] = *(*ss)++;
	llline[i] = '\0';
	ifd[++ifdc] = open(llline,0);
	if(ifd[ifdc] == -1){
		panic("Can't read include file");
		--ifdc;
		}
	goto eocline;

    case 31:					/* macoff	*/
	goto eocline;

    case 32:					/* macon	*/
	goto eocline;

    case 33:					/* message	*/
p1err:	printf("%%%% User message:\t");
p1errl:	while(neqs(**ss) && (**ss != '%'))putchar(*(*ss)++);
	if(eqs(**ss)){
		putchar('\n');
		numerrs++;
		goto eocline;
		}
	sscan(arg,ss);
	if((i = llu(arg,stable)) != -1){
		basout(vtable[i],16);
		}
	    else printf("[undefined]");
	goto p1errl;


       }		/*  END OF SWITCH  */
     }

  else if((j = llu(sarg,mot))!= -1){
	eatspace(ss);
	if(mvt[j][4] != 0){
		pc =+ 1;
		goto eocline;		/* inherant */
		}
	if(eqs(**ss))panic(MI);
	if((**ss)== '#'){
		pc =+ bytcnt(mvt[j][0]);/* immediate	*/
		if(mvt[j][0] == 0)panic("Invalid mode");
		goto eocline;
		}
	while((**ss != ';')&&(**ss != ',')&&(**ss != ' ')&&
		(**ss != '\t'))(*ss)++;
	if(**ss == ','){
		(*ss)++;
		if(**ss == 'x'){
			pc =+ 2;	/* indexed */
			if(mvt[j][3] == 0)panic("Invalid mode");
			goto eocline;
			}
		if(**ss == 'd'){
			pc =+ 2;	/* base-mode address */
			if(mvt[j][1] == 0)panic("Invalid mode");
			goto eocline;
				}
		panic("Odd addressing mode");
		}
	if(mvt[j][3] != 0){
		pc =+ 3;		/* extended	*/
		goto eocline;
		}
	if(mvt[j][5] != 0){
		pc =+ 2;		/* relative	*/
		goto eocline;
		}
	panic("Valid instruction with invalid mode");
	}
     panic("Undefined instruction:");
	printf("%%%%\t%s\n",sarg);
       goto eocline;


   }

		/* IN PASS TWO   */


	fb2 = fb3 = 0;
	b1 = 1;
	if((**ss == ';')&&(*(*ss+1) == '\0')){putchar('\n');goto eocline;}
	if(islet(**ss) == 1){
	   if((i = sscan(arg,ss))== ':'){
		(*ss)++;
		hclable = 1;		/* ignore :*/
		if(**ss == ':'){
			(*ss)++;
			hdclable = 0;
			}
		}
	   if(i == -1){
		panic("Missing opcode");
		prsl(line);
		goto eocline;
		}
	   if(llu(arg,stable)== -1){
		cement(arg,stable);
		vtable[llu(arg,stable)] = pc;
		hlable = 1;
		vtable[0]=+ 1;
		if(macnest && (hdclable || rlpass[macnest] == 1))
			cement(arg,macstack[macnest]);
		}
	    }
	else {
	    if((**ss == ';')||(**ss == '*')){prsl(line);goto eocline;}
	    if((**ss != ' ')&&(**ss != '\t')){
		if(**ss == '\n'){putchar('\n');goto eocline;}
		panic("Invalid lable");
		while((**ss != ' ')&&(**ss != '\t')&&(**ss != '\n'))
			(*ss)++;
		if(**ss == '\n'){prsl(line);goto eocline;}
		}
	     }
	i = sscan(sarg,ss);
	if(sarg[0] == '\0'){
	    if(hlable && (hclable == 0)){
		panic("Warning -lable with no opcode");
		printf("\t%s\n",arg);
		}
	    prsl(line);
	    goto eocline;
	   }
   if((**ss == ' ') && ((*(*ss + 1) == 'a') || (*(*ss + 1) == 'b')
	|| (*(*ss + 1) == 'x')) &&
	((*(*ss + 2) == ' ') || (*(*ss + 2) == '\t') || (*(*ss + 2)
	== '\n') || (*(*ss + 2) == ';'))){
	llline[0] = *(*ss + 1);
	llline[1] = '\0';
	cats(sarg,llline);
	(*ss)++;	(*ss)++;
	}
	if((j = llu(sarg,mdef))!= -1){		/*  if macro */
		prsl(line);
		goto p1mac;
		}

	if((j = llu(sarg,soptab))!= -1){	/*  psdo -op	*/

    switch(j){

    case 1:					/*  equ */
	goto p1equ;

    case 2:					/*  end */
	passend = 0;
	prsl(line);
	for(i = 0;i != NINTVAR;i++)chisel(1,stable);
	slide(vtable,1,NINTVAR + 1,vtable[0]);
	vtable[0] =- NINTVAR;
	dumpst();
	goto eocline;

    case 3:					/*  db	*/
  p2db:	eatspace(ss);
	 b1 = evolexp(ss,stable,vtable);
	if(eqs(**ss) || (**ss == ' ') || (**ss == '\t')){
		prn();	prl(line);	pc =+ 1;	goto eocline;
		}
	    else {
		prsl(line);
		prn();
		}
	i = 1;
	pc =+ 1;
	while(**ss == ','){
		(*ss)++;
		b1 = evolexp(ss,stable,vtable);
			if(listflag && !(nestc && macflag)){
				if(i < 16){
					putchar(' ');
					jnum(b1,16,2,0);
					i++;
					}
				   else {
					putchar('\n');
					prn();
					i = 1;
				}
			}
		pbytes();
		pc =+ 1;
		}
	if(listflag && !(nestc && macflag))putchar('\n');
	goto eocline;

    case 4:					/*  ds	*/
   p2ds: prsl(line);
	eatspace(ss);
p2dsb: if(neqs(**ss)&&(**ss != ' ')&&(**ss != '\t')){
	while(**ss == ','){
		(*ss)++;
		eatspace(ss);
		}
		dseos = **ss;
		ii = 20;	jj = 0;		j = 1;
      p2dsa:   (*ss)++;
		if(**ss == '\\'){
			(*ss)++;
			i = **ss;
			if(**ss == 't')i = '\t';
			if(**ss == 'n')i = 012;
			if(**ss == 'r')i = 015;
			if(**ss == 'b')i = 010;
			if(**ss == 'e')i = 04;	/* eot */
			if(**ss == '0')i = '\0';
			b1 = i;
			if(ii < 16){
				j = 0;
				pbytes();
				if(listflag && !(nestc && macflag)){
					putchar(' ');
					jnum(b1,16,2,0);
					}
				ii++;
				}
			   else {
				if(jj && listflag && !(nestc && macflag))
					putchar('\n');
				prn();
				ii = 1;
				jj = 1;
				}
			pc =+ 1;
			goto p2dsa;
			}
		if((**ss != dseos)&&(**ss != '\n')){
			b1 = **ss;
			if(ii < 16){
				j = 0;
				pbytes();
				if(listflag && !(nestc && macflag)){
					putchar(' ');
					jnum(b1,16,2,0);
					}
				ii++;
				}
			   else {
				if(jj && listflag && !(nestc && macflag))
					putchar('\n');
				prn();
				ii = 1;
				jj = 1;
				}
			pc =+ 1;
			goto p2dsa;
			}
		if(**ss == dseos)(*ss)++;
		if(listflag && !(nestc && macflag))putchar('\n');
		ii = 1;
		goto p2dsb;
	}
	goto eocline;

    case 5:					/*  dw	*/
  p2dw:	eatspace(ss);
 	i = evolexp(ss,stable,vtable);
	b2 = i&0377;
	b1 = (i>>8)&0377;
	fb2 = 1;
	if(eqs(**ss) || (**ss == ' ') || (**ss == '\t')){
		prn();	prl(line);   fb2 = 0;
		pc =+ 2;
		goto eocline;
		}
	    else {
		prsl(line);
		prn();
		}
	ii = 1;
	pc =+ 2;
	while(**ss == ','){
		(*ss)++;
		i = evolexp(ss,stable,vtable);
		b2 = i&0377;
		b1 = (i>>8)&0377;
		fb2 = 1;
		if(ii < 8){
			pbytes();
			if(listflag && !(nestc && macflag)){
				putchar(' ');
				jnum(i,16,4,0);
				}
			ii++;
			}
		   else {
			if(listflag && !(nestc && macflag))putchar('\n');
			prn();
			ii = 1;
			 }
		pc =+ 2;
		}
	fb2 = 0;
	if(listflag && !(nestc && macflag))putchar('\n');
	goto eocline;

    case 6:					/*   org	*/
	goto p1org;

    case 7:					/*  eval	*/
	eatspace(ss);
	evolexp(ss,stable,vtable);
	prsl(line);
	goto eocline;

    case 8:					/*  repeat	*/
	goto p1rep;

    case 9:					/*  fixtab  */
	prsl(line);
	goto p1fixt;

    case 10:					/*  expunge	*/
	prsl(line);
	goto p1exps;

    case 11:					/*  dup	*/
	prsl(line);
	goto p1dup;

    case 12:					/*  defmacro */
	goto p1defm;

    case 13:					/*  fixmacro  */
	prsl(line);
	goto p1fixm;

    case 14:					/*  expunge\ macro*/
	prsl(line);
	goto p1expm;

    case 15:					/*  end macro */
	prsl(line);
	goto p1endm;

    case 16:					/*  enddup	*/
	prsl(line);
	goto p1endd;

    case 17:					/* fcb */
	goto p2db;

    case 18:					/* fdb */
	goto p2dw;

    case 19:					/* fcc */
	goto p2ds;

    case 20:					/* rmb	*/
	prsl(line);
	goto p1rmb;

    case 21:					/* if	*/
	prsl(line);
	goto p1if;

    case 22:					/* else	*/
	prsl(line);
	goto p1else;

    case 23:					/* endif	*/
	prsl(line);
	goto p1endif;

    case 24:					/* base 	*/
	prsl(line);
	goto p1base;

    case 25:					/* listoff */
	listflag = 0;
	goto eocline;

    case 26:					/* liston */
	listflag = 1;
	goto eocline;

    case 27:					/* binoff */
	prsl(line);
	binflag = 1;
	goto eocline;

    case 28:					/* binon */
	prsl(line);
	binflag = 0;
	goto eocline;

    case 29:					/* cycle_time */
	printf("MPU cycles to this point are %d.\n",cytime);
	cytime = 0;
	goto eocline;

    case 30:					/* include */
	prsl(line);
	goto p1incl;

    case 31:					/* macoff	*/
	macflag = 1;
	goto eocline;

    case 32:					/* macon	*/
	macflag = 0;
	goto eocline;

    case 33:					/* message	*/
	goto p1err;


	   }	/*  END SWITCH  */
	}


	j = llu(sarg,mot);
	if(j == -1){
		panic("Undefined  Instruction:");
		printf("%%%%\t%s\n",sarg);
		prsl(line);
		goto eocline;
		}

	eatspace(ss);
	fb2 = fb3 = 0;
	if(mvt[j][4] != 0){		/*	if inherant	*/
		b1 = mvt[j][4];
		goto dumper;
		}
	if(eqs(**ss))panic(MI);
	if(**ss == '#'){		/*	if imediate	*/
		b1 = mvt[j][0];
		(*ss)++;
		b2 = evolexp(ss,stable,vtable);
		if(bytcnt(mvt[j][0]) == 3){
		    b3 = b2 & 0377;
		    b2 = (b2>>8) & 0377;
		    fb2 = 1;
		    fb3 = 1;
		    }
		else {
			fb2 = 1;
			b2 =& 0377;
			   }
		if(mvt[j][0] == 0){
		    panic("Imed. mode on non-imed. instruction");
		    }
		goto dumper;
		}
	ii = ss;
	i  = (*ss);
	while((**ss != ',')&&(**ss != ';')&&
		(**ss != ' ')&&(**ss != '\t'))(*ss)++;
	if(**ss == ','){		/* indexed or base mode	*/
	    (*ss)++;
	    if(**ss == 'x'){		/* 	indexed		*/
		ss = ii;
		(*ss) = i;
		b2 = evolexp(ss,stable,vtable);
		fb2 = 1;
		if(mvt[j][2] == 0){
		    panic("Indexed mode on non-Indexed instruction");
		    }
		b1 = mvt[j][2];
		goto dumper;
		}
	    if(**ss == 'd'){		/*	direct		*/
		ss = ii;
		(*ss) = i;
		b2 = evolexp(ss,stable,vtable);
		fb2 = 1;
		if(mvt[j][1] == 0){
		    panic("Direct mode on non-direct instruction");
		    }
		b1 = mvt[j][1];
		goto dumper;
		}
	    }
	ss = ii;
	(*ss) = i;
	if(mvt[j][3] != 0){		/*	extended	*/
		if(**ss == 'x'){	/* possible indexed */
		    (*ss)++;
		    if((**ss == ' ')||(**ss == '\t')||(**ss == ';')){
			if(llu("x",stable) == -1){
				b1 = mvt[j][2];
				b2 = 0;
				b3 = 1;
				fb2 = fb3 = 1;
				prn();	prl(line);
				pc =+ 3;
				cytime =+ bytime(b1);
				goto eocline;
				}
			}
		    (*ss)--;
		    }
		b1 = mvt[j][3];
		j = evolexp(ss,stable,vtable);
		b2 = (j>>8)& 0377;
		b3 = j & 0377;
		fb2 = 1;
		fb3 = 1;
		goto dumper;
		}
	if(mvt[j][5] != 0){		/*	relative	*/
		b1 = mvt[j][5];
		j = evolexp(ss,stable,vtable);
		if(((j - pc) > 129)||((pc - j) > 125)){ /*range error*/
		    panic("Relative address out of range");
		    j = 0;
		    b2 = 0;
		    }
		if(pc <= j){			/* forward	*/
		    b2 = (j - pc - 2);
		    }
		if(pc > j){			/* backward	*/
		    b2 = ~(pc - j +1);
		    }
		fb2 = 1;
		goto dumper;
		}
	panic("Internal error- type of instruction unknown");
	printf("J is %d, instruction is %s.\n",j,sarg);
for(i=0;i!=5;i++)printf("%d  ",mvt[j][i]);
putchar('\n');
	goto dumper;
bdumper:	panic(MI);

dumper:	prn();
	prl(line);
	pc =+ bytcnt(b1);
	cytime =+ bytime(b1);
eocline:	vtable[2]--;
	return;

}


evolexp(p,table,ntableau)
	char **p;
	char **table;	/* symbol talbe	*/
	int  *ntableau;	/* value table	*/
{
	char s[22];
	int q,d;
	register char *k;
	int nf,o,of;
	register i,j;
	char c;

	i =  nf = of  = 0;
	o = ' ';
	/* Old Macdonald had a farm, e = i = e = i = 0		*/

loop:	c = *(*p)++;
	if((c == ' ')||(c == '\t'))goto popout;
	if(c == ')'){
		if(of == 0)return(i);
popout:		if(of){panic("Paren garbege");	return(i);}
		if(of == 0){(*p)--;return(i);}
		}
	if(c == '\0'){panic("UnExEOF??");(*p)--;return(i);}
	if(c == '('){
		j = evolexp(p,table,ntableau);
		goto numerin;
		}
	if(c == '\''){
		c = *(*p)++;
		if(c == '\\'){
		    c = *(*p)++;
			switch(c){
				case 'n':	c = '\n';
					break;
				case 't':	c = '\t';
					break;
				case 'r':	c = '\r';
					break;
				case 'b':	c = 010;
					break;
				case 'e':	c = 04;	/* eot */
					break;
				case '0':
				if(islet(**p)== 0){
					c = enum(p);
					}
					else c = '\0';
					break;
				}
			}
		j = c;
		if(**p == '\'')(*p)++;
		goto numerin;
		}
	if(c == '*'){
		if(nf == 0){
			j = pc;
			goto numerin;
			}
		if(nf && of){
			j = pc;
			goto numerin;
			}
		}
	if((islet(c)== 0) || (c == '%') || (c == '$') ||
	(c == '\0100')){
		j = enum(p);
	numerin:
		if(nf == 0){i = j;nf = 1;goto loop;}
		if(of == 0){panic("2 #, no ops. ??");goto loop;}
		i = eval(i,j,o);	/* evaluate the binary	*/
		of = 0;
		o = ' ';
		goto loop;
		}

	if(c == '='){
		if(**p == '='){
			*(*p)++;
			o = '==';
			of = 1;
			goto loop;
			}
		if(((d = **p) == '+')||(d == '-')||(d == '*')||
			(d == '/')||(d == '%')||(d == '>')||
			(d == '<')||(d == '&')||(d == '^')||
			(d == '|')){
			if(d == '>'){d= '>>';*(*p)++;}
			if(d == '<'){d= '<<';*(*p)++;}
			*(*p)++;
			j = evolexp(p,table,ntableau);
			if((q = llu(s,table))== -1)
				panic("Lvalue required");
			  else {
				ntableau[q]= eval(ntableau[q],j,d);
				j = ntableau[q];	}
			goto closee;
			}
		j = evolexp(p,table,ntableau);
		if((q = llu(s,table))== -1)
			panic("Lvalue required");
		  else
			ntableau[q]= j;
	closee:	if((*((*p)-1)== ')')||(*((*p)-1)== ';')||
		(*((*p)-1)== ',')||(*((*p)-1)== '\n'))(*p)--;
		nf = 0;
		goto numerin;
		}

  /*	Not (,# or ) .  Should be operator or filler	*/
oper:	if(islet(c) == 2){
	if( of || !nf){
		switch(c){
			case '-':
				j = -uneval(p,table,ntableau);
				goto numerin;
			case '~':
				j = ~uneval(p,table,ntableau);
				goto numerin;
			case '!':
				j = !uneval(p,table,ntableau);
				goto numerin;
			case ';':
				goto popout;
			case '+':
				goto loop;
			case ',':
				goto popout;
			case '\n':
				goto popout;
			case '\t':
				goto popout;
			case ' ':
				goto popout;
			default:
				panic("Invalid Unary -");
				printf("%%%%\t\'");
				putchar(c & 0177);
				printf("\'\n");
				goto loop;
			}
		}
	if(c== ';')goto popout;
	if(c == ',')goto popout;
	if(c == '\n')goto popout;
	if(c == ' ')goto popout;
	if(c == '\t')goto popout;
	if((islet(**p)==2)&&(**p != '(')&&(**p != '\'')&&
	(**p != '-')&&(**p != '~')&&(**p != '!')&&(**p != '*') 	
	&&(**p != '$')&&(**p != '%')&&(**p != '\0100')){
		o = c << 8;
		o = o + *(*p)++;
		}
	else o = c;
	of = 1;
	}

	if(island(c)){
		k = s;
		(*p)--;
		while(island(*k++ = *(*p)++));
		(*p)--;
		*--k = '\0';
		if((q = llu(s,table)) == -1){
			panic("Undefined Symbol");
			printf("%%%%\t%s\n",s);
			goto loop;
				}
		j = ntableau[q];
		goto numerin;
		}
	goto loop;
}



enum(p)
	char **p;
{
	char s[22];
	register char *pp;
	register char *ll;
	int base;
	base = curbase;
	(*p)--;
	if(**p == '$'){base = 16;(*p)++;}
	if(**p == '%'){base =  2;(*p)++;}
	if(**p == '\0100'){base = 8;(*p)++;}
	pp = s;
	while(island(*pp++ = *(*p)++));	/* copy string */
	if(*(pp-1) != '.'){
		(*p)--;
		}
	   else pp++;
	*--pp = '\0';
	ll = --pp;
	pp = s;
	if(*ll == 'h'){base = 16;*ll = '\0';}
	if(*ll == 'q'){base = 8; *ll = '\0';}
	if(*ll == '.'){base = 10; *ll = '\0';}
	pp = s;
	while(*pp != '\0')
		if(islet(*pp++)!= 0)base = 16;
	return(basin(s,base));
}

eval(a1,a2,o)
	int a1,a2,o;
{
	switch(o){
	
	case '+':
		return(a1 + a2);
	case '-':
		return(a1 - a2);
	case '*':
		return(a1 * a2);
	case '&&':
		return(a1 && a2);
	case '||':
		return(a1 || a2);
	case '==':
		return(a1 == a2);
	case '=!':		/* really !=	*/
		return(a1 != a2);
	case '>>':
		return(a1 >> a2);
	case '<<':
		return(a1 << a2);
	case '^':
		return(a1 ^ a2);
	case '>':
		return(a1 > a2);
	case '<':
		return(a1 < a2);
	case '=<':		/* really >=	*/
		return(a1 <= a2);
	case '=>':		/* really >=	*/
		return(a1 >= a2);
	case '&':
		return(a1 & a2);
	case '|':
		return(a1 | a2);
	case '/':
		return(a1 / a2);
	case '%':
		return(a1 % a2);
	default:
		panic("invalid operator -\");
		o = o & 0177;
		printf("%%%%\t\'");
		putchar(o);
		printf("\'\n");
		return(0);
	}
}


uneval(p,table,ntableau)
	char **p,**table;
	int *ntableau;
{
	int *ss,*sss,s[50];
	register char c;
	register char *k;
	register i;

	ss = &s;
	sss = &ss;
	k = s;
	i = 0;
more:	while(island(c = (*k++ = *(*p)++)));
	if((c == ' ')||(c == '\t'))goto more;
	if(c == '(')i++;
	if((c == ')')&& i )i--;
	if(i)goto more;
	if(c != ')')k--;
	*k++ = ';';
	*k = '\0';
	(*p)--;
	return(evolexp(sss,table,ntableau));
}
