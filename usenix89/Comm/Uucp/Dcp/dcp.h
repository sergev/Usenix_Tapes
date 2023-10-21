/* DCP a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
#include <stdio.h>		/* Standard UNIX  definitions */
#include "dcps.h"
#define MSGTIME         20
#define MAXPACK         256

#define ACK     4       /*      general definitions     */
#define NAK     2
#define DATA    0
#define CLOSE   1
#define ERROR   10
#define EMPTY   11


#define TRUE    -1
#define FALSE   0

/*<FF>*/
typedef int (*procref)();

typedef struct {
        char type;
        procref a;
        procref b;
        procref c;
        procref d;
} Proto;

/* the various protocols available. Add here for others */
extern procref          getpkt,sendpkt,openpk,closepk;

extern int              ggetpkt(),gsendpkt(),gopenpk(),gclosepk();
extern int              kgetpkt(),ksendpkt(),kopenpk(),kclosepk();
extern int              rgetpkt(),rsendpkt(),ropenpk(),rclosepk();
extern int              tgetpkt(),tsendpkt(),topenpk(),tclosepk();

/*<FF>*/
extern int              pktsize;                /* packet size for this pro*/
extern int              flog;                   /* system log file */
extern int              fw;                     /* cfile pointer */
extern int              fpr,fpw;                /* comm dev pointer */
extern char             cfile[80];              /* work file pointer */
extern int              remote;                 /* -1 means we're remote*/
extern int              debug;                  /* debugging level */
extern int              msgtime;                /* timout setting */
extern char             fromfile[132];
extern char             tofile[132];
extern char             state;                  /* present state */
extern int              fp;                     /* current disk file ptr */
extern int              size;                   /* nbytes in buff */
extern int              fsys;
extern char             tty[20];
extern char             myname[20];
extern char             username[20];
extern char             spooldir[80];
extern char             rmtname[20];
extern char             cctime[20];
extern char             device[20];
extern char             type[10];
extern char             speed[10];
extern char             proto[5];
extern char             loginseq[132];
extern unsigned int     checksum();
