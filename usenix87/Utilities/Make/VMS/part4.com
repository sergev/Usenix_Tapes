$
$ ! This is VMS make part 4.
$ 
$	babble :== write sys$output
$	babble "Creating STAB.C"
$	create stab.c
/*
 * Handles the symbol table.  Due to be rewritten.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# include <stdio.h>

/*
 * A symbol table entry.
 */
struct stab
{
	char * st_sym;		/* The symbol to be defined */
	char * st_val;		/* The value of that symbol */
	struct stab *st_next;	/* Next entry in the list   */
};
/*
 * The symbol table.
 */
static struct stab *s_table[128] = { 0 };

extern char *malloc ();

st_defsym (sym, val)
char *sym, *val;
/*
 * Define the given symbol as "val".
 */
{
	int index;
	struct stab *stp;
/*
 * Try to find the symbol in the table.
 */
	index = st_hash (sym);
	for (stp = s_table[index]; stp; stp = stp->st_next)
		if (! strcmp (stp->st_sym, sym))
			break;
/*
 * If we found it, free up the old definition.
 */
	if (stp)
		free (stp->st_val);
/*
 * Else add a new entry at this index.
 */
	else
	{
		stp = (struct stab *) malloc (sizeof (struct stab));
		stp->st_sym = malloc (strlen (sym) + 1);
		strcpy (stp->st_sym, sym);
		stp->st_next = s_table[index];
		s_table[index] = stp;
	}
/*
 * In either case, allocate and store space for the definiton.
 */
	stp->st_val = malloc (strlen (val) + 1);
	strcpy (stp->st_val, val);
}




char *st_getsym (sym)
char *sym;
/*
 * Return the value of the given symbol.  If no definition exists, NULL
 * is returned.
 */
{
	int index;
	struct stab *stp;
/*
 * Try to find the symbol in the table.
 */
	index = st_hash (sym);
	for (stp = s_table[index]; stp; stp = stp->st_next)
		if (! strcmp (stp->st_sym, sym))
			return (stp->st_val);
/*
 * Nope.
 */
	return (NULL);
}




st_dump ()
/*
 * Print out the symbol table.
 */
{
	int s;
	struct stab *stp;

	printf ("\nSlot\tSymbol\t\t   Value");
	for (s = 0; s < 128; s++)
		for (stp = s_table[s]; stp; stp = stp->st_next)
			printf ("\n%3d\t%-20s  '%s'",
				s, stp->st_sym, stp->st_val);
}




char *st_symcat (symbol, string)
char *symbol, *string;
/*
 * Concatenate the value of "string" onto the given symbol's value.  If
 * "symbol" is not defined, this routine will define it.  A pointer to the
 * new value is returned.
 */
{
	int index;
	struct stab *stp;
	char *sp;
/*
 * Try to find the symbol in the table.
 */
	index = st_hash (symbol);
	for (stp = s_table[index]; stp; stp = stp->st_next)
		if (! strcmp (stp->st_sym, symbol))
			break;
/*
 * If the symbol is defined, allocate a new string, copy in the info,
 * then release the old string.
 */
	if (stp)
	{
		sp = malloc (strlen (stp->st_val) + strlen (string) + 1);
		strcpy (sp, stp->st_val);
		strcat (sp, string);
		free (stp->st_val);
		stp->st_val = sp;
		return (sp);
	}
/*
 * Otherwise define the symbol.
 */
	else
	{
		stp = (struct stab *) malloc (sizeof (struct stab));
		stp->st_sym = malloc (strlen (symbol) + 1);
		strcpy (stp->st_sym, symbol);
		stp->st_val = malloc (strlen (string) + 1);
		strcpy (stp->st_val, string);
		stp->st_next = s_table[index];
		s_table[index] = stp;
		return (stp->st_val);
	}
}



int
st_hash (sym)
char *sym;
/*
 * Return the hash entry for the given symbol.
 *
 * The current strategy is:
 *	if (length (string) >= 6)
 *		return 6th char + 1st char % 127
 *	else
 *		return last char + 1st char % 127
 * 
 * The idea is to cause all of the MAKE$ symbols to end up in different
 * slots.
 */
{
	int sl = strlen (sym);

	if (sl >= 6)
		return ((sym[0] + sym[5]) % 127);
	else
		return ((sym[0] + sym[sl - 1]) % 127);
}



st_undef (sym)
char *sym;
/*
 * Undefine a symbol.
 */
{
	int index = st_hash (sym);
	struct stab *stp, *last;
/*
 * Try to find the symbol.
 */
	for (stp = s_table[index]; stp; stp = stp->st_next)
	{
		if (! strcmp (stp->st_sym, sym))
		{
		/*
		 * Found it.  Free up the storage and
		 * unlink the structure.
		 */
			free (stp->st_sym);
			free (stp->st_val);
			if (stp == s_table[index])
				s_table[index] = stp->st_next;
			else
				last->st_next = stp->st_next;
			free (stp);
			return;
		}
		last = stp;
	}
}
$
$	babble "Creating SUBPROC.C"
$	create subproc.c
/*
 * Subprocess routines.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# include <ssdef.h>
# include <iodef.h>
# include <stdio.h>
# include "make.h"
# include "debug.h"

static char opened = FALSE, closed = FALSE;
static short p_chan = 0;
static int p_pid;


p_open ()
/*
 * Create the subprocess.
 */
{
	int status;
	short l_p_name;
	char proc_name[80], name[80], p_name[80];
	int p_ribbit ();

# ifdef pr_name
/*
 * Get the process name.
 */
	who_am_i (descr_n (proc_name, 80), &l_p_name);
	proc_name[l_p_name] = NULL;
	str$upcase (descr (proc_name), descr (proc_name));
# else
	sprintf (proc_name, "%X", getpid ());
# endif
/*
 * Create the mailbox.
 */
	strcpy (name, "MKMB_");
	strcat (name, proc_name);
	status = sys$crembx (0,			/* Temporary mailbox 	*/
			     &p_chan,		/* Channel to use	*/
			     ___,		/* Default max message	*/
			     ___,		/* Default quota	*/
			     ___,		/* Default protection	*/
			     ___,		/* Default access mode	*/
			     descr (name));	/* Logical name to use	*/
	if (error (status))
	{
		printf ("Unable to create mailbox., status: %d", status);
		exit (status);
	}
/*
 * Now create the subprocess.
 */
	strcpy (p_name, "MK_");
	strcat (p_name, proc_name);
	p_name[15] = 0;	/* $creprc complains otherwise for long names */
	strcat (name, ":");
	sys$setast (0);
	status = lib$spawn (
		___,			/* No command			*/
		descr (name),		/* Read from the mailbox	*/
		descr ("SYS$ERROR"),	/* Write to sys$error		*/
		&1,			/* NOWAIT flag set		*/
		descr (p_name),		/* Process name			*/
		&p_pid,			/* Get process ID for later	*/
		___, ___,		/* No completion status, EF	*/
		p_ribbit, ___);		/* Call p_ribbit on croak	*/
	opened = TRUE;
	sys$setast (1);
/*
 * Check on subprocess completion.  If the process already exists,
 * put out a warning, and assume we can use it.
 */
	if (status == SS$_DUPLNAM)
	  printf ("\nWARNING: Subprocess already exists!  Will try to use it.");
/*
 * Anything else other than SS$_NORMAL is fatal.
 */
	else if (error (status))
	{
		printf ("\nUnable to create subprocess! status: %d", status);
		exit (status);
	}
/*
 * Make the subprocess die on warnings.
 */
	p_write ("$ On warning then stop/id=0");
}



p_zap ()
/* 
 * Kill the subprocess, immediately, with no regard to
 * any operation in progress.
 */
{
	int status;

	if (opened)
	{
		opened = FALSE;
		closed = TRUE;
		if (p_pid)
			status = sys$delprc (&p_pid, ___);
		else
		{
			printf ("P_pid is zero!");
			status = 1;
		}
		if (error (status))
		{
			printf ("Unable to kill subprocess.");
			exit (status);
		}
	}
}



p_ribbit ()
/*
 * Called on an AST when the subprocess dies.
 */
{
	if (opened && ! closed)
	{
		printf ("\nSubprocess died.");
		opened = FALSE;
		closed = TRUE;
		m_exit ();
	}
}




p_write (string)
char *string;
/*
 * Write a line to the subprocess.
 */
{
	int status;

	if (debug (D_P_WRITE))
		printf ("\nP_write: %s", string);
/*
 * Make sure the subprocess exists.
 */
	if (! opened)
		p_open ();
/* 
 * Do the write.
 */
	status = sys$qiow (___,			/* Default event flag	*/
			   p_chan,		/* Our mbx channel	*/
			   IO$_WRITEVBLK,	/* Write...		*/
			   ___, ___, ___,	/* No IOSB, AST.	*/
			   string,		/* Our line ...		*/
			   strlen (string),	/* ... and its length	*/
			   ___, ___, ___, ___);	/* P3 - P6 null		*/
	if (error (status))
	{
		p_zap ();
		printf ("Unable to write to subprocess.");
		exit (status);
	}
}




p_close ()
/*
 * Gracefully close the subprocess.
 */
{
	if (! opened) 
		return;
/*
 * Wait for the current operation to complete.
 */
	p_write ("$ ");
	p_write ("$ ");
	closed = TRUE;
/*
 * Now send the suicide command.
 */
	p_write ("$ stop /identification=0");
	opened = FALSE;
}
$
$	babble "Creating TIMER.C"
$	create timer.c
int timing = 0;

tim_init ()
/*
 * Initialize the timer.
 */
{
	lib$init_timer ();
	timing = 1;		/* Let handler know. */
}


tim_show ()
/*
 * Display the timer setting.
 */
{
	if (timing)
		lib$show_timer ();
	timing = 0;
}
$
$	babble "Creating WHOAREWE.C"
$	create whoarewe.c
who_are_we ()
/*
 * Find out who is running the program.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char u_name[80];
	int len;

	my_name (descr_n (u_name, 80), &len);
	u_name[len] = 0;
	st_defsym ("MAKE$USERNAME", u_name);
}
$
$ 	babble "Creating CPANIC.C"
$	create cpanic.c
/*
 * c_panic:
 *
 * A panic call for C programs that does a sprintf first...
 */
c_panic (fmt, args)
char *fmt;
int args;
{
	char line[133];
	extern ftl_panic ();


	sprintrmt (line, fmt, &args);
	lib$signal (ftl_panic, 1, descr (line));
}
$
$	babble "Creating DESCR.C"
$	create descr.c
#define COM_DESCR
# include "descr.h"

/*
 * This routine fills character descriptors (stored internally) and
 * returns the address of the descriptor as its value.  An array is 
 * used in a circular manner, in the hopes that no conflicts will take
 * place...
 */
# define N_DESCR 128
static struct st_descr d_array[N_DESCR];
static int cur_descr = 0;

struct st_descr *descr (string)
char *string;
{
	struct st_descr *ret;

	d_array[cur_descr].st_length = strlen (string);
	d_array[cur_descr].st_address = string;
	d_array[cur_descr].st_type = 0x10D;	/* Class 1, type 14 */
	ret = &d_array[cur_descr];
	if (++cur_descr >= N_DESCR)
		cur_descr = 0;
	return (ret);
}


struct st_descr *descr_n (string, len)
char *string;
int len;
{
	struct st_descr *ret;

	d_array[cur_descr].st_length = len;
	d_array[cur_descr].st_address = string;
	d_array[cur_descr].st_type = 0x10D;	/* Class 1, type 14 */
	ret = &d_array[cur_descr];
	if (++cur_descr >= N_DESCR)
		cur_descr = 0;
	return (ret);
}
$
$	babble	"Creating DESCR.H"
$	create descr.h
/*
 * Descr.h
 *
 * Definition of string descriptors.
 */
struct st_descr
{
	short st_length;	/* Length of the string */
	short st_type;
	char * st_address;	/* Address of the string */
};
#ifndef COM_DESCR
struct st_descr *descr (), *descr_n ();
#endif
$
$	babble "Creating ERRMES.MAR"
$	create errmes.mar
      .title   errmes
;
;     This routine prints the message associated with a given
;     error code.
;     It is an equivalent of the previous FORTRAN routine
;     of the same name. The advantage of this implimentation is
;     that it is re-entrant and hence callable from an AST.
;
      .list meb
      .ENTRY   ERRMES,^M<R2>
      clrl     -(sp)
;     pushl    @4(ap)
;     pushl    #1
      movzbl   (ap),r1     ; r1 has counter of # of args
loop: movl     (ap)[r1],r2 ; r2 has addr of an arg (in reverse order)
      pushl    (r2)        ; store arg on stack in reverse order
      sobgtr   r1,loop  ; decrement ptr and loop till no more left
      pushl    (ap)     ; save arg count.
      moval    (sp),r2
      $putmsg_s   msgvec=(r2)
      blbc     r0,baderr
      ret
baderr:
      $exit_s  code=#ss$_abort
      .end
$
$	babble "Creating FTLERRMSG.MSG"
$	create ftlerrmsg.msg
.facility ftl,52
.severity fatal
FTLERR <Program generated fatal error.>
FTLERR2 <FATAL ERROR: !AS>/fao=1
PANIC <!AS>/fao=1
.end
$
$	babble "Creating LENS.FOR"
$	create lens.for
	integer function lens(line,max)
c+	integer function lens(line,max)
c
c entry:
c	line is a character string
c	max is the length of the line
c 
c exit:
c	the value of the function is the position of the last non-blank
c	character
c-
	character*(*) line
	character*1 tab
	integer max
	parameter (tab=char(9))

	j = max
10	if ((line(j:j) .ne. ' ') .and. (line(j:j) .ne. tab)) goto 100
	j = j - 1
	if (j .gt. 1) goto 10
	
100	lens = j
	return
	end
$
$	babble "Creating MYNAME.FOR"
$	create myname.for
	subroutine my_name (name, length)
c
c Find the name of the person running the program.
c
	implicit none
	include '($ssdef)'
	include '($jpidef)'
	character *(*) name
	integer*4 item(4), status, flag, lib$len, length
	integer *2 iosb(4), l
	integer lib$get_ef, sys$waitfr, len, sys$getjpi, ipd, ulen, llen
	integer lens

c
c Get an event flag.
c
	call iferr (lib$get_ef(flag))
c
c Set up the item list.
c
      item(1) = ishft (jpi$_username, 16) .or. len (name)
      item(2) = %loc(name)
      item(3) = %loc(l)
      item(4) = 0
c
c Now call sys$getjpi.
c
	ipd = 0
99      status = sys$getjpi (%val(flag), ipd, , item, iosb, ,)
	call iferr (status)
c
c Wait for the service to complete.
c
	status = sys$waitfr (%val(flag))
	call iferr (status)
	call lib$free_ef (flag)
	status = iosb(1)
	if (status .gt. 1) then
		call errmes (status)
		name = 'BIZARRE'
		length = 7
	end if
c
c Find the real length, since the system service always returns 12.
c
	length = l
	length = lens (name, length)
c
c Done.
c
	end
$
$	babble "Creating QUADMATH.MAR"
$	create quadmath.mar
      .TITLE QUADMATH
;
;        SUBROUTINE SUBQUAD.MAR
;
;        FORTRAN CALLABLE ROUTINE TO SUBTRACT TWO QUAD WORD INTEGERS
;
;        CALL SUBQUAD (A,B,C)
;
;        RETURNS: A - B -> C
;
         .ENTRY          SUBQUAD ^M<R2>
;           note that you cannot enable integer overflow or this will fail.
;
;
A = 4
B = 8
C = 12
D = 16
         MOVQ    @A(AP),R0       ;GET FIRST PARAM FOR SUBTRACT
                                 ;NEED TO USE REGISTERS BECAUSE
                                 ;SUBWC IS ONLY 2 ADDRESS INSTR.
         MOVAQ   @B(AP),R2       ;GET ADDRESS OF SECOND PARAM.
         SUBL    (R2)+,R0        ;SUBTRACT FIRST HALF OF ARGUMENTS
         SBWC    (R2),R1                 ;THEN DO THE SECOND HALF
         MOVQ    R0,@C(AP)       ;STORE THE RESULT
         RET
;
;
;        SUBROUTINE ADDQUAD.MAR
;
;        FORTRAN CALLABLE ROUTINE TO ADD TWO QUAD WORD INTEGERS
;
;        CALL ADDQUAD (A,B,C)
;
;        RETURNS: A + B -> C
;
         .ENTRY          ADDQUAD ^M<R2>
;           note that you cannot enable integer overflow or this will fail.
;
;
A = 4
B = 8
C = 12
         MOVQ    @A(AP),R0       ;GET FIRST PARAM FOR SUBTRACT
                                 ;NEED TO USE REGISTERS BECAUSE
                                 ;SUBWC IS ONLY 2 ADDRESS INSTR.
         MOVAQ   @B(AP),R2       ;GET ADDRESS OF SECOND PARAM.
         ADDL    (R2)+,R0        ;SUBTRACT FIRST HALF OF ARGUMENTS
         ADWC    (R2),R1                 ;THEN DO THE SECOND HALF
         MOVQ    R0,@C(AP)       ;STORE THE RESULT
         RET
;
;
;
         .ENTRY          EDIV    ^M<R2,IV>
;
;        CALL EDIV (A,B,C)
;        RETURNS  A/B -> C
;
         MOVQ    @A(AP),R0       ;GET FIRST ARGUMENT
;        MOVAL   @B(AP),R2       ;GET LONGWORD DIVISOR ADDRESS
;        EDIV    (R2),R0,R0,R1   ;DO THE DIVISION
         MOVL    @B(AP),R2
         EDIV    R2,R0,R0,R1
         MOVL    R0,@C(AP)       ;STORE INTEGER QUOTIENT,IGNORE REM.
         RET
         .ENTRY          EMUL    ^M<R2,IV>
;
;        CALL EMUL (A,B,C,D)
;        RETURNS  D = A*B + C
;
         MOVAQ    @D(AP),R0      ;GET RESULT ADDRESS
         EMUL  @A(AP),@B(AP),@C(AP),(R0)  ;DO MULT AND STORE RESULT
         RET
         .ENTRY          QMUL    ^M<R2,R3,R4,R5,IV>
;
;        CALL QMUL (A,B,C)
;        RETURNS C = A * B
;        where A, B, & C are quadwords.
;
;        this code is copied from the VAX11 Architecture Handbook
;
         MOVAQ @A(AP),R2
         MOVAQ @B(AP),R3
         EMUL  (R2),(R3),#0,R4
         MULL3 4(R2),(R3),R0
         MULL3 (R2),4(R3),R1
         ADDL  R1,R0
         TSTL  (R2)
         BGEQ  10$
         ADDL  (R3),R0
10$:     TSTL  (R3)
         BGEQ  20$
         ADDL  (R2),R0
20$:     ADDL  R0,R5
         MOVQ  R4,@C(AP)
         RET
         .END
$
$	babble "Creating SCTLC.FOR"
$	create sctlc.for
	integer function set_ctl_c(funct)

c+	subroutine set_ctl_c(funct)
c
c this routine enables a control_C interrupt.  Funct is the name of a
c routine to be called on the receipt of a control_C.  Note that after
c a control_C is caught, set_ctl_c must be called again to reenable the
c ast.  A return value of 1 indicates that the call was successful
c-

	character *10 in
	logical first
	data first /.true./
	integer setctlc

	if (first) then
c
c assign a channel to the terminal
c
		call sys$assign ('TT', ichani,,)
		first = .false.
	endif
c
c enable the AST
c
	set_ctl_c = setctlc(ichani,funct)
	return
	end
$
$	babble "Creating SETCTLC.MAR"
$	create setctlc.mar
	.title set_ctlc
;
; setctlc -- catch control_C interrupts.
; call in fortran as:
;
;	call setctlc(channel,routine)
;
; where:
;	channel is a channel to the terminal generating control_C's
;	routine is the name of the routine to be called on a control_C.
;	this routine must be declared external in the calling program.
;
	.entry setctlc,^M<R2>
	$qiow_s 	CHAN=@4(ap),func=#io$_setmode!io$m_ctrlcast,-
			P1=@8(ap),P3=#3
	ret


	.entry setctly,^M<R2>
	$qiow_s 	CHAN=@4(ap),func=#io$_setmode!io$m_ctrlyast,-
			P1=@8(ap),P3=#3
	ret
	.end
$
$	babble "Creating SETEXH.FOR"
$	create setexh.for
	subroutine set_exh (handler)
c
c Set an exit handler for the image.
c
	integer c_blk(5), sys$dclexh, status
	external handler
c
c Fill in the control block.  Note that the exit reason is essentially
c being thrown away.
c
	c_blk(1) = 0
	c_blk(2) = %loc(handler)
	c_blk(3) = 0
	c_blk(4) = %loc(reason)
	c_blk(5) = 0
c
c Now declare the exit handler.
c
	status = sys$dclexh (c_blk)
	call iferr (status)
	return

	end
$
$	babble "Creating SPRINTRMT.C"
$	create sprintrmt.c
/* sprintrmt(buf,arg) does what sprintf(buf,"%r",arg) used to */
sprintrmt(buf, fmt, arg)
char   *buf, *fmt;
register char **arg; {		/* this silly routine should really be
				   calling _doprnt */
    register char  *t1,
                   *t2, *t3;
    int sprintf ();
/*
 * DON'T ASK, YOU DON'T WANT TO KNOW...
 */
    t1 = *--arg;
    *arg = fmt;
    t2 = *--arg;
    *arg = buf;
    t3 = *--arg;
    *arg = (char *) 255;
    lib$callg (arg, sprintf);
    *arg++ = t3;
    *arg++ = t2;
    *arg++ = t1;
}
$
$	babble "Creating MAKEMAKE.COM"
$	create makemake.com
X$ ! This is MAKEMAKE.COM, which will put together the initial version of
X$ ! MAKE for you.  Be sure to remove the leading X's before invoking!
X$ !
X$ ! Compile everything.
X$ !
X$ cc		CCONS.C
X$ cc		CHARUTL.C
X$ cc		CHDIR.C
X$ cc		CHECKDEF.C
X$ cc		COMLINE.C
X$ cc		CONS.C
X$ cc		CPANIC.C
X$ cc		DCL.C
X$ cc		DEFCLP.C
X$ cc		DESCR.C
X$ cc		DIRTY.C
X$ cc		DOCOND.C
X$ macro	ERRMES.MAR
X$ cc		EXECUTE.C
X$ cc		FDATE.C
X$ cc		FILE.C
X$ cc		FILETYPE.C
X$ cc		FINDCONS.C
X$ message	FTLERRMSG.MSG
X$ cc		GETTARGET.C
X$ fortran	LENS.FOR
X$ cc		LINETYPE.C
X$ cc		LOG.C
X$ cc		MAKE.C
X$ cc		MEXIT.C
X$ fortran	MYNAME.FOR
X$ cc		PROCESSL.C
X$ macro	QUADMATH.MAR
X$ cc		REFSYM.C
X$ fortran	SCTLC.FOR
X$ cc		SEARCH.C
X$ macro	SETCTLC.MAR
X$ cc		SETDEBUG.C
X$ fortran	SETEXH.FOR
X$ cc		SHARP.C
X$ cc		SHEXEC.C
X$ cc		SHMAKE.C
X$ cc		SHSKIP.C
X$ cc		SKIPTOELS.C
X$ cc		SPRINTRMT.C
X$ cc		STAB.C
X$ cc		SUBPROC.C
X$ cc		TIMER.C
X$ cc		WHOAREWE.C
X$ !
X$ ! Now create the library and stuff everything into it.
X$ !
X$ library /create/object makelib *.obj
X$ delete *.obj;
X$ !
X$ ! Now link the goddam thing.
X$ !
X$	link /exe=make makelib/lib/incl=make, sys$library:crtlib/lib
$
$	babble "Creating LIBTARGET.C"
$	create libtarget.c
# include <stdio.h>

int
lib_target (target, source, module, type)
char *target, *source, *module, *type;
/*
 * Parse a library reference.  Target is a symbol containing the full
 * specification, source is a symbol to define as the source file
 * name, and module is the symbol to define as the module name.
 * "target" is redefined to have only the library name.  Target
 * is also forced to have a type, using "type" if necessary.
 *
 * Return status is TRUE iff the target was successfully parsed.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *strchr (), *prp, *vbp, *tp, *file_type (), *st_getsym ();
	char *temp, *mknstr (), *cp;

/*
 * Separate out the library name.
 */
	if ((tp = st_getsym (target)) == NULL)
		c_panic ("No file for %s", target);
	if ((prp = strchr (tp, '(')) == NULL)
		c_panic ("Bad lib_target call: %s", tp);
	temp = mknstr (tp, prp - tp);
/*
 * Make a copy of the whole target, the redefine "target"
 */
	st_defsym ("MAKE$LIB_TEMP", tp);
	st_defsym (target, temp);
	free (temp);
/*
 * Make sure the library name has a type.
 */
	if (! file_type (st_getsym (target)))
		st_symcat (target, type);
/*
 * Separate out the module name.
 */
	tp = st_getsym ("MAKE$LIB_TEMP");
	prp = strchr (tp, '(');
	if ((vbp = strchr (prp, '|')) == NULL)
		if ((vbp = strchr (prp, ')')) == NULL)
		{
			printf ("\nBad library format: %s", tp);
			return (FALSE);
		}
	temp = mknstr (prp + 1, vbp - prp - 1);
	st_defsym (module, temp);
/*
 * Now we have to come up with a source name.  If we found a |,
 * it is in the library specification.  Otherwise we have to derive
 * it from the module name.
 */
	if (*vbp == '|')
	{
		if ((prp = strchr (vbp, ')')) == NULL)
		{
			printf ("\nBad library format: %s", tp);
			return (FALSE);
		}
		free (temp);
		temp = mknstr (vbp + 1, prp - vbp - 1);
	}
	else
	{
	/*
	 * For VMS V3 systems, fix up the file name.
	 */
		if (! st_getsym ("MAKE$VMS_V4"))
		{
			for (cp = temp; *cp; )
				if (*cp == '_')
					strcpy (cp, cp + 1);
				else
					cp++;
			if (strlen (temp) > 9)
				temp[9] = NULL;
		}
	}
/*
 * Now define the source name.  We don't worry about the type here.
 */
	st_defsym (source, temp);
	free (temp);
	return (TRUE);
}
$
$	babble "Creating LBRDEF.H"
$	create lbrdef.h
/*
 * Lbrdef for C.
 */
# define LBR$C_TYP_UNK 	0
# define LBR$C_TYP_OBJ	1
# define LBR$C_TYP_MLB	2
# define LBR$C_TYP_HLP	3
# define LBR$C_TYP_TXT	4
# define LBR$C_TYP_SHSTB	5
# define LBR$C_TYP_DECMX	5
# define LBR$C_TYP_RDEC	127
# define LBR$C_TYP_USRLW	128
# define LBR$C_TYP_USRHI	255

# define LBR$C_CREATE	0
# define LBR$C_READ	1
# define LBR$C_UPDATE	2

# define LBR$_KEYNOTFND	0x269062	/* Obtained thru testlib...	*/
$
$	babble "Creating MAKE.DOC	<--- Documentation! "
$	create make.doc
		MAKE - A program maintainer.

	MAKE is a program that keeps track of other programs, insuring
that everything is up to date.  It is most useful for large programs that
are kept in many source files, each of which may call for the inclusion
of other files.  MAKE reads a file describing all of the file
dependancies, and makes sure that all dependant files are more recent
than those depended upon, executing DCL commands to bring that state
about if necessary.
	To use MAKE, you need to have the following line in your login
file:
		X$ make :== $ds:[corbet.cmd.make]make

[NOTE TO FOLKS WITH THE NET DISTRIBUTION: change the directory above to the
 one that contains your make executable!]

			MAKEFILES
	MAKE takes its input from a file termed a "makefile."  The
makefile describes file dependancies, contains DCL commands to be
executed directly, and contains commands that MAKE interprets itself.
The default name for the makefile is "1MAKEFILE.", so named so it will
sort out at the top of a directory listing, but makefiles with other
names may be used if desired.
	There are three different types of lines that may appear in the
makefile.  Direct command, or "DCL" lines are DCL commands to be
executed directly.  DCL lines always start with "$".  Directives to MAKE
start with "#", and are sometimes called "sharp" lines for that reason.
Finally, "conditional" or "dependancy" lines start with the name of a
target.  All of these line types are discussed in greater detail below.
	Comments are marked with "!".  Anything past an exclamation mark
is ignored, up to the end of the line.  If a line needs to be continued
on another line, the first line should be ended with a backslash ("\").
There is no limit on the number of continuation lines.

			Command execution
	In the performance of its duties, MAKE often has to pass
commands to the command interpreter (DCL).  To do this, MAKE spawns
a subprocess when it is initially invoked.  Commands are passed to this
process for execution.  Note that command execution is asynchronous!  A
command to be executed is handed to the subprocess, and MAKE goes on to
its next task.  This allows some overlap of computations, but requires
some care on the part of the user.  Places where synchronization is
important are pointed out below.  Only one command is in execution at
a time; if MAKE needs to execute a command, the program will block until
execution of the previous command is finished.

			Symbol substitution
	MAKE maintains a symbol table, and substitutes symbols into
all lines read from the makefile.  Symbols are manipulated with the
"#define", "#undef", and "#ifdef" directives described below.  A symbol
reference has the form "^symbol".  When MAKE encounters such a string,
the symbol table is searched for "symbol", and the corresponding value
is substituted for the reference.  If the symbol is not defined, a
warning is printed to the terminal and the null string is substituted.
There is no restriction on the length of a symbol or its value.
	Note that a string like "^symbol" has the same effect as
"'symbol'" in DCL, or "$symbol" in the UNIX shells.  An alternate
reference form is "^{symbol}", which is useful when the symbol value is
to be run up next to an alphanumeric string, where it would otherwise be
impossible to parse out the symbol name.
	For your own safety, symbol names that start with "MAKE$" should
be avoided, as all symbols used by MAKE itself start that way.

			DCL lines
	Any line that starts with "$" is executed directly by the
subprocess.  Thus a line like

		X$ directory

would cause a listing of the current directory to appear on the screen.
	There are two variants on DCL lines.  If a hyphen ("-") appears
after the "$", the command itself is not echoed to the screen.  By
default, all DCL lines are echoed to the screen before execution.  If a
right angle bracket (">") appears after the "$" (or the "-") then the
leading "$" is not prepended to the line sent to the subprocess.

			MAKE directives
	Lines that start with "#" are interpreted directly by MAKE.  The
legal "#" lines are:

	# chdir directory
			Causes MAKE to change the default directory of
			both the parent and subprocesses to "directory."
			This directive causes a subprocess
			syncronization.
	
	# cons		Defines a construction.  Constructions are
			described in their own section, below.
	
	# dirty	file	Deletes the entry for the given file or library
			reference from the file or library cache.  The
			caches are described in their own section,
			below.  It is not an error if the cache entry
			does not exist.
	
	# dump 		Lists out information to the terminal.  The
			valid arguments to #dump are:

			constructions	Lists the construction table.
					See the section on
					constructions, below.
			file_cache	Lists the contents of the file
					cache.
			file_stats	Lists information on the file
					and library caches.
			lib_cache	Lists the contents of the
					library cache.
			symbols		Lists the contents of the symbol
					table.
	
	# defclp	Defines parameters on the command line as
			symbols with null value.  See the section on the
			command line, below.

	# define symbol value
			Defines the symbol "symbol" with the given
			value, which may be null.  If a definition of
			"symbol" already exists, it is silently
			discarded.

	# ifdef symbol
			If "symbol" has an entry in the symbol table,
			all lines up to the corresponding "#else" or
			"#endif" are executed.  Otherwise, those lines
			are skipped, and the lines after the "#else" are
			executed, if one exists.  All "#ifdef" lines
			must have a corresponding "#endif."  #Ifdef's
			may be nested.
	
	# exit		Causes execution of the current makefile to be
			terminated.  If the current makefile is the only
			makefile, MAKE exits.
	
	# include file
			MAKE opens the given file and interprets its
			contents.  When the end of the file is
			encountered, execution continues at the line
			after the "#include."

	# message line
			MAKE prints "line" to the terminal.
	
	# quit		MAKE synchronizes with the subprocess, then
			immediately exits.
	
	# undef	symbol  The definition of "symbol" is removed from the
			symbol table.  If "symbol" is not defined, the
			"#undef" line is a silent no-op.
	
	# wait		Causes a subprocess synchronization.

			Constructions.
	A construction is a series of commands used by MAKE to generate
one file from another.  For example, if MAKE determines that the file
PROGRAM.FOR is more recent than PROGRAM.OBJ, a construction from type
".FOR" to ".OBJ" will be searched out of the construction table, and
invoked.  The default construction in this case is a single command,
being:

		fortran ^for_opts ^MAKE$SOURCE

where the symbol "for_opts" is defined as the command line options for
the FORTRAN command, which by default are "/check /nolist".  The symbol
"MAKE$SOURCE" is defined as the name of the file to be compiled,
"PROGRAM" in this case.
	MAKE contains a substantial list of constructions by default.
However, you will probably find that you wish to add new constructions,
or change the existing ones.  The "#cons" directive is used for this
purpose.  The form of a construction definition is:

	# cons from to
		line 1 
		.
		.
		.
		line n
	# end

where "from" is the source type, such as ".FOR"; and "to" is the
destination type, like ".OBJ".  The lines consists of DCL commands to be
executed to generate the destination file, without the leading "$".  The
"-" and ">" may be present, just like in DCL commands.
	Most MAKE directives may also appear within the construction.
They are not interpreted until the construction is invoked.  The
directives that may not appear in a construction are: #cons, #include,
#ifdef, #else, and #endif.
	Symbol references inside a construction are not substituted
until the construction is invoked.
	Sometimes there is no source file from which to create the
destination.  A typical example of this situation is the creation of an
object library.  To define a construction of this type, specify the
source type as "X".  The default construction to create an object
library would be given as:

	# cons X .olb
		library /create ^MAKE$TARGET
		# wait
	# end

The necessity for the "# wait" is discussed below.
	Finally, sometimes the destination of a construction is an entry
in a library.  In this case, the destination type should be the type of
the library, enclosed in parentheses.  For example, the default
construction to generate an object library entry from a FORTRAN file is:

	# cons .for (.olb)
		fortran ^for_opts ^MAKE$SOURCE
		library ^MAKE$TARGET ^MAKE$SOURCE
		delete ^MAKE$SOURCE.OBJ
	# end

	One important thing to remember when defining library
constructions:  It is critical that any DCL commands that operate on the
library MUST terminate before the program resumes after invoking the
construction.  Since subprocess execution is asynchronous, the last
command in a construction is usually still executing when the program
goes on to the next line.  If this line operates on a library, MAKE may
collide with the subprocess in trying to access the library.  Thus,
the construction above to create a library must include a "# wait".
The DELETE command in the fortran construction is sufficient to insure
that the subprocess is done with the library before MAKE tries to access
it again.

			Dependancy lines
	Dependancy lines are the heart of MAKE.  These lines specify
which files must be up to date, and which files they depend on.  The
form of a dependency line is:

	flref:		flref flref ....

where "flref" is a file or library reference.  The simplest form of a
"flref" is just a file name.  Thus, the simplest dependency line is:

	program:

The response to the this line by MAKE would be: (1) a default type of
".OBJ" would be assumed, thus making the target "PROGRAM.OBJ".  MAKE's
task would thus be to insure that PROGRAM.OBJ is up to date.  (2) MAKE
would derive the appropriate source type.  MAKE does this by looking at
all files with name "program", searching for one that it can use to
generate PROGRAM.OBJ.  If the files that exist are "PROGRAM.OBJ",
"PROGRAM.FOR", and "PROGRAM.LIS"; MAKE would take PROGRAM.FOR as the
source, since a construction exists from .FOR to .OBJ.  Once a
construction is found, it is invoked if (1) PROGRAM.OBJ does not exist,
or (2) PROGRAM.FOR is more recent than PROGRAM.OBJ.
	Now suppose that PROGRAM.FOR contained a line of the form
"include 'incl.inc'".  If "incl.inc" has been modified since the last
time PROGRAM.FOR was compiled, then PROGRAM.OBJ is out of date.  To
include this dependancy, the line should read as:

	program:	incl.inc

The default type for include files -- that is, those that appear to the
right of the colon -- is ".H".
	MAKE also supports libraries.  If you have a subroutine SUBR
that exists in SUBR.FOR, but you keep the object code in LIB.OLB, the
appropriate dependency line is:

	lib(subr):

The full form of a library reference is 

	lib_name(module|source)

where "lib_name" is the name of the library, "module" is the name of the
module, and "source" is the name of the source file.  So, if
"UTILITY.FOR" contained a function named "FUN", the reference would be:

	lib(fun|utility)

If the source name is not present in the library reference, it is
derived from the module name by (1) deleting all underscores, and (2)
truncating the name to 9 characters.  (If the symbol MAKE$VMS_V4 is defined
this translation is not done).
	Library references may appear on either side of the colon.  The
default library type is ".OLB" for references to the left of the colon,
and ".TLB" for those on the right.
	The following symbols are defined before a construction is
invoked, and may be used by the construction.

	MAKE$TARGET	The name (no type) of the file to be generated.
			For library constructions, the target name is
			the name of the library.
	MAKE$TARGET_TYPE
			The type of the target file.
	MAKE$SOURCE
			The name (no type) of the source file.  For
			compatibility, the symbol "$" is also defined
			with the source file name.
	MAKE$SOURCE_TYPE
			The type of the source file.
	MAKE$MODULE	The name of the module being generated, in a
			library construction.
	MAKE$OUT_OF_DATE
			The name of the file that is more recent than
			the target file.  If more than one file is more
			recent, only the first one found by MAKE is
			defined as this symbol's value.  If the target
			does not exist, the target name is used.

			The command line
	The format of the MAKE command line is:

	X$ make [flags] [params]

Flags to the MAKE command are UNIX style, set off with "-".  The legal
flags are:
	-dsymbol=value
		Defines the given symbol as having the given value.  An
		alternate form is "-dsymbol", which defines the symbol
		as having a null value
	
	-f file
		Uses "file" as the makefile, instead of the default of
		"1MAKEFILE.".

The symbols "1" through "9" are defined as the remaining parameters on
the command line.  Thus for the following command

	X$ MAKE -f junk. xxx yyy zzz

MAKE would read the file "JUNK."; the symbol reference "^1" would
substitute as "xxx", "^2" as "yyy", "^3" as "zzz", and "^4 through "^9"
as the null string.
	A note about case:  DCL, on reading the command line, converts
the entire thing to upper case.  The C startup routine, not to be
outdone, responds by converting the whole thing back to lower case.
Thus, unless defeated with double quotes, all symbols and values will be
defined in lower case.
	Finally, the #defclp command is another way to define symbols on
the command line.  When MAKE encounters a #defclp (DEFine Command Line
Parameters) command, every parameter on the command line is defined as
the name of a symbol with null value.  See the example makefile at the
end of this document for a use of this command.  To deal with the case
switching that goes on, all command line parameters are defined in both
upper and lower case.

			File and library caches
	Checking the date on a file or library entry is a relatively
expensive operation.  In an attempt to minimize these operations, MAKE
caches both file and library module dates.  This attempt is not always
successful, and may cause trouble in a few cases.  The operation of each
cache is described below, to aid the user in obtaining the best results.
	The file cache is a simple pairing between file names and dates.
A hashing scheme is used, so a search of the cache is usually a
relatively quick operation.  However, if file cache hits are rare, the
cache does little if any good.  The file cache is most helpful for
programs where the same files are included into several different source
files.  In a typical makefile, include files are the only ones that are
checked more than once, so if they are relatively few the user is
probably better off disabling this cache.  To disable the cache, simply
define the symbol MAKE$DIS_FILE_CACHE.
	The library cache is more complicated, since it must keep track
of both libraries and modules.  Library names are hashed, but modules
are kept in a linear linked list.  Search times can thus be long for
makefiles with large numbers of library references.  If it is determined
that the library cache does not help, it may be disabled by defining the
symbol MAKE$DIS_LIB_CACHE.
	Statistics on cache hit rates may be obtained with the "#dump
file_stats" command.  The use of this command, along with MAKE$TIMER,
described below, can aid the user in finding out whether (s)he is better
off with or without the caches.

			Special symbols
	Certain symbols have a special meaning to the program, and may
be used to alter its behaviour.  They are:

	MAKE$DIS_FILE_CACHE
			When this symbol is defined, the file cache is
			disabled.
	MAKE$DIS_LIB_CACHE
			When this symbol is defined, the library cache
			is disabled.
	MAKE$SILENT	When MAKE$SILENT is defined, no DCL commands are
			echoed to the terminal, whether they are
			preceded by a hyphen or not.
	MAKE$TIMER	This symbol may be used for performance
			measurements.  When this symbol is defined, the
			run-time timer is initialized.  When either (1)
			it is undefined again, or (2) the program
			terminates, elapsed time, CPU time and I/O
			statistics are put out to the terminal screen.
			MAKE$TIMER may be used in evaluating the
			usefullness of the library and file caches.
	MAKE$VMS_V4	This symbol changes the way file names are derived
			from library module names.  If it is not defined, 
			module names are (1) stripped of underscore characters,
			and (2) truncated to nine characters.  Otherwise, there
			is no translation performed.

			Miscellaneous hints
	The subprocess created by MAKE thinks it's a batch job.  You may
that it is deluded, but it certainly is not interactive.  Anyway, the
result is that unless you specify otherwise, the compilers will produce
listing files, and the linker will produce a map.  Thus, the default
options for the compilers all include "/nolist".
	If you decide to abort a MAKE run, it is suggested that you do
so with ^C, instead of ^Y.  ^C causes immediate termination of the
subprocess, while ^Y does not cause such termination until image
rundown.
	When the system is loaded, spawning a subprocess can take a long
time.  If MAKE seems to go off into the ozone when first invoked, it is
probably hung up in the spawn.  The worst part of this "feature" of VMS
is that the spawn, once initiated, can not be stopped even with ^Y.  If
you start a MAKE on a busy day, then change your mind, you may have to
wait a while anyway.
	MAKE tries to optimize library module checking by only closing
libraries when necessary.  The definition of "necessary" is (1) when a
construction is invoked, or (2) when a DCL command is executed.  Thus,
DCL commands sprinkled throughout a makefile that uses libraries can
signifigantly slow execution.  Speed may be increased in this case by
grouping the DCL commands together whenever possible.

			An example makefile.
Here is the makefile for MAKE itself.  The exact actions result from
just how MAKE is invoked.  If the command is

	X$ MAKE

then a version of MAKE is created, but nothing special is done with it.
When the command is

	X$ MAKE debug

a version with the interactive debugger is created.  Finally, when the
command is 

	X$ MAKE install

then the copy made is installed as the production version of make.

!
! Makefile for make version 3.
!
! Define the command line parameters as symbols.
!
# defclp
!
! If we are making a debug version, define options appropriately.
!
# ifdef DEBUG
	# define c_opts /nolist /debug /nooptimize
	# define linkopts /nomap /debug
# else
	# define c_opts /nolist
	# define linkopts /nomap
# endif
!
! Disable traceback if this is to be an installed version.
!
# ifdef INSTALL
	# define linkopts ^linkopts /notrace
# endif
# define LIB	makelib.olb
!
! Get timer info
!
# define MAKE$TIMER
!
! Fancy constructions.  They just print a line telling which files are
! to be recompiled, then do the work silently.
!
# cons .c (.olb)
	# message ^MAKE$SOURCE\	[^MAKE$OUT_OF_DATE]
	- cc ^c_opts ^MAKE$SOURCE	! All this stuff executes silently
	- library ^LIB ^MAKE$SOURCE
	- delete ^MAKE$SOURCE.obj;*
# end
# cons .c .obj
	# message ^MAKE$SOURCE\	[^MAKE$OUT_OF_DATE]
	- cc ^c_opts ^MAKE$SOURCE
# end

# cons x .olb
	# message Library is gone -- recreating
	- library /create ^$
	# wait
# end

!
! Make sure that the library exists.
!
^LIB:
!
! The main program does not go into the library.
!
make.obj:		make
!
! Everything else does.
!
^LIB(chdir):		make
^LIB(cons):		ltype
^LIB(ccons):
^LIB(charutl):
^LIB(checkdef):
^LIB(comline):
^LIB(dcl):
^LIB(defclp):
^LIB(dirty):
^LIB(docond):		fdate
^LIB(execute):
^LIB(fdate):		fdate	make
^LIB(file):		make
^LIB(filetype):
^LIB(findcons):		make
^LIB(gettarget):
^LIB(libtarget):
^LIB(linetype):		ltype
^LIB(mexit):
^LIB(processl):		ltype
^LIB(refsym):
^LIB(search):		make
^LIB(setdebug):
^LIB(sharp):		make
^LIB(skiptoels):	make
^LIB(stab):
^LIB(timer):		make
!
! Do the link, if required.
!
# cons .obj .exe
	link ^linkopts make,^LIB/lib,sys$library:crtlib/lib
# end
# wait	! Make sure all work is done.
make.exe:	^LIB(chdir)	^LIB(cons)	^LIB(ccons)	^LIB(charutl) \
		^LIB(comline)	^LIB(dcl)	^LIB(dirty)	^LIB(docond)  \
		^LIB(execute)	^LIB(fdate)	^LIB(file)	^LIB(filetype)\
		^LIB(findcons)	^LIB(gettarget)	^LIB(libtarget)	^LIB(linetype)\
		^LIB(mexit)	^LIB(processl)	^LIB(refsym)	^LIB(sharp)   \
		^LIB(skiptoels) ^LIB(stab)	^LIB(search)	^LIB(checkdef)\
		^LIB(timer)	^LIB(defclp)
!
! Install the executable if necessary.  The previous version is saved,
! just in case this one does not work.
!
# ifdef INSTALL
	X$ rename cmd:make.exe cmd:make.sav
	X$ copy make.exe cmd:
	X$ purge cmd:
# else
	# message ------------------- No installation -----------------
# endif
# dump file_stats
$	babble "Done.					(whew!)
$	exit
-- 
Jonathan Corbet
National Center for Atmospheric Research, Field Observing Facility
{seismo|hplabs}!hao!boulder!jon		(Thanks to CU CS department)
