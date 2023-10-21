#define	NUMEXT	500
/*
*
*	6800 macro assembler
*
*	Peter D Hallenbeck
*
*	(c) copyright Dec, 1977
*
*
*/

	extern fin,fout;
	int stable[500];		/* symbol table */
	char sstable[2500];
	int mbody[70];		/* macro body table*/
	char mmbody[2000];
	int mdef[70];		/* macro name definition table */
	char mmdef[700];
	int edtab[60];		/* external definition table */
	char eedtab[600];
	int exrtab[NUMEXT];	/* ext. ref. index to name of ext. */
	int exatab[NUMEXT];	/* ext. ref. tab. phys. address in program */

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
		-1
		};

	int mot[140];		/* machine opcode table */
	char mmot[560];
	int mvt[140][6];	/* machine value table	*/

	int pc;		/* program counter	*/
	int pass;	/* pass number		*/
	int lineno;	/* line number		*/
	int passend;	/* end of pass flag	*/
	int poe;	/* Point of Entry (or, Pasos on Everthing) */
	int rpoe;	/* not re-entrant flag (0 = re-entrant)	   */
	int stlen;	/* symbol table lenght	*/
	int mtlen;	/* macro table lenght	*/
	int vtable[500];/* symbol table value	*/
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
	int relob;	/* relocation bits	*/
	int symtabr;	/* symbol table referance counter	*/
	int relocate;	/* relocatoion flag	*/
	int exttabr;	/* ext. ref. table flag	*/
	int extrtp;	/* ext. ref. table next entry pointer */
	int curbase;	/*  current radix for numbers	*/
	int macnest;	/* nested macro counter		*/
	int macstack[50];/*nested macro variable list stack pointer*/
	int listflag;	/* flag for list suppression		*/
	int binflag;	/* flag for supprision of binary out	*/
	int cytime;	/* cycle time counter	*/


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

	pass = 1;
	lineno = 0;
	passend = 1;
	poe = 0;
	extrtp = 0;
	vtable[0] = 1;
	rpoe = 0;
	stlen = 1;
	mtlen = 1;
	macnest = 0;
	curbase = 10;
	listflag = 1;
	binflag = 0;
	cytime = 0;

	mixerup(stable,sstable);
	mixerup(mbody,mmbody);
	mixerup(mdef,mmdef);
	mixerup(mot,mmot);
	mixerup(edtab,eedtab);
	pcocnt = 0;
	pcolpc = 0;

	fd = open("/usr2/wa1yyn/6800/mas68.optab",0);
	  if(fd== -1){perror("/usr2/wa1yyn/6800/mas68.optab");return;}
	for(i =0;i != 113;i++)for(ii = 0;ii != 6;ii++)mvt[i][ii]=0;
	bc = 1;
	b = 1 ;
	while(bc==1){
	  s = &line;
	  ss = &s ;
	  k = line ;
		while((b != '\n')&& (bc == 1) ){
			bc = read(fd,&b,1);
			*k++ = b;
			}
		b = 1;
		if(bc==1){
			sscan(arg,ss);
			cement(arg,mot);
			j = llu(arg,mot);
			for(ii= 0;ii != 6;ii++){
				sscan(arg,ss);
				mvt[j][ii]= basin(arg,16);
				}
			}
		}
	close(fd);

	pc = 0;
	numerrs = 0;
	nestc = 0;
	fin = open(argv[1],0);
	if(fin == -1){
	    printf("Can't find '%s'.\n",argv[1]);
	     return;
	     }

foobar:	while(passend){		/* in memory of AI Lab	*/
	getline(line);
	cline(line);		/* get and process line	*/
	}
	if(pass == 2){
		if(pcocnt)pdump();
		j = 'S';	i = write(fd,&j,1);
		j = '6';	i = write(fd,&j,1);
		pdumb(poe >> 8);	pdumb(poe & 0377);
		j = '\n';	i = write(fd,&j,1);
		i = 0;
		while(i < extrtp){
		    j = 'S';	ii = write(fd,&j,1);
		    j = '7';	ii = write(fd,&j,1);
		    pdumb((exatab[i])>> 8);
		    pdumb(exatab[i] & 0377);
		    k = edtab[exrtab[i]];
		    while(*k != '\0'){
			j = *k++;	ii = write(fd,&j,1);
			}
		    j = '\n';	ii = write(fd,&j,1);
		    i++;
		    }
		j = 'S';	i = write(fd,&j,1);
		j = '8';	i = write(fd,&j,1);
		pdumb(pc >> 8);	pdumb(pc & 0377);
		j = '\n';	i = write(fd,&j,1);
		j = 'S';	i = write(fd,&j,1);
		j = '9';	i = write(fd,&j,1);
		j = '\n';
		i = write(fd,&j,1);
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

	printf("PASS TWO\n");
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
	j = 'L';
	i = write(fd,&j,1);
	j = '\n';
	i =  write(fd,&j,1);
	goto foobar;
}

getline(line)
	char *line;
{
	register char *k;
	register char c;
	register i;
	int p,j;

	if(nestc){
		c = *nest[nestc]++;
		}
	   else {
		c = getchar();
		lineno=+ 1;
		}
	k = line;
	i = 0;
	while((c != '\n')&&(c != '\0')&&(i < 119)){
		*k++ = c;
		i++;
		if(nestc)c = *nest[nestc]++;
		   else c = getchar();
		}

	if(i >= 119){
		panic("Warning- Input line too long.  Line truncated.");			--k;	*k = '\0';
		while((c != '\n')&&(c != '\0')){
			if(nestc)c = *nest[nestc]++;
			    else c = getchar();
			}
		}
	if(c == '\0'){
		panic("No end statement");
		passend = 0;
		line[0] = '\0';
		}
	if(c == '\n'){
		*k++ = ';';
		*k = '\0';
		}
}


mexp(body,arglist,flag)
	char *body,**arglist;
	int flag;
{
	char line[80];
	char bline[500];
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
	p = stable;
	p++;
	i = 1;
	while(*p != -1){
		jnum(i,10,3,0);
		printf("- %s  \t",*p++);
		jnum(vtable[i++],16,4,0);
		putchar('\n');
	}
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
    if(listflag){
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
	if(listflag)printf("                 ");
}
prl(line)
	char *line;
{
	register char *k;
	register char delay;
	if(listflag == 0)return;
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
	    relob = relob << 1;
	    }
	if(pcocnt > 13){
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
	j = 'S';
	i = write(fd,&j,1);
	j = '2';
	i = write(fd,&j,1);
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
	j = '\n';
	i = write(fd,&j,1);
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
	j = write(fd,&i,1);
	i = n & 017;
	i =+ 060;
	if(i > '9')i =+ ('A'-':');
	j = write(fd,&i,1);
	pcoch =+ n;
}
cline(line)
	char *line;
{
	char arg[21];
	char sarg[21];
	char lline[500];
	char llline[100];
	int s;
	char **ss;
	int hlable;
	int hclable;
	int ii,jj;
	int marg[10];
	char mmarg[100];
	int maclab[20];		/* macro lable save table	*/
	int mmaclab[150];
	register char c;
	register i,j;

	hlable = 0;
	hclable = 0;
	mixerup(marg,mmarg);
	mixerup(maclab,mmaclab);
	s = line;
	ss = &s;
	arg[0] = '\0';

	if( pass == 1 )  {


   if(islet(**ss)== 1){
	if((i = (sscan(arg,ss)))== ':'){
		(*ss)++;			/* ignor : */
		hclable = 1;
		}
	if(i == -1){
		panic("Missing Opcode");
		return;
		}
	if((i = llu(arg,stable))!= -1){
		panic("Double defined symbol:");
		printf("%%%%\t%s\n",arg);
		}
	if(i == -1){
	cement(arg,stable);	/* put into stable */
	vtable[llu(arg,stable)] = pc;
	hlable = 1;
	vtable[0] =+ 1;
	if(macnest)cement(arg,macstack[macnest]);
	}
      }
   else {
	if((**ss == ';')||(**ss == '*'))return;
	if((**ss != ' ')&&(**ss != '\t')){
		if(**ss == '\n')return;
		panic("Invalid lable");
		while((**ss != ' ')&&(**ss != '\t')&&(**ss != '\n'))
			(*ss)++;
		if(**ss == '\n')return;
		}
	}
   i = sscan(sarg,ss);		/* get opcode/psydo-op	*/
   if(sarg[0] == '\0'){
	if(hlable && (hclable == 0)){
		panic("Warning- lable with no opcode");
		printf("%%%%\t%s\n",arg);
		}
	return;
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
	macstack[++macnest]= maclab;
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
	return;
	}
  else
   if((j = llu(sarg,soptab))!= -1){		/* pysdo op	*/

     switch(j){

      case 1:					/* equ		*/
  p1equ:  if(arg[0]== '\0'){panic("EQU with no symbol");goto p1equa;}
	if(i == -1){
		panic("Missing operhand");
		chisel(arg,stable);
		vtable[0] =- 1;
		goto p1equa;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
		chisel(arg,stable);
		vtable[0] =- 1;
		goto p1equa;
		}
	vtable[llu(arg,stable)] = evolexp(ss,stable,vtable);
   p1equa:	if(pass == 2)prsl(line);
	return;


     case 2:					/* end */
	passend = 0;
	return;


     case 3:					/* db */
  p1db: if(i == -1){
		panic("Missing operhand");
		pc =+ 1;
		return;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
		pc =+ 1;
		return;
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
	return;


   case 4:					/* ds */
 p1ds: if(i == -1){panic("Missing operhand");return;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
		return;
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
		   return;
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
			return;
			}
		}
	}
	return;


   case 5:					/* dw	*/
   p1dw: if(i == -1){panic("Missing Operhand");
		pc =+ 2;
		return;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
		pc =+ 2;
		return;
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
	return;


   case 6:					/* org  */
   p1org:   if(i == -1){panic("Missing operhand");goto p1orga;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
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
   p1orga:	if(pass == 2)prsl(line);
	return;


   case 7:					/* eval	*/
	if(i == -1){panic("missing operhand");
		return;
		}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
		return;
		}
	evolexp(ss,stable,vtable);
	return;


   case 8:					/* entry	*/
	if(i == -1){panic("Missing Operhand");
		return;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
		return;
		}
	return;


   case 9:					/* not\_re_entrant */
	rpoe = -1;
	return;


   case 10:					/* repeat */
  p1rep:  if(i == -1){panic("Missing operator");goto p1repa;}
	eatspace(ss);
	if((**ss == '\n')||(**ss == ';')){
		panic("Missing operhand");
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
	return;
  p1repa:  if(pass == 2)prsl(line);
	return;


   case 11:					/* fixtab */
  p1fixt:  i = 1;
	while(stable[i++] != -1);
	stlen = --i;
	return;


   case 12:					/* expunge	*/
  p1exps:  if(i != -1){
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';'))i = -1;
		}
	if(i == -1){
		stable[stlen] = -1;
		vtable[0] = stlen + 1;
		return;
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
	if(i == -1)return;
	eatspace(ss);
	if(**ss == ','){
		j =+ 1;
		(*ss)++;
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			return;
			}
		  else goto lop1;		/* cringe	*/
		}
	if((**ss == '\n')||(**ss == ';'))return;
	panic("Extranious symbols in operhand field");
	return;


   case 13:					/* dup	*/

   p1dup:  if(i == -1){panic("Missing operhand");return;}
	eatspace(ss);
	if(eqs(**ss)){panic("Missing operhand");return;}
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
          while(((lline[j] = llline[i++])!= '\0')&&(j < 500)){
		j++;
		}
	   lline[j++] = '\n';
	   if(j >= 500){
		panic("Too many charactors in 'dup' argument");
		lline[499] = '\n';
		lline[500] = '\0';
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
	return;


   case 14:					/* external */
	if(i != -1){
		eatspace(ss);
		if((**ss == '\n')|(**ss == ';'))i = -1;
		}
	if(i == -1){
		panic("Missing operhand");
		return;
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
	if(i == -1)return;
	eatspace(ss);
	if(**ss == ','){
		j =+ 1;
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			return;
			}
		else goto lop2;
		}
	if((**ss == '\n')||(**ss == ';'))return;
	panic("Extraneous symbols in operhand field");
	return;


  case 15:					/* defmacro */

   p1defm:	hlable = 0;
	b1 = 1;
	if(i == -1){panic("Missing operhand");return;}
	eatspace(ss);
	if(eqs(**ss)){panic("Missing operhand");return;}
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
         while(((lline[j] = llline[i++])!= '\0')&&(j < 500)){
		j++;
		}
	   lline[j++] = '\n';
	   if(j >= 500){
		panic("Too many charactors in 'defmacro' argument");
		lline[499] = '\n';
		lline[500] = '\0';
		j--;
		goto lop5;
		}
	   }
   lop5: lline[j] = '\0';
	if(((pass == 1)||(hlable))&& b1)cement(lline,mbody);
	return;


  case 16:					/* fixmacro	*/
   p1fixm:  i =1;
	while(stable[i++] != -1);
	mtlen = --i;
	return;


  case 17:					/* expunge macro */
   p1expm:   if(i != -1){
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';'))i = -1;
		}
	if(i == -1){
		mdef[mtlen] = -1;
		return;
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
	if(i == -1)return;
	eatspace(ss);
	if(**ss == ','){
		j =+ 1;
		(*ss)++;
		eatspace(ss);
		if((**ss == '\n')||(**ss == ';')){
			pcommer(j);
			return;
			}
		  else goto lop3;
		}
	if((**ss == '\n')||(**ss == ';'))return;
	panic("Extranious charactor in operhand field");
	return;


   case 18:					/* endmacro */
   p1endm:   panic("Extra 'endmacro'");
	return;


   case 19:					/* endallmacro */
    p1enda:	panic("Exta 'endallmacro'");
	return;


   case 20:					/* enddup */
   p1endd:   panic("Extra 'enddup'");
	return;


   case 21:					/* endalldup */
   p1ena:  panic("Extra 'endalldup'");
	return;


   case 22:					/* fcb	*/
   p1fcb:	goto p1db;
	return;

    case 23:					/* fdb	*/
   p1fdb:	goto p1dw;
	return;

    case 24:					/* fcc */
    p1fcc:	goto p1ds;
	return;


    case 25:				/* rmb	*/
	goto p1ds;


   case 26:					/* if	*/
	p1if:  j =  evolexp(ss,stable,vtable);
	ifcount++;
	if(j)return;
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
				return;
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
	return;


   case 27:					/* else */
    p1else:   if(ifcount == 0){
		panic("Extra 'else' statment");
		return;
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
				return;
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
	return;


   case 28:					/* endif	*/
	p1endif:	if(ifcount == 0){
		panic("extra 'endif'");
		return;
		}
	ifcount--;
	return;

    case 29:					/* base		*/
p1base:	eatspace(ss);
	if(eqs(**ss)){
		panic("Missing Operhand");
		return;
		}
	curbase = 10;
	curbase = evolexp(ss,stable,vtable);
	return;

    case 30:					/* listoff */
	listflag = 0;
	return;

    case 31:					/* liston	*/
	listflag = 1;
	return;

    case 32:					/* binoff */
	binflag = 1;
	return;

    case 33:					/* binon */
	binflag = 0;
	return;

    case 34:					/* cycle_time */
	return;

       }		/*  END OF SWITCH  */
     }

  else if((j = llu(sarg,mot))!= -1){
	eatspace(ss);
	if(mvt[j][4] != 0){
		pc =+ 1;
		return;		/* inherant */
		}
	if((**ss)== '#'){
		pc =+ bytcnt(mvt[j][0]);/* immediate	*/
		if(mvt[j][0] == 0)panic("Invalid mode");
		return;
		}
	while((**ss != ';')&&(**ss != ','))(*ss)++;
	if(**ss == ','){
		(*ss)++;
		if(**ss == 'x'){
			pc =+ 2;	/* indexed */
			if(mvt[j][3] == 0)panic("Invalid mode");
			return;
			}
		if(**ss == 'd'){
			pc =+ 2;	/* base-mode address */
			if(mvt[j][1] == 0)panic("Invalid mode");
			return;
				}
		panic("Odd addressing mode");
		}
	if(mvt[j][3] != 0){
		pc =+ 3;		/* extended	*/
		return;
		}
	if(mvt[j][5] != 0){
		pc =+ 2;		/* relative	*/
		return;
		}
	panic("Valid instruction with invalid mode");
	}
     panic("Undefined instruction:");
	printf("%%%%\t%s\n",sarg);
       return;


   }

		/* IN PASS TWO   */


	symtabr = 0;	exttabr = 0;	relocate = 0;
	fb2 = fb3 = 0;
	b1 = 1;
	if(extrtp >= NUMEXT){
		panic("Out of external symbol referance table space");
		panic("Internally, change 'NUMEXT' to increase table");
		extrtp =- 1;
		}

	if((**ss == ';')&&(*(*ss+1) == '\0')){putchar('\n');return;}
	if(islet(**ss) == 1){
	   if((i = sscan(arg,ss))== ':'){
		(*ss)++;
		hclable = 1;		/* ignore :*/
		}
	   if(i == -1){
		panic("Missing opcode");
		prsl(line);
		return;
		}
	   if(llu(arg,stable)== -1){
		cement(arg,stable);
		vtable[llu(arg,stable)] = pc;
		hlable = 1;
		vtable[0]=+ 1;
		if(macnest)cement(arg,macstack[macnest]);
		}
	    }
	else {
	    if((**ss == ';')||(**ss == '*')){prsl(line);return;}
	    if((**ss != ' ')&&(**ss != '\t')){
		if(**ss == '\n'){putchar('\n');return;}
		panic("Invalid lable");
		while((**ss != ' ')&&(**ss != '\t')&&(**ss != '\n'))
			(*ss)++;
		if(**ss == '\n'){prsl(line);return;}
		}
	     }
	i = sscan(sarg,ss);
	if(sarg[0] == '\0'){
	    if(hlable && (hclable == 0)){
		panic("Warning -lable with no opcode");
		printf("%%%%\t%s\n",arg);
		}
	    prsl(line);
	    return;
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
	dumpst();
	return;

    case 3:					/*  db	*/
  p2db:	b1 = evolexp(ss,stable,vtable);
	if(eqs(**ss)){
		prn();	prl(line);   pc =+ 1;
		return;
		}
	    else {
		prsl(line);	prn();
		}
	i = 1;
	pc =+ 1;
	while(**ss == ','){
		(*ss)++;
		b1 = evolexp(ss,stable,vtable);
		if(listflag){
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
	if(listflag)putchar('\n');
	return;

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
			if(**ss == '0')i = '\0';
			b1 = i;
			if(ii < 16){
				pbytes();
				if(listflag){
					j = 0;
					putchar(' ');
					jnum(b1,16,2,0);
					}
				ii++;
				}
			   else {
				if(jj && listflag)putchar('\n');
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
				if(listflag){
					j = 0;
					putchar(' ');
					jnum(b1,16,2,0);
					}
				pbytes();
				ii++;
				}
			   else {
				if(jj && listflag)putchar('\n');
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
	return;

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
		return;
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
	return;

    case 6:					/*   org	*/
	goto p1org;

    case 7:					/*  eval	*/
	evolexp(ss,stable,vtable);
	prsl(line);
	return;

    case 8:					/* entry */
	prsl(line);
	eatspace(ss);
	if(eqs(**ss)){
		panic("Missing operhand");
		return;
		}
	poe = evolexp(ss,stable,vtable);
	return;

    case 9:				/*  not_re_entrant	*/
	prsl(line);
	return;

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
	return;			/* all handled in pass one */

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

    case 22:					/* fcb */
	goto p2db;

    case 23:					/* fdb */
	goto p2dw;

    case 24:					/* fcc */
	goto p2ds;

    case 25:					/* rmb	*/
	goto p2ds;

    case 26:					/* if	*/
	prsl(line);
	goto p1if;

    case 27:					/* else	*/
	prsl(line);
	goto p1else;

    case 28:					/* endif	*/
	prsl(line);
	goto p1endif;

    case 29:					/* base		*/
	prsl(line);
	goto p1base;

    case 30:					/* listoff	*/
	listflag = 0;
	return;

    case 31:					/* liston	*/
	listflag = 1;
	return;

    case 32:					/* binoff */
	prsl(line);
	binflag = 1;
	return;

    case 33:					/* binon */
	prsl(line);
	binflag = 0;
	return;

    case 34:					/* cycle_time */
	printf("MPU cycles to this point are %d.\n",cytime);
	cytime = 0;
	return;

	   }	/*  END SWITCH  */
	}


	j = llu(sarg,mot);
	if(j == -1){
		panic("Undefined  Instruction:");
		printf("%%%%\t%s\n",sarg);
		prsl(line);
		return;
		}

	eatspace(ss);
	fb2 = fb3 = 0;
	if(mvt[j][4] != 0){		/*	if inherant	*/
/*	here is were the funny fix was	*/
		b1 = mvt[j][4];
		goto dumper;
		}
	if(**ss == '#'){		/*	if imediate	*/
		b1 = mvt[j][0];
		(*ss)++;
		b2 = evolexp(ss,stable,vtable);
		if(bytcnt(mvt[j][0]) == 3){
		    b3 = b2 & 0377;
		    b2 = (b2>>8) & 0377;
		    fb2 = 1;
		    fb3 = 1;
		    if(symtabr)relocate = 2;
		    if(exttabr){
			exrtab[extrtp] = exttabr;
			exatab[extrtp++] = pc + 1;
			}
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
	while((**ss != ',')&&(**ss != ';'))(*ss)++;
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
		b1 = mvt[j][3];
		j = evolexp(ss,stable,vtable);
		b2 = (j>>8)& 0377;
		b3 = j & 0377;
		fb2 = 1;
		fb3 = 1;
		if(symtabr)relocate = 2;
		if(exttabr){
		    exrtab[extrtp] = exttabr;
		    exatab[extrtp++] = pc + 1;
		    }
		goto dumper;
		}
	if(mvt[j][5] != 0){		/*	relative	*/
		b1 = mvt[j][5];
		j = evolexp(ss,stable,vtable);
		if((j> (pc+ 129))||(j < (pc -125))){ /*rage error*/
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
	panic("Internal error- type of instruction unknown");
	printf("J is %d, instruction is %s.\n",j,sarg);
for(i=0;i!=5;i++)printf("%d  ",mvt[j][i]);
putchar('\n');
	goto dumper;
bdumper:	panic("Missing operhand");

dumper:	prn();
	prl(line);
	pc =+ bytcnt(b1);
	cytime =+ bytime(b1);
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
				case 'b':	c = 010;
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
