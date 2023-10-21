/*
**  System-specific stuff.  This module will need to be ported to
**  other systems.
*/
/* LINTLIBRARY */
#include "shar.h"
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef	SYS_WAIT
#include <sys/wait.h>
#else
#endif	/* SYS_WAIT */
RCS("$Header: sysfuncs.c,v 1.7 87/03/13 13:08:34 rs Exp $")


/* How to fork(), what to wait with. */
#ifdef	SYS_WAIT
#define FORK()		 vfork()
#define W_VAL(w)	 ((w).w_retcode)
typedef union wait	 WAITER;
#else
#define FORK()		 fork()
#define W_VAL(w)	 ((w) >> 8)
typedef int		 WAITER;
#endif	/* SYS_WAIT */


/* Mask of executable bits. */
#define	EXE_MASK	(S_IEXEC | (S_IEXEC >> 3) | (S_IEXEC >> 6))

/* Stat buffer for last file. */
static struct stat	 Sb;


/*
**  Get user name.  Not secure, but who sends nastygrams as shell archives?
*/
char *
User()
{
    extern struct passwd	*getpwuid();
    struct passwd		*p;
    char			*g;

    if (g = getenv(USER_ENV))
	return(g);
    return((p = getpwuid(getuid())) ? p->pw_name : "USER");
}


/*
**  Set up a signal handler.
*/
SetSigs(What, Func)
    int		  What;
    int		(*Func)();
{
    if (What == S_IGNORE)
	Func = SIG_IGN;
    else if (What == S_RESET)
	Func = SIG_DFL;
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	(void)signal(SIGINT, Func);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	(void)signal(SIGQUIT, Func);
}


/*
**  Stat the file if it's not the one we did last time.
*/
int
GetStat(p)
    char		*p;
{
    static char		 Name[BUFSIZ];

    if (*p == Name[0] && EQ(p, Name))
	return(TRUE);
    return(stat(strcpy(Name, p), &Sb) < 0 ? FALSE : TRUE);
}


/*
**  Return the file type -- directory or regular file.
*/
int
Ftype(p)
    char	*p;
{
    return(GetStat(p) && ((Sb.st_mode & S_IFMT) == S_IFDIR) ? F_DIR : F_FILE);
}


/*
**  Return the file size.
*/
off_t
Fsize(p)
    char	*p;
{
    return(GetStat(p) ? Sb.st_size : 0);
}


/*
**  Is a file executable?
*/
int
Fexecute(p)
    char	*p;
{
    return(GetStat(p) && (Sb.st_mode & EXE_MASK) ? TRUE : FALSE);
}


/*
**  Return the process ID.
*/
int
Pid()
{
    static int	 X;

    if (X == 0)
	X = getpid();
    return(X);
}


/*
**  Return the text string that corresponds to errno.
*/
char *
Ermsg(e)
    int			 e;
{
    extern int		 sys_nerr;
    extern char		*sys_errlist[];
    static char		 buff[30];

    if (e > 0 && e < sys_nerr)
	return(sys_errlist[e]);
    (void)sprintf(buff, "Error code %d", e);
    return(buff);
}


/*
**  Fork off a command.
*/
int
Execute(av)
    char		*av[];
{
    register int	 i;
    register int	 j;
    WAITER		 W;

    if ((i = FORK()) == 0) {
	SetSigs(S_RESET, (int (*)())NULL);
	(void)execvp(av[0], av);
	perror(av[0]);
	_exit(1);
    }

    SetSigs(S_IGNORE, (int (*)())NULL);
    while ((j = wait(&W)) < 0 && j != i)
	;
    SetSigs(S_RESET, (int (*)())NULL);
    return(W_VAL(W));
}


#ifdef	NEED_MKDIR
/*
**  Quick and dirty mkdir routine for them that's need it.
*/
int
mkdir(name, mode)
    char	*name;
    int		 mode;
{
    char	*av[3];
    int		 i;

    av[0] = "mkdir";
    av[1] = name;
    av[2] = NULL;
    U = umask(~mode);
    i = Execute(av);
    (void)umask(U);
    return(i ? -1 : 0);
}
#endif	/* NEED_MKDIR */


#ifdef	NEED_QSORT
/*
**  Bubble sort an array of arbitrarily-sized elements.  This routine
**  can be used as an (inefficient) replacement for the Unix qsort
**  routine, hence the name.  If I were to put this into my C library,
**  I'd do two things:
**	-Make it be a quicksort;
**	-Have a front routine which called specialized routines for
**	 cases where Width equals sizeof(int), sizeof(char *), etc.
*/
qsort(Table, Number, Width, Compare)
    register char	 *Table;
    register int	  Number;
    register int	  Width;
    register int	(*Compare)();
{
    register char	 *i;
    register char	 *j;

    for (i = &Table[Number * Width]; (i -= Width) >= &Table[Width]; )
	for (j = i; (j -= Width) >= &Table[0]; )
	    if ((*Compare)(i, j) < 0) {
		register char	*p;
		register char	*q;
		register int	 t;
		register int	 w;

		/* Swap elements pointed to by i and j. */
		for (w = Width, p = i, q = j; --w >= 0; *p++ = *q, *q++ = t)
		    t = *p;
	    }
}
#endif	/* NEED_QSORT */


#undef NOTDEF
#ifdef	NOTDEF
/*
**  Cons all the arguments together into a single command line and hand
**  it off to the shell to execute.  This is only here for someone to use
**  as the basis of, say, an MSDOS port.
*/
int
Execute(av)
    register char	*av[];
{
    register char	**v;
    register char	 *p;
    register char	 *q;
    register int	 i;

    /* Get length of command line. */
    for (i = 5, v = av; *v; v++)
	i += strlen(*v) + 1;

    /* Create command line and execute it. */
    p = NEW(char, i);
    for (q = p + strlen(strcpy(p, "exec")), v = av; *v; v++) {
	*q++ = ' ';
	q += strlen(strcpy(q, *v));
    }

    return(system(p));
}
#endif	/* NOTDEF */
