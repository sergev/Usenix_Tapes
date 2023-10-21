#include <site.h>
#include <error.h>
#define	nextarg	{argc--; argv++;}
#define	MDIRENT	496					/* must be zero mod 8 */
#define DIRSIZE	16
#define	BSIZE	((d->d_size0&0377)<<7) | (((d->d_size1)>>9)&0177)
#define	MBOOT	"/usr/mdec/mboot"
#define TBOOT	"/usr/mdec/tboot"
#define	MKDIR	"/bin/mkdir"

#define	tapeblk	&tpentry[0]
#define tapeb	&tpentry[0]

struct tent
{							/* Structure of a tape directory block */
    char    pathnam[32];
    int     mode;
    char    uid;
    char    gid;
    char    spare;
    char    size0;
    int     size1;
    int     time[2];					/* timen == time[n] */
    int     tapea;					/* tape address */
    int     unused[8];
    int     cksum;
}
            tpentry[8];

struct dent
{							/* in core version of tent with "unused" removed * and pathname
							   replaced by pointer to same in a * packed area (nameblock). 
							*/
    char   *d_namep;
    int     d_mode;
    char    d_uid;
    char    d_gid;
    char    d_spare;
    char    d_size0;
    int     d_size1;
    int     d_time[2];
    int     d_tapea;
}
            dir[MDIRENT];

struct
{
    int     s_skip[2];
    int     s_flags;
    char    s_nlinks;
    char    s_uid;
    char    s_gid;
    char    s_size0;
    int     s_size1;
    int     s_addr[8];
    int     s_active[2];
    int     s_modtime[2];
}
        statb;
char    map[4096];
int     catlb[10];
char    name[32];
char    name1[32];
char    tc[] "/dev/tapx";
char    mt[] "/dev/mt0";

int     narg,
        rnarg;
char  **parg;
int     wseeka,
        rseeka;
int     tapsiz;
int     fio;
char *boot;
int     ndirent,
        ndentd8;
char   *edir;
struct dent *lastd;					/* for improvement */
int     (*command) ();

char   *nameblk;
char   *top;
char   *nptr;

int     flags;						/* default is flu */
#define	flc	0001
#define	flf	0002
#define	fli	0004
#define	flm	0010
#define	flu	0020
#define	flv	0040
#define	flw	0100
#define flp	0200
#define flo	0400

struct
{
    int     integer;
};
