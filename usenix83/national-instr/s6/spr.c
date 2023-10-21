/*
 * Print a cost summary.
 *
 * usage:
 *      spr [-e#] [-f[#]] [-g[#]] [-i<abbr>] [-o] [-p#.#.#] [-r ratefile] [-t] [-x#:#]  [costdata]
 *
 *              -e#     means calculate escalation factors based on # months
 *                      (default is 0, i.e., no escalation)
 *              -f      means suppress fee accounting
 *              -f1     means apply fee to each labor and cost category
 *              -f2     means apply fee to labor and direct cost separately
 *              -f3     means apply fee only to total
 *                      default is to apply fee wherever G&A is applied (or to total if G&A suppressed)
 *              -g      means suppress G&A accounting
 *              -g1     means apply G&A to each labor and cost category
 *              -g2     means apply G&A to labor and direct cost separately
 *              -g3     means apply G&A only to total (default)
 *                      if f# specified and g# specified then both must agree
 *              -i<abbr> means suppress accounting of cost item or cost category
 *                      given by <abbr>; if <abbr> is : then all non-recurring
 *                      costs are suppressed; if <abbr> is :: then all recurring
 *                      costs are suppressed
 *              -o      means suppress overhead accounting
 *              -p#.#.# means set the precision (number of digits after the decimal)
 *                      for the hours, rate, and cost, to the respective numbers
 *                      (default is 1.2.2 which shows tenths of hours, cents of rate,
 *                      and cents of cost)
 *              -r      means use the next argument as the rate file
 *                      (default is /usr/lib/rates)
 *              -t      round off and sum rounded cost figures
 *                      (default is to keep trailing significance in rate and cost figures)
 *              -x#:#   means multiply all the recurring hour/quantity values by the first #
 *                      and all the nor-recurring values by the second #
 *
 *              if no costdata file is specified then STDIN is used.
 *
 * The format of a rate file is:
 *      comment lines which do not contain any ':' characters, and
 *      rate lines which consist of:
 *              mnemonic:  rate:   category:  full_title
 *
 *      where   rate is a floating point number
 *              category is:
 *                      L1, L2, ...     for labor categories (rate is $/hr)
 *                      D1, D2, ...     for other direct cost categories (rate is $/whatever)
 *                      LE1, LE2, ...   for labor escalation factors (rate is %/yr)
 *                      DE1, DE2, ...   for other cost escalation factors (rate is %/yr)
 *                      LO1, LO2, ...   for labor overhead factors (rate is %)
 *                      G               for G&A factor (rate is %)
 *                      F               for fee factor (rate is %)
 *
 * A costdata file contains text lines, which are ignored, and data lines which
 * are summed up.  Data lines are of the form:
 *      <abbr> : <qnt> ...
 * or,
 *      <abbr> :: <qnt> ...
 * and text lines must not contain ':'.  The first form is used for non-recurring
 * costs, and the second form is used for recurring costs.
 * <qnt> is any legal arithmetic expression involving ( ) * / + - and numbers,
 * where numbers may have $ prepended or % appended.
 */

#define MAXITEM 1000

struct item {
	char    *tag, *title;
	double  rate, qnt;
	int     cat;
	} item[MAXITEM+1];
int     nitem;

char    fflag= -1, gflag=3, oflag, tflag;
double  months, roundc, roundq, roundr, mult, rmult;
char    *ratefile= "/usr/lib/rates";

enum {  LABOR, DIRECT, LESC, DESC, LOVHD, DOVHD, GA, FEE };

struct categ {
	char    *abbr;
	int     major;
	} categ[]= {
	"L",    LABOR,
	"D",    DIRECT,
	"LE",   LESC,
	"DE",   DESC,
	"LO",   LOVHD,
	"DO",   DOVHD,
	"G",    GA,
	"F",    FEE,
	0};

char    *nroff, *lbhd, *dchd, *dcshd, *fmtqrc, *fmtqc, *fmtc, *addqc, *addc, *addtc;
char    *ind1= "  ", *ind2= "    ", *dashes= "--------------------";
int     prcrate=2, prchour=1, prccost=2;

struct { char lobyte, hibyte; };        /* int access */
char    lbuf[128];
struct  iobuf {
	int     bfd, bnleft;
	char    *bnxtp, bbuf[512];
	} ibuf;
extern  fin[];
double  atof(), pow(), floor(), rnd(), rndq(), rndr(), rnde(), ceil(), log10();
char    **ignore;

char    *label[]{
		"Labor subtotal",
		"Other direct cost subtotal",
		"Escalated labor subtotal",
		"Escalated direct cost subtotal",
		"Labor and overhead subtotal",
		"Direct cost and overhead subtotal",
		"Labor and G&A subtotal",
		"Direct cost and G&A subtotal",
		"Subtotal, labor",
		"Subtotal, other direct costs",
		"Total labor",
		"Total other direct costs",
		"Total labor and direct costs",
		"Total labor, direct costs, and G&A",
		"Total",
		0};


main(argc,argv) char **argv;{
	register int i;
	register char *s, **ig;

	ignore= ig= argv;
	for(argc--, argv++; argc && argv[0][0]=='-'; argc--, argv++)
		switch(argv[0][1]){
			case 'e': months= atof(&argv[0][2]); break;
			case 'f': i= atoi(&argv[0][2]);
				if(i<0 || i>3) goto bad;
				fflag= i;
				break;
			case 'g': i= atoi(&argv[0][2]);
				if(i<0 || i>3) goto bad;
				gflag= i;
				break;
			case 'i': if(argv[0][2])
					*ig++= &argv[0][2];
				break;
			case 'o': oflag++; break;
			case 'p': if(scanl(&argv[0][2],"%u.%u.%u$",&prchour,&prcrate,&prccost)<0)
				       goto bad;
				break;
			case 'r': ratefile= *++argv; argc--; break;
			case 't': tflag++; break;
			case 'x': rmult= atof(s= &argv[0][2]);
				if((i=index(':',s))>=0) mult= atof(&s[i+1]);
				break;
			default:
			bad:    writef(2,"bad switch: %s\n",*argv);
				break;
			}
	if(argc>1){
		writef(2,"too many args, %s?\n",argv[1]);
		exit(); }
	*ig= 0;
	if(fflag<0) fflag= gflag? gflag:3;
	else if(fflag && gflag && fflag<gflag){
		writef(2,"inconsistent fee and G&A flags\n");
		fflag= gflag; }
	if(tflag){
		roundq= pow(10.,(double)prchour);
		roundr= pow(10.,(double)prcrate);
		roundc= pow(10.,(double)prccost); }
	if(mult<=0.0) mult= 1.0;
	if(rmult<=0.0) rmult= 1.0;
	if(fopen(ratefile, &ibuf)<0){
		writef(2,"can't open ratefile: %s\n",ratefile);
		exit(); }
	readrates();
	close(ibuf.bfd);
	if(argc<=0) spr(fin);
	else if(fopen(*argv, &ibuf)>=0) spr(&ibuf);
	else writef(2,"can't open %s\n", *argv);
	}

readrates(){
	register int i;
	register char *s;
	register struct item *it;
	char *tg, *tt;
	int icmp();

	if((s=alloc(4000))!=0177777) free(s);
	for(it=item; (i=getl(&ibuf,lbuf))>=0; ){
		if(i==0 || (i=index(':',lbuf))<0) continue;
		if(it>=&item[MAXITEM]){
			writef(2,"no room for rate, %s\n",lbuf);
			continue; }
		s= &lbuf[i];
		*s++= 0;
		tg= trim(lbuf);
		if(igitem(tg)) continue;
		if((i=index(':',s))<0){
			writef(2,"missing cost category on line %s\n",tg);
			continue; }
		s[i]= 0;
		if((it->rate=rndr(atof(s)))<0){
			writef(2,"bad rate on line %s, '%s'\n",tg,s);
			continue; }
		s=+ i+1;
		if((i=index(':',s))>=0){
			tt= trim(&s[i+1]);
			s[i]= 0; }
		if((it->cat=getcat(trim(s)))<0){
			writef(2,"bad cost category on line %s, '%s'\n",tg,s);
			continue; }
		if(igcat(it->cat)) continue;
		it->tag= mkstr(tg);
		it->title= mkstr(tt);
		it++; }
	nitem= it-item;
	qsort(item,nitem,sizeof(struct item), &icmp);
	item[nitem].cat= -1; }

getcat(s) char *s;{
	register char *t, *c;
	register struct categ *cp;
	int n;

	for(cp=categ; c=cp->abbr; cp++){
		for(t=s; *c==*t; t++, c++)
			if(*c==0) break;
		if(*c) continue;
		if(*t==0) n= 0;
		else if(scanl(t,"%u$", &n)<=0) continue;
		return (cp->major<<8) +n; }
	return -1; }

igitem(s) register char *s;{
	register char **ig, *abbr;
	if(s) for(ig=ignore; (abbr= *ig++); )
			if(eqstr(s,abbr)) return 1;
	return 0; }

igcat(c) register int c;{
	register char **ig, *abbr;
	for(ig=ignore; (abbr= *ig++); )
		if(c==getcat(abbr)) return 1;
	return 0; }

mkstr(s) char *s;{
	register int i;
	register char *b;
	if((i=strlen(s))>0){
		if((b=alloc(i+1))==0177777){
			writef(2,"no space\n");
			exit(); }
		copystr(s,b);
		return b; }
	return 0; }

icmp(ip1,ip2) register struct item *ip1, *ip2;{
	if(ip1->cat < ip2->cat) return -1;
	return (ip1->cat > ip2->cat); }

spr(ib) char *ib;{
	register int i;
	register char *s;
	register struct item *it;
	double x, eval();
	int igr, ignr, rc;

	igr= igitem("::");
	ignr= igitem(":");
	for( ; (i=getl(ib,lbuf))>=0; ){
		if(i==0 || (i=index(':',lbuf))<0) continue;
		s= &lbuf[i];
		*s++= 0;
		rc= 0;
		if(*s==':'){
			s++;
			rc++;
			if(igr) continue; }
		else if(ignr) continue;
		if((it=loc(trim(lbuf))) && (x=eval(s))>0.0)
			it->qnt=+ rc? x*rmult : x*mult;
		}
	mkfmt();
	print(); }

loc(s) char *s;{
	register struct item *it;
	for(it=item; it<&item[nitem]; it++)
		if(eqstr(s,it->tag)) return it;
	return 0; }

mkfmt(){
	register struct item *it;
	register int n, w;
	double totc, toth, maxr;
	char fh[32], fr[32], fc[32];
	int maxt, ll, dh, dr, dc, wh, wr, wc;

	ll= 66;
	for(it=item, totc=0, toth=0, maxr=0, maxt=0; it<&item[nitem]; it++){
		if(it->cat.hibyte==LABOR){
			toth=+ it->qnt;
			if(maxr<it->rate) maxr= it->rate; }
		totc=+ it->qnt * it->rate;
		if(maxt<(w=strlen(it->title))) maxt= w;
		}
	for(n=0; label[n]; n++)
		if(maxt<(w=strlen(label[n]))) maxt= w;
	dh= ceil(log10(toth+1.0));
	dr= ceil(log10(maxr+1.0));
	dc= ceil(log10(totc+1.0)+1.0);
	wh= dh + (dh-1)/3 + (prchour? 1+prchour:0);
	wr= dr + (dr-1)/3 + (prcrate? 1+prcrate:0);
	wc= dc + (dc-1)/3 + (prccost? 1+prccost:0);
	w= maxt+6+wh+3+wr+3+wc+3;
	w= w<ll? (ll-w)/4 : 0;
	writefb(lbuf,".po 8\n.ll %d\n.nf\n.ta %d %d %d\n", ll,
		maxt+6 +w,
		maxt+6+wh+3 +w,
		maxt+6+wh+3+wr+3 +w );
	nroff= mkstr(lbuf);

	writefb(fh,"%%%d.%c%dF",  wh, prchour?' ':'0', prchour);
	writefb(fr,"%%c%%%d.%dF", wr, prcrate);
	if(prccost) writefb(fc,"%%c%%%d.%dF", wc, prccost);
	else writefb(fc,"%%c%%%dF", wc);

	writefb(lbuf,"%%s%%-%ds\t%s\t%s\t%s\n",maxt,fh,fr,fc);
	fmtqrc= mkstr(lbuf);
	writefb(lbuf,"%%s%%-%ds\t%s\t\t%s\n",maxt,fh,fc);
	fmtqc= mkstr(lbuf);
	writefb(lbuf,"%%s%%-%ds\t\t\t%s\n",maxt,fc);
	fmtc= mkstr(lbuf);

	if((n=wh+w-5)<0) n=0;
	writefb(fh,"%s",&("        Hours"[8-n/2]));
	if((n=wr+w+1-4)<0) n=0;
	writefb(fr,"%s",&("        Rate"[8-n/2]));
	if((n=wc+w+1-4)<0) n=0;
	writefb(fc,"%s",&("        Cost"[8-n/2]));
	writefb(lbuf,"\n\nDirect Labor\t%s\t%s\t%s\n",fh,fr,fc);
	lbhd= mkstr(lbuf);
	writefb(lbuf,"\n\nOther Direct Costs\t\t\t%s\n",fc);
	dchd= mkstr(lbuf);
	dcshd= "\n\nOther Direct Costs\n";

	writefb(fh,"\t%%%dS\t\t%%%dS\n",wh,wc+1);
	writefb(fc,"\t\t\t%%%dS\n",wc+1);
	writefb(lbuf,fh,dashes,dashes);
	addqc= mkstr(lbuf);
	writefb(lbuf,fc,dashes);
	addc= mkstr(lbuf);
	writefb(&lbuf[1],fc,"====================");
	lbuf[0]= '\n';
	addtc= mkstr(lbuf);
	}

print(){
	register struct item *t, *it;
	register int i;
	double tq, stq, tlb, stlb, tdc, stdc, q, c, r;
	int tt, lb, dc, k, dol;

	tt= 0; lb= 0; dc= 0;
	for(it=item, tq=0, tlb=0; it->cat.hibyte==LABOR; ){
		for(i= it->cat, stq=0, stlb=0, dol='$'; it->cat==i; it++){
			if((q=rndq(it->qnt))<=0) continue;
			if(tt==0){ tt++; writef(1,"%s",nroff); }
			if(lb==0){ lb++; ne(5); writef(1,"%s",lbhd); }
			c= rnd(q * it->rate);
			writef(1,fmtqrc,ind1,it->title,q,dol,it->rate,dol,c);
			stq=+ q;
			stlb=+ c;
			dol= ' ';
			}
		if(stlb<=0) continue;
		writef(1,addqc);
		writef(1,fmtqc,ind2,label[0],stq,'$',stlb);
		if(months>0 && (t=ifind((LESC<<8)|i&0377)) && (r=rnde(t->rate*months/12.0))>0){
			ne(3);
			c= rnd(stlb * r);
			stlb=+ c;
			writefb(lbuf,t->title,r*100.0);
			writef(1,fmtc,ind2,lbuf,' ',c);
			writef(1,addc);
			writef(1,fmtc,ind2,label[2],'$',stlb);
			writef(1,"\n");
			}
		if(!oflag && (t=ifind((LOVHD<<8)|i&0377)) && (r=t->rate)>0){
			ne(3);
			c= rnd(stlb * r);
			stlb=+ c;
			writef(1,fmtc,ind2,t->title,' ',c);
			writef(1,addc);
			writef(1,fmtc,ind2,label[4],'$',stlb);
			writef(1,"\n");
			}
		if(gflag==1 && (t=ifind(GA<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(stlb * r);
			stlb=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addc);
			writef(1,fmtc,0,label[6],'$',stlb);
			writef(1,"\n");
			}
		if(fflag==1 && (t=ifind(FEE<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(stlb * r);
			stlb=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addtc);
			writef(1,fmtc,0,label[8],'$',stlb);
			writef(1,"\n");
			}
		tq=+ stq;
		tlb=+ stlb;
		}
	if(tlb>0){
		writef(1,addtc);
		writef(1,fmtc,0,label[8],'$',tlb);
		if(gflag==2 && (t=ifind(GA<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(tlb * r);
			tlb=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addc);
			writef(1,fmtc,0,label[6],'$',tlb);
			writef(1,"\n");
			}
		if(fflag==2 && (t=ifind(FEE<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(tlb * r);
			tlb=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addtc);
			writef(1,fmtc,0,label[10],'$',tlb);
			writef(1,"\n");
		}       }
	for(tdc=0; it->cat.hibyte==DIRECT; ){
		for(i=it->cat, stq=0, stdc=0, dol='$'; it->cat==i; it++){
			if((q=it->qnt)<=0) continue;
			if(tt==0){ tt++; writef(1,"%s",nroff); }
			if(dc==0){ dc++; ne(5); writef(1,"%s", lb? dcshd:dchd); }
			c= rnd(q * it->rate);
			writef(1,fmtc,ind1,it->title,dol,c);
			stdc=+ c;
			dol= ' ';
			}
		if(stdc<=0) continue;
		writef(1,addc);
		writef(1,fmtc,ind2,label[1],'$',stdc);
		if(months>0 && (t=ifind((DESC<<8)|i&0377)) && (r=rnde(t->rate*months/12.0))>0){
			ne(3);
			c= rnd(stdc * r);
			stdc=+ c;
			writefb(lbuf,t->title,r*100.0);
			writef(1,fmtc,ind2,lbuf,' ',c);
			writef(1,addc);
			writef(1,fmtc,ind2,label[3],'$',stdc);
			writef(1,"\n");
			}
		if(!oflag && (t=ifind((DOVHD<<8)|i&0377)) && (r=t->rate)>0){
			ne(3);
			c= rnd(stdc * r);
			stdc=+ c;
			writef(1,fmtc,ind2,t->title,' ',c);
			writef(1,addc);
			writef(1,fmtc,ind2,label[5],'$',stdc);
			writef(1,"\n");
			}
		if(gflag==1 && (t=ifind(GA<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(stdc * r);
			stdc=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addc);
			writef(1,fmtc,0,label[7],'$',stdc);
			writef(1,"\n");
			}
		if(fflag==1 && (t=ifind(FEE<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(stdc * r);
			stdc=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addtc);
			writef(1,fmtc,0,label[9],'$',stdc);
			writef(1,"\n");
			}
		tdc=+ stdc;
		}
	if(tdc>0){
		ne(2);
		writef(1,addtc);
		writef(1,fmtc,0,label[9],'$',tdc);
		if(gflag==2 && (t=ifind(GA<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(tdc * r);
			tdc=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addc);
			writef(1,fmtc,0,label[7],'$',tdc);
			writef(1,"\n");
			}
		if(fflag==2 && (t=ifind(FEE<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(tdc * r);
			tdc=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addtc);
			writef(1,fmtc,0,label[11],'$',tdc);
			writef(1,"\n");
		}       }
	tdc=+ tlb;
	if(tdc>0){
		writef(1,addtc);
		writef(1,fmtc,0,label[12],'$',tdc);
		if(gflag==3 && (t=ifind(GA<<8)) && (r=t->rate)>0){
			ne(3);
			c= rnd(tdc * r);
			tdc=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addc);
			writef(1,fmtc,0,label[13],'$',tdc);
			writef(1,"\n");
			}
		if(fflag==3 && (t=ifind(FEE<<8)) && (r=t->rate)>0){
			ne(4);
			c= rnd(tdc * r);
			tdc=+ c;
			writef(1,fmtc,0,t->title,' ',c);
			writef(1,addtc);
			writef(1,fmtc,0,label[14],'$',tdc);
			}
		else {  ne(2);
			writef(1,"\n");
			writef(1,fmtc,0,label[14],'$',tdc);
	}       }       }

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

double rnd(x) double x;{
	if(roundc>0) return floor(x*roundc+0.5)/roundc;
	return x; }

double rndq(x) double x;{
	if(roundq>0) return floor(x*roundq+0.5)/roundq;
	return x; }

double rndr(x) double x;{
	if(roundr>0) return floor(x*roundr+0.5)/roundr;
	return x; }

double rnde(x) double x;{
	return floor(x*100.0+0.5)/100.0; }

ifind(c){
	register struct item *it;
	for(it=item; it->cat>0; it++)
		if(it->cat==c) return it;
	return 0; }

ne(n){  writef(1,".ne %d\n",n); }
