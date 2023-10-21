#
/*
 *	pm -- post mortem dump analizer
 *
 *	syntax:
 *
 *	% pm [<file>]
 *
 *	<file> is the dump file (/usr/sys/core is default)
 *
 */

#include "/usr/sys/param.h"
#include "/usr/sys/user.h"
#include "/usr/sys/reg.h"

#define eject putchar('\014');

#define	REGS	04	/* starting location of registers */
#define KA6	022	/* starting location of KA6 register */
#define	ULEN	1024	/* size of USER table */
#define LAST_R5	0141756	/* last valid R5 in the loop */

#define SP	6
#define PC	7

struct statb {
	char name[8];
	int flag;
	char *value;
};

char ubuf[ULEN];
int header[8];
struct user *up	ubuf;

int reg[8];
int	ka6;
int	fd;

char *core	"/usr/sys/core";
char	*names	"/deadstart/new-unix";
int fout;
int nfd;
int off1, off2;

main(argc, argp)
char **argp;
{
	register i;

	if(argc >1){
		if(argc > 2){
			printf("syntax: pm [<file>]\n");
			exit(1);
		}
		else
			core = argp[1];
	}
	if((fd = open(core,0)) <  0){
		printf("can't read %s\n", core);
		exit(2);
	}
	get_u();
	pregs();
	dump(ka6);
	interp();
	trace();
	flush();
	exit(0);
}

/*
 * getu -- read in user structure and other stuff
 */

get_u()
{
	register k;

	if((nfd = open(names, 0)) < 0){
		printf("can't open namelist file: %s\n",names);
		exit(3);
	}
	read(nfd, header, 16);
	k = (header[7] != 1 ? 2 : 1);
	off1 = ((header[1]/512) + (header[2]/512)) * k;
	off2 = ((header[1]%512) + (header[2]%512)) * k;
	off2 =+ 020;
	off1 =+ off2/512;
	off2 =% 512;
	seek(fd, REGS, 0);
	read(fd, reg, 16);
	seek(fd, KA6, 0);
	read(fd, &ka6, 2);
	seek(fd, ka6/8, 3);
	seek(fd, (ka6%8)<<6, 1);
	read(fd, ubuf, ULEN);
}

/*
 * print registers:
 */

pregs()
{
	register i;

	printf("\n\n\tProcessor Registers:\n\n");
	for(i=0; i<6; i++)
		printf("\t\tR%d = %s\n", i, o6(reg[i]));
	printf("\t\tSP = %s\n\t\tK6 = %s\n\n", o6(reg[SP]), o6(ka6));
}

/*
 * dump -- unformatted dump of user structure
 */
dump(base)
{
	register int *buf;
	register i,j;
	char *vaddr, *paddr;

	vaddr = 0140000;
	paddr = base << 3;
	buf = up;
	printf("\n\n\n\t\t\tunformatted dump of user structure\n");
	printf("\n\n\n\t phys\t user\n");
	printf("\t addr\t addr\t\t\t\tdata\n");
	printf("\t======\t======\t======================================================\n\n");
	for(i=0; i<64; i++){
		printf("\n\t%6o0\t%6o", paddr, vaddr);
		vaddr =+ 020;
		paddr =+ 02;
		for(j=0; j<8; j++)
			printf(" %s", o6(*buf++));
	}
	eject;
}

/*
 * interpret selected "u" info.
 */
interp()
{
	register i, p;

	printf("\n\n\n\t\tFormatted dump of User structure\n\n\n");
	printf("\tuid:\t%d\n", up->u_uid);
	if(up->u_uid != up->u_ruid)
		printf("\treal uid:\t%d\n", up->u_ruid);
	printf("\n\n\tpending signals:\t");
	p = 0;
	for(i=0; i<NSIG; i++){
		p =| up->u_signal[i];
		if(up->u_signal[i])
			printf("%d   ",i);
	}
	if(p)
		putchar('\n');
	else
		printf("<none>\n");
	printf("\n\n\t\tI/O info:\n");
	printf("\tbase = %6o, count = %6o\n",up->u_base, up->u_count);
	printf("\n\n\n\t\tSegmentation registers:\n\n");
	printf("\tu_sep = %o\n\n", up->u_sep);
	printf("\n\t\t USIA\t USID\n\t\t======\t======\n\n");
	for(i=0; i<16; i++)
		printf("\t%d\t%6o\t%6o\n", i, up->u_uisa[i], up->u_uisd[i]);
	eject;
}

/*
 * map & unmap -- map a pointer to/from
 * ubuf and actual "u" addresses.
 */
int *map(p)
{
	return(0140000 + (p-ubuf));
}
char *unmap(p)
{
	return((p - 0140000) + ubuf);
}

/*
 * trace -- print execution traceback info
 */

trace()
{
	register int i, *p;

	p = unmap(reg[5]);
	i = 0;
	printf("\n\n\n\t\t\ttraceback info:\n\n");
	printf("\t  R5\t  PC\t  name \t\targuments\n");
	printf("\t------\t------\t---------\t------------------------------\n\n");
	while(*p != LAST_R5){
		if(p < &ubuf[0] || p > &ubuf[ULEN]){
			printf("\n\t******* bad R5 = %6o, trace terminated ********\n\n", map(p));
			break;
		}
		if(i++ > 35){
			printf("\n\t****** excessive nesting -- trace terminated ********\n\n");
			break;
		}
		tbf(p);
		p = unmap(*p);
	}
	printf("\n\t******\t******\t<end of trace>\n\n\n");
}

/*
 *  tbf -- trace buffer formatted
 */
tbf(p)
int *p;
{
	printf("\t%6o\t%6o",p[0],p[1]);
	printf("\t%9.9s\n", pname(p[1]));
}

/*
 * pname --  find the name in the a.out file
 */

pname(apc)
{
	register char *pc, *diff;
	register int d;
	static char n[9];
	static struct statb buf;

	pc = apc;
	diff = 077777;
	save(n, "???");
	seek(nfd, off1, 3);
	seek(nfd, off2, 1);
	while(read(nfd, &buf, 12) == 12){
		d = pc - buf.value;
		if(d > 0 && d < diff){
			save(n, buf.name);
			diff = d;
		}
	}
	return(n);
}

save(n,b)
char *n, *b;
{
	int i;
	for(i=0; i<8; i++)
		*n++ = *b++;
	*n++ = 0;
}

o6(v)
{
	register i;
	register char *p;
	static char buf[8];

	buf[0] = (v<0 ? '1' : '0');
	buf[6] = 0;
	p = buf+1;
	for(i=0; i<5; i++){
		*p++ = (((v>>12)&7) + '0');
		v =<< 3;
	}
	return(buf);
}

o3(v)
{
	return(o6(v) + 3);
}
