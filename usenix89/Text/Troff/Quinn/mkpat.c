/* mkpat -- make a pattern for sed to use to match argument */
#include <stdio.h>

main(argc, argv)
     char *argv[];
{
     char *rindex();

     char *progname = rindex(argv[0], '/');
     char * fullprogname = argv[0];
     char *ap;

     if (!progname)
	  progname = argv[0];
     
     if (argc != 2) {
	  fprintf(stderr, "%s:usage - %s name\n", fullprogname, progname);
	  exit(1);
     }

     for (ap = argv[1]; *ap; ap++ ) {
	  switch(*ap) {

	  case '*':
	  case '[':
	  case '.':
	  case '&':
	  case '~':	/* ~ is special to ex and ucbgrep */
	  case '$':
	  case '^':
	  case '\\':
	  case '/':
	       printf("\\%c", *ap);
	       break;
	  default:
	       putchar(*ap);
	  }
     }
     exit(0);
}
