/* Fully extensible Emacs, running on Unix, intended for GNU.
   Copyright (C) 1985, 1986, 1987 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


#include <signal.h>
#include <errno.h>

#include "config.h"
#include <stdio.h>
#undef NULL
#include "lisp.h"
#include "commands.h"

#include <sys/types.h>
#include <sys/file.h>

#ifdef VMS
#include <ssdef.h>
#endif

#ifdef USG5
#include <fcntl.h>
#endif

#ifdef BSD
#include <sys/ioctl.h>
#endif

#ifdef APOLLO
#include <default_acl.h>
#endif

#ifndef O_RDWR
#define O_RDWR 2
#endif

#define PRIO_PROCESS 0

/* Command line args from shell, as list of strings */
Lisp_Object Vcommand_line_args;

/* Set nonzero after Emacs has started up the first time.
  Prevents reinitialization of the Lisp world and keymaps
  on subsequent starts.  */
int initialized;

/* Variable whose value is symbol giving operating system type */
Lisp_Object Vsystem_type;
  
/* If non-zero, emacs should not attempt to use an window-specific code,
   but instead should use the virtual terminal under which it was started */
int inhibit_window_system;

#ifdef HAVE_X_WINDOWS
/* If -d option is used, this variable points to the name of
   the display to use.  */
char *alternate_display;
#endif /* HAVE_X_WINDOWS */

/* Nonzero means running Emacs without interactive terminal.  */

int noninteractive;

/* Value of Lisp variable `noninteractive'.
   Normally same as C variable `noninteractive'
   but nothing terrible happens if user sets this one.  */

int noninteractive1;

/* Signal code for the fatal signal that was received */
int fatal_error_code;

/* Nonzero if handling a fatal error already */
int fatal_error_in_progress;

/* Handle bus errors, illegal instruction, etc. */
fatal_error_signal (sig)
     int sig;
{
#ifdef BSD
  int tpgrp;
#endif /* BSD */

  fatal_error_code = sig;
  signal (sig, SIG_DFL);

  /* If fatal error occurs in code below, avoid infinite recursion.  */
  if (fatal_error_in_progress)
    kill (getpid (), fatal_error_code);

  fatal_error_in_progress = 1;

  /* If we are controlling the terminal, reset terminal modes */
#ifdef BSD
  if (ioctl(0, TIOCGPGRP, &tpgrp) == 0
      && tpgrp == getpgrp (0))
#endif /* BSD */
    {
      reset_sys_modes ();
      if (sig != SIGTERM)
	fprintf (stderr, "Fatal error (%d).", sig);
    }

  /* Clean up */
#ifdef subprocesses
  kill_buffer_processes (Qnil);
#endif
  Fdo_auto_save (Qt);

#ifdef CLASH_DETECTION
  unlock_all_files ();
#endif /* CLASH_DETECTION */

#ifdef VMS
  kill_vms_processes ();
  LIB$STOP (SS$_ABORT);
#else
  /* Signal the same code; this time it will really be fatal.  */
  kill (getpid (), fatal_error_code);
#endif /* not VMS */
}

/* Code for dealing with Lisp access to the Unix command line */

static
init_cmdargs (argc, argv, skip_args)
     int argc;
     char **argv;
     int skip_args;
{
  register int i;

  Vcommand_line_args = Qnil;

  for (i = argc - 1; i >= 0; i--)
    {
      if (i == 0 || i > skip_args)
	Vcommand_line_args
	  = Fcons (build_string (argv[i]), Vcommand_line_args);
    }
}

/* ARGSUSED */
main (argc, argv, envp)
     int argc;
     char **argv;
     char **envp;
{
  int skip_args = 0;
  extern int errno;
  extern void malloc_warning ();

#ifdef VMS
#ifdef LINK_CRTL_SHARE
#ifdef SHAREABLE_LIB_BUG
  extern noshare char **environ;
#endif /* SHAREABLE_LIB_BUG */
#endif /* LINK_CRTL_SHARE */

  /* If -map specified, map the data file in */
  if (argc > 2 && ! strcmp (argv[1], "-map"))
    {
      skip_args = 2;
      mapin_data (argv[2]);
    }

#ifdef LINK_CRTL_SHARE
#ifdef SHAREABLE_LIB_BUG
  /* Bletcherous shared libraries! */
  if (!stdin)
    stdin = fdopen (0, "r");
  if (!stdout)
    stdout = fdopen (1, "w");
  if (!stderr)
    stderr = fdopen (2, "w");
  if (!environ)
    environ = envp;
#endif /* SHAREABLE_LIB_BUG */
#endif /* LINK_CRTL_SHARE */
#endif /* VMS */

  clearerr (stdin);

#ifdef APOLLO
  /* If USE_DOMAIN_ACLS environment variable exists,
     use ACLs rather than UNIX modes. */
  if (egetenv ("USE_DOMAIN_ACLS"))
    default_acl (USE_DEFACL);
#endif /* APOLLO */

#ifdef APOLLO			/* Reserve memory space for sbrk to get */
  set_sbrk_size (4000000);
#else /* not APOLLO */
  /* Arrange for warnings when nearly out of space.  */
  malloc_init (0, malloc_warning);
#endif /* not APOLLO */

#ifdef HIGHPRI
  setpriority (PRIO_PROCESS, getpid (), HIGHPRI);
  setuid (getuid ());
#endif HIGHPRI

  inhibit_window_system = 0;

/* Handle the -t switch, which specifies filename to use as terminal */
  if (skip_args + 2 < argc && !strcmp (argv[skip_args + 1], "-t"))
    {
      skip_args += 2;
      close (0);
      close (1);
      open (argv[skip_args], O_RDWR, 2 );
      dup (0);
      fprintf (stderr, "Using %s\n", argv[skip_args]);
#ifdef HAVE_X_WINDOWS
      inhibit_window_system = 1;	/* -t => -nw */
#endif
    }
#ifdef HAVE_X_WINDOWS
/* Handle the -d switch, which means use a different display for X */
  if (skip_args + 2 < argc && (!strcmp (argv[skip_args + 1], "-d") ||
			       !strcmp (argv[skip_args + 1], "-display")))
    {
      skip_args += 2;
      alternate_display = argv[skip_args];
    } 
  else
    alternate_display = 0;
#endif	/* HAVE_X_WINDOWS */

  if (skip_args + 1 < argc
      && (!strcmp (argv[skip_args + 1], "-nw")))
    {
      skip_args += 1;
      inhibit_window_system = 1;
    }

/* Handle the -batch switch, which means don't do interactive display.  */
  noninteractive = 0;
  if (skip_args + 1 < argc && !strcmp (argv[skip_args + 1], "-batch"))
    {
      skip_args += 1;
      noninteractive = 1;
    }

  if (
#ifndef CANNOT_DUMP
      ! noninteractive || initialized
#else
      1
#endif
      )
    {
      /* Don't catch these signals in batch mode if not initialized.
	 On some machines, this sets static data that would make
	 signal fail to work right when the dumped Emacs is run.  */
      signal (SIGHUP, fatal_error_signal);
      signal (SIGQUIT, fatal_error_signal);
      signal (SIGILL, fatal_error_signal);
      signal (SIGTRAP, fatal_error_signal);
      signal (SIGIOT, fatal_error_signal);
#ifdef SIGEMT
      signal (SIGEMT, fatal_error_signal);
#endif
      signal (SIGFPE, fatal_error_signal);
      signal (SIGBUS, fatal_error_signal);
      signal (SIGSEGV, fatal_error_signal);
      signal (SIGSYS, fatal_error_signal);
      signal (SIGTERM, fatal_error_signal);
#ifdef SIGXCPU
      signal (SIGXCPU, fatal_error_signal);
#endif
#ifdef SIGXFSZ
      signal (SIGXFSZ, fatal_error_signal);
#endif SIGXFSZ
    }

  noninteractive1 = noninteractive;

/* Perform basic initializations (not merely interning symbols) */

  if (!initialized)
    {
      init_alloc_once ();
      init_obarray ();
      init_eval_once ();
      init_syntax_once ();	/* Create standard syntax table.  */
		      /* Must be done before init_buffer */
      init_buffer_once ();	/* Create buffer table and some buffers */
      init_minibuf_once ();	/* Create list of minibuffers */
			      /* Must precede init_window_once */
      init_window_once ();	/* Init the window system */
    }

  init_alloc ();
#ifdef MAINTAIN_ENVIRONMENT
  init_environ ();
#endif
  init_eval ();
  init_data ();
  init_read ();

  init_cmdargs (argc, argv, skip_args);	/* Create list Vcommand_line_args */
  init_buffer ();	/* Init default directory of main buffer */
  if (!noninteractive)
    {
#ifdef VMS
      init_vms_input ();/* init_display calls get_screen_size, that needs this */
#endif /* VMS */
      init_display ();	/* Determine terminal type.  init_sys_modes uses results */
    }
  init_keyboard ();	/* This too must precede init_sys_modes */
  init_sys_modes ();	/* Init system terminal modes (RAW or CBREAK, etc.) */
  init_xdisp ();
  init_macros ();
  init_editfns ();
  init_callproc ();
#ifdef VMS
  init_vmsfns ();
#endif /* VMS */
#ifdef subprocesses
  init_process ();
#endif /* subprocesses */

/* Intern the names of all standard functions and variables; define standard keys */

  if (!initialized)
    {
      /* The basic levels of Lisp must come first */
      /* And data must come first of all
	 for the sake of symbols like error-message */
      syms_of_data ();
      syms_of_alloc ();
#ifdef MAINTAIN_ENVIRONMENT
      syms_of_environ ();
#endif MAINTAIN_ENVIRONMENT
      syms_of_read ();
      syms_of_print ();
      syms_of_eval ();
      syms_of_fns ();

      syms_of_abbrev ();
      syms_of_buffer ();
      syms_of_bytecode ();
      syms_of_callint ();
      syms_of_casefiddle ();
      syms_of_callproc ();
      syms_of_cmds ();
#ifndef NO_DIR_LIBRARY
      syms_of_dired ();
#endif /* not NO_DIR_LIBRARY */
      syms_of_display ();
      syms_of_doc ();
      syms_of_editfns ();
      syms_of_emacs ();
      syms_of_fileio ();
#ifdef CLASH_DETECTION
      syms_of_filelock ();
#endif /* CLASH_DETECTION */
      syms_of_indent ();
      syms_of_keyboard ();
      syms_of_keymap ();
      syms_of_macros ();
      syms_of_marker ();
      syms_of_minibuf ();
      syms_of_mocklisp ();
#ifdef subprocesses
      syms_of_process ();
#endif /* subprocesses */
      syms_of_search ();
      syms_of_syntax ();
      syms_of_undo ();
      syms_of_window ();
      syms_of_xdisp ();
#ifdef HAVE_X_WINDOWS
      syms_of_xfns ();
#ifdef HAVE_X_MENU
      syms_of_xmenu ();
#endif /* HAVE_X_MENU */
#endif /* HAVE_X_WINDOWS */

#ifdef SYMS_SYSTEM
      SYMS_SYSTEM;
#endif

#ifdef SYMS_MACHINE
      SYMS_MACHINE;
#endif

      keys_of_casefiddle ();
      keys_of_cmds ();
      keys_of_buffer ();
      keys_of_keyboard ();
      keys_of_keymap ();
      keys_of_macros ();
      keys_of_minibuf ();
      keys_of_window ();
    }

  if (!initialized)
    {
      /* Handle -l loadup-and-dump, args passed by Makefile. */
      if (argc > 2 + skip_args && !strcmp (argv[1 + skip_args], "-l"))
	Vtop_level = Fcons (intern ("load"),
			    Fcons (build_string (argv[2 + skip_args]), Qnil));
#ifdef CANNOT_DUMP
      /* Unless next switch is -nl, load "loadup.el" first thing.  */
      if (!(argc > 1 + skip_args && !strcmp (argv[1 + skip_args], "-nl")))
	Vtop_level = Fcons (intern ("load"),
			    Fcons (build_string ("loadup.el"), Qnil));
#endif /* CANNOT_DUMP */
    }

  initialized = 1;

  /* Enter editor command loop.  This never returns.  */
  Frecursive_edit ();
  /* NOTREACHED */
}

DEFUN ("kill-emacs", Fkill_emacs, Skill_emacs, 0, 1, "P",
  "Exit the Emacs job and kill it.  ARG means no query.\n\
If emacs is running noninteractively and ARG is an integer,\n\
return ARG as the exit program code.")
  (arg)
     Lisp_Object arg;
{
  Lisp_Object answer;
  int i;

  if (feof (stdin))
    arg = Qt;
  if (!noninteractive && NULL (arg))
    {
      if ((i = ModExist())
	  && (answer = Fyes_or_no_p (format1 (
"%d modified buffer%s exist%s, do you really want to exit? ",
					      i, i == 1 ? "" : "s",
					      i == 1 ? "s" : "")),
	      NULL (answer)))
	return Qnil;

#ifdef subprocesses
      if (count_active_processes()
	  && (answer = Fyes_or_no_p (format1 (
"Subprocesses are executing; kill them and exit? ")),
	      NULL (answer)))
	return Qnil;
#endif /* subprocesses */
    }

#ifdef subprocesses
  kill_buffer_processes (Qnil);
#endif /* subprocesses */

#ifdef VMS
  kill_vms_processes ();
#endif /* VMS */

  Fdo_auto_save (Qt);

#ifdef CLASH_DETECTION
  unlock_all_files ();
#endif /* CLASH_DETECTION */

  fflush (stdout);
  reset_sys_modes ();
/* Is it really necessary to do this deassign
   when we are going to exit anyway?  */
/* #ifdef VMS
  stop_vms_input ();
 #endif  */
  stuff_buffered_input (arg);
  exit ((XTYPE (arg) == Lisp_Int) ? XINT (arg)
#ifdef VMS
	: 1
#else
	: 0
#endif
	);
  /* NOTREACHED */
}

#ifndef CANNOT_DUMP
/* Nothing like this can be implemented on an Apollo.
   What a loss!  */

DEFUN ("dump-emacs", Fdump_emacs, Sdump_emacs, 2, 2, 0,
  "Dump current state of Emacs into executable file FILENAME.\n\
Take symbols from SYMFILE (presumably the file you executed to run Emacs).")
  (intoname, symname)
     Lisp_Object intoname, symname;
{
  register unsigned char *a_name = 0;
  extern int my_edata;
  Lisp_Object tem;
  extern void malloc_warning ();

  CHECK_STRING (intoname, 0);
  intoname = Fexpand_file_name (intoname, Qnil);
  if (!NULL (symname))
    {
      CHECK_STRING (symname, 0);
      if (XSTRING (symname)->size)
	{
	  symname = Fexpand_file_name (symname, Qnil);
	  a_name = XSTRING (symname)->data;
	}
    }

  tem = Vpurify_flag;
  Vpurify_flag = Qnil;

  fflush (stdout);
#ifdef VMS
  mapout_data (XSTRING (intoname)->data);
#else
  /* Tell malloc where start of impure now is */
  /* Also arrange for warnings when nearly out of space.  */
  malloc_init (&my_edata, malloc_warning);
  unexec (XSTRING (intoname)->data, a_name, &my_edata, 0, 0);
#endif /* not VMS */

  Vpurify_flag = tem;

  return Qnil;
}

#endif /* not CANNOT_DUMP */

#ifdef VMS
#define SEPCHAR ','
#else
#define SEPCHAR ':'
#endif

Lisp_Object
decode_env_path (evarname, defalt)
     char *evarname, *defalt;
{
  register char *path, *p;
  extern char *index ();

  Lisp_Object lpath;

  path = (char *) egetenv (evarname);
  if (!path)
    path = defalt;
  lpath = Qnil;
  while (1)
    {
      p = index (path, SEPCHAR);
      if (!p) p = path + strlen (path);
      lpath = Fcons (p - path ? make_string (path, p - path) : Qnil,
		     lpath);
      if (*p)
	path = p + 1;
      else
	break;
    }
  return Fnreverse (lpath);
}

syms_of_emacs ()
{
#ifndef CANNOT_DUMP
  defsubr (&Sdump_emacs);
#endif /* not CANNOT_DUMP */

  defsubr (&Skill_emacs);

  DEFVAR_LISP ("command-line-args", &Vcommand_line_args,
    "Args passed by shell to Emacs, as a list of strings.");

  DEFVAR_LISP ("system-type", &Vsystem_type,
    "Symbol indicating type of operating system you are using.");
  Vsystem_type = intern (SYSTEM_TYPE);

  DEFVAR_BOOL ("noninteractive", &noninteractive1,
    "Non-nil means Emacs is running without interactive terminal.");
}
