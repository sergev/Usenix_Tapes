#
char copr[] "copyright (c) 1975 Thomas S. Duff";
/*
 *	dump analyzer
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
 */
#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"
#define UMODE 0140000
#define SYSINTR "call" /* the routine that interrupts go thru */
#define FORMFEED 014
#define ASIZE 200
#define TSIZE 350
#define BUF "a6od3odo"
#define C64 "a64c"
#define MAP "aoo"
#define TTY "a3(doo)oonbbccocfoo"
#define SPEEDTAB "a16o"
#define DEVTAB "abm4o"
#define FILE "abb3o"
#define INODE "abmolo3mbon10o"
#define B16 "a16b"
#define PROC "abbe3mbeodd5od"
#define TEXT "a4obb"
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
	"", 0, 0440,	/* end of vectors, skip to next symbol */
	0, 0, 0177777},
b[]{
"_bdevsw", "a4o", 0,
"_bfreeli", BUF, 0,
"_bk_hit", "a2o", 0,
"_bk_miss", "a2o", 0,
"_buf", BUF, 0,
/* "_buffers", "a3l10(n10l)nl10(n10l)n4b2o7(n7l)", 0, */
"_buffers", "a32(16bn)bb", 0,
"_callout", "adoo", 0,
"_canonb", C64, 0,
"_cdevsw", "a5o", 0,
"_cfree", "a6co", 0,
"_coremap", MAP, 0,
"_cputype", "ad", 0,
"_curpri", "aef", 0,
"_dc11", TTY, 0,
"_dcrstab", SPEEDTAB, 0,
"_dctstab", SPEEDTAB, 0,
"_dk_busy", "ao", 0,
"_dk_numb", "a3(2o)", 0,
"_dk_time", "a3(2o)", 0,
"_dk_wds", "a3(2o)", 0,
"_dl11", TTY, 0,
"_dv_size", "aoo", 0,
"_dvhdr", "a4o", 0,
"_dvq", "aoobb", 0,
"_dvtab", DEVTAB, 0,
"_dx", "a2(doo)oo", 0,
"_dz11", TTY, 0,
"_dzspeed", B16, 0,
"_end", "q", 0,
"_execnt", "ad", 0,
"_file", FILE, 0,
"_grab", "a4o", 0,
"_grabmap", MAP, 0,
"_gw", "abbobbooobbbb", 0,
"_gwkb", TTY, 0,
"_httab", DEVTAB, 0,
"_icode", "a11o", 0,
"_iflags", "aoo", 0,
"_ik_hit", "a2o", 0,
"_ik_miss", "a2o", 0,
"_inode", INODE, 0,
"_ipc", "a4o", 0,
"_ka6", "ao", 0,
"_kl11", TTY, 0,
"_lbolt", "ad", 0,
"_lks", "ao", 0,
"_lp11", "a7o", 0,
"_lx", "adooobbddd", 0,
"_lxaddr", "aoo", 0,
"_maplock", "ao", 0,
"_maptab", C64, 0,
"_maxmem", "ad", 0,
"_mount", "a3o", 0,
"_moveto", "aodd", 0,
"_mpid", "ad", 0,
"_nblkdev", "ad", 0,
"_nchrdev", "ad", 0,
"_ngrab", "ad", 0,
"_nswap", "ad", 0,
"_number_", "ad", 0,
"_out_cou", "ad", 0,
"_panicst", "as", 0,
"_partab", B16, 0,
"_pc11", "ao2(doo)", 0,
"_proc", PROC, 0,
"_rablock", "ad", 0,
"_rdvbuf", BUF, 0,
"_regloc", "a9e", 0,
"_rktab", DEVTAB, 0,
"_rrkbuf", BUF, 0,
"_rootdev", "abb", 0,
"_rootdir", "ao", 0,
"_rtmbuf", BUF, 0,
"_runin", "abf", 0,
"_runout", "abf", 0,
"_runrun", "abf", 0,
"_rx_open", "ae", 0,
"_rx_erro", "ae", 0,
"_rxtab", DEVTAB, 0,
"_scrlq", TTY, 0,
"_sk_cnt", "a6(2o)", 0,
"_si_size", "aoo", 0,
"_swapdev", "abb", 0,
"_swplo", "ad", 0,
"_swapmap", MAP, 0,
"_swbuf", BUF, 0,
"_sysent", "aoo", 0,
"_sys_dat", "a9c", 0,
"_sys_nam", "a21c", 0,
"_sys_ver", "a5c", 0,
"_t_openf", "a8b", 0,
"_t_blkno", "a8o", 0,
"_t_nxrec", "a8o", 0,
"_tab", "abboddobbooo", 0,
"_tab_trk", "aooooooooo", 0,
"_tab_ink", "abbooddbbdddooooo", 0,
"_tab_event", "aoddoooooooddooooo", 0,
"_text", TEXT, 0,
"_time", "aoo", 0,
"_tk_nin", "a2o", 0,
"_tk_nout", "a2o", 0,
"_tmtab", DEVTAB, 0,
"_tout", "aoo", 0,
"_tran_ta", C64, 0,
"_updlock", "ao", 0,
"_vect", "aee", 0,
"badtrap", "ao", 0,
"nofault", "ao", 0,
"ssr", "a3o", 0,
0, "a16o", 0},
t[TSIZE]{
	"0", 0, 0,
	0, 0, 0177777},
u[]{
"rsave", "aoo", 0,
"fsave", "ao6(n4o)", 0,
"segflg", "ab", 0,
"error", "am", 0,
"uid", "am", 0,
"gid", "am", 0,
"ruid", "am", 0,
"rgid", "am", 0,
"procp", "ao", 0,
/* "grabp", "ao", 0, */
"base", "ao", 0,
"count", "ao", 0,
"offset", "aoo", 0,
"cdir", "ao", 0,
"dbuf", "a14c", 0,
"dirp", "ao", 0,
"ino", "ad", 0,
"name", "a14c", 0,
"pdir", "ao", 0,
"uisa", "a8on8o", 0,
"uisd", "a8on8o", 0,
"ofile", "a15o", 0,
"arg", "a5o", 0,
"tsize", "ao", 0,
"dsize", "ao", 0,
"ssize", "ao", 0,
/* "wsize", "ao", 0, */
"sep", "ao", 0,
"qsave", "aoo", 0,
"ssave", "aoo", 0,
"signal", "a10on10o", 0,
"utime", "ao", 0,
"stime", "ao", 0,
"cutime", "aoo", 0,
"cstime", "aoo", 0,
"ar0", "ao", 0,
"prof", "a4o", 0,
/* "nice", "ae", 0, */
"intflg", "ao", 0,
/* "dsleep", "ao", 0, */
"stack", "a15o22(na16o)", 0,
0, 0, 0};
int symf, dumpf;
int nsyms;
int datalast;
int textlast 1;
int buf[8];
int nerrs 0;
char *addr;
char tran(ac)
char ac;
{
	register char c;
	if((c=ac)<' ' || c==0177)
		return('?');
	return(c);
}
int ubuf[USIZE*32];
extern int fout;
main(argc, argv)
char *argv[];
{
	register i;
	register struct a *p, *q;
	int r5;
	int kdsa6;
	int upc;
	char *procaddr;
	fout=dup(1);
	if(argc<3){
		printf("arg count\n");
		quit();
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
		quit();
	}
	if(read(symf, buf, sizeof buf)!=sizeof buf)
		symerr();
	seek(symf, sizeof buf, 0);
	useek(symf, buf[1], 1);
	useek(symf, buf[2], 1);
	if(!buf[7]){
		useek(symf, buf[1], 1);
		useek(symf, buf[2], 1);
	}
	nsyms=ldiv(0,buf[4],sizeof sym);
	for(i=0;i<nsyms;i++){
		if(read(symf, &sym, sizeof sym)!=sizeof sym)
			symerr();
		if((sym.type&07)==3 || (sym.type&07)==4){	/*data or bss*/
			for(p=a; sym.val>p->addr; p++);
			if(datalast==ASIZE-1){
				printf("table overflow");
				quit();
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
				printf("table overflow");
				quit();
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
		quit();
	}
	time(buf);
	printf("Dump of %s. Names from %s. time: %s",argv[1], argv[2],
		ctime(buf));
	/*
	 * First, root around for in-core user areas, and print them
	 * and trace them back.
	 * Then read the dump file, printing symbols where appropriate,
	 * and dumping according to format.
	 */
	seek(dumpf, 016, 0);
	if(read(dumpf, &r5, 2)!=2)
		dumperr();
	seek(dumpf, 022, 0);
	if(read(dumpf, &kdsa6, 2)!=2)
		dumperr();
	for(p=a;p->name;p++)
		if(eq(p->name, "_proc"))
	break;
	if(!p->name){
		printf("no proc table\n");
		quit();
	}
	seek(dumpf, p->addr, 0);
	if(read(dumpf, proc, sizeof proc)!=sizeof proc)
		dumperr();
	for(i=0;i<NPROC;i++){
		if(proc[i].p_stat){
			printf("\nprocess %3d; state= %5l; addr= %4o\n",
				i, proc[i].p_stat, proc[i].p_addr);
			if(proc[i].p_flag&SLOAD){
				if((procaddr=proc[i].p_addr)==kdsa6)
					printf("this process was executing at the time of the dump\n");
				seek(dumpf, proc[i].p_addr>>3, 3);
				useek(dumpf, (proc[i].p_addr&07)<<6, 1);
				read(dumpf, ubuf, sizeof ubuf);
				seek(dumpf, -sizeof ubuf, 1);
				printf("per process area:\n");
				addr=0140000;
				for(p=u;p->name;p++){
					printf("%s:\n", p->name);
					pfmt(p->fmt);
				}
				if(procaddr==kdsa6)
					traceback(r5,upc);
				else
					traceback(ubuf[1],0);
				putchar(FORMFEED);
			}
			else
				printf("not in core\n");
		}
	}
	seek(dumpf, addr=0, 0);
	p=a;
	for(;;){
		if (p->fmt) {
			if(addr!=p->addr){
				printf("out of sync\n");
				seek(dumpf, addr=p->addr, 0);
			}
			printf("\n%s:\n", p->name);
			while(addr<p[1].addr) {
				pfmt(p->fmt);
			}
		}
		p++;
	}
}
symerr(){
	printf("symbol file error\n");
	quit();
}

copy(as)
char *as;
{
	register char *s, *q;
	register i;
	s=as;
	q=alloc(9);
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
dumperr(){
	nerrs++;
}
char *pfmt(as)
char *as;
{
	register char *s;
	register repcnt;
	char c;
	register char *n;
	char *news;
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
				printf("%3o ", getc()&0377);
				break;
			case 'c':
				printf("%c",tran(getc()));
				break;
			case 'd':
				printf("%5d. ", getw());
				break;
			case 'e':
				printf("%4d. ", getc());
				break;
			case 'f':
				getc();
				break;
			case 'l':
				printf("%5l. ", getw());
				break;
			case 'm':
				printf("%3l. ", getc()&0377);
				break;
			case 'n':
				printf("\n       ");
				break;
			case 'o':
				printf("%6o ", getw());
				break;
			case 's':
				if(seek(dumpf, n=getw(), 0)<0){
					printf("%6o ", n);
				}
				else{
					putchar('"');
					while(read(dumpf, &c, 1)==1 && c)
						putchar(tran(c));
					printf("\" ");
				}
				seek(dumpf, addr, 0);
				break;
			case 'q':
				printf("\nend of dump\n");
				if(nerrs)
					printf("%d dump error%c\n", nerrs, nerrs!=1?'s':'\0');
				quit();
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
				if((ubuf[s+6] & UMODE)==UMODE)
					printf("user mode:    ");
				else
					printf("system mode:  ");
				printf("dev=%6o   sp=%6o   r1=%6o   nps=%6o   r0=%6o   pc=%6o   ps=%6o\n",
					ubuf[s],ubuf[s+1],ubuf[s+2],ubuf[s+3],
					ubuf[s+4],ubuf[s+5],ubuf[s+6]);
				pc=ubuf[s+5];
				if((ubuf[s+6] & UMODE)==UMODE)
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
	if(ubuf[1] != ar5)
	{
		printf("Traceback restarts with r5 from u table\n");
		traceback(ubuf[1],lightpc);
	}
}
quit(){
	flush();
	exit();
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
			quit();
		}
		r = r*8 + *c - '0';
	}
	return(r);
}


useek(sf,sa,st)
{
	if(sa<0)
	{
		seek(sf, 040000, 1);
		seek(sf, 040000, 1);
		seek(sf, sa&077777, 1);
	}
	else
		seek(sf, sa, 1);
}
