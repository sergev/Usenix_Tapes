#include        "empdef.h"
/*
** Compile: cc -O -c -q erlog.c; ar r empsub.a erlog.o
*/

extern  int thisprog;

erlog(string)               /* mail "string" to the priveleged Empire user */
char *string;
{
        char buf[256], *copy();

        switch (fork()) {
        case -1:
            printf("Unable to fork()!\n");
            break;
        case 0:
            close(0);           /* these two are because of "to" prog bugs */
            close(1);
            copy(string, buf);    /* to avoid collisions in the fmt buffer */
            sprintf(fmtbuf,"-TPROG:%d, COM:%s\n%s\n", thisprog, combuf, buf);
            copy(fmtbuf, buf);
            execl("/bin/to", "to" , privlog, "-SEmpire Error", buf, 0);
            printf("Unable to execl the mail program!\n");
            exit(2);
        default:
            wait((int *)0);
        }
}
