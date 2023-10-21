#
/*
 */

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */
extern
	nullsys(),
	rexit(),
	fork(),
	read(),
	write(),
	open(),
	close(),
	wait(),
	creat(),
	link(),
	unlink(),
	exec(),
	chdir(),
	gtime(),
	mknod(),
	chmod(),
	chown(),
	sbreak(),
	stat(),
	seek(),
	getpid(),
	smount(),
	sumount(),
	setuid(),
	getuid(),
	stime(),
	ptrace(),
	alarm(),
	fstat(),
	pause(),
	nullsys(),
	stty(),
	gtty(),
	saccess(),
	nice(),
	sslep(),
	sync(),
	kill(),
	getswit(),
	nosys(),
	tell(),
	dup(),
	pipe(),
	times(),
	profil(),
	nosys(),
	setgid(),
	getgid(),
	ssig(),
	geteuid(),
	clrtty(),
	nicer(),
	wake(),
	nosys(),
	nosys(),
	nosys(),
	nosys(),
	nosys(),
	nosys(),
	nosys(),
	nosys(),
	nosys(),
	nosys(),
	empty();			
	timlim();

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
	0, &gtime,			/* 13 = time */
	3, &mknod,			/* 14 = mknod */
	2, &chmod,			/* 15 = chmod */
	2, &chown,			/* 16 = chown */
	1, &sbreak,			/* 17 = break */
	2, &stat,			/* 18 = stat */
	2, &seek,			/* 19 = seek */
	0, &getpid,			/* 20 = getpid */
	3, &smount,			/* 21 = mount */
	1, &sumount,			/* 22 = umount */
	0, &setuid,			/* 23 = setuid */
	0, &getuid,			/* 24 = getuid */
	0, &stime,			/* 25 = stime */
	3, &ptrace,			/* 26 = ptrace */
	0, &alarm,			/* 27 = alarm */
	1, &fstat,			/* 28 = fstat */
	0, &pause,			/* 29 = pause */
	1, &nullsys,			/* 30 = smdate; inoperative */
	1, &stty,			/* 31 = stty */
	1, &gtty,			/* 32 = gtty */
	2, &saccess,			/* 33 = access */
	0, &nice,			/* 34 = nice */
	0, &sslep,			/* 35 = sleep */
	0, &sync,			/* 36 = sync */
	1, &kill,			/* 37 = kill */
	0, &getswit,			/* 38 = switch */
	0, &nosys,			/* 39 = x */
	0, &tell,			/* 40 = tell */
	0, &dup,			/* 41 = dup */
	0, &pipe,			/* 42 = pipe */
	1, &times,			/* 43 = times */
	4, &profil,			/* 44 = prof */
	0, &nosys,			/* 45 = tiu */
	0, &setgid,			/* 46 = setgid (nop) */
	0, &getgid,			/* 47 = getgid (nop) */
	2, &ssig,			/* 48 = sig */
	0, &geteuid,			/* 49 = geteuid (16 bit effective uid) */
	0, &clrtty,			/* 50 = clrtty (clear controlling tty) */
	1, &nicer,			/* 51 = nicer (nice on another job) */
	0, &wake,			/* 52 = wakeup (for debugging) */
	0, &timlim,                     /* 53 = time limit  */
	0, &nosys,			/* 54 = x */
	0, &nosys,			/* 55 = x */
	0, &nosys,			/* 56 = x */
	0, &nosys,			/* 57 = x */
	0, &nosys,			/* 58 = x */
	0, &nosys,			/* 59 = x */
	0, &nosys,			/* 60 = x */
	0, &nosys,			/* 61 = x */
	0, &nosys,			/* 62 = x */
	0, &empty			/* 63 = empty (rand) */
};
