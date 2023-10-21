/*
	signal -- old system call emulation for 4.2BSD (VAX version)
		(adapted from BRL UNIX System V emulation for 4.2BSD)

	last edit:	25-Aug-1984	D A Gwyn

	NOTE:  Although this module is VAX-specific, it should be
	possible to adapt it to other fairly clean implementations of
	4.2BSD.  The difficulty lies in avoiding the automatic restart
	of certain system calls when the signal handler returns.  I use
	here a trick first described by Donn Seeley of UCSD Chem. Dept.
*/

#include	<errno.h>
#include	<signal.h>
#include	<syscall.h>

extern int	sigvec();
extern int	sigsetmask();

extern		etext;
extern int	errno;

static int	(*handler[NSIG])() =	/* "current handler" memory */
	{
	BADSIG				/* initially, unknown state */
	};
static int	inited = 0;		/* for initializing above */

static int	catchsig();
static int	ret_eintr();

int	(*
signal( sig, func )			/* returns previous handler */
	)()
	register int	sig;		/* signal affected */
	register int	(*func)();	/* new handler */
	{
	register int	(*retval)();	/* previous handler value */
	struct sigvec	oldsv;		/* previous state */
	struct sigvec	newsv;		/* state being set */

	if ( func >= (int (*)())&etext )	/* "lint" hates this */
		{
		errno = EFAULT;
		return BADSIG;		/* error */
		}

	/* cancel pending signals */
	newsv.sv_handler = SIG_IGN;
	newsv.sv_mask = newsv.sv_onstack = 0;
	if ( sigvec( sig, &newsv, &oldsv ) != 0 )
		return BADSIG;		/* error */

	/* C language provides no good way to initialize handler[] */
	if ( !inited )			/* once only */
		{
		register int	i;

		for ( i = 1; i < NSIG; ++i )
			handler[i] = BADSIG;	/* initialize */

		++inited;
		}

	/* the first time for this sig, get state from the system */
	if ( (retval = handler[sig-1]) == BADSIG )
		retval = oldsv.sv_handler;

	handler[sig-1] = func;	/* keep track of state */

	if ( func == SIG_DFL )
		newsv.sv_handler = SIG_DFL;
	else if ( func != SIG_IGN )
		newsv.sv_handler = catchsig;	/* actual sig catcher */

	if ( func != SIG_IGN		/* sig already being ignored */
	  && sigvec( sig, &newsv, (struct sigvec *)0 ) != 0
	   )
		return BADSIG;		/* error */

	return retval;			/* previous handler */
	}


/* # bytes to skip at the beginning of C ret_eintr() function code: */
#define	OFFSET	2			/* for VAX .word reg_mask */

/* PC will be pointing at a syscall if it is to be restarted: */
typedef unsigned char	opcode;		/* one byte long */
#define	SYSCALL		((opcode)0xBC)	/* VAX CHMK instruction */
#define	IMMEDIATE	((opcode)0x8F)	/* VAX immediate addressing */


/*ARGSUSED*/
static int
catchsig( sig, code, scp )		/* signal interceptor */
	register int		sig;	/* signal number */
	int			code;	/* code for SIGILL, SIGFPE */
	register struct sigcontext	*scp;	/* -> interrupted context */
	{
	register int		(*uhandler)();	/* user handler */
	register opcode		*pc;	/* for snooping instructions */
	struct sigvec		newsv;	/* state being set */

	/* at this point, sig is blocked */

	uhandler = handler[sig - 1];

	/* most UNIXes usually want the state reset to SIG_DFL */
	if ( sig != SIGILL && sig != SIGTRAP )
		{
		handler[sig-1] = newsv.sv_handler = SIG_DFL;
		newsv.sv_mask = newsv.sv_onstack = 0;
		(void)sigvec( sig, &newsv, (struct sigvec *)0 );
		}

	(void)sigsetmask( scp->sc_mask );	/* restore old mask */

	/* at this point, sig is not blocked, usually have SIG_DFL;
	   a longjmp may safely be taken by the user signal handler */

	(void)(*uhandler)( sig );	/* user signal handler */

	/* must now avoid restarting certain system calls */
	pc = (opcode *)scp->sc_pc;
	if ( *pc++ == SYSCALL
	  && (*pc == SYS_read || *pc == SYS_write || *pc == SYS_ioctl
	   || *pc++ == IMMEDIATE
	   && (*pc == SYS_wait || *pc == SYS_readv || *pc == SYS_writev)
	     )
	   )
		scp->sc_pc = (int)ret_eintr + OFFSET;

	/* return here restores interrupted context */
	}


static int
ret_eintr()				/* substitute for system call */
{
	errno = EINTR;
	return -1;
}
