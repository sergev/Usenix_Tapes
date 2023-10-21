#define ASVENO 2	/* assembler version number	*/
#define NINTVAR 12	/* number of internal varaibles	*/
#define NINCL 10	/* max # 'include's - 2	*/
#define NUMFR 500	/* max. number of forward referances */
#define NUMSYM 500	/* max number of symbols	*/
#define NUMMAC 70	/* max number of macros		*/
#define NCIM 600	/* max number of char. in a macro	*/
/*
*
*	6502 Macro Assembler
*
*	Peter D. Hallenbeck
*
*	(c) copyright July, 1978
*
*/

	extern fin,fout;
	int stable[NUMSYM];		/* symbol table */
	char sstable[NUMSYM*6];
	int mbody[NUMMAC];		/* macro body table*/
	char mmbody[NUMMAC*60];
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
		"entry",
		"not_re_entrant",
		"repeat",
		"fixtable",
		"expunge",
		"dup",
		"external",
		"defmacro",
		"fixmacro",
		"expunge_macro",
		"endmacro",
		"endallmacro",
		"enddup",
		"endalldup",
		"if",
		"else",
		"endif",
		"base",
		"listoff",
		"liston",
		"binoff",
		"binon",
		"include",
		"macoff",
		"macon",
		"message",
		-1
		};


	char *mot[]{		/* machine op code table */
		"adc",
		"and",
		"asl",
		"asla",
		"bcc",
		"bcs",
		"beq",
		"bit",
		"bmi",
		"bne",
		"bpl",
		"brk",
		"bvc",
		"bvs",
		"clc",
		"cld",
		"cli",
		"clv",
		"cmp",
		"cpx",
		"cpy",
		"dec",
		"dex",
		"dey",
		"eor",
		"inc",
		"inx",
		"iny",
		"jmp",
		"jsr",
		"lda",
		"ldx",
		"ldy",
		"lsr",
		"lsra",
		"nop",
		"ora",
		"pha",
		"php",
		"pla",
		"plp",
		"rol",
		"rola",
		"ror",
		"rora",
		"rti",
		"rts",
		"sbc",
		"sec",
		"sed",
		"sei",
		"sta",
		"stx",
		"sty",
		"tax",
		"tay",
		"tsx",
		"txa",
		"txs",
		"tya",
		-1
		};


	int mvt[][13]{		/* machine value table */
	105, 109, 101, 255, 255, 97, 113, 117, 125, 121, 255, 255, 255, 
	41, 45, 37, 255, 255, 33, 49, 53, 61, 57, 255, 255, 255, 
	255, 14, 6, 255, 255, 255, 255, 22, 30, 255, 255, 255, 255, 
	255, 255, 255, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 144, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 176, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 240, 255, 255, 
	255, 44, 36, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 48, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 208, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 16, 255, 255, 
	255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 80, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 112, 255, 255, 
	255, 255, 255, 255, 24, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 216, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 88, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 184, 255, 255, 255, 255, 255, 255, 255, 255, 
	201, 205, 197, 255, 255, 193, 209, 213, 221, 217, 255, 255, 255, 
	224, 236, 228, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	192, 204, 196, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 206, 198, 255, 255, 255, 255, 214, 222, 255, 255, 255, 255, 
	255, 255, 255, 255, 202, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 136, 255, 255, 255, 255, 255, 255, 255, 255, 
	73, 77, 69, 255, 255, 65, 81, 85, 93, 89, 255, 255, 255, 
	255, 238, 230, 255, 255, 255, 255, 246, 254, 255, 255, 255, 255, 
	255, 255, 255, 255, 232, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 200, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 76, 255, 255, 255, 255, 255, 255, 255, 255, 255, 108, 255, 
	255, 32, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	169, 173, 165, 255, 255, 161, 177, 181, 189, 185, 255, 255, 255, 
	162, 174, 166, 255, 255, 255, 255, 255, 255, 190, 255, 255, 182, 
	160, 172, 164, 255, 255, 255, 255, 180, 188, 255, 255, 255, 255, 
	255, 78, 70, 255, 255, 255, 255, 86, 94, 255, 255, 255, 255, 
	255, 255, 255, 74, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 234, 255, 255, 255, 255, 255, 255, 255, 255, 
	9, 13, 5, 255, 255, 1, 17, 21, 29, 25, 255, 255, 255, 
	255, 255, 255, 255, 72, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 8, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 104, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 40, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 46, 38, 255, 255, 255, 255, 54, 62, 255, 255, 255, 255, 
	255, 255, 255, 42, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 110, 102, 255, 255, 255, 255, 118, 126, 255, 255, 255, 255, 
	255, 255, 255, 106, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 64, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 96, 255, 255, 255, 255, 255, 255, 255, 255, 
	233, 237, 229, 255, 255, 225, 241, 245, 253, 249, 255, 255, 255, 
	255, 255, 255, 255, 56, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 248, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 120, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 141, 133, 255, 255, 129, 145, 149, 157, 153, 255, 255, 255, 
	255, 142, 134, 255, 255, 255, 255, 255, 255, 255, 255, 255, 150, 
	255, 140, 132, 255, 255, 255, 255, 148, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 170, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 168, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 186, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 138, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 154, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 152, 255, 255, 255, 255, 255, 255, 255, 255, 
	};

	int pc;		/* program counter	*/
	int pass;	/* pass number		*/
	char rlpass[24];/* nested pass # for macro lable hack	*/
	int lineno;	/* line number		*/
	int passend;	/* end of pass flag	*/
	int poe;	/* Point of Entry (or, Pesos on Everthing) */
	int rpoe;	/* not re-entrant flag (0 = re-entrant)	   */
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
	int macflag;	/* flag for macro code printing hack	*/
	int binflag;	/* flag for suppressed binary facility */
	int frtab[NUMFR];	/* forward referance table */
	int frtabp;	/* forward referance table pointer */
	int frtabo;	/* forward referance table overflow */
	int errloc;	/* evolexp error loc-out word	*/
	int errdet;	/* error detection flag	*/
	int basepp;	/* base page possible flag */
	char obuff[513];	/* binary output buffer */
	int obuffc;		/* bin out counter	*/
	int ifd[NINCL];	/* 'include' file descriptors	*/
	int ifdc;	/* nested 'include' counter	*/


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
	poe = 0;
	vtable[0] = NINTVAR + 1;
	vtable[1] = 0;
	vtable[2] = 0;
	rpoe = 0;
	stlen = 1;
	mtlen = 1;
	curbase = 10;
	macnest = 0;
	listflag = 1;
	macflag = 0;	/* 0 = enable listing of macro code	*/
	binflag = 0;	/* 0 means punch binary */
	frtabp = 0;
	frtabo = 0;
	errloc = 1;	/* 1 means print errors */
	obuffc = 0;
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
	    getline(line);
	    cline(line);		/* get and process line	*/
	    }
	if(pass == 2){
		if(pcocnt)pdump();
		put(';');	put('0');	put('0');
		put('\n');
		fput();
		fout = 1;
		if(numerrs){
		   printf("%d error%s\n",numerrs,numerrs==1?"":"s");
			}
		flush();
		return;
		}
	while(getchar());
	seek(fin,0,0);
	fout = creat("mt.lst",0604);

/*			*/
	fd = creat("mt.out",0604);
	if(fd == -1){panic("Can't make 'mt.out'.\n"); return;}

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
    if(!ifdc && canlist()){
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
	if(!ifdc && canlist())printf("\t\t ");
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
	put(';');
	i = 0;
	pcoch = 0;
	pdumb(pcocnt);
	pdumb(pcobs >> 8);
	pdumb(pcobs & 0377);
	while(pcocnt){
		pdumb(pcob[i++]);
		pcocnt--;
		}
	pdumb((pcoch) >> 8);
	pdumb((pcoch) & 0377);
	put('\n');
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
	put(i);
	i = n & 017;
	i =+ 060;
	if(i > '9')i =+ ('A'-':');
	put(i);
	pcoch =+ n;
}

stfr()		/* store fact that bad forward ref. was made */
{
	if(frtabo)return;	/* can't store if table overflowed */
	frtab[frtabp++] = pc;
	if(frtabp == NUMFR)frtabo = lineno + 1;
}

ckfr()		/* check for current operhand being unknown in pass 1*/
{
	register i;

	if(frtabo && (lineno > frtabo))return(0);
	for(i = 0;i < frtabp;i++)
		if(frtab[i] == pc)return(0);
	return(1);
}

put(item)
	char item;
{
	obuff[obuffc++] = item;
	if(obuffc == 512){
		write(fd,&obuff,512);
		obuffc = 0;
		}
}

fput()
{
	if(obuffc){
		write(fd,&obuff,obuffc);
		}
	close(fd);
	flush();
}

canlist(){
	return(listflag && !(nestc && macflag));
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
	int hclable;
	int hdclable;
	int ii,jj;
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
   if((**ss == ' ') && (*(*ss + 1) == 'a') &&
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
		panic("MI");
		chisel(arg,stable);
		vtable[0] =- 1;
		goto p1equa;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
		chisel(arg,stable);
		vtable[0] =- 1;
		goto p1equa;
		}
	vtable[i = llu(arg,stable)] = evolexp(ss,stable,vtable);
   p1equa:	if((pass == 2) && !ifdc && canlist()){
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
		panic("MI");
		pc =+ 1;
		goto eocline;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
		pc =+ 1;
		goto eocline;
		}
	j = (commaer(ss,1) -1);
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
 p1ds: if(i == -1){panic("MI");goto eocline;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
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
   p1dw: if(i == -1){panic("MI");
		pc =+ 2;
		goto eocline;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
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
   p1org:   if(i == -1){panic("MI");goto p1orga;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
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
   p1orga:	if((pass == 2) && !ifdc && canlist()){
			printf("     ");
			jnum(pc,16,4,0);
			printf("        ");
			prl(line);
			}
	goto eocline;


   case 7:					/* eval	*/
	if(i == -1){panic("MI");
		goto eocline;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
		goto eocline;
		}
	evolexp(ss,stable,vtable);
	goto eocline;


   case 8:					/* entry	*/
	if(i == -1){panic("MI");
		goto eocline;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
		goto eocline;
		}
	goto eocline;


   case 9:					/* not\_re_entrant */
	rpoe = -1;
	goto eocline;


   case 10:					/* repeat */
  p1rep:  if(i == -1){panic("Missing operator");goto p1repa;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("MI");
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


   case 11:					/* fixtab */
  p1fixt:  i = 1;
	while(stable[i++] != -1);
	stlen = --i;
	goto eocline;


   case 12:					/* expunge	*/
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


   case 13:					/* dup	*/

   p1dup:  if(i == -1){panic("MI");goto eocline;}
	eatspace(ss);
	if(eqs(**ss)){panic("MI");goto eocline;}
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


   case 14:					/* external */
	goto eocline;


  case 15:					/* defmacro */

   p1defm:	hlable = 0;
	b1 = 1;
	if(i == -1){panic("MI");goto eocline;}
	eatspace(ss);
	if(eqs(**ss)){panic("MI");goto eocline;}
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


  case 16:					/* fixmacro	*/
   p1fixm:  i =1;
	while(stable[i++] != -1);
	mtlen = --i;
	goto eocline;


  case 17:					/* expunge macro */
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


   case 18:					/* endmacro */
   p1endm:   panic("Extra 'endmacro'");
	goto eocline;


   case 19:					/* endallmacro */
    p1enda:	panic("Exta 'endallmacro'");
	goto eocline;


   case 20:					/* enddup */
   p1endd:   panic("Extra 'enddup'");
	goto eocline;


   case 21:					/* endalldup */
   p1ena:  panic("Extra 'endalldup'");
	goto eocline;



   case 22:					/* if	*/
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


   case 23:					/* else */
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


   case 24:					/* endif	*/
	p1endif:	if(ifcount == 0){
		panic("extra 'endif'");
		goto eocline;
		}
	ifcount--;
	goto eocline;

    case 25:					/* base 	*/
p1base:	eatspace(ss);
	if(eqs(**ss)){
		panic("MI\n");
		goto eocline;
		}
	curbase = 10;
	curbase = evolexp(ss,stable,vtable);
	goto eocline;

    case 26:					/* listoff */
	listflag = 0;
	goto eocline;

    case 27:					/* liston */
	listflag = 1;
	goto eocline;

    case 28:					/* binoff */
	binflag = 1;
	goto eocline;

    case 29:					/* binon */
	binflag = 0;
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

    case 32:				/* macon	*/
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
	/* now will munch on 6502 instruction	*/
	if(mvt[j][4] != 255){		/* implied */
		pc =+ 1;
		goto eocline;
		}
	if(mvt[j][10] != 255){		/* relative */
		pc =+ 2;
		goto eocline;
		}
	eatspace(ss);
	if(eqs(**ss)){
		if(mvt[j][3] != 255){	/* accumulator */
			pc =+ 1;
			goto eocline;
			}
		else panic("MI");
		goto eocline;
		}
	if(**ss == '#'){		/* imediate */
		pc =+ 2;
		goto eocline;
		}
	if(comstr(sarg,"jmp") && (**ss == '(')){
		pc =+ 3;
		goto eocline;
		}
	i =s;	ii = ss;	/* save place */
	if(**ss == '('){		/* possible inderect */
		jj = 1;
		(*ss)++;
indyq:		while((**ss != ',') && (**ss != ';') && (**ss != '(')&&
		(**ss != ')'))(*ss)++;
		switch(**ss){
			case ',':
				(*ss)++;
				if(jj == 1){
					goto indyx;
					}
				    else if(jj == 0){
					goto indyx;
					}
				    else panic("Bad operhand");
					goto eocline;
			case ';':
				if(jj)panic("Bad operhand");
				goto eocline;
			case '(':
				(*ss)++;
				jj++;
				goto indyq;
			case ')':
				(*ss)++;
				jj--;
				goto indyq;
			}
		indyx:	pc =+ 2;
		goto eocline;
		}
	s = i;	ss = ii;	/* try to evaluate */
	errloc = 0;		/* kill error output */
	errdet = 0;
	jj = evolexp(ss,stable,vtable);
	if(errdet)stfr();
	if(((jj & 0177400) == 0 ) && (errdet == 0) && (frtabo == 0))basepp = 1;
	    else basepp = 0;
	errloc = 1;
	if(**ss == ','){	/* have indexing */
		(*ss)++;
		if((**ss == 'x') && basepp && (mvt[j][7] != 255)){
			pc =+ 2;
			goto eocline;
			}
		if((**ss == 'x') && (mvt[j][8] != 255)){
			pc =+ 3;
			goto eocline;
			}
		if((**ss == 'y') && basepp && (mvt[j][12] != 255)){
			pc =+ 2;
			goto eocline;
			}
		if((**ss == 'y')&&(mvt[j][9] != 255)){  /* abs,y */
			pc =+ 3;
			goto eocline;
			}
		panic("Invalid indexing mode");
		goto eocline;
		}
	if(basepp && (mvt[j][2] != 255)){	/* zero page */
		pc =+ 2;
		goto eocline;
		}
	if(mvt[j][1] != 255){			/* absolute */
		pc =+ 3;
		goto eocline;
		}




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
  p2db:	b1 = evolexp(ss,stable,vtable);
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
			if(canlist()){
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
	if(canlist())putchar('\n');
	goto eocline;

    case 4:					/*  ds	*/
   p2ds: prsl(line);
      p2dsb:  eatspace(ss);
	if(neqs(**ss)){
	while(**ss == ','){
		(*ss)++;
		eatspace(ss);
		}
	if(**ss == '"'){
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
				if(canlist()){
					putchar(' ');
					jnum(b1,16,2,0);
					}
				ii++;
				}
			   else {
				if(jj && canlist())putchar('\n');
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
				j = 0;
				pbytes();
				if(canlist()){
					putchar(' ');
					jnum(b1,16,2,0);
					}
				ii++;
				}
			   else {
				if(jj && canlist())putchar('\n');
				prn();
				ii = 1;
				jj = 1;
				}
			pc =+ 1;
			goto p2dsa;
			}
		if(**ss == '"')(*ss)++;
		if((ii != 1) || j){
			if(listflag)putchar('\n');
			ii = 1;
			}
		goto p2dsb;
		}
	pc =+ evolexp(ss,stable,vtable);
	goto p2dsb;
	}
	goto eocline;

    case 5:					/*  dw	*/
  p2dw:	i = evolexp(ss,stable,vtable);
	b2 = (i >> 8) & 0377;
	b1 = i & 0377;
	fb2 = 1;
	if(eqs(**ss)){
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
		b1 = i&0377;
		b2 = (i>>8)&0377;
		fb2 = 1;
		if(ii < 8){
			pbytes();
			if(listflag){
				putchar(' ');
				jnum(i,16,4,0);
				}
			ii++;
			}
		   else {
			if(listflag)putchar('\n');
			prn();
			ii = 1;
			 }
		pc =+ 2;
		}
	fb2 = 0;
	if(listflag)putchar('\n');
	goto eocline;

    case 6:					/*   org	*/
	goto p1org;

    case 7:					/*  eval	*/
	evolexp(ss,stable,vtable);
	prsl(line);
	goto eocline;

    case 9:				/*  not_re_entrant	*/
	prsl(line);
	goto eocline;

    case 10:					/*  repeat	*/
	goto p1rep;

    case 11:					/*  fixtab  */
	prsl(line);
	goto p1fixt;

    case 12:					/*  expunge	*/
	prsl(line);
	goto p1exps;

    case 13:					/*  dup	*/
	prsl(line);
	goto p1dup;

    case 14:					/*  external	*/
	prsl(line);
	goto eocline;			/* all handled in pass one */

    case 15:					/*  defmacro */
	goto p1defm;

    case 16:					/*  fixmacro  */
	prsl(line);
	goto p1fixm;

    case 17:					/*  expunge\ macro*/
	prsl(line);
	goto p1expm;

    case 18:					/*  end macro */
	prsl(line);
	goto p1endm;

    case 19:					/*  endallmacro */
	prsl(line);
	goto p1enda;

    case 20:					/*  enddup	*/
	prsl(line);
	goto p1endd;

    case 21:					/*  endalldup	*/
	prsl(line);
	goto p1ena;


    case 22:					/* if	*/
	prsl(line);
	goto p1if;

    case 23:					/* else	*/
	prsl(line);
	goto p1else;

    case 24:					/* endif	*/
	prsl(line);
	goto p1endif;

    case 25:					/* base 	*/
	prsl(line);
	goto p1base;

    case 26:					/* listoff */
	listflag = 0;
	goto eocline;

    case 27:					/* liston */
	listflag = 1;
	goto eocline;

    case 28:					/* binoff */
	prsl(line);
	binflag = 1;
	goto eocline;

    case 29:					/* binon */
	prsl(line);
	binflag = 0;
	goto eocline;

    case 30:					/* include */
	prsl(line);
	goto p1incl;

    case 31:					/* macoff */
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
	if(mvt[j][4] != 255){		/* implied */
		b1 = mvt[j][4];
		j = 1;
		goto dumper;
		}
	if(eqs(**ss)){
		if(mvt[j][3] != 255){	/* accumulator */
			b1 = mvt[j][3];
			j = 1;
			goto dumper;
			}
		    else {panic("MI");	goto bdumb;}
		goto eocline;
		}
	if(mvt[j][10] != 255){		/* relative */
		b1 = mvt[j][10];
		fb2 = 1;
		j = evolexp(ss,stable,vtable);
		if(((j - pc) > 129)||((pc - j) > 125)){  /* range err*/
		    panic("Relative address out of range");
		    b2 = 0;
		    j = 2;
		    goto dumper;
		    }
		if(pc <= j){
		    b2 = (j - pc - 2);	/* forward */
		    }
		if(pc > j){
		    b2 = ~(pc -j +1);
		    }
		j = 2;
		goto dumper;
		}
	if(**ss == '#'){		/* imidiate */
		(*ss)++;
		if(mvt[j][0] == 255){
		     panic("Imediate mode on non-imediate instruction");
		     j = 2;
		     goto dumper;
		     }
		b1 = mvt[j][0];
		b2 = evolexp(ss,stable,vtable);
		fb2 = 1;
		j = 2;
		goto dumper;
		}
	if(comstr(sarg,"jmp") && (**ss == '(')){
		(*ss)++;
		j = evolexp(ss,stable,vtable);
		b1 = 0154;
		b2 = j & 0377;
		b3 = (j >> 8) & 0377;
		j = 3;
		fb2 = fb3 = 1;
		goto dumper;
		}
	i = s;	ii = ss;		/* save place */
	if(**ss == '('){		/* possible inderect */
	    jj = 1;
	    (*ss)++;
ind2yq:	    while((**ss != ',')&&(**ss != ';')&&(**ss != '(')&&
	    (**ss != ')'))(*ss)++;
	    switch(**ss){
		case ',':
		    (*ss)++;
		    if(jj == 1){
			goto ind2yx;
			}
			else if(jj == 0){
			    goto ind2yy;
			    }
			    else {panic("Bad operhand");
printf("%% Extra %d parenthesis.\n",jj);
				  goto bdumb;}
		case ';':
		    if(jj)panic("Bad operhand");
		    goto bdumb;
		case '(':
		    (*ss)++;
		    jj++;
		    goto ind2yq;
		case ')':
		    (*ss)++;
		    jj--;
		    goto ind2yq;
		}
ind2yx:	    s = i;	ss = ii;
	    (*ss)++;		/* skip first '(' */
	    if(mvt[j][5] == 255){
		panic("(ind,x) mode with non-inderect,x instruction");
		j = 2;
		goto dumper;
		}
	    b1 = mvt[j][5];
	    b2 = evolexp(ss,stable,vtable);
	    if(b2 & 0177400)panic("Base page adress error");
	    j = 2;
	    fb2 = 1;
	    goto dumper;
ind2yy:     s = i;	ss = ii;	(*ss)++; /* skip first '(' */
	    if(mvt[j][6] == 255){
		panic("(ind),y mode with non inderect,y instruction");
		j = 2;
		goto dumper;
		}
	    b1 = mvt[j][6];
	    b2 = evolexp(ss,stable,vtable);
	    if(b2 & 0177400)panic("Bass page adress error");
	    j = 2;	fb2 = 1;
	    goto dumper;
	    }
	s = i;	ss = ii;
	jj = evolexp(ss,stable,vtable);
	if(((jj & 0177400)==0)&& ckfr() &&((lineno < frtabo)||
	    (frtabo == 0)))basepp = 1;
	    else basepp = 0;
	if(**ss == ','){		/* have some indexing */
	    (*ss)++;
	    if((**ss == 'x') && basepp && (mvt[j][7] != 255)){
		b1 = mvt[j][7];
		b2 = jj;
		fb2 = 1;	j = 2;
		goto dumper;
		}
	    if((**ss == 'x')&& (mvt[j][8] != 255)){
		b1 = mvt[j][8];
		b2 = jj & 0377;
		b3 = (jj >> 8) & 0377;
		fb2 = fb3 = 1;	j = 3;
		goto dumper;
		}
	    if((**ss == 'y') && basepp && (mvt[j][12] != 255)){
		b1 = mvt[j][12];
		b2 = jj;
		fb2 = 1;	j = 2;
		goto dumper;
		}
	    if((**ss == 'y') && (mvt[j][9] != 255)){
		b1 = mvt[j][9];
		b2 = jj & 0377;
		b3 = (jj >> 8) & 0377;
		fb3 = fb2 = 1;	j = 3;
		goto dumper;
		}
	    panic("Invalid indexing mode");
	    goto eocline;
	    }
	if(basepp && (mvt[j][2] != 255)){	/* zero page */
		b1 = mvt[j][2];
		b2 = jj;
		fb2 = 1;	j = 2;
		goto dumper;
		}
	if(mvt[j][1] != 255){			/* absolute */
		b1 = mvt[j][1];
		b2 = jj & 0377;
		b3 = (jj >> 8) & 0377;
		fb2 = fb3 = 1;	j = 3;
		goto dumper;
		}


	panic("Internal error- type of instruction unknown");
	printf("J is %d, instruction is %s.\n",j,sarg);
for(i=0;i!=5;i++)printf("%d  ",mvt[j][i]);
putchar('\n');
	goto dumper;
bdumper:	panic("MI");
bdumb:	j = 2;

dumper:	prn();
	prl(line);
	pc =+ j;
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
	if(c == ')'){
		if(of == 0)return(i);
popout:		if(of){
			if(errloc)panic("Paren garbage");
			errdet++;
			return(i);}
		if(of == 0){(*p)--;return(i);}
		}
	if(c == '\0'){
			if(errloc)panic("UnExEOF??");
			errdet++;
			(*p)--;
			return(i);
			}
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
	if(c == '$'){
		j = pc;
		goto numerin;
		}
	if(islet(c) == 0){
		j = enum(p);
	numerin:
		if(nf == 0){i = j;nf = 1;goto loop;}
		if(of == 0){
			if(errloc)panic("2 #, no ops. ??");
			errdet++;
			goto loop;
			}
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
			if((q = llu(s,table))== -1){
				if(errloc)panic("Lvalue required");
				errdet++;
				}
			  else {
				ntableau[q]= eval(ntableau[q],j,d);
				j = ntableau[q];	}
			goto closee;
			}
		j = evolexp(p,table,ntableau);
		if((q = llu(s,table))== -1){
			if(errloc)panic("Lvalue required");
			errdet++;
			}
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
				if(errloc)panic("Invalid Unary -");
				if(errloc)printf("%%%%\t\'");
				if(errloc)putchar(c & 0177);
				if(errloc)printf("\'\n");
				errdet++;
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
			if(errloc)panic("Undefined Symbol");
			if(errloc)printf("%%%%\t%s\n",s);
			errdet++;
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
		if(errloc)panic("invalid operator -\");
		o = o & 0177;
		if(errloc)printf("%%%%\t\'");
		if(errloc)putchar(o);
		if(errloc)printf("\'\n");
		errdet++;
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
