/*
 * Extract cost data from specified paragraphs of cost data files.
 *
 * usage:
 *      pgrep [-#:[#]] [pfile] ...
 *
 *              -#      means multiply the recurring cost data obtained using the following
 *                      pfile by # (default is 1.0)
 *              -#:#    means multiply the recurring cost data by the first number and the
 *                      non-recurring cost data by the second number
 *
 *              if no pfile is specified then STDIN is used.
 *
 * The format of a pfile is:
 *      comment lines which do not contain any ':' characters, and
 *      paragraph selection lines of the form:
 *              filename: paragraph ...
 *
 *      where   filename is the name of a costdata breakdown file and
 *              paragraph is the number of a paragraph whose data is to be
 *              included in the output.  Arbitrary comments which follow the
 *              paragraph number are ignored.
 *
 * The output of pgrep is a sequence of costdata lines extracted from the
 * specified paragraphs of the specified files and extended by the specified
 * multiplier.
 *
 * A costdata breakdown file contains text lines, which are ignored, paragraph
 * lines, which begin with a digit, and data lines which are of the form:
 *         <abbr> : <qnt> ...
 * or,
 *         <abbr> :: <qnt> ...
 *    where <qnt> is any legal arithmetic expression involving ( ) * / + - and
 *      numbers, and numbers may have $ prepended or % appended.
 * The first form is used for non-recurring costs and the second is used for
 * recurring costs.
 * Text lines must not start with a digit, nor contain any ':' characters.
 *
 * A typical modus operandi is:
 *      1. ep the costdata breakdown file(s), e.g., ep cbfnnnn
 *      2. make the pfile(s) by
 *         a. grep "^ *[0123456789]" cbfnnnn... >pfxxxx
 *         b. ep the pfile to discard undesired paragraphs, e.g., ep pfxxxx
 *      3. optionally check that the resulting pfiles do not overlap, e.g.,
 *         comm -12 pfaaaa pfbbbb
 *      4. optionally check that the pfiles are complete, e.g.,
 *         grep "^ *[0123456789]" cbf* | sort >tmpcbf
 *         sort pf* >tmppf
 *         diff tmpcbf tmppf
 *      5. ep the nroff format headers for the cost summary sheets, e.g.,
 *         ep sumkkkk.n
 *      6. generate the cost summary sheets, e.g.,
 *         (cat sumkkkk.n; pgrep [-#] pfkkkk | spr ) | nroff | opr
 *
 */


char    lbuf[128];
struct  iobuf {
	int     bfd, bnleft;
	char    *bnxtp, bbuf[512];
	} ibuf;
extern  fin[];
double  mult, rmult, atof();


main(argc,argv) char **argv;{
	register char *s;
	register int i;

	argc--; argv++;
	if((s=alloc(4000))!=0177777) free(s);
	if(argc>0 && argv[0][0]=='-'){
		s= &argv[0][1];
		rmult= atof(s);
		if((i=index(':',s))>=0)
			mult= atof(&s[i+1]);
		argc--; argv++; }
	if(argc<=0){
		if(mult<=0.0) mult= 1.0;
		if(rmult<=0.0) rmult= 1.0;
		pgrep(fin); }
	else for( ; argc>0; argc--, argv++)
		if(argv[0][0]=='-'){
			s= &argv[0][1];
			rmult= atof(s);
			if((i=index(':',s))>=0)
				mult= atof(&s[i+1]);
			}
		else {  if(mult<=0.0) mult= 1.0;
			if(rmult<=0.0) rmult= 1.0;
			if(fopen(*argv, &ibuf)<0)
				writef(2,"can't open %s\n",*argv);
			else {  pgrep(&ibuf);
				close(ibuf.bfd); }
			mult= 1.0;
	}               }

pgrep(ib) char *ib;{
	register int i;
	register char *s;
	char *fn, *pg;

	for( ; (i=getl(ib,lbuf))>=0; ){
		if(i==0 || (i=index(':',lbuf))<0) continue;
		s= &lbuf[i];
		*s++= 0;
		fn= trim(lbuf);
		while(*s==' ' || *s=='\t') s++;
		pg= s;
		while(*s && *s!=' ' && *s!='\t') s++;
		*s= 0;
		pass(fn,pg);
	}       }

char    plbuf[128], fnb[128], pgb[128];
struct  iobuf pibuf;

pass(fn,pg) char *fn, *pg;{
	register int i;
	register char *s;
	int eof;
	double eval();

	if(!eqstr(fnb,fn)){
		if(fnb[0]) close(pibuf.bfd);
		fnb[0]= 0;
		if(fopen(fn,&pibuf)<0){
			writef(2,"can't open %s\n",fn);
			return; }
		copystr(fn,fnb);
		pgb[0]= 0; }
	for(eof=0;;)
		if(eqstr(pg,pgb)){
			for( ; (i=getl(&pibuf,plbuf))>=0; ){
				if(i==0) continue;
				for(s=plbuf; *s==' ' || *s=='\t'; s++) ;
				if(*s>='0' && *s<='9'){
					copystr(s,pgb);
					for(s=pgb; *s && *s!=' ' && *s!='\t'; s++) ;
					*s= 0;
					return; }
				if((i=index(':',plbuf))<0) continue;
				s= &plbuf[i+1];
				if(*s==':'){
					s++;
					if(rmult!=1.0) writefb(s,"%f",eval(s)*rmult);
					}
				else if(mult!=1.0) writefb(s,"%f",eval(s)*mult);
				writef(1,"%s\n",plbuf);
				}
			close(pibuf.bfd);
			fnb[0]= 0; pgb[0]= 0;
			return; }
		else {  for( ; (i=getl(&pibuf,plbuf))>=0; ){
				if(i==0) continue;
				for(s=plbuf; *s==' ' || *s=='\t'; s++) ;
				if(*s>='0' && *s<='9'){
					copystr(s,pgb);
					for(s=pgb; *s && *s!=' ' && *s!='\t'; s++) ;
					*s= 0;
					break;
				}       }
			if(i>0) continue;
			close(pibuf.bfd);
			fnb[0]= 0; pgb[0]= 0;
			if(eof){
				writef(2,"can't find %s in %s\n",pg,fn);
				return; }
			eof++;
			fopen(fn,&pibuf);
	}               }


static char *str;
static int token;
static double value;

double eval(s) char *s;{
	register int i;
	double x, expr();

	str= s;
	if((i=index(';',s))>=0) s[i]= 0;
	if((x=expr())<0.0){
		if(i>=0) s[i]= ';';
		writef(2,"using 0 for expression: %s\n",s);
		x= 0.0; }
	return x; }

double expr(){
	double t, e;
	int aop, mop, state;

	for(e=0.0, aop=0, state=0; (token=nxtoken())>=0; )
		switch(action(&state)){
			case 0: return aop? e-t : e+t;
			case 1: if(aop) e=- t;
				else e=+ t;
				aop= value; break;
			case 2: mop= value; break;
			case 3: value= expr();
				if(token<0) return -1.0;
			case 4: if(!mop) t=* value;
				else if(value==0.0){
					writef(2,"dividing by 0\n");
					token= -1;
					return -1.0; }
				else t=/ value;
				break;
			case 5: value= expr();
				if(token<0) return -1.0;
			case 6: t= value;
				break;
			default: goto out;
			}
out:    writef(2,"syntax error\n");
	return -1.0; }

static char act[3][6]= {
	 6, -1, -1,  5, -1, -1,
	 4, -1, -1,  3, -1, -1,
	-1,  2,  1, -1,  0,  0 };

static char nxts[3][6]= {
	 2, -1, -1,  2, -1, -1,
	 2, -1, -1,  2, -1, -1,
	-1,  1,  0, -1, -1, -1 };

action(ip) register int *ip;{
	register int s;

	s= *ip;
	*ip= nxts[s][token];
	return act[s][token]; }

nxtoken(){
	register int c;

loop:   switch(c= *str++){
		case ' ': case '\t': case '$':
			goto loop;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case '.':
			if((c=scanl(--str,"%f",&value))<0) return -1;
			str=+ c;
			while((c= *str++)==' ' || c=='\t') ;
			if(c=='%')
				value=/ 100.;
			else str--;
			return 0;
		case '*': value= 0.0; return 1;
		case '/': value= 1.0; return 1;
		case '+': value= 0.0; return 2;
		case '-': value= 1.0; return 2;
		case '(': return 3;
		case ')': return 4;
		case 0:   return 5;
		default:  return 5;
	}       }
