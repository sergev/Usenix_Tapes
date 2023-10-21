/* user area decoding */

#include "proc.h"

udecode(ka6, size, pp)
int	ka6, size;
int	pp;
{
	long	ka6l;

	ka6l = ka6; ka6l =<< 6;
	if(uflg | sflg | iflg | fflg)
		{
		printf(U_PTAB, "User____ Per-process___ _______ Area____\n\n");

		if (capfflg)
			odump(ka6l, size * 64l);
		}

	DEBUG
		printf("seek to u area ");
	lseek(corefd, ka6l, 0);
	DEBUG
		printf("successfully and read it into u\n");
	if (read(corefd, &u, sizeof u) < 0)
		crash("cannot read user area");

	if (u.u_procp != pp) {
		printf(U_PTAB, "\"u_procp\" was %d (%o) and proc structure address is %d (%o)\n",
				u.u_procp, u.u_procp, pp, pp);
		warn("\"u_procp\" and pp do not agree");
	}

	if (uflg)
		ustatic();

	if (sflg || uflg)
		u1(ka6);

	if (iflg || uflg)
		u2();

	if (fflg || uflg)
		u3();

	/* the ever loving end of it */
}

/**************************/

ustatic()
{
	/* static user area data, not decodeable */

	long	l1, l2;
	register	p;
	register	i;

	printf(U_PTAB, "effective user id was %d (%o) - %s", u.u_uid, u.u_uid, getuser(u.u_uid));
	printf("%55tReal user id was %d (%o) - %s\n\n", u.u_ruid, u.u_ruid, getuser(u.u_ruid));

#ifdef	GROUP_ACCESS
	/* groups  are not used at UNSW */
	printf(U_PTAB, "effective group id was %d (%o) - %s", u.u_gid, u.u_gid, getgroup(u.u_gid));
	printf("%55tReal group id was %d (%o) - %s\n\n", u.u_rgid, u.u_rgid, getgroup(u.u_rgid));
#endif

	/* all to do with times and things */

	printf(U_PTAB, "User, system and time limit times\n\n");

#ifndef	UNSW
	l1.hiword = u.u_cutime[0];
	l1.loword = u.u_cutime[1];
	l2.hiword = u.u_cstime[0];
	l2.loword = u.u_cstime[1];
#endif
#ifdef	UNSW
	l1 = u.u_cutime;
	l2 = u.u_cstime;
#endif
#ifndef	UNSW
	printf(U_PTAB, "%20t\"u_utime\"  = %u (%o) tix; %u sec.\n", u.u_utime, u.u_utime, u.u_utime/HZ);
	printf(U_PTAB, "%20t\"u_stime\"  = %u (%o) tix; %u sec.\n", u.u_stime, u.u_stime, u.u_stime/HZ);
#endif
#ifdef	UNSW
	printf(U_PTAB, "%20t\"u_utime\"  = %D (%O) tix; %D sec.\n", u.u_utime, u.u_utime, u.u_utime/HZ);
	printf(U_PTAB, "%20t\"u_stime\"  = %D (%O) tix; %D sec.\n", u.u_stime, u.u_stime, u.u_stime/HZ);
#endif
	printf(U_PTAB, "%20t\"u_cutime\" = %D (%O) tix; %D sec.\n", l1, l1, l1/HZ);
	printf(U_PTAB, "%20t\"u_cstime\" = %D (%O) tix; %D sec.\n", l2, l2, l2/HZ);
#ifdef	TIME_LIMITS
	printf(U_PTAB, "%20t\"u_cpusec\" = %u sec.\n", u.u_cpusec);
	printf(U_PTAB, "%20t\"u_tix\"    = %u tix.\n\n", u.u_tix);
#endif

	/* text data and stack sizes */

	printf(U_PTAB, "Text, data and stack sizes, all *64 bytes.\n\n");
	printf("%20t\"u_tsize\" = %d (%o)", u.u_tsize, u.u_tsize);
	printf("%50t\"u_dsize\" = %d (%o)", u.u_dsize, u.u_dsize);
	printf("%80t\"u_ssize\" = %d (%o)\n\n", u.u_ssize, u.u_ssize);
#ifdef	_1145 | _1170
	printf(U_PTAB, "I/D separation flag \"u_sep\" was %d (%o)\n\n", u.u_sep, u.u_sep);
#endif

#ifdef	FPU
	/* floating regs... not used much at UNSW since all 11/40s */
	printf(U_PTAB, "Saved floating point registers\n\n");
	printf(U_PTAB, "FP Status \"u_fsav[0]\" %6o\n\n",u.u_fsav[0]);
	for (p = 1; p < 25 ; p = p + 4)
		printf(U_PTAB, "\"u_fsav[%2d-%2d]  %6o %6o %6o %6o\n",
			p, p+3, u.u_fsav[p], u.u_fsav[p+1], u.u_fsav[p+2], u.u_fsav[p+3]);
	printf("\n");
#endif

	/* segmentation registers and descriptors */

	printf(U_PTAB, "Segmentation addresses and associated descriptors\n\n");
	printf(U_PTAB, "%8tSet number%30t\"u_uisa\"%49t\"u_uisd\"%70tSet number%92t\"u_uisa\"%111t\"u_uisd\"\n\n");
	for(p = 0; p < 4; p++)
		printf(U_PTAB, "%12t%2d%27t%6d (%6o)%46t%6d (%6o)%74t%2d%89t%6d (%6o)%108t%6d (%6o)\n",
			p, u.u_uisa[p], u.u_uisa[p], u.u_uisd[p], u.u_uisd[p], 
			p+4, u.u_uisa[p+4], u.u_uisa[p+4], u.u_uisd[p+4], u.u_uisd[p+4]);
	printf("\n");
#ifdef	_1145 | _1170
	printf(U_PTAB, "%8tSet number%30t\"u_uisa\"%49t\"u_uisd\"%70tSet number%92t\"u_uisa\"%111t\"u_uisd\"\n\n");
	for(p = 8; p < 12; p++)
		printf(U_PTAB, "%12t%2d%27t%6d (%6o)%46t%6d (%6o)%74t%2d%89t%6d (%6o)%108t%6d (%6o)\n",
			p, u.u_uisa[p], u.u_uisa[p], u.u_uisd[p], u.u_uisd[p], 
			p+4, u.u_uisa[p+4], u.u_uisa[p+4], u.u_uisd[p+4], u.u_uisd[p+4]);
	printf("\n");
#endif


	/* things to do with IO */

	printf(U_PTAB, "\"u_segflg\" indicates IO was in ");
	if (u.u_segflg == 0)
		printf("user space\n");
	else if (u.u_segflg == 1)
		printf("kernel space\n");
	else {
		printf("a strange place %d (%o)\n", u.u_segflg, u.u_segflg);
		WARNING
			warn("funny value for u_segflg");
	}
	printf("\n");
	printf(U_PTAB, "Base address for IO \"u_base\" was %d (%o)\n", u.u_base, u.u_base);
	printf(U_PTAB, "Bytes remaining in IO \"u_count\" were %d (%o)\n", u.u_count, u.u_count);
	printf(U_PTAB, "\"u_offset\" = %D (%O)\n\n", u.u_offset, u.u_offset);


	/* system calls errors and saved r0 */

	printf(U_PTAB, "Last error return code was ");
	switch (u.u_error) {

	case 0:
		printf("null");
		break;

	case EFAULT:
		printf("EFAULT");
		break;		/* EFAULT 106 */

	case EPERM:
		printf("EPERM");
		break;		/* EPERM 1 */

	case ENOENT:
		printf("ENOENT");
		break;		/* ENOENT 2 */

	case ESRCH:
		printf("ESRCH");
		break;		/* ESRCH 3 */

	case EINTR:
		printf("EINTR");
		break;		/* EINTR 4 */

	case EIO:
		printf("EIO");
		break;		/* EIO 5 */

	case ENXIO:
		printf("ENXIO");
		break;		/* ENXIO 6 */

	case E2BIG:
		printf("E2BIG");
		break;		/* E2BIG 7 */

	case ENOEXEC:
		printf("ENOEXEC");
		break;		/* ENOEXEC 8 */

	case EBADF:
		printf("EBADF");
		break;		/* EBADF 9 */

	case ECHILD:
		printf("ECHILD");
		break;		/* ECHILD 10 */

	case EAGAIN:
		printf("EAGAIN");
		break;		/* EAGAIN 11 */

	case ENOMEM:
		printf("ENOMEM");
		break;		/* ENOMEM 12 */

	case EACCES:
		printf("EACCES");
		break;		/* EACCES 13 */

	case ENOTBLK:
		printf("ENOTBLK");
		break;		/* ENOTBLK 15 */

	case EBUSY:
		printf("EBUSY");
		break;		/* EBUSY 16 */

	case EEXIST:
		printf("EEXIST");
		break;		/* EEXIST 17 */

	case EXDEV:
		printf("EXDEV");
		break;		/* EXDEV 18 */

	case ENODEV:
		printf("ENODEV");
		break;		/* ENODEV 19 */

	case ENOTDIR:
		printf("ENOTDIR");
		break;		/* ENOTDIR 20 */

	case EISDIR:
		printf("EISDIR");
		break;		/* EISDIR 21 */

	case EINVAL:
		printf("EINVAL");
		break;		/* EINVAL 22 */

	case ENFILE:
		printf("ENFILE");
		break;		/* ENFILE 23 */

	case EMFILE:
		printf("EMFILE");
		break;		/* EMFILE 24 */

	case ENOTTY:
		printf("ENOTTY");
		break;		/* ENOTTY 25 */

	case ETXTBSY:
		printf("ETXTBSY");
		break;		/* ETXTBSY 26 */

	case EFBIG:
		printf("EFBIG");
		break;		/* EFBIG 27 */

	case ENOSPC:
		printf("ENOSPC");
		break;		/* ENOSPC 28 */

	case ESPIPE:
		printf("ESPIPE");
		break;		/* ESPIPE 29 */

	case EROFS:
		printf("EROFS");
		break;		/* EROFS 30 */

	case EMLINK:
		printf("EMLINK");
		break;		/* EMLINK 31 */

	case EPIPE:
		printf("EPIPE");
		break;		/* EPIPE 32 */
#ifdef	LARGE_FILE_REFERENCES
	case ENREF:
		printf("ENREF");
		break;		/* ENREF 33 */
#endif

	default:
		printf("unknown - %u (%o)\n", u.u_error, u.u_error);
		WARNING
			warn("unknown system error code");
		break;
	}
	printf("\n");
	printf(U_PTAB, "Arguments to system call\n\n");

#ifndef	SHARED_DATA | EP_ADDRESS
	for (p = 0; p < 5; p++)
#endif
#ifdef	SHARED_DATA | EP_ADDRESS
	for (p = 0; p < 8; p++)
#endif
		printf("%25t\"u_arg[%d]\"%40t%6d (%6o)\n", p, u.u_arg[p], u.u_arg[p]);
	printf("\n");
	printf(U_PTAB, "Saved r0 \"u_ar0\" was at %d (%o)\n\n", u.u_ar0, u.u_ar0);

	/* disposition of signals */

	printf(U_PTAB, "Disposition of signals\n\n");
	printf("%8tSignal type%40tNumber%49tDisposition%70tSignal type%102tNumber%111tDisposition\n\n");
	for (p = 0; p < (NSIG + 1)/2; p++) {
		printf("%8t%s%42t%2d", signals[p], p);
		if (u.u_signal[p] & 01)
			printf("%49tignored");
		else if (u.u_signal[p] == 0)
			printf("%49tdefault");
		else
			printf("%49t%d (%o)", u.u_signal[p], u.u_signal[p]);
		i = p + (NSIG + 1)/2;
		if (i < NSIG) {
			printf("%70t%s%104t%2d", signals[i], i);
			if (u.u_signal[i] & 01)
				printf("%111tignored");
			else if (u.u_signal[i] == 0)
				printf("%111tdefault");
			else
				printf("%111t%d (%o)", u.u_signal[i], u.u_signal[i]);
		}
		printf("\n");
	}
	printf("\n");


	/* finally value of flags */

#ifndef	SHARED_DATA
	printf(U_PTAB, "\"u_intflg\" was (%o)\n\n", u.u_intflg);
#endif
#ifdef	SHARED_DATA
	printf(U_PTAB, "\"u_flags\" was (%o)\n", u.u_flags);
	if (u.u_flags & USHRDATA)
		printf(U_PTAB, "\tthe (%o) bit indicates special r/w text segment\n", USHRDATA);
#endif


	/* profiling */

	printf(U_PTAB, "Profiling arguments\n\n");
	for (p = 0; p < 2; p++)
		printf("%20t\"u_prof[%d]\" = %d (%o)%60t\"u_prof[%d]\" = %d (%o)\n",
			p, u.u_prof[p], u.u_prof[p], p+2, u.u_prof[p+2], u.u_prof[p+2]);
	printf("\n");

	/* qsav and ssav */

	printf(U_PTAB, "Label variables\n\n");
	printf(U_PTAB, "\"u_qsav[0]\" = %u (%o) and \"u_qsav[1]\" = %u (%o)\n",
		u.u_qsav[0], u.u_qsav[0], u.u_qsav[1], u.u_qsav[1]);
	printf(U_PTAB, "\"u_ssav[0]\" = %u (%o) and \"u_ssav[1]\" = %u (%o)\n\n",
		u.u_ssav[0], u.u_ssav[0], u.u_ssav[1], u.u_ssav[1]);
}

/*********************************/

u1(ka6)
unsigned ka6;
{
	/* stack trace and decode */

	/* r5 and r6, and possible stack trace */

	printf(U_PTAB, "Saved r6 \"u_rsav[0]\" was %d (%o)%55tSaved r5 \"u_rsav[1] was %d (%o)\n\n",
				u.u_rsav[0], u.u_rsav[0], u.u_rsav[1], u.u_rsav[1]);
#ifdef	BIG_UNIX
	printf(U_PTAB, "Saved kpar5 \"u_ksave\" was %d (%o)\n\n", u.u_ksave, u.u_ksave);
#endif

	if (sflg)
		{
		if (ka6 == regs.r_kisa6)
			{
#ifndef	BIG_UNIX
			stackdecode(regs.r_reg[5], regs.r_reg[6], ka6);
#endif
#ifdef	BIG_UNIX
			stackdecode(regs.r_reg[5], regs.r_reg[6], u.u_ksave, ka6);
#endif
			}
		  else
			{
#ifndef	BIG_UNIX
		stackdecode(u.u_rsav[1], u.u_rsav[0], ka6);
#endif
#ifdef	BIG_UNIX
		stackdecode(u.u_rsav[1], u.u_rsav[0], u.u_ksave, ka6);
#endif
			}
		}
}

/*****************************/

u2()
{
	/* inode decoding */

	register	p;

	/* present directory, inodes and pthnames etc */

	printf(U_PTAB, "Current directory i-node pointer \"u_cdir\" was %d (%o)\n\n", u.u_cdir, u.u_cdir);
	if (iflg)
		idecode(u.u_cdir);
	printf(U_PTAB, "Current pathname component \"u_dbuf[0..%d]\" was =>", DIRSIZ-1);
	for (p = 0; p < DIRSIZ; p++)
		putc(u.u_dbuf[p]);
	printf("<=\n\n");
	printf(U_PTAB, TAB, "Last pathname scanning entries\n");
	printf(U_PTAB, TAB, "current pointer to i-node \"u_dirp\" was %d (%o)\n", u.u_dirp, u.u_dirp);
	printf(U_PTAB, TAB, TAB, "i-number \"u_ino\" %d (%o)\n", u.u_dent.u_ino, u.u_dent.u_ino);
	printf(U_PTAB, TAB, TAB, "pathname \"u_name[0..%d]\" =>", DIRSIZ-1);
	for (p = 0; p < DIRSIZ; p++)
		putc(u.u_dent.u_name[p]);
	printf("<=\n");
	printf(U_PTAB, TAB, "parent directory inode pointer \"u_pdir\" was %d (%o)\n\n", u.u_pdir, u.u_pdir);
	if (iflg)
		idecode(u.u_pdir);
}

/********************************/

u3()
{
	/* file structure decoding etc */

	register	p;

	/* all open files and structures if requested */

	printf(U_PTAB, "Pointers to file structures of open files\n\n");
	for (p = 0; p < NOFILE; p++) {
		printf(U_PTAB, TAB, "\"u_ofile[%2d]\" = %6d (%6o)\n", p, u.u_ofile[p], u.u_ofile[p]);
		if (fflg)
			filedecode(u.u_ofile[p]);
	}
	printf("\n");
}
