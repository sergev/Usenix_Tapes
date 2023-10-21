#ifdef MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef MSC
#include <dos.h>
#define STRUCTURE union REGS r;
#define RAX r.x.ax 
#define RBX r.x.bx
#define RCX r.x.cx 
#define RDX r.x.dx 
#define RSI r.x.si 
#define RDI r.x.di
#define CALLDOS intdos(&r, &r)
#define RESULT r.x.cflag
#define PUTC(i) (tsc) ? putch(i) : fputc(i,stdout)
#define PUTS(i) (tsc) ? cputs(i) : fputs(i,stdout)
#define GETC (bdos(7) & 0xFF) 
#define KBCHECK kbhit()
#endif

#ifdef CIC86
#define STRUCTURE struct{int ax, bx, cx, dx, si, di, ds, es;}r;\
struct{int cs,ss,dz,ez;}s;int x_z;
#define RAX r.ax 
#define RBX r.bx
#define RCX r.cx 
#define RDX r.dx 
#define RSI r.si 
#define RDI r.di
#define CALLDOS  segread(&s); r.ds = s.dz; x_z = sysint21(&r, &r) & 1
#define RESULT x_z
#define PUTC(i) fputc(i, stdout)
#define PUTS(i) fputs(i, stdout)
#define GETC (bdos(7) & 0xFF) 
#define KBCHECK (bdos(0x0B) & 1)
#endif

#ifdef DESMET
#define STRUCTURE extern int _rax, _rbx, _rcx, _rdx, _rsi, _rdi, _rds, _carryf;
#define RAX _rax
#define RBX _rbx
#define RCX _rcx
#define RDX _rdx
#define RSI _rsi
#define RDI _rdi
#define RSI _rsi
#define CALLDOS _rds = -1;_doint(0x21)
#define RESULT (_carryf & 1)
#define PUTC(i) fputc(i, stdout)
#define PUTS(i) fputs(i, stdout)
#define GETC ci()
#define KBCHECK csts()
#endif

/* customizing constants */

#define ID      1           /* always identify directory if 1 */
#define ALL     0           /* show hidden files by default if 1 */
#define LONG    0           /* long listing by default if 1 */
#define SCOLM   0           /* 1-column short listing by default if 1 */
#define LCOLM   1           /* 1-column long listing by default if 1 */
#define RSORT   0           /* reverse sort by default if 1 */
#define TSORT   0           /* time sort by default if 1 */
#define DU      0           /* include disk use by default if 1 */

#define BAR    "\272 "      /* double bar separator for long listing */
#define NAMESIZ 13          /* 12 character name + NULL */
#define ONECS 512           /* cluster size on one-sided floppy */
#define TWOCS 1024          /* cluster size on two-sided floppy */
#define SCRSIZ 22           /* scrolling size of display screen */

EXTERN struct dta                    /* DOS Disk Transfer Address table */
    {
    char reserved[21];               /* used in "find next" operation */
    unsigned char attr;              /* file attribute byte */
    int ftime;                       /* time of last modification */
    int fdate;                       /* date of last modification */
    long fsize;                      /* file size in bytes */
    unsigned char fname[NAMESIZ];    /* filename and extension */
    };

EXTERN struct outbuf       /* output buffer -- array of file structs */
    {
    unsigned oattr;
    unsigned odate;
    unsigned otime;
    long osize;
    unsigned char oname[NAMESIZ+1];
    };

EXTERN struct outbuf *obuf;        /* pointer into outbuf */
EXTERN unsigned char spath[128];   /* holds current pathname string */

/* global variables and flags */

EXTERN int allf;                   /* include hidden & system files */
EXTERN int ll;                     /* long listing */
EXTERN int colm;                   /* 1-column format */
EXTERN int id;                     /* show directory name */
EXTERN int rev;                    /* reverse sort */
EXTERN int tsrt;                   /* timesort the listing */
EXTERN int usage;                  /* print disk usage */
EXTERN int recd;                   /* recursive descent requested */
EXTERN int sizonly;                /* only print sizes */

EXTERN unsigned int qs;            /* pathname separator character */
EXTERN int np;                     /* number of groups printed */
EXTERN int clsize;                 /* size of a cluster, in bytes */
EXTERN int clmask;                 /* clsize-1 for rounding & chopping */
EXTERN int drive;                  /* code number for drive requested */
EXTERN int tsc;                    /* 1 if output is to console screen */
EXTERN long left;                  /* unused space left on disk */
EXTERN long total;                 /* total of sizes encountered */

