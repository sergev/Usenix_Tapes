/*
 *  Top - a top users display for Berkeley Unix
 *
 *  Definitions for things that might vary between installations.
 */

/*
 *  "Table_size" defines the size of the hash tables used to map uid to
 *  username.  The number of users in /etc/passwd CANNOT be greater than
 *  this number.  If the error message "table overflow: too many users"
 *  is printed by top, then "Table_size" needs to be increased.  Things will
 *  work best if the number is a prime number that is about twice the number
 *  of lines in /etc/passwd.
 */
#ifndef Table_size
#define Table_size	421
#endif

/*
 *  "Nominal_TOPN" is used as the default TOPN when Default_TOPN is Infinity
 *  and the output is a dumb terminal.  If we didn't do this, then
 *  installations who use a default TOPN of Infinity will get every
 *  process in the system when running top on a dumb terminal (or redirected
 *  to a file).  Note that Nominal_TOPN is a default:  it can still be
 *  overridden on the command line, even with the value "infinity".
 */
#ifndef Nominal_TOPN
#define Nominal_TOPN	18
#endif

/*
 *  File name for the system image and the memory devices.
 */
#define VMUNIX	"/vmunix"
#define KMEM	"/dev/kmem"
#define MEM	"/dev/mem"
