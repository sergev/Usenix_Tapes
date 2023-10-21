/*
 *
 *	This is a program to analyse the contents
 *	of the proc array, found in a dump of memory
 *	after a crash.
 *
 *	P. Ivanov.	UNSW.	14-4-77.
 *
 *	P. Ivanov.	UNSW.	April-78
 *
 */

#include "proc.h"

char	*corefile	"/user3/EECF-Maint/sysdump/core";

char	*namelist	"/user3/EECF-Maint/sysdump/unix";

#ifdef	ZOMBIE
#define	ZOMBIE	(pp->p_stat == SZOMB)
#endif

main(argc, argv)
int	argc;
char	*argv[];
{
	extern	bomb();
	extern	fout;

	register int	p, q;
	register struct	proc	*pp;
	int	r;
	struct symbol	*procs, *currs;
	long	l1, l2;
	int	start, stop;
	int	sz;

	fout = dup(1);
	pflg--;
	/*for (p = 1; p <= NSIG; p++)
		signal(p, bomb);*/

	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*(++*argv) != '\0')
			switch (**argv) {

			case 'a':
				aflg++;
				capfflg++;
				continue;

			case 'b':
				bflg++;
				capfflg++;
				continue;

			case 'c':
				cflg++;
				capfflg++;
				continue;

			case 'C':
				capcflg++;
				continue;

			case 'd':
				dflg++;
				continue;

			case 'f':
				fflg++;
				continue;

			case 'F':
				aflg++;
				bflg++;
				cflg++;
				capfflg++;
				oflg++;
				continue;

			case 'g':
				gflg++;
				continue;

			case 'i':
				iflg++;
				continue;

			case 'k':
				kflg++;
				continue;

			case 'l':
				lflg++;
				continue;

			case 'n':
				nflg++;
				continue;

			case 'o':
				oflg++;
				capfflg++;
				continue;

			case 'p':
				/* default process when 'p' alone is 0 */
				pflg = 0;
				for ((*argv)++; (**argv >= '0') && (**argv <= '9'); (*argv)++)
					pflg = (pflg * 10) + **argv - '0';
				(*argv)--;
				if (pflg<0 || pflg>NPROC)
					crash("process request out of range");
				continue;

			case 's':
				sflg++;
				continue;

			case 'S':
				capsflg++;
				continue;

			case 't':
				tflg++;
				continue;

			case 'u':
				uflg++;
				continue;

			case 'w':
				wflg++;
				continue;

			case 'x':
				xflg++;
				continue;

			case 'y':
				yflg++;
				continue;

			case 'z':
				zflg++;
				aflg++;
				bflg++;
				cflg++;
				fflg++;
				capfflg++;
				gflg++;
				iflg++;
				lflg++;
				oflg++;
				sflg++;
				tflg++;
				uflg++;
				xflg++;
				yflg++;
				continue;

			default:
				continue;
			}
		argc--;
	}

	DEBUG
		quest("args decoded, continue ??");

	if (--argc >= 0)
		corefile = *(++argv);
	printf("core image file used was \"%s\"\n", corefile);

	/* get the name list for future reference */

	if (nflg)
		if (--argc >= 0)
			namelist = *(++argv);
		else
			crash("Can't find namelist arg");
	printf("namelist file used was \"%s\"\n\n", namelist);
	if ((namefd = open(namelist, 0)) < 0)
		crash("No namelist file!");

	/* alloc and setup */

	symsort();
	procs = albin("_proc\0\0\0");

	/* open the core file */

	if ((corefd = open(corefile, 0)) < 0)
		crash("No core file");

	/*
	 * this section proper dumps the proc array,
	 * if it can find it that is......
	 */

	DEBUG
		printf("read registers and ");
	if (read(corefd, &regs, sizeof regs) < 0)
		crash("Can't read registers");
	DEBUG
		printf("seek to \"_proc\" at %o ", procs->s_symval);
	seek(corefd, procs->s_symval, 0);
	DEBUG
		printf("successfully\n");

	if (read(corefd, proc, sizeof proc) < 0)
		crash("Can't read proc array");

	/* now decode the contents */
	/* the registers are always useful */

	putregs();

	/* the maps if required */

	getmap();

	/* one or many */

	if (pflg < 0) {
		start = 0;
		stop = NPROC;
	} else
		start = stop = pflg;

	/* first decode the p_stat codes */

	for (p = start; p <= stop; p++) {
		pp = &proc[p];
		if (!lflg && (pp->p_stat == 0))
			continue;
		page();
		printf("\n\nProcess_______ Slot____ %d (%o) at__ address_______ %d (%o)\n\n", p, p,
				procs->s_symval + p * sizeof proc[0],
				procs->s_symval + p * sizeof proc[0]);

		if (capfflg) {
			l1 = procs->s_symval.unsign;
			l2 = sizeof proc[0];
			odump(l1 + (p * sizeof proc[0]), l2);
		}

		printf("\"p_stat\" indicates this process was ");
		if (pp->p_stat>=0 && pp->p_stat<=NOSTAT) {
			printf("%s", stat[pp->p_stat]);
			if (pp->p_addr == regs.r_kisa6)
				warn("this process was running_______");
		} else {
			printf("in an impossible state (%o)\n", pp->p_stat);
			WARNING
				warn("");
		}

		/* now flag codes */

		printf("and \"p_flag\" says it was ");
		if (pp->p_flag == 0)
			printf("swapped out");
		else
			for (q = 0; q < NOFLAG ; q++)
				if ((pp->p_flag >> q) & 01)
					printf("%s", flag[q]);
		printf(".\n\n");

		/* now signals */
		printf("Last signal \"p_sig\" received was %d (%o) - ", pp->p_sig, pp->p_sig);
		if (pp->p_sig>=0 && pp->p_sig<=NSIG)
			printf("%s\n\n", signals[pp->p_sig]);
		else {
			printf("impossible\n\n");
			WARNING
				warn("");
		}

		/* other variables */

#ifdef	ZOMBIE
/* either a zombie or not.... ie pz_shit or p_shit */
	if(ZOMBIE)
		{
		/* pz_shit */
		printf("%5t\"p_pri\"");
#ifdef	AUSAM16
		printf("%29t\"p_uid\"");
#endif
		printf("%65tscheduling%108tprocess\n");
		printf("(negative is high)");
		printf("%22tvalue%38towner");
#ifdef	AUSAM16
		printf("%48t\"p_time\"");
#endif
		printf("%65t\"p_cpu\"%100t\"p_pid\"%117t\"p_ppid\"\n\n");
		printf("%2t%d (%o)", pp->p_pri, pp->p_pri);
#ifdef	AUSAM16
		printf("%22t%d (%o)%38t%s", pp->p_uid, pp->p_uid, getuser(pp->p_uid));
		printf("%48t%d (%o)", pp->p_time, pp->p_time);
#endif
		printf("%65t%d (%o)", pp->p_cpu, pp->p_cpu);
		printf("%100t%d (%o)", pp->p_pid, pp->p_pid);
		printf("%117t%d (%o)\n\n", pp->p_ppid, pp->p_ppid);
		printf("%22t\"pz_ur0\" = %u (%o) /// (%o, %o)\n\n", pp->pz_ur0, pp->pz_ur0, pp->pz_ur0.lobyte, pp->pz_ur0.hibyte);
		printf("%22t\"pz_utime\" =   %D (%O) tix; %D sec.\n", pp->pz_utime, pp->pz_utime, pp->pz_utime/HZ);
		printf("%22t\"pz_stime\" =   %D (%O) tix; %D sec.\n", pp->pz_stime, pp->pz_stime, pp->pz_stime/HZ);
		printf("\n\n");
		}
	  else
		{
#endif	ZOMBIE
		printf("%5t\"p_pri\"");
		printf("%29t\"p_uid\"");
		printf("%65tscheduling%108tprocess\n");
		printf("(negative is high)");
		printf("%22tvalue%38towner");
		printf("%48t\"p_time\"");
		printf("%65t\"p_cpu\"%82t\"p_nice\"%100t\"p_pid\"%117t\"p_ppid\"\n\n");
		printf("%2t%d (%o)", pp->p_pri, pp->p_pri);
		printf("%22t%d (%o)%38t%s", pp->p_uid, pp->p_uid, getuser(pp->p_uid));
		printf("%48t%d (%o)", pp->p_time, pp->p_time);
		printf("%65t%d (%o)", pp->p_cpu, pp->p_cpu);
		printf("%82t%d (%o)", pp->p_nice, pp->p_nice);
		printf("%100t%d (%o)", pp->p_pid, pp->p_pid);
		printf("%117t%d (%o)\n\n", pp->p_ppid, pp->p_ppid);
#ifdef	ZOMBIE
		}
#endif	ZOMBIE

		/* now for other values */

		printf("Controlling tty structure \"p_ttyp\" at address %d (%o) \n\n", pp->p_ttyp, pp->p_ttyp);
		/* decode the tty structure */

		if (yflg && (pp->p_ttyp != NULL))
			ttydecode(pp->p_ttyp);
#ifdef	ZOMBIE
		if (!ZOMBIE) {
#endif
		printf("Swappable image \"p_addr\" at %d (%o) ", pp->p_addr, pp->p_addr);
		printf("of size \"p_size\" %d (%o) * 64 bytes\n\n", pp->p_size, pp->p_size);
		/* decode the u area */

		if (pp->p_flag & (SLOAD | SLOCK))
			{
			if(infree(coremap, pp->p_size, pp->p_addr))
				warn("user area is in free core");
			udecode(pp->p_addr, pp->p_size, procs->s_symval + (p * sizeof proc[0]));
			}

		/* is process on disc ok ? */

		if(pp->p_flag & (SSWAP | SLOCK))
			if(infree(swapmap, (pp->p_size + 7) >> 3, pp->p_addr))
				warn("disc image in free swap space");

		/* what is process waiting for */

		printf("Process is ");
		if (pp->p_wchan == 0)
			printf("not waiting\n\n");
		else {
			currs = getnum(pp->p_wchan, 0);
			printf("waiting on %d (%o)", pp->p_wchan, pp->p_wchan);
			printf(" at %8.8s", currs->s_symbol);
			r = pp->p_wchan - currs->s_symval;
			for(q=0, sz=0; symbols[q].st_sym[0] != 0; q++)
				{
				if(alcom(symbols[q].st_sym,currs->s_symbol) == 0)
					sz=symbols[q].st_sz;
				}
			if (sz != 0) {
				printf("[%d.]", r/sz);
				r =% sz;
			}
			if (r != 0)
				printf(" + %d (%o)", r, r);
			printf("\n\n");
		}
		printf("Text structure \"*p_textp\" was at %d (%o)\n\n", pp->p_textp, pp->p_textp);
		if (xflg && pp->p_textp)
			textdecode(pp->p_textp);
#ifdef	ZOMBIE
		}
#endif
		if (tflg) {
			printf("Forking tree from this process is as follows:");
			treedecode(p);
		}
		dash();
	}
	flush();
}

quest(string)
char	*string;
{
	printf("%s\nreturn to continue, else rubout\n", string);
	flush();
	read(0, wkbuf, 2);
}

crash(string)
char	*string;
{
	printf("proc: %s\n", string);
	flush();
	exit(0);
}

warn(msg)
char	*msg;
{
	printf("%119s%120t%s\n", msg, " *** NOTE ***");
}

page()
{
	putchar('\f');
}

dash()
{
	register	p;
	for (p = 0; p < LINEWIDTH; p++)
		putchar('-');
	putchar('\n');
}

star()
{
	register	p;

	for (p = 0; p < LINEWIDTH; p++)
		putchar('*');
	putchar('\n');
}
