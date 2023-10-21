#
char copr[] "copyright (c) 1975 Thomas S. Duff";
/*
 * To get version which works on 11/40 machines,
 * #define MOD40 MOD40
 *
 */
/*
 *	dump analyzer
 *	Modified for v7 by B Thomson Oct/80
 *
 *	format codes
 *	a	address
 *	b	byte(octal)
 *	c	char
 *	d	decimal word
 *	e	decimal byte
 *	f	filler byte
 *	l	unsigned decimal word
 *	m	unsigned decimal byte
 *	n	print a newline
 *	o	octal word
 *	s	string (indirect, zero terminated
 *	q	end of dump
	Next two by BT Oct/80:
 *	O	octal long int
 *	D	decimal long int
 */
#include <sys/param.h>
#include <sys/proc.h>

struct proc proc[NPROC];
long int getlong();

#define CURRMODEUSER 0140000
#define SYSINTR "call" /* the routine that interrupts go thru */
#define FORMFEED 014
#define ASIZE 800
#define BUF "a6oloD2bl"
#define TSIZE 800
#define C16 "a16c"
#define MAP "aoo"
#define TTY "a3(doo)8ond3m3cmm6c"
#define SPEEDTAB "a16o"	/* What is this? BT */
#define DEVTAB BUF	/* They are identical in V7 */
#define FILE "abmoO"
#define FILSYS "alDl5(n10D)nl10(n10l)n4bOD3ln6c6c3(n12l)ff"
#define INODE "abmolo3lO2(n7O)"
#define B16 "a16b"
#define PROC "abbe3mo4d5ol"
#define ZOMB "abbe3mo4doOO"	/* New for zombie PROCs BT */
#define TEXT "a4ommbf"
struct{
	char sym[8];
	int type;
	char *val;
}sym;
struct a{
	char *name;
	char *fmt;
	char *addr;
}
a[ASIZE]{
	"0", "o", 0,
	"r0", "o", 04,
	"r1", "o", 06,
	"r2", "o", 010,
	"r3", "o", 012,
	"r4", "o", 014,
	"r5", "o", 016,
	"sp", "o", 020,
	"kdsa6", "o", 022,
	"vectors", "aoo", 024,
	0, 0, 0177777},
b[]{
"_acctbuf", "a10c3oO3doobf", 0,
"_acctp", "ao", 0,
"_bdevsw", "a4o", 0,
"_bfreeli", BUF, 0,
"_buf", BUF, 0,
"_buffers", FILSYS, 0,
"_callout", "adoo", 0,
"_canonb", C16, 0,
"_cbad", "al", 0,
"_cdevsw", "a7o", 0,
"_cfree", "ao14c", 0,
"_cfreelist", "ao", 0,
"_coremap", MAP, 0,
"_cputype", "ad", 0,
"_curpri", "aef", 0,
"_dc11", TTY, 0,	/* Not tested BT */
"_dcrstab", SPEEDTAB, 0,	/* Not tested BT */
"_dctstab", SPEEDTAB, 0,	/* Not tested BT */
"_dk_busy", "ao", 0,
"_dk_numb", "a3D", 0,
"_dk_time", "a8D3(n8D)", 0,
"_dk_wds", "a3D", 0,
"_dl11", TTY, 0,	/* Not tested BT */
"_dv_size", "aoo", 0,	/* Not tested BT */
"_dvhdr", "a4o", 0,	/* Not tested BT */
"_dvq", "aoobb", 0,	/* Not tested BT */
"_dvtab", DEVTAB, 0,	/* Not tested BT */
"_end", "q", 0,
"_file", FILE, 0,
"_grab", "a4o", 0,	/* Not tested BT */
"_grabmap", MAP, 0,	/* Not tested BT */
"_gw", "abbobbooobbbb", 0,	/* Not tested BT */
"_gwkb", TTY, 0,	/* Not tested BT */
"_inode", INODE, 0,
"_io_info", "ad4D5(n6D)", 0,	/* V7 iostat stuff BT */
"_ipc", "a4o", 0,
"_kl11", TTY, 0,
"_linesw", "a10o", 0,	/* New in V7 BT */
"_lp11", "a7o", 0,	/* Not tested BT */
"_maplock", "ao", 0,	/* Useful only on 11/70 BT */
"_maptab", C16, 0,
"_maxmem", "ao", 0,
"_mount", "a3o", 0,
"_moveto", "aodd", 0,	/* Not tested BT */
"_mpid", "ad", 0,
"_mpxip", "ao", 0,
"_msgbuf", "a16c", 0,
"_msgbufp", "ao", 0,
"_nblkdev", "ad", 0,
"_nchrdev", "ad", 0,
"_ngrab", "ad", 0,	/* Not tested BT */
"_nldisp", "ad", 0,
"_nswap", "ad", 0,
"_out_cou", "ad", 0,	/* Not tested BT */
"_panicstr", "as", 0,
"_partab", B16, 0,
"_pc11", "ao2(doo)", 0,	/* Not tested BT */
"_pipedev", "ao", 0,
"_proc", PROC, 0,
"_rablock", "aD", 0,
"_rdvbuf", BUF, 0,	/* Not tested BT */
"_regloc", "a9d", 0,
"_rktab", DEVTAB, 0,	/* Not tested BT */
"_rl_loc", "ao", 0,
"_rltab", DEVTAB, 0,
"_rootdev", "ao", 0,
"_rootdir", "ao", 0,
"_runin", "ao", 0,
"_runout", "ao", 0,
"_runq", "ao", 0,
"_runrun", "ao", 0,
"_rrkbuf", BUF, 0,	/* Not tested BT */
"_rtmbuf", BUF, 0,	/* Not tested BT */
"_scrlq", TTY, 0,	/* Not tested BT */
"_si_size", "aoo", 0,	/* Not tested BT */
"_slpque", "ao", 0,
"_swapdev", "ao", 0,
"_swapmap", MAP, 0,
"_swbuf1", BUF, 0,
"_swbuf2", BUF, 0,
"_swplo", "ao", 0,
"_sysent", "aoo", 0,
"_t_openf", "a8b", 0,	/* Not tested BT */
"_t_blkno", "a8o", 0,	/* Not tested BT */
"_t_nxrec", "a8o", 0,	/* Not tested BT */
"_tab", "abboddobbooo", 0,	/* Not tested BT */
"_tab_trk", "aooooooooo", 0,	/* Not tested BT */
"_tab_ink", "abbooddbbdddooooo", 0,	/* Not tested BT */
"_tab_event", "aoddoooooooddooooo", 0,	/* Not tested BT */
"_text", TEXT, 0,
"_time", "aO", 0,
"_tk_nin", "aD", 0,
"_tk_nout", "aD", 0,
"_tmtab", DEVTAB, 0,	/* Not tested BT */
"_tran_ta", C16, 0,	/* Not tested BT */
"_updlock", "ao", 0,
"_vect", "aee", 0,	/* Not tested BT */
0, 0, 0},
/* Note: a fmt of 0 in the line above indicates default printing in octal
 * until the next address.
 */



t[TSIZE]{
	"0", 0, 0,
	0, 0, 0177777},
u[]{
"rsave", "a6o", 0,
"fper", "ao", 0,
"fpsaved", "ao", 0,
"fps", "ao6(n4o)", 0,
"segflg", "ab", 0,
"error", "am", 0,
"uid", "al", 0,
"gid", "al", 0,
"ruid", "al", 0,
"rgid", "al", 0,
"procp", "ao", 0,
"ap", "ao", 0,
"r", "aO", 0,
"base", "ao", 0,
"count", "al", 0,
"offset", "aO", 0,
"cdir", "ao", 0,
"rdir", "ao", 0,
"dbuf", "a14c", 0,
"dirp", "ao", 0,
"dent", "al14c", 0,
"pdir", "ao", 0,
"uisa", "a8on8o", 0,
"uisd", "a8on8o", 0,
"ofile", "a10on10o", 0,
"pofile", "a10bn10b", 0,
"arg", "a5o", 0,
"tsize", "ao", 0,
"dsize", "ao", 0,
"ssize", "ao", 0,
"qsav", "a6o", 0,
"ssav", "a6o", 0,
"signal", "a17o", 0,
"utime", "aO", 0,
"stime", "aO", 0,
"cutime", "aO", 0,
"cstime", "aO", 0,
"ar0", "ao", 0,
"prof", "a4o", 0,
"intflg", "ab", 0,
"sep", "ab", 0,
"ttyp", "ao", 0,
"ttyd", "ao", 0,
"exdata", "a8o", 0,
"comm", "a14c", 0,
"start", "aO", 0,
"acflag", "abf", 0,
"fpflag", "ao", 0,
"cmask", "ao", 0,
"stack", "a7o19(na16o)", 0,
0, 0, 0};
int symf, dumpf;
int nsyms;
int datalast;
int textlast 1;
unsigned buf[8];
int nerrs 0;
int nocore 0;
int noprocs 0;
char *addr;

char *tran(c)
	register char c;
{
	static char s[10];
	if(c<' ' || c>=0177)
		sprintf(s, "\\%o", c&0377);
	else
		sprintf(s, "%c", c);
	return(s);
}
int ubuf[USIZE*32];
char *pfmt();

main(argc, argv)
char *argv[];
{
	register i;
	register struct a *p, *q;
	int r5;
	int kdsa6;
	int upc;
	char *procaddr;
	while(argc>1 && argv[1][0]=='-') {
		switch(argv[1][1]) {
		case 'p':
			nocore++;
			break;
		case 'c':
			noprocs++;
			break;
		default:
			printf("bad option\n");
			exit(0);
		}
		argc--;
		argv++;
	}
	if(argc<3){
		printf("arg count\n");
		exit(0);
	}
	if(argc>3)
		upc = onum(argv[3]);
	else
		upc = 0;
	for(datalast=0;a[datalast].name;datalast++);
	/*
	 * first, read the text, data and bss symbols from the a.out,
	 * sort them by address, and look up the format strings
	 * of the data & bss symbols
	 */
	if((symf=open(argv[2], 0))<0){
		perror(argv[2]);
		exit(0);
	}
	if(read(symf, buf, sizeof buf)!=sizeof buf)
		symerr();
	lseek(symf, (long) sizeof buf, 0);
	lseek(symf, (long) buf[1], 1);
	lseek(symf, (long) buf[2], 1);
	if(!buf[7]){
		lseek(symf, (long) buf[1], 1);
		lseek(symf, (long) buf[2], 1);
	}
	nsyms = (long) (unsigned) buf[4] / sizeof sym;
	for(i=0;i<nsyms;i++){
		if(read(symf, &sym, sizeof sym)!=sizeof sym)
			symerr();
		if((sym.type&07)==3 || (sym.type&07)==4){	/*data or bss*/
			for(p=a; sym.val>p->addr; p++);
			if(datalast==ASIZE-1){
				printf("ASIZE table overflow");
				exit(0);
			}
			for(q= &a[datalast++]; q>=p; q--){
				q[1].addr=q->addr;
				q[1].name=q->name;
				q[1].fmt=q->fmt;
			}
			p->addr=sym.val;
			p->name=copy(sym.sym);
			for(q=b; q->name;q++)
				if(eq(q->name, p->name))
			break;
			p->fmt=q->fmt;
		}
		else if((sym.type&07)==2){	/*text*/
			for(p=t; sym.val>p->addr; p++);
			if(textlast==TSIZE-1){
				printf("TSIZE table overflow");
				exit(0);
			}
			for(q= &t[textlast++]; q>=p; q--){
				q[1].addr=q->addr;
				q[1].name=q->name;
				q[1].fmt=q->fmt;
			}
			p->addr=sym.val;
			p->name=copy(sym.sym);
		}
	}
	close(symf);
	if((dumpf=open(argv[1], 0))<0){
		perror(argv[1]);
		exit(0);
	}
	time(buf);
	printf("Dump of %s. Names from %s. time: %s",argv[1], argv[2],
		ctime(buf));
	if(noprocs)
		goto Printcore;
	/*
	 * First, root around for in-core user areas, and print them
	 * and trace them back.
	 * Then read the dump file, printing symbols where appropriate,
	 * and dumping according to format.
	 */
	lseek(dumpf, (long) 016, 0);
	if(read(dumpf, &r5, 2)!=2)
		dumperr();
	lseek(dumpf, (long) 022, 0);
	if(read(dumpf, &kdsa6, 2)!=2)
		dumperr();
	for(p=a;p->name;p++)
		if(eq(p->name, "_proc"))
	break;
	if(!p->name){
		printf("no proc table\n");
		exit(0);
	}
	lseek(dumpf, (long) p->addr, 0);
	if(read(dumpf, proc, sizeof proc)!=sizeof proc)
		dumperr();
	for(i=0;i<NPROC;i++){
		if(proc[i].p_stat){
			printf("\nprocess %3d; state= %5u (", i, proc[i].p_stat);
			switch(proc[i].p_stat) {
			case SSLEEP:
				printf("hi prio sleep");
				break;
			case SWAIT:
				printf("lo prio sleep");
				break;
			case SRUN:
				printf("ready to run");
				break;
			case SIDL:
				printf("being created");
				break;
			case SZOMB:
				printf("zombie");
				break;
			case SSTOP:
				printf("tracing stop state");
				break;
			default:
				printf("incorrect state");
				break;
			}
			printf("); addr= %4o\n", proc[i].p_addr);
			if(proc[i].p_stat!=SZOMB && proc[i].p_flag&SLOAD){
				if((procaddr=proc[i].p_addr)==kdsa6)
					printf("this process was executing at the time of the dump\n");
				lseek(dumpf, ((long)(unsigned)proc[i].p_addr)<<6, 0);
				read(dumpf, ubuf, sizeof ubuf);
				lseek(dumpf, ((long)(unsigned)proc[i].p_addr)<<6, 0);
				printf("per process area:\n");
				addr=0140000;
				for(p=u;p->name;p++){
					printf("%s:\n", p->name);
					pr(p);
				}
				if(procaddr==kdsa6)
					traceback(r5,upc);
				else
					traceback(ubuf[3],0);
				putchar(FORMFEED);
			}
			else
				printf("not in core\n");
		}
	}
Printcore:
	if(nocore)
		exit(0);
	lseek(dumpf, (long) (addr=0), 0);
	p=a;
#ifdef MOD40
	while(addr < 0400){
		if(addr!=p->addr){
			printf("out of sync\n");
			lseek(dumpf, (long)(addr=p->addr), 0);
		}
		printf("\n%s:\n", p->name);
		while(addr<p[1].addr && addr < 0400)
			pr(p);
		p++;
	}
	lseek(dumpf, (long)(addr=t[textlast-1].addr), 0);
#endif
	for(;;){
		if(addr!=p->addr){
			printf("out of sync\n");
			lseek(dumpf, (long) (addr=p->addr), 0);
		}
		printf("\n%s:\n", p->name);
		while(addr<p[1].addr)
			pr(p);
		p++;
	}
}
symerr(){
	printf("symbol file error\n");
	exit(0);
}

copy(as)
char *as;
{
	register char *s, *q;
	register i;
	s=as;
	q=malloc(9);
	for(i=0;i<8;i++)
		q[i]=s[i];
	q[8]='\0';
	return(q);
}

eq(as, aq)
char *as, *aq;
{
	register char *s, *q;
	register i;
	s=as;
	q=aq;
	for(i=0;i<8;i++){
		if(*s++!=*q)
	return(0);
		if(*q++=='\0')
	return(1);
	}
	return(1);
}
getc(){
	char c;
	addr++;
	if(read(dumpf, &c, 1)!=1)
		dumperr();
	return(c);
}
getw(){
	int n;
	if(read(dumpf, &n, 2)!=2)
		dumperr();
	addr=+ 2;
	return(n);
}
long int
getlong(){
	long int r;
	if(read(dumpf, &r, sizeof r)!=sizeof r)
		dumperr();
	addr += sizeof r;
	return(r);
}
dumperr(){
	nerrs++;
}
pr(p)
	register struct a *p;
{
	register i;

	if(p->fmt)
		pfmt(p->fmt);
	else {
		i = 0;
		while(addr<p[1].addr) {
			if(i%16==0)
				printf("%6o:", addr);
			printf("%6o ", getw());
			if(i%16==15)
				putchar('\n');
			i++;
		}
		if(i%16)
			putchar('\n');
	}
}

char *pfmt(as)
char *as;
{
	register char *s;
	register repcnt;
	char c;
	register char *n;
	char *news;
	int i;
	s=as;
	for(;;){
		repcnt=0;
		while('0'<=*s && *s<='9'){
			repcnt =* 10;
			repcnt =+ *s++-'0';
		}
		if(repcnt==0)
			repcnt=1;
		news=s+1;
		while(repcnt--){
			switch(*s){
			case '\0':
				putchar('\n');
				return(s);
			case '(':
				news=pfmt(s+1);
				break;
			case ')':
				return(s+1);
			case 'a':
				printf("%6o:", addr);
				break;
			case 'b':
				printf("%6o ", getc()&0377);
				break;
			case 'c':
				printf("  %4s ",tran(getc()));
				break;
			case 'd':
				printf("%5d. ", getw());
				break;
			case 'e':
				printf("%5d. ", getc());
				break;
			case 'f':
				getc();
				break;
			case 'l':
				printf("%5u. ", getw());
				break;
			case 'm':
				printf("%5u. ", getc()&0377);
				break;
			case 'n':
				printf("\n       ");
				break;
			case 'o':
				printf("%6o ", getw());
				break;
			case 's':
				if(lseek(dumpf, (long) (n=getw()), 0)) {
					printf("%6o ", n);
				}
				else{
					putchar('"');
					for(i=0; i<80 && read(dumpf, &c, 1)==1 && c; i++)
						printf("%s", tran(c));
					printf("\" ");
				}
				lseek(dumpf, (long) addr, 0);
				break;
			case 'q':
				printf("\nend of dump\n");
				if(nerrs)
					printf("%d dump error%c\n", nerrs, nerrs!=1?'s':'\0');
				exit(0);
			case 'O':
				printf("%11O ", getlong());
				break;
			case 'D':
				printf("%11D ", getlong());
				break;
			}
		}
		s=news;
	}
}
traceback(ar5,lightpc)
char *lightpc;
{
	register r5, i;
	register char *pc;
	int j;
	int s;
	r5=ar5-0140000;
	printf("traceback");
	if(lightpc)
	{
		for(j=0; t[j].addr<=lightpc; j++)
			;
		j--;
		printf(" %s+%o:\n",t[j].name,lightpc-t[j].addr);
	}
	else
		printf(":\n");
	printf("  r2     r3     r4    routine\n");
	if(r5>=0 && r5<sizeof ubuf)
	{
		pc=ubuf[r5/2+1];
		for(i=0;i<60;i++){
			for(j=0;t[j].addr<=pc;j++);
			j--;
			printf("%6o %6o %6o ", ubuf[r5/2-3], ubuf[r5/2-2], ubuf[r5/2-1]);
			printf("%s+%o\n", t[j].name, pc-t[j].addr);
			if(eq(t[j].name, SYSINTR))
			{
				printf("********** Interrupt from ");
				s = r5/2 + 2;
				/* it's magic from here on */
				if((ubuf[s+6] & CURRMODEUSER)==CURRMODEUSER)
					printf("user mode:    ");
				else
					printf("system mode:  ");
				printf("dev=%6o   sp=%6o   r1=%6o   nps=%6o   r0=%6o   pc=%6o   ps=%6o\n",
					ubuf[s],ubuf[s+1],ubuf[s+2],ubuf[s+3],
					ubuf[s+4],ubuf[s+5],ubuf[s+6]);
				pc=ubuf[s+5];
				if((ubuf[s+6] & CURRMODEUSER)==CURRMODEUSER)
		break;
			}
			else
			{
				r5=ubuf[r5/2]-0140000;
				if(r5<0 || r5>=sizeof ubuf)
		break;
				pc=ubuf[r5/2+1];
			}
		}
	}
	if(i>=60)
		printf("aborted ");
	printf("end of traceback\n");
	if(ubuf[3] != ar5)
	{
		printf("Traceback restarts with r5 from u table\n");
		traceback(ubuf[3],lightpc);
	}
}


onum(cstr)
char *cstr;
{
	register char *c;
	register int r;
	r=0;
	for(c=cstr; *c != '\0';c++)
	{
		if(*c<'0' || *c>'9')
		{
			printf("Bad octal number (%s) supplied for pc\n",cstr);
			exit(0);
		}
		r = r*8 + *c - '0';
	}
	return(r);
}
