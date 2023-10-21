#ifdef LOCALWARN
/* lwarn.c::lwarn() printfs it's arguments as a warning message if
 * warninglevel & WARN_ALL; if warninglevel & WARN_ONCE, it just prints a
 * message the 1st time.
 *
 * REQ Sept. 86
 */
#define WARN_ONCE	1
#define WARN_ALL	2
#endif LOCALWARN
#ifdef REPORTERRS
/* note that EWARN provides 2 arguments to a call to error.  One day the way
 * troff exits should be rationalised, and then EWARN would be a single value!
 */
#define EWARN 0,0
extern int errno;
char *realname();
void errmsg();

/* flags for diffent kingds of error */
/* these are OR'd together -- mostly used in atoi() */
#define LERR_BADEXPSTART 01	/* no expression found */
#define LERR_PSNUMWARN   02	/* "\s94 is a 9 point 4" msg enabled */
#define LERR_WARNINGS	 04	/* no warnings at all if false */

/* LERR_EVERYTHING is the result of ORing all of the LERR values together;
 * it's used to initialise the repoerterrs variable in local.h
 */
#define LERR_EVERYTHING (LERR_BADEXPSTART|LERR_WARNINGS)
/* let's not use LERR_PSNUMWARN for now! */
#endif REPORTERRS
#if defined(REPORTERRS) || defined(TCHARTOS)
extern char *strcpy();
extern char *strcat();
#endif /* REPORTERRS||TCHARTOS */
