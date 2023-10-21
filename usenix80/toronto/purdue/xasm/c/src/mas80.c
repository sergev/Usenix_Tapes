#define ASVENO 3	/* assembler version number	*/
#define NINTVAR 12	/* number of internal variables	*/
#define NINCL 10	/* max # of nested 'include's	*/
#define NUMSYM 500	/* Symbol table lenght	*/
#define NUMMAC 70	/* max. number of macros	*/
#define	NCIM 800	/* max. number of char. in a macro	*/
/*
*
*	8080 absolute macro assembler
*
*	Peter D Hallenbeck
*
*	(c) copyright Oct, 1977
*
*
*/

	extern fin,fout;
	int stable[NUMSYM];		/* symbol table */
	char sstable[NUMSYM*6];
	int mbody[NUMMAC];		/* macro body table*/
	char mmbody[NUMMAC*50];
	int mdef[NUMMAC];		/* macro name definition table */
	char mmdef[NUMMAC*10];

	char *MI "Missing operhand";	/* error message */

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
		"if",
		"else",
		"endif",
		"base",
		"include",
		"liston",
		"listoff",
		"macon",
		"macoff",
		"binon",
		"binoff",
		"message",
		"da",
		-1
		};

	char *mot[]{	/* machine opcode table */
	"mov",
	"mvi",
	"lxi",
	"lda",
	"sta",
	"lhld",
	"shld",
	"ldax",
	"stax",
	"xchg",
	"add",
	"adi",
	"adc",
	"aci",
	"sub",
	"sui",
	"sbb",
	"sbi",
	"inr",
	"dcr",
	"inx",
	"dcx",
	"dad",
	"daa",
	"ana",
	"ani",
	"xra",
	"xri",
	"ora",
	"ori",
	"cmp",
	"cpi",
	"rlc",
	"rrc",
	"ral",
	"rar",
	"cma",
	"cmc",
	"stc",
	"push",
	"pop",
	"xthl",
	"sphl",
	"in",
	"out",
	"ei",
	"di",
	"hlt",
	"nop",
	"jmp",
	"jnz",
	"jz",
	"jnc",
	"jc",
	"jpo",
	"jpe",
	"jp",
	"jm",
	"call",
	"cnz",
	"cz",
	"cnc",
	"cc",
	"cpo",
	"cpe",
	"cp",
	"cm",
	"ret",
	"rnz",
	"rz",
	"rnc",
	"rc",
	"rpo",
	"rpe",
	"rp",
	"rm",
	"pchl",
	"rst",
	-1
	};

	int t[]{		/* type */
	1,
	2,
	3,
	4,
	4,
	4,
	4,
	5,
	5,
	6,
	7,
	8,
	7,
	8,
	7,
	8,
	7,
	8,
	9,
	9,
	5,
	5,
	5,
	6,
	7,
	8,
	7,
	8,
	7,
	8,
	7,
	8,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	5,
	5,
	6,
	6,
	8,
	8,
	6,
	6,
	6,
	6,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	10,
	0
	};

	int l[]{		/* lenght */
	1,
	2,
	3,
	3,
	3,
	3,
	3,
	1,
	1,
	1,
	1,
	2,
	1,
	2,
	1,
	2,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	2,
	1,
	2,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	1,
	1,
	1,
	1,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0
	};

	int mvt[]{		/* machine value table */
	64,
	6,
	1,
	58,
	50,
	42,
	34,
	10,
	2,
	235,
	128,
	198,
	136,
	206,
	144,
	214,
	152,
	222,
	4,
	5,
	3,
	11,
	9,
	39,
	160,
	230,
	168,
	238,
	176,
	246,
	184,
	254,
	7,
	15,
	23,
	31,
	47,
	63,
	55,
	197,
	193,
	227,
	249,
	219,
	211,
	251,
	243,
	118,
	0,
	195,
	194,
	202,
	210,
	218,
	226,
	234,
	242,
	250,
	205,
	196,
	204,
	212,
	220,
	228,
	236,
	244,
	252,
	201,
	192,
	200,
	208,
	216,
	224,
	232,
	240,
	248,
	233,
	199,
	0
	};

	int pc;		/* program counter	*/
	int pass;	/* pass number		*/
	char rlpass[26];/* nested pass # for symbols in macros	*/
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
	int curbase;	/* current radix for numbers */
	int macnest;	/* nested counter for macro-made variables */
	int macstack[50];/*nested macro variable list stack pointer*/
	int macflag;	/* macro expand flag, 1 = expand	*/
	int listflag;	/* listing enable falg, 1 = list	*/
	int binflag;	/* binary punch option flag 0 = punch	*/
	char dbuff[513];/* object file disk buffer	*/
	int dbuffc;	/* object file charctor counter */
	int ifd[NINCL];	/* include file descriptors	*/
	int ifdc;	/* nested 'include' fd counter	*/

main(argc,argv)
	int argc;
	char **argv;
{
	char line [121];
	register char *k,c;
	register i;
	char arg[22];
	int j,bc,s;
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
	macnest = 0;
	dbuffc = 0;
	macflag = listflag = 1;	/* print list and macro expantions */
	binflag = 0;	/* 0 means punch the binary	*/

	mixerup(stable,sstable);
	mixerup(mbody,mmbody);
	mixerup(mdef,mmdef);
	pcocnt = 0;
	pcolpc = 0;

	curbase = 10;
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
	for(j = 0;j != 9;j++)vtable[i++] = *kaboom++;

foobar:	while(passend){		/* in memory of AI Lab	*/
	if(vtable[0] > (NUMSYM -4)){
		panic("Symbol table overflow- increase NUMSYM");
		flush();
		}
	getline(line);
	cline(line);		/* get and process line	*/
	}
	if(pass == 2){
		if(pcocnt)pdump();
		pcobs = 0;
		pdump();
		fdput();
		close(fd);
		flush();
		fout = 1;
		if(numerrs){
		   printf("%d error%s\n",numerrs,numerrs==1?"":"s");
			}
		flush();
		return;
		}
	while(getchar());
	seek(fin,0,0);
	fout = creat("i.lst",0600);

/*			*/
	fd = creat("i.out",0600);

	pass = 2;
	lineno = 0;
	passend = 1;
	pc = 0;
	pcocnt = 0;
	pcost = 1;
	numerrs = 0;
	nestc = 0;
	macflag = listflag = 1;
	binflag = 0;
	goto foobar;
}

getline(line)
	char *line;
{
	register char *k;
	register char c;
	register i;
	int p,j,conc;

	conc = 0;
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
		if((c < ' ')&&(c != '\t'))conc++;
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
	if(conc && (pass == 1))panic("Warning- control charactors in line");
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
	char bline[NCIM];
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

getbits(s)
	char *s;
{
	register i;

	i = *s;
	i =+ (*(s+1)<<8);
	if((*(s+1)!= '\0')&&(*(s+2)!= '\0')){
	   if(*(s+2) == 'w')
	     if((*s == 'p')&&(*(s+1)== 's')&&(*(s+3)== '\0'))
		return(6);
		return(-1);
	     }
	switch(i){
		case 'a':
			return(7);
		case 'b':
			return(0);
		case 'c':
			return(1);
		case 'd':
			return(2);
		case 'e':
			return(3);
		case 'h':
			return(4);
		case 'l':
			return(5);
		case 'm':
			return(6);
		case 'sp':
			return(6);
		case 'ps':
			return(6);
		case 'bc':
			return(0);
		case 'de':
			return(2);
		case 'hl':
			return(4);
		default:
			return(-1);
		}
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
	jnum(lineno,10,4,0);
	printf(": Missing operhand after the ");
	ordnp(j++);
	printf(" comma\n");
}

panic(s)
	char *s;
{
	putchar('%');	putchar('%');
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
	if((ifdc== 0)&&(listflag== 1)&&((nestc && (macflag == 0))== 0)){
		jnum(lineno,10,4,0);
		printf(" ");
		jnum(pc,16,4,0);
		printf(" ");
		jnum(b1,16,2,0);
		putchar(' ');
		if(fb3){
			i = b3;
			i = (b3 & 0377) | (b2 << 8);
			if(b2 != 0)
				jnum(i,16,4,0);
			    else {
				if(b3 == 0){
					printf(" 000");
				    } else {
					if((b3 & 0360) == 0){
						printf(" 00");
						jnum(b3,16,1,0);
						}
					    else {
						printf(" 0");
						jnum(b3,16,2,0);
						}
					}
				}
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
	if((ifdc== 0)&&(listflag)&&((nestc && (macflag == 0))== 0))
		printf("\t\t ");
}
prl(line)
	char *line;
{
	register char *k;
	register char delay;
	if(ifdc)return;
	if(listflag == 0)return;
	if(nestc && (macflag == 0))return;
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
{	return(listflag && !ifdc && !(nestc && macflag));
}
pbytes()		/* all variables dealing with punching */
{			/* start with 'pco'.			*/
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
	dput(':');
	i = 0;
	pcoch = 0;
	pdumb(pcocnt);
	pdumb(pcobs >> 8);
	pdumb(pcobs & 0377);
	pdumb(0);
	while(pcocnt){
		pdumb(pcob[i++]);
		pcocnt--;
		}
	pdumb((~pcoch)+1);
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

fdput(){
	if(dbuffc)write(fd,&dbuff,dbuffc);
}

cline(line)
	char *line;
{
	char arg[21];
	char sarg[21];
	char lline[NCIM];
	char llline[100];
	int s;
	char **ss;
	int hlable;
	int hclable,hdclable;
	int ii,jj;
	int marg[10];
	char mmarg[110];
	int maclab[20];		/* macro lable save table	*/
	char mmaclab[150];
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
			hdclable = 0;
			(*ss)++;
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
	if(hlable && (hclable == 0)){
		panic("Warning- lable with no opcode");
		printf("%%%%\t%s\n",arg);
		}
	goto eocline;
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
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		pc =+ 1;
		goto eocline;
		}
	if(**ss == '\''){
	    ii = *ss;
	    (*ss)++;
	    if(**ss == '\\'){
		(*ss)++;
		if(**ss == '0'){
		    while(islet(**ss)== 0)(*ss)++;
		    (*ss)--;
		    }
		}
	    (*ss)++;
	    if((**ss == '\'')&&(*(*ss+1)!= '\'')){*ss = ii;goto p1dbn;}
	    *ss = ii;
	    (*ss)++;
	    ii = 1;
	    i = 0;
	    while(ii){
		while((**ss != '\'')&&(**ss != ';')&&(**ss != '\n')){
		    pc =+ 1;
		    (*ss)++;
		    }
		if(eqs(**ss)){
		    ii = 0;
		    panic("Missing closing quote in string");
		    }
		  else {
		    (*ss)++;
		    if(**ss == '\''){pc =+ 1; (*ss)++;}
		        else ii = 0;
		    }
		}
	    goto eocline;
	    }
    p1dbn:  j = (commaer(ss,1) -1);
	pc =+ j;
	while((**ss != '\n')&&(**ss != ';')){
		if(**ss == ','){
			(*ss)++;
			pc =+ 1;
			j =+ 1;
			eatspace(ss);
			while(**ss == ','){
				pc =+ 1;
				pcommer(j);
				j =+ 1;
				(*ss)++;
				eatspace(ss);
				}
			if((**ss == '\n')||(**ss == ';')){
				pcommer(j);
				printf("Zero assumed\n");
				(*ss)--;
				}
			}
		(*ss)++;
	}
	pc =+ 1;
	goto eocline;


   case 4:					/* ds */
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		goto eocline;
		}
	j = (commaer(ss,1)-1);
	while((**ss != '\n')&&(**ss != ';')){
	   if(**ss == '"'){
		i = 0;
		(*ss)++;
		while((**ss != '"')&&(**ss != '\n')){
			if(**ss == '\\')
				(*ss)++;
			(*ss)++;
			i++;
			}
		if(**ss == '\n'){
		   panic("Missing clossing quote in string");
		   pc =+ i;
		   goto eocline;
		   }
		(*ss)++;
		pc =+ i;
		eatspace(ss);
		}
	else pc =+ evolexp(ss,stable,vtable);
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
brigda1: eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		pc =+ 2;
		goto eocline;
		}
	j = commaer(ss,1) - 1;
	pc =+ j*2;
	while((**ss != '\n')&&(**ss != ';')){
	   if(**ss == ','){
		(*ss)++;
		j =+ 1;
		pc =+ 2;
		eatspace(ss);
		while(**ss == ','){
			pc =+ 2;
			pcommer(j);
			j =+ 1;
			(*ss)++;
			eatspace(ss);
			}
		if((**ss == '\n')||(**ss == ';')){
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
   p1org:  eatspace(ss);eatspace(ss);
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
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		goto eocline;
		}
	evolexp(ss,stable,vtable);
	goto eocline;


   case 8:					/* repeat */
   p1rep:   eatspace(ss);
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
			panic("Undefinded symbol:");
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

   p1dup:  eatspace(ss);
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
		lline[NCIM-1] = '\n';
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

   case 17:					/* if	*/
	p1if:  j =  evolexp(ss,stable,vtable);
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


   case 18:					/* else */
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


   case 19:					/* endif	*/
	p1endif:	if(ifcount == 0){
		panic("extra 'endif'");
		goto eocline;
		}
	ifcount--;
	goto eocline;

    case 20:					/* base		*/
p1base:	eatspace(ss);
	if(eqs(**ss)){
		panic("Missing  Operhand");
		goto eocline;
		}
	curbase = 10;
	curbase = evolexp(ss,stable,vtable);
	goto eocline;

    case 21:					/* include */
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

    case 22:					/* liston */
	goto eocline;

    case 23:					/* listoff */
	goto eocline;

    case 24:					/* macon */
	goto eocline;

    case 25:					/* macoff	*/
	goto eocline;

    case 26:					/* binon	*/
	goto eocline;

    case 27:					/* binoff	*/
	goto eocline;

    case 28:					/* message	*/
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

    case 29:					/* da (same as 'DW') */
	goto brigda1;


       }		/*  END OF SWITCH  */
     }

  else if((j = llu(sarg,mot))!= -1){
	pc=+ l[j];
	goto eocline;
	}
     panic("Undefined instruction:");
	printf("%%%%\t%s\n",sarg);
       goto eocline;


   }

		/* IN PASS TWO   */


	fb2 = fb3 = 0;
	if((**ss == ';')&&(*(*ss+1) == '\0')){putchar('\n');goto eocline;}
	if(islet(**ss) == 1){
	   if((i = sscan(arg,ss))== ':'){
		(*ss)++;
		hclable = 1;		/* ignore :*/
		if(**ss == ':'){
			hdclable = 0;	/* global macro symbol */
			(*ss)++;
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
		printf("%%%%\t%s\n",arg);
		}
	    prsl(line);
	    goto eocline;
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
	eatspace(ss);
	if(**ss == '\''){
	    ii = *ss;
	    (*ss)++;
	    if(**ss == '\\'){
		(*ss)++;
		if(**ss == '0'){
		    while(islet(**ss) == 0)(*ss)++;
		    (*ss)--;
		    }
		}
	    (*ss)++;
	    if((**ss == '\'')&&(*(*ss+1)!= '\'')){*ss = ii;goto p2dbn;}
	    prsl(line);
	    *ss = ii;
	    i = 20;
	    jj = 0;
	    ii =1;
	    (*ss)++;
	    while(ii){
		while(neqs(**ss)&& ii ){
		    if(**ss == '\\'){
			(*ss)++;
			b1 = **ss;
			if(b1 == 't')b1 = '\t';
			if(b1 == 'n')b1 = 012;
			if(b1 == 'r')b1 = 015;
			if(b1 == '0')b1 = '\0';
			if(b1 == 'b')b1 = 010;
			if(i < 16){
			    if(listflag && ((nestc && (macflag ==0))==0))
				putchar(' ');
			    pbytes();
			    if(listflag && ((nestc && (macflag ==0))==0))
				jnum(b1,16,2,0);
			    i++;
			    }
			  else {
			    if(jj && listflag && ((nestc && (macflag == 0)) == 0))
				putchar('\n');
			    prn();
			    i =1;
			    jj = 0;
			    }
			pc =+ 1;
			(*ss)++;
			}
		      else {
			if((**ss == '\'')&&(*(*ss+1)!= '\''))ii = 0;
			  else {
			    if(**ss == '\'')(*ss)++;
			    b1 = **ss;
			    if(i < 16){
			    if(listflag && ((nestc && (macflag ==0))==0))
				putchar(' ');
				pbytes();
			    if(listflag && ((nestc && (macflag ==0))==0))
				jnum(b1,16,2,0);
				i++;
				}
			      else {
				if(jj && listflag && ((nestc && (macflag == 0))==0))
				    putchar('\n');
				prn();
				i = 1;
				jj = 1;
				}
			    pc =+ 1;
			    (*ss)++;
			    }
			}
		    }
		}
	    if(listflag && ((nestc && (macflag == 0)) == 0))
		putchar('\n');
	    goto eocline;
	    }
    p2dbn:  b1 = evolexp(ss,stable,vtable);
	if(eqs(**ss)){
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
		if(i < 16){
			if(listflag && ((nestc && (macflag == 0))==0)){
			    putchar(' ');
			    jnum(b1,16,2,0);
			    }
			i++;
			}
		   else {
			if(listflag && ((nestc && (macflag == 0))==0))
			    putchar('\n');
			prn();
			i = 1;
			}
		pbytes();
		pc =+ 1;
		}
	if(listflag && ((nestc && (macflag == 0))==0))
	    putchar('\n');
	goto eocline;

    case 4:					/*  ds	*/
	prsl(line);
      p2dsb:  eatspace(ss);
	if(neqs(**ss)){
	while(**ss == ','){
		(*ss)++;
		eatspace(ss);
		}
	if(**ss == '"'){
		ii = 20;
		jj = 0;
      p2dsa:   (*ss)++;
		if(**ss == '\\'){
			(*ss)++;
			i = **ss;
			if(**ss == 't')i = '\t';
			if(**ss == 'n')i = 012;
			if(**ss == 'r')i = 015;
			if(**ss == 'e')i = 04;	/* eot */
			if(**ss == 'b')i = '\b';
			if(**ss == '0')i = '\0';
			b1 = i;
			if(ii < 16){
				if(listflag&& ((nestc&& (macflag ==0))==0))
				    putchar(' ');
				pbytes();
				if(listflag&& ((nestc&& (macflag ==0))==0))
				    jnum(b1,16,2,0);
				ii++;
				}
			   else {
				if(jj && listflag && ((nestc&& (macflag == 0))==0))
				    putchar('\n');
				prn();
				ii = 1;
				jj = 1;
				}
			pc =+ 1;
			goto p2dsa;
			}
		if((**ss != '"')&&(**ss != '\n')){
			b1 = **ss;
			if(ii < 16){
				if(listflag&& ((nestc&& (macflag ==0))==0))
				    putchar(' ');
				pbytes();
				if(listflag&& ((nestc&& (macflag ==0))==0))
				   jnum(b1,16,2,0);
				ii++;
				}
			   else {
				if(jj && listflag && ((nestc && (macflag == 0))==0))
				    putchar('\n');
				prn();
				ii = 1;
				jj = 1;
				}
			pc =+ 1;
			goto p2dsa;
			}
		if(**ss == '"')(*ss)++;
		if(ii != 1){
			if(listflag && ((nestc && (macflag ==0))==0))
			    putchar('\n');
			ii = 1;
			}
		goto p2dsb;
		}
	pc =+ evolexp(ss,stable,vtable);
	goto p2dsb;
	}
	goto eocline;

    case 5:					/*  dw	*/
			/* note the  adress byte swap */
brigda2: i = evolexp(ss,stable,vtable);
	b1 = i&0377;
	b2 = (i>>8)&0377;
	fb2 = 1;
	if(eqs(**ss)){
		prn();	prl(line);
		fb2 = 0;	pc =+ 2;
		goto eocline;
		}
	    else {
		prsl(line);	prn();
		}
	ii = 1;
	pc =+ 2;
	while(**ss == ','){
		(*ss)++;
		i = evolexp(ss,stable,vtable);
		b1 = i&0377;
		b2 = (i>>8)&0377;
		fb2 = 1;
		if(ii < 8){
			pbytes();
			if(listflag && ((nestc && (macflag ==0))==0)){
			    putchar(' ');
			    jnum(i,16,4,0);
			    }
			ii++;
			}
		   else {
			if(listflag && ((nestc && (macflag == 0))==0))
			    putchar('\n');
			prn();
			ii = 1;
			 }
		pc =+ 2;
		}
	fb2 = 0;
	if(listflag && ((nestc && (macflag ==0))==0))
	    putchar('\n');
	goto eocline;

    case 6:					/*   org	*/
	goto p1org;

    case 7:					/*  eval	*/
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

   case 17:					/* if	*/
	prsl(line);
	goto p1if;

   case 18:					/* else	*/
	prsl(line);
	goto p1else;

   case 19:					/* endif	*/
	prsl(line);
	goto p1endif;

    case 20:					/* base		*/
	prsl(line);
	goto p1base;

    case 21:					/* include	*/
	prsl(line);
	goto p1incl;

    case 22:					/* liston */
	listflag = 1;
	goto eocline;

    case 23:					/* listoff */
	listflag = 0;
	goto eocline;

    case 24:					/* macon	*/
	macflag = 1;
	goto eocline;

    case 25:					/* macoff	*/
	macflag = 0;
	goto eocline;

    case 26:					/* binon	*/
	binflag = 0;
	goto eocline;

    case 27:					/* binoff	*/
	binflag = 1;
	goto eocline;

    case 28:					/* message	*/
	goto p1err;

    case 29:					/* da		*/
	goto brigda2;

	   }	/*  END SWITCH  */
	}


	j = llu(sarg,mot);
	if(j == -1){
		panic("Undefined  Instruction:");
		printf("%%%%\t%s\n",sarg);
		prsl(line);
		goto eocline;
		}


	fb2 = fb3 = 0;
   switch(t[j]){
     case 1:				/* mov r1,r2 */
	b1 = mvt[j];
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	sscan(arg,ss);
	if((i = getbits(arg))== -1){
		panic("Bad destination register operhand");
		goto dumper;
		}
	ii = i;
	b1 =+ i<<3;
	eatspace(ss);
	if(eqs(**ss)){panic("Missing second operhand");goto dumper;}
	sscan(arg,ss);
	if((i = getbits(arg))== -1){
		panic("Bad source register operhand");
		goto dumper;
		}
	b1 =+ i;
	if((i == 6)&&(ii == 6)){
		panic("mov  m,m  is a halt.  Replaced with nop");
		b1 = 0;
		}
	goto dumper;

    case 2:				/* type mvi a,data */
	b1 = mvt[j];
	b2 = 0;
	fb2 = 1;
	if(i == -1)goto bdumper;
	eatspace(ss);
	sscan(arg,ss);
	if((i = getbits(arg))== -1){
		panic("Bad destination register");
		goto dumper;
		}
	b1 =+ i<<3;
	eatspace(ss);
	if(eqs(**ss)){panic("Missing imidiate operhand");goto dumper;}
	(*ss)++;
	b2 = evolexp(ss,stable,vtable);
	goto dumper;

    case 3:				/* type lxi rp,data */
	b1 = mvt[j];
	b2 = b3 = 0;
	fb2 = fb3 = 1;
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	sscan(arg,ss);
	if((i = getbits(arg))== -1){
		panic("Bad register pair operhand");
		goto dumper;
		}
	b1 =+ (i & 6)<<3;
	eatspace(ss);
	if(eqs(**ss)){
		panic("Missing imidiate operhand");
		goto dumper;
		}
	(*ss)++;
	i = evolexp(ss,stable,vtable);
	b2 = i&0377;
	b3 = ((i & 0177400) >> 8 ) & 0377;
	goto dumper;

    case 4:				/* type lda addr */
	b1 = mvt[j];
	b2 = b3 = 0;
	fb2 = fb3 = 1;
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	i = evolexp(ss,stable,vtable);
	b2 = i & 0377;
	b3 = (i>>8)& 0377;
	goto dumper;

    case 5:				/* type ldax rp */
	b1 = mvt[j];
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	sscan(arg,ss);
	if(((i = getbits(arg))== -1) || (i == 7) || (i == 1) || (i == 3) || (i == 5)){
	    if(i == -1){
		panic("Bad register pair");
		}
	      else {
		panic("Instuction requires a resgister pair and none was given");
		}
	    goto dumper;
	    }
	if((i == 4) && (b1 == 012)){
		panic("'ldax  h' is the same as a 'mov  a,m'.");
		panic("'mov  a,m' is substituted for it.");
		b1 = 0176;
		goto dumper;
		}
	if((i == 4) && (b1 == 2)){
		panic("'stax  h' is the same as a 'mov  m,a'.");
		panic("'mov  m,a' is substituted for it.");
		b1 = 0167;
		goto dumper;
		}
	if(((b1 == 0305)||(b1 == 0301)) && comstr(arg,"sp")){
		panic("Can not push or pop the stack pointer, dummy.");
		}
	b1 =+ ((i & 6)<<3);
	goto dumper;

    case 6:				/* type xchg (single byte) */
	b1 = mvt[j];
	goto dumper;

    case 7:			/* type  'add  r' */
	b1 = mvt[j];
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	sscan(arg,ss);
	if(((i = getbits(arg))== -1) || (arg[1] != '\0')){
		panic("Bad register");
		goto dumper;
		}
	b1 =+ i;
	goto dumper;

   case 8:			/* 'adi  data' */
	b1 =  mvt[j];
	b2 = 0;
	fb2 = 1;
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	b2 = evolexp(ss,stable,vtable);
	goto dumper;

    case 9:			/* 'inr  r' */
	b1 = mvt[j];
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	sscan(arg,ss);
	if((i = getbits(arg))== -1){
		panic("Bad register");
		goto dumper;
		}
	if(arg[1] != '\0'){
	    if(comstr(arg,"hl")||comstr(arg,"de")||comstr(arg,"bc")||comstr(arg,"sp")){
		if(b1 == 4){
		    panic("'inr' needs a single register. Use 'inx' for register pairs");
		    putchar('%');	putchar('%');
		    printf("\t'inx  %s' used instead of 'inr  %s'\n",arg,arg);
		    b1 = 3+ ((i & 6) << 3);
		    goto dumper;
		    }
		if(b1 == 5){
		    panic("'dcr' needs a single register.  Use 'dcx' for register pairs");
		    putchar('%');	putchar('%');
		    printf("\t'dcx  %s' used instead of 'inr  %s'\n",arg,arg);
		    b1 = 013 + ((i & 6) << 3);
		    goto dumper;
		    }
		}
	    panic("Bad register");
	}
	b1 =+ i<<3;
	goto dumper;

    case 10:			/* 'in  port' */
	b1 = mvt[j];
	if(i == -1)goto bdumper;
	eatspace(ss);
	if(eqs(**ss))goto bdumper;
	i = evolexp(ss,stable,vtable);
	b1 =+ (i&7)<<3;
	goto dumper;

    default:
	panic("Undefined mop type (internal error)");
	prs();
	prl(line);
	return;
	}

bdumper:	panic(MI);

dumper:	prn();
	prl(line);
	pc =+ l[j];
eocline:	vtable[2]++;
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
				case 'e':	c = 04;
					break;
				case 'b':	c = '\b';
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
		(*p)++;
		goto numerin;
		}
	if(c == '$'){
		j = pc;
		goto numerin;
		}
	if(islet(c) == 0){
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
	if((islet(**p)==2)&&(**p != '(')&&(**p != '\'')&&
	(**p != '-')&&(**p != '~')&&(**p != '!')){	/* compound op	*/
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
	(*p)--;
	pp = s;
	while(island(*pp++ = *(*p)++));	/* copy string */
	if(*(pp-1) != '.'){
		(*p)--;
		}
	  else pp++;
	*--pp = '\0';
	ll = --pp;
	pp = s;
	base = curbase;
	if(*pp == '0')base = 8;
	if((*pp == '0')&&(*(pp+1) == '0'))base = 16;
	if((*pp == '0')&&(*(pp+1) == '0')&&(*(pp+2) == '0'))base = 2;
	if(*ll == 'h'){base = 16;*ll = '\0';}
	if(*ll == 'q'){base = 8; *ll = '\0';}
	if(*ll == '.'){base = 10;  *ll = '\0';}
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
