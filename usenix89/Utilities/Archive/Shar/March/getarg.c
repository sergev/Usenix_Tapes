/*
 *	$Header: getarg.c,v 1.4 86/10/04 12:31:14 roger Exp $
 */
/********************************************************
 *							*
 *			getarg.c			*
 *							*
 ********************************************************/
#include <stdio.h>
/*
 *	External variables.
 */
extern char *index();
/*
 *	Accumulation variables.
 */
char *optarg;		/* Global argument pointer. */
int optind = 0;		/* Global argv index. */
/*
 *	Getarg:
 *
 *	  Parses the command line sequentially. Arguments starting with
 *	a '-' are defined to be switches. The following character is the
 *	switch value. If it is present in the option string, its value
 *	is returned. If the option character is followed by the ':'
 *	character, the most meaningful argument string is returned in
 *	the argument 'optarg' and its index is returned as 'optind'. By
 *	the way, 'optind' is the working index and can be carefully
 *	fiddled with. A '\0' value is returned for unswitched arguments
 *	and 'optarg'/'optind' refer to the argument. A '?' is returned
 *	for switches that are unspecified. An EOF is returned if the
 *	command line is empty.
 */
int getarg(argc,argv,optstring)
register int argc;
register char *argv[];
register char *optstring;
{
  /*
   *	Locals.
   */
  register int i;
  register char *swc;
  /*
   *	Set up the global variables. Restrict the range of 'optind'
   *	and terminate if it is upside out of bounds.
   */
  optarg = NULL;
  if (++optind < 1)
    optind  = 1;
  else if (optind >= argc)
    return (EOF);
  /*
   *	See a switch is present. If not, then return a switchless
   *	argument.
   */
  if (*argv[optind] != '-'){
    optarg = argv[optind];
    return ('\0');
  }
  /*
   *	See if the switch is in the option string.
   */
  if (!(swc = index(optstring,*(argv[optind] + 1))))
    return ('?');
  /*
   *	See if an argument needs to be extracted.
   */
  if (swc[1] != ':')
    return (swc[0]);
  /*
   *	Extract the argument.
   */
  if (*(argv[optind] + 2))
    optarg = argv[optind] + 2;
  else if (++optind < argc)
    optarg = argv[optind];
  return (swc[0]);
}
