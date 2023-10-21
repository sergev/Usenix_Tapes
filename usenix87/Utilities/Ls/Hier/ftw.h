/*
**  This is a version of <ftw.h> for sites that don't have it.  I just
**  picked four values at random for the #define's that sftw needed; I
**  doubt that this is binary-compatible with AT&T's <ftw.h>, and this
**  may well be missing something.  Oh, well, it's public-domain...
**	-Rich $alz, mirror!rs
*/


/*
**  These values are passed on to the user's function, as the third
**  parameter.
*/
#define	FTW_F		1	/* A file				*/
#define	FTW_D		2	/* A directory				*/
#define	FTW_DNR		3	/* A directory that couldn't be read	*/
#define	FTW_NS		4	/* A stat() failure			*/
