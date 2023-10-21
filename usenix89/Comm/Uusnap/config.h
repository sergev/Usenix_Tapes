/*
 * config.h
 * configure the local options
 *
 * Based on OS:
 *	termio.h vs. sgtty.h
 *	index/rindex vs strchr/strrchr
 *	if lstat() symbolic link stat is supported
 *
 * You pick:
 *	if fixed len or variable len directory entries are supported
 */


/*
 * bsd 4.1 and 4.2 uses sgtty for ioctl's
 * s3, s5 uses termio.h for ioctls
 * On output, defines WTERMIO==1 or undefined
 * On output, defines TERMIO to be either sgttyb or termio, for defining
 * variables.
 */

#if BSD4_1 == 1 || BSD4_2 == 1
#define TERMIO sgttyb
/* don't want termio so leave WTERMIO undefined */
#else !BSD4_1
#if S3 == 1 || S5 == 1 || S5R2 == 1 || CPGCOS == 1
#define TERMIO termio
#define WTERMIO 1
#else !S3
Unknown version of UNIX.  Only BSD 4.[12], S3, S5, S5R2, and CPGCOS known.
#endif S3
#endif BSD4_1


/*
 * on 4.1 and 4.2 BSD systems, the S3, S5, and S5R2 string(3)
 * names are not around
 */
#if BSD4_1 == 1 || BSD4_2 == 1
#define strchr index
#define strrchr rindex
#endif

#if BSD4_2 == 1 || CPGCOS == 1
#define WSYMLINK 1
#endif

#if BSD4_2 == 1 || CPGCOS == 1
# define DIRVARLEN 1
#else !BSD4_2
# if S3 == 1 || S5 == 1 || S5R2 == 1
#  define DIRFIXLEN 1
# else
   I don't know the directory conventions on this machine
   bsd4.1
# endif
#endif
