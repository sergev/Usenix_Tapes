#define ASVENO 6	/* assembler version number	*/
#define	NUMEXT	500	/* max. number of external refernces */
#define NINCL 10	/* max # of recursive includes - 2 */
#define NUMSYM 600	/* max # of symbols	*/
#define NUMGLOB 70	/* max # of globals	*/
#define NUMLOAD 15	/* max. # of force loaded moduals	*/
#define	NUMMAC 60	/* max # of macros	*/
#define	NCIM 600	/* max # of charactors in a macro	*/
#define	MAXED 60	/* max. # of external variables	*/
#define NINTVAR 12	/* number of internal variables	*/
/*
*
*	6800 relocatible macro assembler
*
*	Peter D Hallenbeck
*
*	(c) copyright Dec, 1977
*
*/

	extern fin,fout;
	int stable[NUMSYM];		/* symbol table */
	char sstable[NUMSYM*8];
	int mbody[NUMMAC];		/* macro body table*/
	char mmbody[NUMMAC*70];
	int mdef[NUMMAC];		/* macro name definition table */
	char mmdef[NUMMAC*10];
	int edtab[MAXED];		/* external definition table */
	char eedtab[MAXED*10];
	int exrtab[NUMEXT];	/* ext. ref. index to name of ext. */
	int exatab[NUMEXT];	/* ext. ref. tab. phys. address in program */
	int globs[NUMGLOB];	/* global symbol table	*/
	char gglobs[NUMGLOB*8];
	int loadtab[NUMLOAD];	/* force loaded modaul table	*/
	char lloadtb[NUMLOAD*7];

	char *MI "Missing operhand";	/* error massage */


	extern char *soptab[],*mot[],*mot10[],*mot11[],*spmot[];
	extern char mvt[][6],mlt[],lmlt10[],lmlt11[];

	int pc;		/* program counter	*/
	int pass;	/* pass number		*/
	char rlpass[24];/* nested pass # for macro lables hack	*/
	int lineno;	/* line number		*/
	int passend;	/* end of pass flag	*/
	int poe;	/* Point of Entry (or, Pasos on Everthing) */
	int stlen;	/* symbol table lenght	*/
	int mtlen;	/* macro table lenght	*/
	int vtable[NUMSYM];/* symbol table value	*/
	int prebyte;	/* prebyte definition	*/
	int b1,b2,b3,fb2,fb3,b4,fb4;
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
	int relob;	/* relocation bits	*/
	int symtabr;	/* symbol table referance counter	*/
	int relocate;	/* relocatoion flag	*/
	int exttabr;	/* ext. ref. table flag	*/
	int extrtp;	/* ext. ref. table next entry pointer */
	int curbase;	/*  current radix for numbers	*/
	int macnest;	/* nested macro counter		*/
	int macstack[50];/*nested macro variable list stack pointer*/
	int listflag;	/* flag for list suppression		*/
	int macflag;	/* 0 = list expanded macro code	*/
	int binflag;	/* flag for supprision of binary out	*/
	int cytime;	/* cycle time counter	*/
	char dbuff[513];/* object output data buffer	*/
	int dbuffc;	/*  "      "   buffer charactor counter	*/
	int ifd[NINCL];	/* 'include' file descriptors	*/
	int ifdc;	/* counter for 'include' file descriptors */
	char ibuff[513];/* 1st include file buffer	*/
	char *ibuffp;	/* 1st include file buffer pointer	*/
	int ibuffc;	/* 1st include file buffer counter	*/
	char indir;	/* indirect flag	*/


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
	extrtp = 0;
	vtable[0] = NINTVAR + 1;
	vtable[1] = 0;
	vtable[2] = 0;
	stlen = 1;
	mtlen = 1;
	macnest = 0;
	curbase = 10;
	listflag = 1;
	macflag = 0;	/* enable listing of expanded macro code */
	binflag = 0;
	cytime = 0;
	dbuffc = 0;
	ifdc = 0;
	ibuffc = 0;	/* zero 1st include buffer counter */

	mixerup(stable,sstable);
	mixerup(mbody,mmbody);
	mixerup(mdef,mmdef);
	mixerup(edtab,eedtab);
	mixerup(globs,gglobs);
	mixerup(loadtab,lloadtb);
	pcocnt = 0;
	pcolpc = 0;


	pc = 0;
	numerrs = 0;
	nestc = 0;
	fin = open(argv[1],0);
	if(fin == -1){
	    printf("Can't find '%s'.\n",argv[1]);
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
	if(vtable[0] > (NUMSYM - 2)){
		panic("Symbol table overflow- increase NUMSYM");
		}
	getline(line);
	cline(line);		/* get and process line	*/
	}
	if(pass == 2){
		if(pcocnt)pdump();
		while(loadtab[1] != -1){
			dput('S');	dput('3');
			k = loadtab[1];
			while(*k)dput(*k++);
			dput('\n');
			chisel(1,loadtab);
			}
		j = 1;
		while(globs[j] != -1){
			if((i = llu(globs[j],stable)) == -1){
				panic("Undefined global:");
				printf("%%%% %s\n",globs[1]);
				}
			    else {
				dput('S');	dput('4');
				pdumb((vtable[i] >> 8) & 0377);
				pdumb(vtable[i] & 0377);
				k = globs[j];
				while(*k)dput(*k++);
				dput('\n');
				}
			j++;
			}
		dput('S');
		dput('6');
		pdumb(poe >> 8);	pdumb(poe & 0377);
		dput('\n');
		i = 0;
		while(i < extrtp){
		    dput('S');
		    dput('7');
		    pdumb((exatab[i])>> 8);
		    pdumb(exatab[i] & 0377);
		    k = edtab[exrtab[i]];
		    while(*k != '\0'){
			dput(*k++);
			}
		    dput('\n');
		    i++;
		    }
		dput('S');
		dput('8');
		pdumb(pc >> 8);	pdumb(pc & 0377);
		dput('\n');
		i = 1;		/* dump symbols	*/
		while(stable[i] != -1){
		    if(llu(stable[i],globs) == -1){
			dput('S');	dput('A');
			pdumb((vtable[i] >> 8) & 0377);
			pdumb(vtable[i] & 0377);
			k = stable[i];
			while(*k != '\0')dput(*k++);
			dput('\n');
			}
		    i++;
		    }
		dput('S');
		dput('9');
		dput('\n');
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
	k = argv[1];	i = 0;
	while(line[i++] = *k++);
	cats(line,".lst");
	fout = creat(line,0604);

/*			*/
	k = argv[1];	i = 0;
	while(line[i++] = *k++);
	cats(line,".rel");
	fd = creat(line,0604);
	dput('S');	dput('0');
	k = argv[1];
	while(*k)dput(*k++);
	dput('\n');

	pass = 2;
	lineno = 0;
	passend = 1;
	pc = 0;
	pcocnt = 0;
	relob = 0;
	relocate = 0;
	exttabr = 0;
	pcost = 1;
	numerrs = 0;
	nestc = 0;
	listflag = 1;
	macflag = 0;
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
			if(ifdc == 1)ibuffc = 0;
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
		if(ifdc == 1){
			if(ibuffc == 0){
				ibuffc = read(ifd[ifdc],&ibuff,512);
				ibuffp = ibuff;
				if(ibuffc == 0)return('\0');
				}
			ibuffc--;
			return(*ibuffp++);
			}
		    else {
			count = read(ifd[ifdc],&data,1);
			if(count == 0)data = '\0';
			}
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
	int i,j;
    if(flag != 0){
	k = bline;
	while(*body != '\0'){
		switch(c = *body++){
			case '&':
			    if(flag){
				c = *body++;
				if(c >= 'A'){
					line[0] = c;
					line[1] = '\0';
					j = llu(line,stable);
					if(j == -1){
						panic("Undef?");
						j = 1;
						}
					j = vtable[j];
					i = *body++ - '0';
					*k++ = arglist[i][j];
					}
				  else {
					i = c - '0';
					p = arglist[i];
					while(*k++ = *p++);
					k --;
					}
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
    if(!ifdc && listflag && !(nestc && macflag)){
	jnum(lineno,10,4,0);
	printf(" ");
	jnum(pc,16,4,0);
	printf("  ");
	if(prebyte)phexd(prebyte);
		else printf("  ");
	b1 =& 0377;
	phexd(b1);
	putchar(' ');
	if(fb2)phexd(b2);
		else printf("  ");
	if(fb3)phexd(b3);
		else printf("  ");
	if(fb4)phexd(b4);
		else printf("  ");
	printf(" ");
	}
    pbytes();
}
prs()
{
	if(!ifdc && listflag && !(nestc && macflag))
	printf("		       ");
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
{	return(listflag && !ifdc & !(nestc && macflag));
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
	if(prebyte){
	    pcob[pcocnt++]= prebyte;	/* if prefix byte */
	    pcolpc++;
	    relob = relob << 1;
	    }
	pcob[pcocnt++]= b1;
	relob = (relob << 1) + (relocate == 1);
	pcolpc =+ 1;
	if(fb2){
	    pcob[pcocnt++] = b2;	/* enter bytes */
	    pcolpc =+ 1;
	    relob = (relob << 1) + (relocate == 2);
		}
	if(fb3){
	    pcob[pcocnt++] = b3;
	    pcolpc =+ 1;
	    relob = (relob << 1) + (relocate == 3);
		}
	if(fb4){
	    pcob[pcocnt++] = b4;
	    pcolpc =+ 1;
	    relob = relob << 1;
	    }
	if(pcocnt > 10){
	    pdump();		/* if line is full */
	    return;
		}
}
/*	Note: relacation bits are a right-justified template
		for the bytes					*/

pdump()
{
	register i;
	int j;
	dput('S');	dput('2');
	i = 0;
	pcoch = 0;
	pdumb(pcocnt+5);
	pdumb(relob >> 8);
	pdumb(relob & 0377);
	pdumb(pcobs >> 8);
	pdumb(pcobs & 0377);
	while(pcocnt){
		pdumb(pcob[i++]);
		pcocnt--;
		}
	pdumb(~pcoch);
	dput('\n');
	relob = 0;
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


phexd(n)
	int n;		/* type a hex byte	*/
{
	register i,j;
	i = n;
	i = (i & 0360) >> 4;
	i =+ 060;
	if(i > '9')i =+ ('a'- ':');
	putchar(i);
	i = n & 017;
	i =+ 060;
	if(i > '9')i =+ ('a' - ':');
	putchar(i);
}


ppb(c)
	char c;		/* return push/pull bit for register */
{
	switch(c){
		case 'a':
			return(2);
		case 'b':
			return(4);
		case 'x':
			return(020);
		case 'p':
			return(0200);
		case 'u':
			return(0100);
		case 'y':
			return(040);
		case 'c':
			return(1);
		case 'd':
			return(010);
		}
}

ibr(c)
	char c;
/*			return bits for index stuff	*/
{
	switch(c){
	case 'x':
		return(0);
	case 'y':
		return(1);
	case 'u':
		return(2);
	case 's':
		return(3);
	}
}

setpre(s)
	char *s;
{
	if(llu(s,mot10) != -1)prebyte = 16;
	if(llu(s,mot11) != -1)prebyte = 17;
}

tfrb(c)
	char c;
{
	switch(c){
		case 'd':
			return(0);
		case 'x':
			return(1);
		case 'y':
			return(2);
		case 'u':
			return(3);
		case 's':
			return(4);
		case 'p':
			return(5);
		case 'a':
			return(010);
		case 'b':
			return(011);
		case 'c':
			return(012);
		}
	return(0);
}

opsize(k)		/* see if operhand is a simple single number */
	char **k;
{
	register i;
	register char *p,*sp;
	char iflag;

	p = *k;			/* get copy of pointer */
	sp = p;
	iflag = 0;
	if(*p == '['){
		iflag = 1;
		p++;
		}
	if(*p == ',')return(0);		/* indexed, no offset */
	if(*(p+1) == ','){
		if((*p == 'a')||(*p == 'b')||(*p == 'd'))
			return(0);	/* register offset */
		}
	if(*p == '-')p++;
	if(islet(*p) == 0){
		p++;
		while((*p != ',')&&(*p != ';')&&(*p != '\n')){
		    if(island(*p++) != 1)return(2);	/* not number*/
		    }
		i = evolexp(k,stable,vtable);	/* evaluate number */
		*k = sp;	/* restore pointer for world	*/
		if(i < 0)i = -i;
		if(i <16){
			if(iflag)return(1);
			return(0);
			}
		if(i < 128)
			return(1);
		return(2);
		}
	return(2);
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
	char *savess;		/* ss save space */
	char ncount;		/* # extra bytes for index stuff */
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
		if(**ss == ':'){	/* ::	*/
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
	if(hlable && (hclable == 0)){
		panic("Warning- lable with no opcode");
		printf("%%%%\t%s\n",arg);
		}
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
   p1equa:	if((pass == 2)&&(canlist() && !ifdc)){
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
 p1ds: if(i == -1){panic(MI);goto eocline;}
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
   p1orga:	if((pass == 2) && !ifdc && canlist()){
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


   case 8:					/* entry	*/
	if(i == -1){panic(MI);
		goto eocline;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic(MI);
		goto eocline;
		}
	goto eocline;


   case 9:					/* repeat */
  p1rep:  if(i == -1){panic("MI");goto p1repa;}
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


   case 10:					/* fixtab */
  p1fixt:  i = 1;
	while(stable[i++] != -1);
	stlen = --i;
	goto eocline;


   case 11:					/* expunge	*/
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


   case 12:					/* dup	*/

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


   case 13:					/* external */
	if(i != -1){
		eatspace(ss);
		if((**ss == '\n')|(**ss == ';'))i = -1;
		}
	if(i == -1){
		panic(MI);
		goto eocline;
		}
	commaer(ss,1);
	j = 0;
   lop2:  i = sscan(arg,ss);
	if(arg[0] == '\0')pcommer(j);
	  else {
		if(llu(arg,edtab) != -1){
		    panic("Warning -double defined external");
		    printf("%%%%\t%s\n",arg);
		    }
		  else cement(arg,edtab);
	   }
	if(i == -1)goto eocline;
	eatspace(ss);
	if(**ss == ','){
		j =+ 1;
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			goto eocline;
			}
		else goto lop2;
		}
	if((**ss == '\n')||(**ss == ';'))goto eocline;
	panic("Extraneous symbols in operhand field");
	goto eocline;


  case 14:					/* defmacro */

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


  case 15:					/* fixmacro	*/
   p1fixm:  i =1;
	while(stable[i++] != -1);
	mtlen = --i;
	goto eocline;


  case 16:					/* expunge macro */
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


   case 17:					/* endmacro */
   p1endm:   panic("Extra 'endmacro'");
	goto eocline;


   case 18:					/* enddup */
   p1endd:   panic("Extra 'enddup'");
	goto eocline;


   case 19:					/* fcb	*/
   p1fcb:	goto p1db;
	goto eocline;

    case 20:					/* fdb	*/
   p1fdb:	goto p1dw;
	goto eocline;

    case 21:					/* fcc */
    p1fcc:	goto p1ds;
	goto eocline;


    case 22:				/* rmb	*/
	goto p1ds;


   case 23:					/* if	*/
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


   case 24:					/* else */
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


   case 25:					/* endif	*/
	p1endif:	if(ifcount == 0){
		panic("extra 'endif'");
		goto eocline;
		}
	ifcount--;
	goto eocline;

    case 26:					/* base		*/
p1base:	eatspace(ss);
	if(eqs(**ss)){
		panic(MI);
		goto eocline;
		}
	curbase = 10;
	curbase = evolexp(ss,stable,vtable);
	goto eocline;

    case 27:					/* listoff */
	listflag = 0;
	goto eocline;

    case 28:					/* liston	*/
	listflag = 1;
	goto eocline;

    case 29:					/* binoff */
	binflag = 1;
	goto eocline;

    case 30:					/* binon */
	binflag = 0;
	goto eocline;

    case 31:					/* cycle_time */
	goto eocline;


    case 32:					/* include */
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

    case 33:					/* macoff	*/
	goto eocline;

    case 34:					/* macon	*/
	goto eocline;

    case 35:					/* global	*/
	eatspace(ss);
	if(eqs(**ss))panic(MI);
	goto eocline;

   case 36:					/* loadmod */
	eatspace(ss);
	if(eqs(**ss)){
		panic(MI);
		goto eocline;
		}
	commaer(ss,1);
	j = 0;
   lop7:  i = sscan(arg,ss);
	if(arg[0] == '\0')pcommer(j);
	  else {
		if(llu(arg,loadtab) != -1){
		    panic("Warning -double defined external");
		    printf("%%%%\t%s\n",arg);
		    }
		  else cement(arg,loadtab);
	   }
	if(i == -1)goto eocline;
	eatspace(ss);
	if(**ss == ','){
		j =+ 1;
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			goto eocline;
			}
		else goto lop7;
		}
	if((**ss == '\n')||(**ss == ';'))goto eocline;
	panic("Extraneous symbols in operhand field");
	goto eocline;

    case 37:					/* message	*/
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

    else if(llu(sarg,spmot) != -1){
	pc =+ 2;
	goto eocline;		/* special 6800 look-alike cases */
	}

  else if((j = llu(sarg,mot))!= -1){
	eatspace(ss);
	savess = *ss;		/* save ss for later analyisis */
	if(mvt[j][4] != 1){
		pc =+ 1;
		j = mvt[j][4];
		if((j >= 52) && (j <= 55))pc++;	/* psh and pul */
		if((j == 30) || (j == 31))pc++;	/* tfr or exg */
		if(llu(sarg,mot10) != -1)pc =+ 1;
		if(llu(sarg,mot11) != -1)pc =+ 1;
		goto eocline;		/* inherant */
		}
	if(eqs(**ss))panic(MI);
	if((**ss)== '#'){
		if((llu(sarg,mot10) != -1) || (llu(sarg,mot11)!= -1)){
			pc =+ 4;
			}
		    else pc =+ mlt[mvt[j][0] & 0377];
		if(mvt[j][0] == 1)panic("Invalid mode");
		goto eocline;
		}
	ncount = opsize(ss);
	while((**ss != ';')&&(**ss != ','))(*ss)++;
	if(**ss == ','){
		(*ss)++;
		if((**ss == 'x')||(**ss == 'y')||(**ss == 's')||(**ss == 'u')){
			if((llu(sarg,mot10)!= -1) || (llu(sarg,mot11)!= -1)){
				pc =+ 3;	/* indexed */
				}
			    else pc =+ mlt[mvt[j][2] & 0377];
			if(*savess != ','){
				savess++;
				if((*savess != 'a')&&(*savess != 'b')&&(*savess != 'd'))pc =+ ncount;
				    else{
					savess++;
					if(*savess != ',')pc =+ ncount;
					}
				}
			if(mvt[j][2] == 1)panic("Invalid mode");
			goto eocline;
			}
		if(**ss == 'd'){
				/* base-mode address */
			if((llu(sarg,mot10)!= -1) || (llu(sarg,mot11)!= -1)){
				pc =+ 3;
				}
			    else pc =+ mlt[mvt[j][1]];
			if(mvt[j][1] == 1)panic("Invalid mode");
			goto eocline;
			}
		if(**ss == 'p'){		/* Pc relative	*/
			if((llu(sarg,mot10)!= -1) || (llu(sarg,mot11)!= -1)){
				pc =+ 5;
				}
			    else pc =+ 4;
			if(mvt[j][2] == 1)panic("Invalid mode");
			goto eocline;
			}
		panic("Odd addressing mode");
		}
	if(mvt[j][3] != 1){
		if((llu(sarg,mot10)!= -1) || (llu(sarg,mot11)!= -1)){
			pc =+ 4;
			}		/* extended	*/
		    else pc =+ mlt[mvt[j][3] & 0377];
		if(*savess == '[')pc =+ 1;	/* if extended indirect */
		goto eocline;
		}
	if(mvt[j][5] != 1){
		if((llu(sarg,mot10)!= -1) || (llu(sarg,mot11)!= -1)){
			pc =+ 4;
			}		/* relative	*/
		    else pc =+ mlt[mvt[j][5] & 0377];
		goto eocline;
		}
	panic("Valid instruction with invalid mode");
	}
     panic("Undefined instruction:");
	printf("%%%%\t%s\n",sarg);
       goto eocline;



   }

		/* IN PASS TWO   */


	symtabr = 0;	exttabr = 0;	relocate = 0;
	fb2 = fb3 = prebyte = indir = 0;
	b1 = 18;
	if(extrtp >= NUMEXT){
		panic("Out of external symbol referance table space");
		panic("Internally, change 'NUMEXT' to increase table");
		extrtp =- 1;
		}

	if((**ss == ';')&&(*(*ss+1) == '\0')){putchar('\n');goto eocline;}
	if(islet(**ss) == 1){
	   if((i = sscan(arg,ss))== ':'){
		(*ss)++;
		hclable = 1;		/* ignore :*/
		if(**ss == ':'){	/* ::	*/
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
		printf("%%%%\t%s\n",arg);
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
	return;

    case 3:					/*  db	*/
  p2db:	b1 = evolexp(ss,stable,vtable);
	if(eqs(**ss)){
		prn();	prl(line);   pc =+ 1;
		goto eocline;
		}
	    else {
		prsl(line);	prn();
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
				pbytes();
				if(canlist()){
					j = 0;
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
				if(canlist()){
					j = 0;
					putchar(' ');
					jnum(b1,16,2,0);
					}
				pbytes();
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
			if(canlist())putchar('\n');
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
	if(symtabr)relocate = 1;
	if(exttabr){
		exrtab[extrtp] = exttabr;
		exatab[extrtp++] = pc;
		}
	b2 = i&0377;
	b1 = (i>>8)&0377;
	fb2 = 1;
	if(eqs(**ss)){
		prn();	prl(line);	fb2 = 0;
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
		symtabr = 0;	exttabr = 0;
		i = evolexp(ss,stable,vtable);
		b2 = i&0377;
		b1 = (i>>8)&0377;
		if(symtabr)relocate = 1;
		if(exttabr){
			exrtab[extrtp] = exttabr;
			exatab[extrtp++] = pc;
			}
		fb2 = 1;
		if(ii < 8){
			pbytes();
			if(canlist()){
				putchar(' ');
				jnum(i,16,4,0);
				}
			ii++;
			}
		   else {
			if(canlist())putchar('\n');
			prn();
			ii = 1;
			 }
		pc =+ 2;
		}
	fb2 = 0;
	if(canlist())putchar('\n');
	goto eocline;

    case 6:					/*   org	*/
	goto p1org;

    case 7:					/*  eval	*/
	evolexp(ss,stable,vtable);
	prsl(line);
	goto eocline;

    case 8:					/* entry */
	prsl(line);
	eatspace(ss);
	if(eqs(**ss)){
		panic(MI);
		goto eocline;
		}
	poe = evolexp(ss,stable,vtable);
	goto eocline;

    case 9:					/*  repeat	*/
	goto p1rep;

    case 10:					/*  fixtab  */
	prsl(line);
	goto p1fixt;

    case 11:					/*  expunge	*/
	prsl(line);
	goto p1exps;

    case 12:					/*  dup	*/
	prsl(line);
	goto p1dup;

    case 13:					/*  external	*/
	prsl(line);
	goto eocline;			/* all handled in pass one */

    case 14:					/*  defmacro */
	goto p1defm;

    case 15:					/*  fixmacro  */
	prsl(line);
	goto p1fixm;

    case 16:					/*  expunge\ macro*/
	prsl(line);
	goto p1expm;

    case 17:					/*  end macro */
	prsl(line);
	goto p1endm;

    case 18:					/*  enddup	*/
	prsl(line);
	goto p1endd;

    case 19:					/* fcb */
	goto p2db;

    case 20:					/* fdb */
	goto p2dw;

    case 21:					/* fcc */
	goto p2ds;

    case 22:					/* rmb	*/
	goto p2ds;

    case 23:					/* if	*/
	prsl(line);
	goto p1if;

    case 24:					/* else	*/
	prsl(line);
	goto p1else;

    case 25:					/* endif	*/
	prsl(line);
	goto p1endif;

    case 26:					/* base		*/
	prsl(line);
	goto p1base;

    case 27:					/* listoff	*/
	listflag = 0;
	goto eocline;

    case 28:					/* liston	*/
	listflag = 1;
	goto eocline;

    case 29:					/* binoff */
	prsl(line);
	binflag = 1;
	goto eocline;

    case 30:					/* binon */
	prsl(line);
	binflag = 0;
	goto eocline;

    case 31:					/* cycle_time */
	printf("MPU cycles to this point are %d.\n",cytime);
	cytime = 0;
	goto eocline;

    case 32:					/* include */
	prsl(line);
	goto p1incl;

    case 33:					/* macoff	*/
	macflag = 1;
	goto eocline;

    case 34:					/* macon	*/
	macflag = 0;
	goto eocline;

    case 35:					/* global	*/
	prsl(line);
	eatspace(ss);
globlop: sscan(arg,ss);
	if(arg[0] == '\0')panic("Extra charactors in operhand field");
	if(llu(arg,globs) != -1){
		panic("Double defined global:");
		printf("%%%% %s\n",arg);
		}
	    else cement(arg,globs);
	eatspace(ss);
	if(**ss == ',')(*ss)++;
	eatspace(ss);
	if(neqs(**ss))goto globlop;
	goto eocline;

    case 36:					/* loadmod	*/
	prsl(line);
	goto eocline;

    case 37:					/* message	*/
	goto p1err;


	   }	/*  END SWITCH  */
	}

	if((j = llu(sarg,spmot)) != -1){	/* special 6800 */
		fb2 = 1;		/* all are two bytes long */
		switch(j){
		    case 1:		/* psha */
			b1 = 52;
			b2 = 2;
			goto dumper;
		    case 2:		/* pshb */
			b1 = 52;
			b2 = 4;
			goto dumper;
		    case 3:		/* pula */
			b1 = 53;
			b2 = 2;
			goto dumper;
		    case 4:		/* pulb */
			b1 = 53;
			b2 = 4;
			goto dumper;
		    case 5:		/* pshx */
			b1 = 52;
			b2 = 020;
			goto dumper;
		    case 6:		/* pulx */
			b1 = 53;
			b2 = 020;
			goto dumper;
		    case 7:		/* inx */
			b1 = 48;
			b2 = 1;
			goto dumper;
		    case 8:		/* dex */
			b1 = 48;
			b2 = 037;
			goto dumper;
		    case 9:		/* ins */
			b1 = 50;
			b2 = 0141;
			goto dumper;
		    case 10:		/* des */
			b1 = 50;
			b2 = 0177;
			goto dumper;
		    }
		}

	j = llu(sarg,mot);
	if(j == -1){
		panic("Undefined  Instruction:");
		printf("%%%%\t%s\n",sarg);
		prsl(line);
		goto eocline;
		}

	eatspace(ss);
	fb2 = fb3 = fb4 = indir = prebyte = 0;
	if(mvt[j][4] != 1){		/*	if inherant	*/
		b1 = mvt[j][4];
		b2 = 0;
		setpre(sarg);		/* set up any pre-byte */
		if((b1 >= 52) && (b1 <= 55)){	/* push or pull	*/
			if(eqs(**ss))panic(MI);
			if(**ss < 'A')panic("Bad push/pull register");
			while(neqs(**ss)){
				b2 =+ ppb(*(*ss)++);
				while((**ss != ',')&& neqs(**ss))(*ss)++;
				if(**ss == ',')(*ss)++;
				}
			fb2 = 1;
			}
		if((b1 == 30) || (b1 == 31)){	/* tfr or exg */
			if(eqs(**ss))panic(MI);
			if(**ss < 'A')panic("Bad exg/tfr register");
			if((**ss == 'd') && (*(*ss + 1) == 'p'))
				b2 = 0260;
			    else b2 = tfrb(**ss) << 4;	/* source reg */
			while((**ss != ',') && neqs(**ss))(*ss)++;
			if(eqs(**ss))panic(MI);
			(*ss)++;		/* skip over ',' */
			if((**ss == 'd')&& (*(*ss + 1) == 'p'))
				b2 =+ 013;
			    else b2 =+ tfrb(**ss);	/* dest. */
			fb2 = 1;
			if(((b2 & 0200)>> 4) ^ (b2 & 010))
			    panic("tfr/exg between different sized registers.");
			}
		goto dumper;
		}
	if(eqs(**ss))panic(MI);
	if(**ss == '#'){		/*	if imidiate	*/
		b1 = mvt[j][0];
		setpre(sarg);
		(*ss)++;
		b2 = evolexp(ss,stable,vtable);
		if(mlt[mvt[j][0] & 0377] == 3){
		    b3 = b2 & 0377;
		    b2 = (b2>>8) & 0377;
		    fb2 = 1;
		    fb3 = 1;
		    if(symtabr){
			relocate = 2;
			}
		    if(exttabr){
			exrtab[extrtp] = exttabr;
			if(prebyte)exatab[extrtp++] = pc + 2;
			    else exatab[extrtp++] = pc + 1;
			}
		    }
		else {
			fb2 = 1;
			b2 =& 0377;
			   }
		if(mvt[j][0] == 1){
		    panic("Imed. mode on non-imed. instruction");
		    }
		goto dumper;
		}
	ii = ss;
	i  = (*ss);
	while((**ss != ',')&&(**ss != ';'))(*ss)++;
	if(**ss == ','){		/* indexed or base mode	*/
	    (*ss)++;
	    if((**ss == 'x')||(**ss == 'y')||(**ss == 'u')||(**ss == 's')){
		b2 = ibr(**ss) << 5;	fb2 = 1;
		b1 = mvt[j][2];
		ss = ii;	(*ss) = i;
		if(mvt[j][2] == 1)
		    panic("Indirect mode on non-indirect instruction.\n");
		setpre(sarg);
		if(**ss == '['){
			(*ss)++;	b2 =+ 020;	/* indirect */
			indir++;
			};
		if(**ss == ','){	/* ,R or ,R++ or ,R- ... */
		    (*ss)++;		/* skip over ','	*/
		    (*ss)++;		/* skip over 'R'	*/
		    switch(*(*ss)++){
			case '+':
			    if(**ss == '+')
				b2 =+ 0201;	/* ,R++	*/
			      else b2 =+ 0200;
			    break;
			case '-':
			    if(**ss == '-')
				b2 =+ 0203;	/* ,R--	*/
			      else b2 =+ 0202;
			    break;
			default:		/* ,R	*/
			    b2 =+ 0204;
			}
		    goto dumper;
		    }
		if(((**ss == 'a')||(**ss == 'b')||(**ss == 'd'))&&
		    (*(*ss + 1) == ',')){	/* Acc. offset	*/
		    switch(**ss){
			case 'a':
			    b2 =+ 0206;
			    break;
			case 'b':
			    b2 =+ 0205;
			    break;
			case 'd':
			    b2 =+ 0213;
			    break;
			}
		    *ss =+ 3;		/* skip over ,R */
		    if((**ss == '+')||(**ss == '-'))
			panic("Can't mix increments with offsets");
		    goto dumper;
		    }
/*				must be n,R	*/
		b4 = evolexp(ss,stable,vtable);
		i = b4;	if(i < 0)i = -i;
		if(i < 16){
			if(indir)goto indr8;
			b2 =+ b4 & 037;
			fb2 = 1;
			goto dumper;
			}
		if(i < 128){
indr8:			b2 =+ 0210;	fb2 = fb3 = 1;
			b3 = b4 & 0377;
			goto dumper;
			}
		b3 = (b4 >> 8) & 0377;
		b4 = b4 & 0377;
		b2 =+ 0211;	fb2 = fb3 = fb4 = 1;
		goto dumper;
		}		/* ends " if((**ss == 'x'))||(**ss ... */
	    if(**ss == 'p'){		/*   n,pcr		*/
		b1 = mvt[j][2];
		if(mvt[j][2] == 1)
		    panic("Indexed addressing on non-indexed instruction");
		ss = ii;	(*ss) = i;
		setpre(sarg);
		b2 = 0215;
		fb2 = fb3 = fb4 = 1;
		b4 = evolexp(ss,stable,vtable);
		b3 = (b4 >> 8) & 0377;
		b4 =& 0377;
		if(indir)b2 =+ 020;
		goto dumper;
		}
	    if(**ss == 'd'){		/*	direct		*/
		ss = ii;
		(*ss) = i;
		b2 = evolexp(ss,stable,vtable);
		fb2 = 1;
		if(mvt[j][1] == 1){
		    panic("Direct mode on non-direct instruction");
		    }
		b1 = mvt[j][1];
		goto dumper;
		}
	    }
	ss = ii;
	(*ss) = i;
	if(mvt[j][3] != 1){		/*	extended	*/
	    setpre(sarg);
	    if(**ss != '['){
		b1 = mvt[j][3];
		j = evolexp(ss,stable,vtable);
		b2 = (j>>8)& 0377;
		b3 = j & 0377;
		fb2 = 1;
		fb3 = 1;
		if(symtabr){
			relocate = 2;
		    }
		if(exttabr){
		    exrtab[extrtp] = exttabr;
		    if(prebyte)exatab[extrtp++] = pc + 2;
			else exatab[extrtp++] = pc + 1;
		    }
		goto dumper;
		}
	    (*ss)++;			/* step over the '['	*/
	    b1 = mvt[j][2];
	    b2 = 0237;	fb2 = fb3 = fb4 = 1;
	    j = evolexp(ss,stable,vtable);
	    b3 = (j >> 8) & 0377;
	    b4 = j & 0377;
	    if(symtabr)relocate = 3;
	    if(exttabr){
		exrtab[extrtp] = exttabr;
		exatab[extrtp++] = pc + 2;
		}
	    goto dumper;
	    }
	if(mvt[j][5] != 1){		/*	relative	*/
	    b1 = mvt[j][5];
	    setpre(sarg);
	    if((llu(sarg,mot10) == -1)&&(b1 != 22)&&(b1 != 23)){
		j = evolexp(ss,stable,vtable);
		if(((j - pc) > 129)||((pc - j) > 125)){ /* range error */
		    panic("Relative address out of range");
		    j = 0;
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
	    if((llu(sarg,mot10) != -1)||(b1 == 22)||(b1 == 23)){
						/* long branch	*/
		j = evolexp(ss,stable,vtable);
		j =- pc + 4;			/* get offset */
		if((b1 == 22)||(b1 == 23))j++;	/* lbsr or lbra */
		b2 = (j >> 8) & 0377;
		b3 = j & 0377;
		fb2 = fb3 = 1;
		goto dumper;
		}
	    }
	panic("Internal error- type of instruction unknown");
	printf("J is %d, instruction is %s.\n",j,sarg);
for(i=0;i!=5;i++)printf("%d  ",mvt[j][i]);
putchar('\n');
	goto dumper;
bdumper:	panic(MI);

dumper:	prn();
	prl(line);
	pc++;	if(prebyte)pc++;
	if(fb2)pc++;	if(fb3)pc++;	if(fb4)pc++;
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
popout:		if(of){panic("Paren garbege");	return(i);}
		if(of == 0){(*p)--;return(i);}
		}
	if(c == '\0'){panic("UnExEOF??");(*p)--;return(i);}
	if((c == '[') || (c == ']')){
		indir++;		/* tag as using indirect adddressing */
		goto loop;
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
		symtabr++;
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
				if(*s != '_')symtabr++;
				ntableau[q]= eval(ntableau[q],j,d);
				j = ntableau[q];	}
			goto closee;
			}
		j = evolexp(p,table,ntableau);
		if((q = llu(s,table))== -1)
			panic("Lvalue required");
		  else {
			if(*s != '_')symtabr++;
			ntableau[q]= j;
			}
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
			if((q = llu(s,edtab))!= -1){
				if(binflag == 0)exttabr = q;
				j = 0;
				goto numerin;
				}
			panic("Undefined Symbol");
			printf("%%%%\t%s\n",s);
			goto loop;
			}
		if(*s != '_')symtabr++;
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
