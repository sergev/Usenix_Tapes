/* LAR Version 2.0 */

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <findfirs.h>
#include <bdos.h>

#define ACTIVE	00
#define UNUSED	0xff
#define DELETED 0xfe

#define MAXFILES 256
#define BLOCK	8192
#define DSIZE	( sizeof(struct ludir) )

#define equal(s1, s2) ( strcmp(s1,s2) == 0 )
/* if you don't have void type just define as blank */
#define VOID	(void)

#define false 0
#define true (char) 0xff
typedef unsigned char bool;
typedef unsigned char byte;

typedef struct {
	byte	hibyte;
	byte	mbyte2;
	byte	mbyte1;
	byte	lobyte;
} lword;

typedef struct {
	byte	hibyte;
	byte	lobyte;
} word;

/* convert word to int */
#define wtoi(w) ((w.hibyte<<8) + w.lobyte)

/* convert int to word */
#define itow(dst, src) (dst.hibyte = ((src & 0xff00) >> 8), dst.lobyte = (src & 0x00ff))

/* convert lword to long */
/* works, but is much slower than stupid assembler routine
#define lwtol(lw) ( ((long) lw.hibyte<<24) + ((long) lw.mbyte2<<16) + ((long) lw.mbyte1<<8) + (long) lw.lobyte)
*/


/* cheap hack */
long rlwtol();
#define lwtol(lw) rlwtol(&lw)

#define ltolw(dst, src) dst.hibyte = ((src & 0xff000000L) >> 24);\
                dst.mbyte2 = ((src & 0x00ff0000L) >> 16);\
                dst.mbyte1 = ((src & 0x0000ff00L) >> 8);\
                dst.lobyte = (src & 0x000000ffL);

union timer {
    struct ftime ftimep;
    long realtime;
};

typedef int fildesc;

struct ludir {			/* Internal library ldir structure */
	byte    l_stat;		/*  status of file */
	byte    l_name[8];      /*  name */
	byte    l_ext[3];       /*  extension */
	lword   l_off;		/*  offset in library */
	lword   l_len;		/*  length of file */
	lword	l_datetime;	/*  date and time of file. */
	byte    l_attrib;	/* attributes of file */
	byte	l_startvol;	/* starting disk number */
	byte	l_endvol;	/* ending volume number */
	word	l_cksum;	/* crc checksum */
	char    l_fill[3];	/*  pad to 32 bytes */
};

#define ERROR -1
