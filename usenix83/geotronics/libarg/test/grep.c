#include <stdio.h>
#include <argument.h>

char *help_grep [] =
{
  arg_replace, " searches a file for a pattern\n",
  NULL
};
static int vflag, cflag, sflag, hflag, eflag;
static char *expression = NULL;
static long first_line, last_line;

main (argc, argv)
int argc;
char **argv;
{
  extern int getfile ();
  FILE *outfile = stdout;
  FILE *infile = stdin;
  int ret;

/* initialize argument package */
  arg_init (&argc, &argv, help_grep);

/* initialize formal argument variables */
  vflag = cflag = sflag = hflag = eflag = 0;
  first_line = last_line = 0;

/* specify formal arguments */
  arg_spec (0, "-v", NULL, "%d", &vflag, 
    "%d", "print all lines but those matching", NULL, NULL);
  arg_spec (0, "-c", NULL, "%d", &cflag, 
    "%d", "print only a count of matching lines", NULL, NULL);
  arg_spec (0, "-s", NULL, "%d", &sflag, 
    "%d", "produce only status output", NULL, NULL);
  arg_spec (0, "-h", "", "%d", &hflag, 
    "%d", "do not print filename headers", NULL, NULL);
  arg_spec (ARG_NOCHECK, "-e", "expression", "", &expression,
    "", "expression, possibly with leading '-'", NULL, NULL);
  arg_spec (0, NULL, "expression", "", &expression, 
    "", "expression to search for", NULL, NULL);
  arg_spec (0, "[]", "filename", "&&r", &infile, 
    NULL, "file to search in", getfile, &infile);
  arg_spec (0, "-out", "file", "&&w", &outfile, 
    "w", "output file name", NULL, NULL);
  arg_spec (0, "-first", "line", "%D", &first_line, 
    "%ld", "first line to process", NULL, NULL);
  arg_spec (0, "-last", "line", "%D", &last_line, 
    "%ld", "last line to process", NULL, NULL);

/* process arguments */
  while ((ret = arg_process ()) > 0);
  if (ret == -1)      /* check for syntax errors */
    exit (-1);
  exit (0);
}
/*
.bp 
*/

static int getfile ()
{
  register char *filename = Arg_Control.Arg_Last -> AS_stdio;

  grep (filename, expression, first_line, last_line);     
}

static int grep (name, exp, first, last)
char *name;
char *exp;
long first, last;
{
  extern int vflag, cflag, sflag, hflag, eflag;

/* This subroutine should contain code to do the work. */
  printf ("grep (%s, %s, %D, %D)\n", name, exp, first, last);
}
