/*
 * Name:	MicroEMACS
 *		VAX/VMS spawn and attach to a DCL subprocess.
 * Created:	rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 * Modified:
 * 		19-May-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *			Add att-to-parent command to attach to the parent
 *			process.  If we can't attach to parent somehow,
 *			spawn a DCL subjob.  This gives us the same
 *			suspend capability as Unix Emacses.
 *
 *			As an added hook, you can DEFINE/JOB
 *			MG$ATTACHTO to a process name, and
 *			the code will try to attach to that name.
 *
 *			Also, if the logical name MG$FILE is
 *			defined, attachtoparent() will visit that file
 *			when you re-attach to Emacs.  This is useful
 *			for a lot of applications, especially MAIL/EDIT...
 *		26-Jun-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *			Specify process we're attaching to when we attempt
 *			to attach to it.
 *		03-Sep-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *			Call savebuffers() before leaving the editor.
 *			Unlike csh, DCL has no problem with people
 *			logging out without completing subjobs...
 *			#define NOSAVEONZ if you don't want this behavior.
 *		13-Oct-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *			Change MICROEMACS$... to MG$... for consistency.
 */
#include	"def.h"

#include	<ssdef.h>
#include	<stsdef.h>
#include	<descrip.h>
#include	<iodef.h>
#include	<jpidef.h>

#define	EFN	0				/* Event flag.		*/

extern	int	oldmode[3];			/* In "ttyio.c".	*/
extern	int	newmode[3];
extern	short	iochan;
extern	int	ckttysize();			/* Checks for new term size */
#ifndef	NOSAVEONZ
extern	int	savebuffers();			/* Save all buffers before  */
#endif

/*
 * Create a subjob with a copy
 * of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as
 * garbage so that you do a full repaint. Bound
 * to "C-C" and called from "C-Z". The message at
 * the start in VMS puts out a newline. Under
 * some (unknown) condition, you don't get one
 * free when DCL starts up.
 */

spawncli(f, n, k)
{
	register int	s;

#ifndef	NOSAVEONZ
	if (savebuffers() == ABORT)		/* TRUE means all saved,*/
		return (ABORT);			/* FALSE means not.	*/
#endif
	eerase();				/* Get rid of echo line	*/
	ttcolor(CTEXT);				/* Normal color.	*/
	ttnowindow();				/* Full screen scroll.	*/
	ttmove(nrow-1, 0);			/* Last line.		*/
	eputs("Starting DCL");
	ttputc('\r');
	ttputc('\n');
	ttflush();
	sgarbf = TRUE;
	s = sys(NULL);				/* NULL => DCL.		*/
	return (s);
}

/*
 * Run a command. The "cmd" is a pointer
 * to a command string, or NULL if you want to run
 * a copy of DCL in the subjob (this is how the standard
 * routine LIB$SPAWN works. You have to do wierd stuff
 * with the terminal on the way in and the way out,
 * because DCL does not want the channel to be
 * in raw mode.
 */
sys(cmd)
register char	*cmd;
{
	struct	dsc$descriptor	cdsc;
	struct	dsc$descriptor	*cdscp;
	long	status;
	long	substatus;
	long	iosb[2];

	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	cdscp = NULL;				/* Assume DCL.		*/
	if (cmd != NULL) {			/* Build descriptor.	*/
		cdsc.dsc$a_pointer = cmd;
		cdsc.dsc$w_length  = strlen(cmd);
		cdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
		cdsc.dsc$b_class   = DSC$K_CLASS_S;
		cdscp = &cdsc;
	}
	status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
	if (status != SS$_NORMAL)
		substatus = status;
	ckttysize();			/* check for new terminal size */
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	if ((substatus&STS$M_SUCCESS) == 0)	/* Command failed.	*/
		return (FALSE);
	return (TRUE);
}

/*
 * Front end for combined attach-to-parent and spawn-cli action
 */

attachtoparent(f, n, k)
{
	register int	s;
	s = attparent();
	if (s == ABORT)
		return (ABORT);
	else if (s == FALSE)
		return spawncli(f, n, k);	/* better than nothing */
	else
		return (TRUE);
}

/*
 * Attach to parent.  If the logical name MG$ATTACHTO
 * is present, attempt to attach to it.  If not, attempt to
 * attach to parent process.
 *
 * On return, see if the logical name MG$FILE contains
 * anything, and try to visit that file.
 */

static $DESCRIPTOR(nmdsc,"MG$ATTACHTO");

attparent()
{
	long		pid, jpi_code;	
	char		equiv[18], msgbuf[60];
	struct	dsc$descriptor_s eqdsc;
	short		eqlen;
	int		status, pos;
	register BUFFER	*bp;
	BUFFER		*findbuffer();
	int		s;


	/* Set up string descriptor */
	eqdsc.dsc$a_pointer = equiv;
	eqdsc.dsc$w_length  = sizeof(equiv);
	eqdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	eqdsc.dsc$b_class   = DSC$K_CLASS_S;

	/* Try to translate MG$ATTACH */
	status = lib$sys_trnlog(&nmdsc, &eqdsc.dsc$w_length, &eqdsc);
	if (status!=SS$_NORMAL && status!=SS$_NOTRAN) {
		ewprintf("Error translating %s",nmdsc.dsc$a_pointer);/* DEBUG */
		return (FALSE);
	}

	if (status == SS$_NORMAL) {
		/* Found a translation -- attempt to attach to it */
		jpi_code = JPI$_PID;

		status = lib$getjpi(&jpi_code,0,&eqdsc,&pid,0);
		equiv[eqdsc.dsc$w_length] = '\0';
		if (status != SS$_NORMAL) {
			ewprintf("Error getting JPI for \"%s\"",equiv);
			return (FALSE);
		}

#ifndef	NOSAVEONZ
		/* Attempt to attach to named process.  Save all buffers,  */
		/* set sgarbf because attach() always trashes the display  */
		if (savebuffers() == ABORT)
			return (ABORT);
#endif
		/* indicate process we're attaching to */
		strcpy(msgbuf,"Attaching to process \"");
		for (pos = strlen(equiv) - 1; pos >= 0; --pos)
			if (equiv[pos] != ' ') {
				equiv[pos+1] = '\0';
				break;
			}
		strcat(msgbuf,equiv);
		strcat(msgbuf,"\"");

		sgarbf = TRUE;
		if (attach(pid,msgbuf) == FALSE)	/* whups -- try spawn */
			return (FALSE);
	}
	else {	/* No translation -- attempt to find parent process */
		jpi_code = JPI$_OWNER;
		status = lib$getjpi(&jpi_code,0,0,&pid,0,0);

		if ((status != SS$_NORMAL) || (pid == 0))	/* not found! */
			return (FALSE);

#ifndef	NOSAVEONZ
		if (savebuffers() == ABORT)
			return (ABORT);
#endif
		sgarbf = TRUE;
		if (attach(pid,"Attaching to parent process") == FALSE)
			return (FALSE);
	}

	newfile();	/* attempt to find a new file, but don't care	*/
			/* if we don't find one...			*/
	refresh(FALSE, 0, KRANDOM);
	return (TRUE);
}

/*
 * If we find after re-attaching that there is
 * a new file to be edited, attempt to read it in,
 * using essentially the same code as findfile().
 */

static newfile()
{
	register BUFFER	*bp;
	register int	s;
	char		filename[NFILEN];
	BUFFER		*findbuffer();

	if ((s = cknewfile(filename, sizeof filename)) != TRUE)
		return (s);
	if ((bp = findbuffer(filename, &s)) == NULL)
		return (s);
	curbp = bp;
	if (showbuffer(bp, curwp, WFHARD) != TRUE)
		return (FALSE);
	if (bp->b_fname[0] == 0)
		return (readin(filename));	/* Read it in. */
	return (TRUE);
}

/*
 * Attach to a process by process number.  Restore the
 * terminal channel to the way it was when we started.
 * Also put out an optional message to the user.
 */ 

static attach(pid, msg)
long pid;
char *msg;
{
	long	status, attstatus;
	long	iosb[2];

	ttcolor(CTEXT);				/* Normal color.	*/
	ttnowindow();				/* Full screen scroll.	*/
	ttmove(nrow-1, 0);			/* Last line.		*/
	if (msg) {				/* Display a message	*/
		eputs(msg);
		ttputc('\r');
		ttputc('\n');
	}
	ttflush();

	/* Set terminal to old modes */
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);

	/* Attach to the process */
	attstatus = LIB$ATTACH(&pid);

	/* Return terminal to the modes MG needs */
	ckttysize();			/* check for new terminal size first */
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);

	return (attstatus == SS$_NORMAL ? TRUE : FALSE);
}


/*
 * Attempt to translate MG$FILE into fname.
 * If it's there and non-empty, return TRUE.
 */

static $DESCRIPTOR(filedsc,"MG$FILE");

static cknewfile(fname,fnsiz)
char *fname;
int fnsiz;
{
	char 	equiv[NFILEN];
	struct dsc$descriptor_s eqdsc;
	short	len;
	register int	status;

	eqdsc.dsc$a_pointer = equiv;
	eqdsc.dsc$w_length  = sizeof(equiv);
	eqdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	eqdsc.dsc$b_class   = DSC$K_CLASS_S;

	status = lib$sys_trnlog(&filedsc, &len, &eqdsc);
	if (status!=SS$_NORMAL && status!=SS$_NOTRAN) {
		ewprintf("Error translating MG$FILE");
		return (FALSE);
	}

	if (status == SS$_NOTRAN)		/* No new file found	*/
		return (FALSE);

	if (equiv[0] == ' ')
		return (FALSE);

	equiv[len] = '\0';
	strcpy(fname, equiv);
	return (TRUE);
}
