/* ls - a Unix-like directory listing program for MS-DOS 2.x
 *
 * Copyright (c) 1984, 1986  R. Edward Nather
 *
 * Define the symbolic constant MSC if compiling with Microsoft C v3.x;
 * Define the symbolic constant CIC86 if compiling with the CI-C86 C compiler.
 * Define the symbolic constant DESMET if compiling with DeSmet C.
 */

#include <stdio.h>
#define MAIN
#include "intdefs.h"

main(argc, argv)
int argc;
unsigned char *argv[];
{
unsigned char *s, *p;
int c = 0;
int nt = 0;

/* pre-set customizing flags */

allf = ALL;             /* include hidden & system files */
ll = LONG;              /* long listing */
colm = 0;               /* 1-column format */
id = ID;                /* show directory name */
rev = RSORT;            /* reverse sort */
tsrt = TSORT;           /* timesort the listing */
usage = DU;             /* print disk usage */

/* process input options */

while(--argc > 0 && (*++argv)[0] == '-') {
    for(s = argv[0]+1; *s != '\0'; s++) {
        switch(*s) {
            case 'a':                             /* -a: list all files */
                allf = (allf == 0);            /* reverse default value */
                break;
            case 'c':                 /* -c: 1-column listing requested */
                colm = (colm == 0);
                break;
            case 'i':               /* -i: suppress pathname identifier */
                id = (id == 0);
                break;
            case 'l':                     /* -l: long listing requested */
                ll = (ll == 0);
                break;
            case 'r':                     /* -r: reverse sort direction */
                rev = (rev == 0);
                break;
            case 's':                           /* -s: print sizes only */
                sizonly = 1;
                clsize = 0;
                if(*(s+1) == '1') {
                    clsize = ONECS;       /* diskuse for 1-sided floppy */
                    s++;
                    nt++;
                    }
                else if(*(s+1) == '2') {
                    clsize = TWOCS;                       /* or 2-sided */
                    s++;
                    nt++;
                    }
                break;
            case 't':                        /* -t: time sort requested */
                tsrt = (tsrt == 0);
                break;
            case 'u':                           /* -u: print disk usage */
                usage = (usage == 0);   
                break;
            case 'R':                   /* -R: recursively list subdirs */
                recd = 1;
                break;
            default:
                fprintf(stderr, "unknown arg %c\n", *s);
                exit(1);
            }
        }
    }

colm = (ll) ? colm ^ LCOLM : colm ^ SCOLM;           /* set listing format */
qs = setps();                          /* select proper pathname separator */
tsc = toscreen();               /* find out if output is to console screen */
obuf = (struct outbuf *)malloc(sizeof(*obuf));     /* point to free memory */

if(argc == 0) {
    argc++;
    (*argv)[0] = (*argv)[1] = '\0';
    }

for(;;) {                                    /* cycle through args present */
    getpath(spath, *argv++);              /* get drive & absolute pathname */
    if(usage || sizonly || ll)
        c = getsize(spath, nt);             /* get use data only if needed */
    if(c == 0)
        search(spath);                              /* go do the hard work */
    if(--argc > 0) 
        spath[0] = '\0';
    else {
        if(usage || sizonly) {
            if(np > 1) {
                fprintf(stdout, "-------\n%7ld bytes total", total);
                if(!nt)
                    fputs("; ", stdout);
                }
            if(!nt)
                fprintf(stdout, "%7ld bytes left on drive %c\n",
                    left, drive + 'a');
            }
        return;
        }
    }
}
