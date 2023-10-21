#include <stdio.h>
#include <argument.h>

static char *help_main[] =
{       /* This message is printed by "\-help". */
  arg_replace, " is similar to the example of getopt(3).\n", 
  "\n",
  NULL
};

static FILE *file = NULL;
main (argc, argv)
int argc;
char **argv;
{
  extern int bproc(), filefunc ();
  int aflag = 0;
  double bflag = 0.0;
  FILE *ifile = stdin;
  FILE *ofile = stdout;
  int ret;

	/* initialize argument package */
  arg_init (&argc, &argv, help_main);

	/* specify formal arguments */
  arg_spec (0, "-a", "", "%d", &aflag
    , "%d", "a flag", NULL, NULL);
  arg_spec (0, "-b", "optarg", "%lf", &bflag
    , "%g", "another flag", bproc, &bflag);
  arg_spec (0, "-f", "ifile", "&&r", &ifile
    , NULL, "input file", NULL, NULL);
  arg_spec (0, "-o", "ofile", "&&w", &ofile
    , NULL, "output file", NULL, NULL);
  arg_spec (0, "[]", "name", "&r", &file
    , NULL, "file name", filefunc, &file);

	/* process actuals according to formals */
  while ((ret = arg_process ()) > 0)
    ;
  if (ret < 0)
    exit (-1);
  exit (0);
}  

static int filefunc (pfp)
char *pfp;
{       /* conversion function for <name> */
  register FILE *file = * ((FILE **) pfp);
  /* etc. */
}

static int bproc (pointer)
char *pointer;
{       /* conversion function for "-b" */
  double bflag = *((double *) pointer);

  if (bflag < 0.0)
    arg_error (NULL, 0, "negative");
  else
    printf ("-b %g\n", bflag);
}
