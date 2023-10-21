/* :ts=8 bk=0
 *
 * Copyright (C) 1986,1987 by Manx Software Systems, Inc.
 * Modified and made available for general use with permission of
 * Manx Software Systems, Inc.
 *
 *------------------------------------------------------------------------
 *
 * This is special code which can be run from the Workbench or from the
 * CLI.  When run from the CLI, the program detaches itself from the CLI
 * and starts running in the background (giving your CLI prompt back).
 * This means that all I/O from the program must be through windows
 * created by the program.
 *
 * This startup routine correctly handles a start from the WorkBench.
 * If started from the CLI, it will pass the command line arguments to the
 * "child" process.  This routine also handles quoted arguments (in a
 * rather primitive way).
 *
 * This suceesfully compiles and runs under MANX 3.20a.  Not guaranteed
 * to work with other compilers.  Probably won't work with MANX 3.40,
 * either (without modification).
 *
 * Original version by (and profuse thanks to)
 *	Jim Goodnow II				86??.??
 * Severely enhanced by Leo L. Schwab		8703.12
 */

#include <ctype.h>
#include <fcntl.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>

#define	ERRSTR		"Improperly quoted argument.\n"
#define	ERRLEN		28L
#define	FATALSTR	"Fatal error on startup.\n"
#define FATALEN		24L


extern long	_Open(), _Input(), _Output(), _CurrentDir(), CreateProc();
extern void	*_OpenLibrary(), *_GetMsg(), *_AllocMem(), *_FindTask();


struct _dev {
	long	fd;
	short	mode;
} _devtab[20];

struct _preserve {
	struct Message msg;
	char **av, *ab;
	long cd;
	int ac, al;
};

long	_savsp;
int	errno, Enable_Abort;
void	*SysBase, *DOSBase, *MathBase, *MathTransBase;


static struct	WBStartup *WBenchMsg;
static int	argc, arglen;
static char	**argv, *argbuf;

static char	unique[] = "Highly Unlikely Program Invocation Name";


_main(alen, aptr)
long alen;
char *aptr;
{
	register struct Process *pp;
	register struct CommandLineInterface *cli;
	register long l;
	register unsigned short c;
	register char *cp;
	struct FileHandle *fp;
	struct MemList *mm;
	struct _preserve *p;
	static long cdir = 0;
	long *lp;
	void *sav;

	if (!(DOSBase = _OpenLibrary (DOSNAME, 0L))) {
		Alert (AG_OpenLib | AO_DOSLib, 0L);
		_exit (100);
	}

	pp = _FindTask(0L);
	if (pp->pr_CLI) {		/*  CLI invocation (first run)  */
		cdir = _CurrentDir (0L);
		_CurrentDir (cdir);

		/*  Parse out command line arguments  */
		cli = (struct CommandLineInterface *) ((long)pp->pr_CLI << 2);
		cp = (char *) ((long) cli -> cli_CommandName << 2);
		arglen = *cp + alen + 2;	/* *cp is BSTR len */
		argbuf = _AllocMem ((long) arglen, MEMF_PUBLIC | MEMF_CLEAR);

		strncpy (argbuf, cp+1, *cp);	/*  Copy command name  */
		strcpy (argbuf + *cp, " ");	/*  Space separator  */
		strncat (argbuf, aptr, (int) alen+1);	/*  Copy args  */
		argbuf[*cp] = '\0';		/* Terminate full cmd name */

		for (argc=1, cp=argbuf + *cp + 1;; argc++) {
			while (isspace (*cp))
				cp++;
			if (*cp < ' ') {
				*cp = 0;
				break;	/*  Stop on ctl char  */
			}
			if (*cp == '"') {	/*  Handle quoted args  */
				*cp = ' ';	/*  Squash quote mark  */
				while ((c = *cp) && c != '"')
					cp++;
				if (!c) {	/*  No matching quote  */
					_Write (_Output(), ERRSTR, ERRLEN);
					_exit (200);
				}
				*cp++ = 0;
			} else {
				while ((c = *cp) && !isspace (c))
					cp++;
				*cp++ = 0;
				if (!c)
					break;	/*  Stop at end-of-line  */
			}
		}

		/*  Assemble argv[] array  */
		argv = _AllocMem ((long) (argc+1) * sizeof (*argv),
				  MEMF_PUBLIC);
		for (c=0, cp=argbuf; c<argc; c++) {
			while (isspace (*cp))
				cp++;
			argv[c] = cp;
			cp += strlen (cp) + 1;
		}
		argv[c] = 0;

		/*  Preserve argument environment  */
		if (!(p = _AllocMem ((long) sizeof (*p), MEMF_PUBLIC))) {
			Alert (AG_NoMemory, 0L);
			_exit (100);
		}
		p -> ac = argc;
		p -> av = argv;
		p -> ab = argbuf;
		p -> al = arglen;
		p -> cd = cdir;

		/*  Argument list finished.  Detatch program.  */
		l = cli -> cli_Module;
		if (!(sav = _OpenLibrary (DOSNAME, 33L))) {
			/*  Malarkee for 1.1 DOS braindamage  */
			lp = (long *)*((long *)*((long *)*((long *)*((long *)
				_savsp+2)+1)-3)-3)+107;
			if (*lp != cli -> cli_Module) {
				_Write (_Output(), FATALSTR, FATALEN);
				_exit (300);
			}
		} else {
			_CloseLibrary (sav);
			lp = 0;
		}

		/*  Prevent DOS from unloading us  */
		if (lp)
			*lp = 0;
		cli -> cli_Module = 0;
#asm
		move.l	__savsp,-(sp)
#endasm
		_Forbid ();
		PutMsg (CreateProc (unique, 0L, l, 5120L), p);
		_CloseLibrary (DOSBase);
#asm
		move.l	(sp)+,sp
		rts
#endasm
	} else	/*  Check if this is this is the second time through  */
	if (!strcmp (pp->pr_Task.tc_Node.ln_Name, unique)) {
		/*  Recover parent's arguments  */
		_WaitPort (&pp -> pr_MsgPort);
		p = _GetMsg (&pp -> pr_MsgPort);
		argv	= p -> av;
		argc	= p -> ac;
		argbuf	= p -> ab;
		arglen	= p -> al;
		cdir	= p -> cd;
		_FreeMem (p, (long) sizeof (*p));

		/*  Change process name to something reasonable  */
		pp -> pr_Task.tc_Node.ln_Name = argv[0];

		/*  Convert DOS's seglist to MemList  */
		lp = (long *) ((long) pp -> pr_SegList << 2);
		lp = (long *) (lp[3] << 2);
		sav = lp;
		c = 0;
		while (lp) {
			lp = (long *) (*lp << 2);
			c++;
		}
		mm = _AllocMem ((long) sizeof (struct MemList)+
				(c-1)*sizeof (struct MemEntry), MEMF_PUBLIC);
		lp = sav;
		mm -> ml_NumEntries = c;
		c = 0;
		while (lp) {
			mm -> ml_me[c].me_Addr = (APTR) lp - 1;
			mm -> ml_me[c].me_Length = lp[-1];
			lp = (long *) (*lp << 2);
			c++;
		}

		/*  Add MemList to task structure to force auto-unload  */
		AddTail (&((struct Task *)pp)->tc_MemEntry, mm);

		_CurrentDir (cdir);
		main (argc, argv);
		_exit (0);

	} else {	/*  Started from WorkBench  */
		_WaitPort (&pp -> pr_MsgPort);
		WBenchMsg = _GetMsg (&pp -> pr_MsgPort);
		if (WBenchMsg -> sm_ArgList)
			_CurrentDir (WBenchMsg -> sm_ArgList -> wa_Lock);

		if (WBenchMsg -> sm_ToolWindow)
			if (_devtab[0].fd =
			    _Open (WBenchMsg->sm_ToolWindow, MODE_OLDFILE)) {
				_devtab[1].fd = _devtab[2].fd = _devtab[0].fd;
				_devtab[0].mode = 0x8000;
				_devtab[1].mode = _devtab[2].mode = 0x8001;
				fp=(struct FileHandle *) (_devtab[0].fd << 2);
				pp -> pr_ConsoleTask = (APTR) fp -> fh_Type;
			}

		main (0, WBenchMsg);
		_exit (0);
	}
}

void (*_cln)() = 0;

_exit (code)
{
	int fd;

	for (fd = 0; fd < 20; fd++)
		close (fd);
	if (_cln)
		(*_cln)();

	if (MathTransBase)	_CloseLibrary (MathTransBase);
	if (MathBase)		_CloseLibrary (MathBase);
	if (DOSBase)		_CloseLibrary (DOSBase);

	if (!WBenchMsg) {	/*  Free the argument list  */
		_FreeMem (argbuf, (long) arglen);
		if (argv)
			_FreeMem (argv, (long) (argc+1) * sizeof (*argv));
	} else {
		_Forbid ();
		_ReplyMsg (WBenchMsg);
	}
#asm
		move.l	8(a5),d0	; Get return code
		move.l	__savsp,sp	; Restore original stack pointer
		rts			; Bye-bye!
#endasm
}
