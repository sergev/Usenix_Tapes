#
/*
 * sysent.c - system call entry vector
 *
 *	modified 21-Nov-1980 by D A Gwyn:
 *	1) added "access", "empty", and "tell";
 *	2) added "trapcatch" for HCR's RT/EMT;
 *	3) added external declarations to keep new compiler happy.
 */

extern	nosys();
extern	nullsys();
extern	rexit();
extern	fork();
extern	read();
extern	write();
extern	open();
extern	close();
extern	wait();
extern	creat();
extern	link();
extern	unlink();
extern	exec();
extern	chdir();
extern	gtime();
extern	mknod();
extern	chmod();
extern	chown();
extern	sbreak();
extern	stat();
extern	seek();
extern	getpid();
extern	smount();
extern	sumount();
extern	setuid();
extern	getuid();
extern	stime();
extern	ptrace();
extern	fstat();
extern	stty();
extern	gtty();
extern	saccess();
extern	nice();
extern	sslep();
extern	sync();
extern	kill();
extern	getswit();
extern	tell();
extern	dup();
extern	pipe();
extern	times();
extern	profil();
extern	setgid();
extern	getgid();
extern	ssig();
extern	trapcatch();
extern	empty();

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */
int	sysent[]
{
	0, &nullsys,			/*  0 = indir */
	0, &rexit,			/*  1 = exit */
	0, &fork,			/*  2 = fork */
	2, &read,			/*  3 = read */
	2, &write,			/*  4 = write */
	2, &open,			/*  5 = open */
	0, &close,			/*  6 = close */
	0, &wait,			/*  7 = wait */
	2, &creat,			/*  8 = creat */
	2, &link,			/*  9 = link */
	1, &unlink,			/* 10 = unlink */
	2, &exec,			/* 11 = exec */
	1, &chdir,			/* 12 = chdir */
	0, &gtime,			/* 13 = time (V7: use ftime) */
	3, &mknod,			/* 14 = mknod */
	2, &chmod,			/* 15 = chmod */
	2, &chown,			/* 16 = chown */
	1, &sbreak,			/* 17 = break */
	2, &stat,			/* 18 = stat */
	2, &seek,			/* 19 = seek (V7: lseek) */
	0, &getpid,			/* 20 = getpid */
	3, &smount,			/* 21 = mount */
	1, &sumount,			/* 22 = umount */
	0, &setuid,			/* 23 = setuid */
	0, &getuid,			/* 24 = getuid */
	0, &stime,			/* 25 = stime */
	3, &ptrace,			/* 26 = ptrace */
	0, &nosys,			/* 27 = alarm */
	1, &fstat,			/* 28 = fstat */
	0, &nosys,			/* 29 = pause */
	0, &nosys,			/* 30 = utime */
	1, &stty,			/* 31 = stty (V7: use ioctl) */
	1, &gtty,			/* 32 = gtty (V7: use ioctl) */
	2, &saccess,			/* 33 = access */
	0, &nice,			/* 34 = nice */
	0, &sslep,			/* 35 = sleep (V7: ftime) */
	0, &sync,			/* 36 = sync */
	1, &kill,			/* 37 = kill */
	0, &getswit,			/* 38 = csw (V7: obsolete) */
	0, &nosys,			/* 39 = setpgrp */
	0, &tell,			/* 40 = tell (V7: use lseek) */
	0, &dup,			/* 41 = dup (V7: also dup2) */
	0, &pipe,			/* 42 = pipe */
	1, &times,			/* 43 = times */
	4, &profil,			/* 44 = profil */
	0, &nosys,			/* 45 = x */
	0, &setgid,			/* 46 = setgid */
	0, &getgid,			/* 47 = getgid */
	2, &ssig,			/* 48 = signal */
	0, &nosys,			/* 49 = reserved for USG */
	0, &nosys,			/* 50 = reserved for USG */
	0, &nosys,			/* 51 = acct */
	0, &nosys,			/* 52 = phys (temporary) */
	0, &nosys,			/* 53 = lock (temporary) */
	0, &nosys,			/* 54 = ioctl */
	0, &nosys,			/* 55 = reboot */
	0, &nosys,			/* 56 = mpx */
	0, &nosys,			/* 57 = pwbsys */
	2, &trapcatch,			/* 58 = trapcatch (HCR) */
	0, &nosys,			/* 59 = exece */
	0, &nosys,			/* 60 = umask */
	0, &nosys,			/* 61 = chroot */
	0, &nosys,			/* 62 = x */
	0, &empty			/* 63 = empty (nonstandard) */
};
