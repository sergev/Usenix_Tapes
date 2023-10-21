
/*
 * MAIN.C
 *
 * Matthew Dillon, 24 Feb 1986
 *
 * el main routine.
 */

#include "shell.h"

char Inline[256];

main(argc, argv)
char *argv[];
{
   char *prompt;

   init_vars();
   init();
   strcpy (Inline, "source ");
   strcat (Inline, (argc == 2) ? argv[1] : "sys:.login");
   do_source (Inline, 0);
   for (;;) {
      if ((prompt = get_var (LEVEL_SET, V_PROMPT)) == NULL)
         prompt = "echo -n \"% \"";
      ++H_stack;
      exec_command (prompt);
      --H_stack;
      fflush (stdout);
      if (gets (Inline) == NULL)
         exit (0);
      exec_command (Inline);
   }
}

init()
{
}

init_vars()
{
   set_var (LEVEL_SET, V_PROMPT, "echo -n \"% \"");
   set_var (LEVEL_SET, V_HIST,   "20");
}


