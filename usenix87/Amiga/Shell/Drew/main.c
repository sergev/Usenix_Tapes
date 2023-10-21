
/*
 * MAIN.C
 *
 * Matthew Dillon, 24 Feb 1986
 * (c)1986 Matthew Dillon     9 October 1986
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 */

#include "shell.h"


int aux; /* for use with aux: driver */

char Inline[256];
#ifndef AZTEC_C
char stdout_buff[STDBUF];
#endif

main(argc, argv)
register char *argv[];
{
#ifdef AZTEC_C
   char *rawgets();
#endif
   char *prompt;
   register int i;
   extern int Enable_Abort;
   init_vars();
   init();
   seterr();
   do_pwd(NULL); /* set $_cwd */

   Enable_Abort = 0;

   for (i = 1; i < argc; ++i) {
      if (argv[i][0] == '-' && argv[i][1] == 'c') {
	    Inline[0] = ' ';
	    Inline[1] = '\000';
	    while (++i < argc) {
		strcat(Inline,argv[i]);
		strcat(Inline," ");
		}
	    exec_command(Inline);
	    main_exit(0);
	    }
      if (argv[i][0] == '-' && argv[i][1] == 'a') {
	  aux = 1;
	  continue;
      }
      strcpy (Inline, "source ");
      strcat (Inline, argv[i]);
      av[1] = argv[i];
      do_source (Inline);
   }

   for (;;) {
      if ((prompt = get_var (LEVEL_SET, V_PROMPT)) == NULL)
	 prompt = "$ ";
      if (breakcheck()) {
	 while (WaitForChar(Input(), 100L) || stdin->_bp < stdin->_bend)
	    gets(Inline);
      }
#ifdef AZTEC_C
      if (Quit || !rawgets(Inline, prompt))
#else
      printf("%s",prompt);
      if (Quit || !gets(Inline))
#endif
	 main_exit(0);
      breakreset();
      if (*Inline)
	 exec_command(Inline);
   }
}

init_vars()
{
   if (IsInteractive(Input()))
      set_var (LEVEL_SET, V_PROMPT, "$ ");
   else
      set_var (LEVEL_SET, V_PROMPT, "");
   set_var (LEVEL_SET, V_HIST,	 "20");
   set_var (LEVEL_SET, V_LASTERR, "0");
   set_var (LEVEL_SET, V_PATH, "ram:,ram:c/,c:,df1:c/,df0:c/");
   set_var (LEVEL_SET, "_insert", "1");
}

init()
{
   static char pipe1[32], pipe2[32];

   stdin->_flags  |= 0x80;         /* make sure we're set as a tty */
   stdout->_flags |= 0x80;	   /* incase of redirection in .login */

#ifdef AZTEC_C
   Close(_devtab[2].fd);
   _devtab[2].mode |= O_STDIO;
   _devtab[2].fd = _devtab[1].fd;  /* set stderr to Output() otherwise */
				   /* don't work with aux driver       */
#else
   stdout->_buff = stdout_buff;
   stdout->_buflen = STDBUF;
#endif

   Myprocess = (struct Process *)FindTask(0L);
   Uniq	 = (long)Myprocess;
   Pipe1 = pipe1;
   Pipe2 = pipe2;
   sprintf (pipe1, "ram:pipe1_%ld", Uniq);
   sprintf (pipe2, "ram:pipe2_%ld", Uniq);
}


main_exit(n)
{
   exit (n);
}

breakcheck()
{
   return (int)(SetSignal(0L,0L) & SIGBREAKF_CTRL_C);
}

breakreset()
{
   SetSignal(0L, SIGBREAKF_CTRL_C);
}


/* this routine causes manx to use this Chk_Abort() rather than it's own */
/* otherwise it resets our ^C when doing any I/O (even when Enable_Abort */
/* is zero).  Since we want to check for our own ^C's			 */

Chk_Abort()
{
return(0);
}

_wb_parse()
{
}
