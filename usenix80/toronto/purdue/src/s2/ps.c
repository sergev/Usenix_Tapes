#

/*
 *	ps - process status
 *	examine and print certain things about processes
 *	modified, 4-nov-76, g goble, add 16 bit uid's
 *	modified, 27-mar-77, G Goble, add "u" option to print user names
 *	modified 30-aug-77, J Bruner, search for only certain terminals
 *	modified 10-sep-77, J Bruner, use seek-format file for user names
 *      modified 12-oct-77, add "z" option
 *	Added -r5 flag  -  TGI	[ 12/25 18:35 ]
 */

#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"
#include "/usr/sys/tty.h"
#include "/usr/sys/user.h"

#define dh11	(4<<8)		/* dh11 major device number part */
#define pty	(18<<8)		/* pseudo terminal dev major dev */
#define swapdev	"/dev/swap"	/* name of swap device */
#define addr50	050	/* address of "_proc" in system */
#define	SEEKFL	"/etc/u-seek"
#define	UTIME	266	/* location of utime in user area */

struct {
	char name[8];
	int  type;
	char  *value;
} nl[3];

struct {
	int	fdes;
	int	nleft;
	char	*nextc;
	char	buff[512];
} inf;

int	uidfil;
int	lastuid	-1;
char	tbuf[16];

struct proc proc[NPROC];
struct tty tty;
struct user u;
int	lflg	0;
int	zflg	0;
int	kflg	0;
int	xflg	0;
int	tflg	0;
int	aflg	0;
int	rflg	0;	/* repeat flag */
int	uflg	0;	/* print user name instead of uid */
int	cflg	0;	/* print p_cpu instead of p_nice */
int	skflg	0;	/* Use seek-format file for user names */
int	mem;
int	procfd;
int	swap;
char	*proc_addr;
char	*header	"TTY F S    UID     PID  PRI  ADDR  SZ  WCHAN NICE COMMAND";

int	stbuf[257];

/*	tty file characters and their minor-device numbers	*/

int	ndev	127;	/*  number of ttys */
/* NOTE: tty0 and ttyq are switched !!!!!!!!!!!!!!!!1 */
char	devc[127]	{'8','q','1','2','3','4','5','6','7',
			'g','9','f','b','c','d','-','a','h',
			'i','j','k','l','m','n','e','p','0',
			'r','s','t','u','v','w','=','y','z',
			'A','B','C','D','E','F','G','H','I',
			'J','K','L','M','N','O','P','Q','R',
			'S','T','U','V','W','X','Y','Z','+',
			'o','!','\'','`','(',')','$','*','[',
			'&',';','?','>','<','_','.','#','^',
			'@','%','{','}',']',':',',','\7','"',
			'\\',' ','~','|',';',0,0};


int	devl[127]	{0,      dh11+0,  dh11+1,  dh11+2,
			dh11+3,  dh11+4,  dh11+5,  dh11+6,  dh11+7,
			dh11+8,  dh11+9,  dh11+10, dh11+11, dh11+12,
			dh11+13, dh11+14, dh11+15, dh11+16, dh11+17,
			dh11+18, dh11+19, dh11+20, dh11+21, dh11+22,
			dh11+23, dh11+24, dh11+25, dh11+26, dh11+27,
			dh11+28, dh11+29, dh11+30, dh11+31, dh11+32,
			dh11+33, dh11+34, dh11+35, dh11+36, dh11+37,
			dh11+38, dh11+39, dh11+40, dh11+41, dh11+42,
			dh11+43, dh11+44, dh11+45, dh11+46, dh11+47,
			dh11+48, dh11+49, dh11+50, dh11+51, dh11+52,
			dh11+53, dh11+54, dh11+55, dh11+56, dh11+57,
			dh11+58, dh11+59, dh11+60, dh11+61, dh11+62,
			dh11+63, dh11+64, dh11+65, dh11+66, dh11+67,
			dh11+68, dh11+69, dh11+70, dh11+71, dh11+72,
			dh11+73, dh11+74, dh11+75, dh11+76, dh11+77,
			dh11+78, dh11+79,
			pty+0, pty+1, pty+2, pty+3, pty+4,
			pty+5, pty+6, pty+7, pty+8, pty+9,
			pty+10, pty+11, pty+12, pty+13, pty+14,
			pty+15 };

int	devt[127];
char	*coref;
struct ibuf {
	char	idevmin, idevmaj;
	int	inum;
	int	iflags;
	char	inl;
	char	iuid1;
	char	iuid0;
	char	isize0;
	int	isize;
	int	iaddr[8];
	char	*ictime[2];
	char	*imtime[2];
	int	fill;
};
int	obuf[259];


main(argc, argv)
char **argv;
{
	int exit(), sig();
	struct proc *p;
	int n, b, k, c, mtty, uid, puid, nprint;
	register i;
	char *ap;

	obuf[0] = 1;
	if (argc>1) {
		ap = argv[1];
		while (*ap) switch (*ap++) {
		case 'a':
			aflg++;
			break;

		case 't':
			tflg++;
			header = "TTY F S    UID     PID  PRI  ADDR  SZ  WCHAN NICE USER/SYS COMMAND";
			break;

		case 'x':
			xflg++;
			break;

		case 'l':
			lflg++;
			break;

		case 'k':
			kflg++;
			break;

		case 'u':
			uflg++;
			break;

		case 'z':
			zflg++;
			break;

		case 'c':
			cflg++;
			break;

		case 'r':
			rflg = 0;
			while ('0' <= *ap && *ap <= '9')
				rflg = rflg * 10 + *ap++ - '0';
			if (rflg <= 0)
				rflg = 5;	/* illegal/none 5 sec */
			if (rflg) {
				signal(SIGHUP, 0);
				signal(SIGINT, 0);
				signal(SIGQIT, exit);
			}
			break;
		}
	}

	if (argc > 2){
		xflg++;
		aflg++;
	}
	if (uflg) {
		if (close(open(SEEKFL,0)) >= 0) skflg++;
		if (skflg == 0)
			if((uidfil=open("/etc/passwd",0)) < 0) {
				printf("Can't open /etc/passwd\n");
				done();
			}
		}
	if(kflg)
		coref = "/usr/sys/core";
	else
		coref = "/dev/mem";
	if ((mem = open(coref, 0)) < 0) {
		printf("\n can't open mem-file: %s \n",coref);
		done();
	}
	if(kflg) {
		seek(mem, addr50,0);
		read(mem,&proc_addr,2);
		seek(mem, proc_addr>>3, 3);
		seek(mem, (proc_addr&07) << 6, 1);
		read(mem, proc, sizeof proc);
	}
	else {
		if((procfd=open("/dev/proc",0)) < 0) {
			printf("Can't open /dev/proc\n");
			done();
		}
		read(procfd, proc, sizeof proc);
	}
	if((swap = open(swapdev, 0)) < 0) {
		printf("\n error -- can't open swap device: %s\n",swapdev);
		done();
	}
	uid = getuid();
	if(lflg)
		printf("%s\n", header);
	else
		printf("TTY  PID COMMAND\n");
rpt:	nprint = 0;
	for (i = 0; i < NPROC; i++) {
		if (zflg && proc[i].p_ppid <= 1 &&
			(proc[i].p_uid <= 1 || proc[i].p_pri == PWAIT))
				continue;
		if (proc[i].p_stat==0)
			continue;
		if (proc[i].p_ttyp==0) {
			if (xflg==0)
				continue;
			c = '*';
		} else {
			for(c=0; c<ndev; c++)
			if(devt[c] == proc[i].p_ttyp) {
				c = devc[c];
				goto out;
			}
			seek(mem, proc[i].p_ttyp, 0);
			read(mem, &tty, sizeof tty);
			for(c=0; c<ndev; c++)
			if(devl[c] == tty.t_dev) {
				devt[c] = proc[i].p_ttyp;
				c = devc[c];
				goto out;
			}
			c = '*';
		out:;
		}
		puid = proc[i].p_uid;
		if (uid != puid && aflg==0)
			continue;
		if (argc < 3) goto match; else {
			k=argc;
			while(k > 2)
				if (c == *argv[--k]) goto match;
		}
		continue;
	match:
		printf("%c:", c);
		if (lflg) {
			printf("%3o %c", proc[i].p_flag&0377, "0SWRIZT"[proc[i].p_stat]);
			if(uflg)
				if(getname(puid, tbuf)==0)
					printf(" %-8.8s", tbuf);
				else
					printf("%9d", puid);
			else
				printf("%9d", puid);
		}
		printf("%6l", proc[i].p_pid);
		if (lflg) {
			printf("%5d%6o%4d", proc[i].p_pri, proc[i].p_addr,
				(proc[i].p_size+7)>>3);
			if (proc[i].p_wchan)
				printf("%7o", proc[i].p_wchan); else
				printf("       ");
			if (cflg)
				printf("%5d", proc[i].p_cpu & 0377); else
				printf("%5d", proc[i].p_nice);
			if(tflg)
				prtims(i);
		}
		if (proc[i].p_stat==5)
			printf(" <defunct>");
		else
			prcom(i);
		putchar('\n');
		nprint++;
	}
	if (rflg && nprint && !kflg) {
		fflush(obuf);
		sleep(rflg);
		if (nprint > 1)
			putchar('\n');
		seek(procfd, 0, 0);
		read(procfd, &proc, sizeof proc);
		goto rpt;
	}
	done();
}

prcom(i)
{
	int baddr, laddr, mf;
	register int *ip;
	register char *cp, *cp1;
	int c, nbad;

	baddr = 0;
	laddr = 0;
	if(proc[i].p_pid == 0){
		printf(" sched()");
		return(1);
	}
	if (proc[i].p_flag&SLOAD) {
		laddr = proc[i].p_addr;
		mf = mem;
	} else {
		baddr = proc[i].p_addr;
		mf = swap;
	}
	laddr =+ proc[i].p_size - 8;
	baddr =+ laddr>>3;
	laddr = (laddr&07)<<6;
	seek(mf, baddr, 3);
	seek(mf, laddr, 1);
	if (read(mf, stbuf, 512) != 512)
		return(0);
	for (ip = &stbuf[256]; ip > &stbuf[0];) {
		if (*--ip == -1) {
			cp = ip+1;
			if (*cp==0)
				cp++;
			nbad = 0;
			for (cp1 = cp; cp1 < &stbuf[256]; cp1++) {
				c = *cp1;
				if (c==0)
					*cp1 = ' ';
				else if (c < ' ' || c > 0176) {
					if (++nbad >= 5) {
						*cp1++ = ' ';
						break;
					}
					*cp1 = '?';
				}
			}
			while (*--cp1==' ')
				*cp1 = 0;
			if(tflg)
				printf(lflg?" %.16s":" %.64s", cp);
			else
				printf(lflg?" %.24s":" %.64s", cp);
			return(1);
		}
	}
	return(0);
}

done()
{

	fflush(obuf);
	exit();
}

putchar(c)
{

	putc(c, obuf);
}

/* this code stolen from ls.c */
getname(uid, buf)
int uid;
char buf[];
{
	int j, i;
	register int c, n;

	if (uid==lastuid)
		return(0);
	if (skflg){
		if (getpw(uid, buf) < 0) return(-1);
		for(j=0; buf[j] != ':'; j++);
		buf[j]=0;
		return(0);
	}
	inf.fdes = uidfil;
	seek(inf.fdes, 0, 0);
	inf.nleft = 0;
	lastuid = -1;
	do {
		i = 0;
		j = 0;
		n = 0;
		while((c=getc(&inf)) != '\n') {
			if (c<0)
				return(-1);
			if (c==':') {
				j++;
				c = '0';
			}
			if (j==0)
				buf[i++] = c;
			if (j==2)
				n = n*10 + c - '0';
		}
	} while (n != uid);
	buf[i++] = '\0';
	lastuid = uid;
	return(0);
}
prtims(i)
{
	register mf, tfx, tfq;
	int tmbuf[2];

	tfx = proc[i].p_addr;
	if(proc[i].p_flag & SLOAD) {
		mf = mem;
		seek(mf, tfx >> 3, 3);
		seek(mf, (tfx & 07) << 6, 1);
	} else {
		mf = swap;
		seek(mf, tfx, 3);
	}
	seek(mf, UTIME, 1);
	read(mf, tmbuf, 4);
	tfx = ldiv(0, tmbuf[0], 60);
	tfq = ldiv(0, tmbuf[1], 60);
	printf("  %3d/%-3d", tfx, tfq);
}
