#include <stdio.h>
#include "intdefs.h"

/* search - search 'path' for filename or directory */

search(path)
unsigned char *path;
{
struct dta dta;                 /* DOS file data table */
extern struct outbuf;           /* array of file structs */
int pl;                         /* length of initial path */
register int z;                 /* char counter */
register int k = 0;             /* counts number of entries found */
int dironly = 0;                /* dirname (only) requested if 1 */
unsigned char work[80];         /* working path string */
int comp();                     /* string, time comparison routine */
int mask = 0x0010;              /* default attribute mask */
long bytes = 0;                 /* count of disk usage this directory */

if(allf)
    mask = 0x001F;
strcpy(work,path);
pl = strlen(work);                           /* save original path length */

if(work[pl-1] == '/' || work[pl-1] == '\\') {
    dironly = 1;                          /* is a dirname, not a filename */
    strcat(work,"*.*");
    }
else if(!find_first(work, &dta, 6)) {
    strcat(work, &qs);             /* not a normal, system or hidden file */
    strcat(work,"*.*");           /* so it must be a directory - add path */
    pl++;                          /* separator and list everything in it */
    }                  

if(find_first(work, &dta, mask)) {
    do {
        if(dta.attr & 0x08)                        /* ignore volume label */
            continue;
        if(dta.fname[0] == '.' && !allf)              /* unless -a option */
            continue;                              /* ignore "." and ".." */

        obuf[k].oattr = dta.attr;                     /* stash this entry */
        obuf[k].otime = dta.ftime;
        obuf[k].odate = dta.fdate;
        obuf[k].osize = dta.fsize;
        strcpy(obuf[k].oname, dta.fname);

        if(usage || sizonly) {
            if((dta.attr & 0x10) && dta.fname[0] != '.') {
                bytes += clsize;                     /* sum up disk usage */
                }
            else if(dta.fsize) {
                obuf[k].osize = ((dta.fsize + clmask) & (long)(~clmask));
                bytes += obuf[k].osize;
                }
            }

        k++;
        } while(find_next(&dta));
    }
else {
    work[pl] = NULL;
    if(dironly)
        fprintf(stderr, 
           "\nCan't find any entries in a directory named \"%s\"\n", work);
    else {
        if(work[pl-1] == qs)
            work[pl-1] = NULL;
        fprintf(stderr, 
           "\nCan't find a file or directory named \"%s\"\n", work);
        }
    return;
    }

if(pl > 3 && work[pl - 1] == qs)
	pl--;
work[pl] = NULL;                            /* restore directory pathname */
if(np++ && !sizonly)
    fputc(endlin(), stdout);                   /* separate listing blocks */
if(usage || sizonly) {
    total += bytes;                                /* total bytes to date */
    fprintf(stdout, "%7ld  ", bytes);
    }
if(id || usage || sizonly) {
    PUTS(work);                                     /* identify the block */
    fputc(endlin(), stdout);
    }
if(!sizonly) {
    qsort(obuf,k,sizeof(obuf[0]),comp);               /* sort the entries */
    if(ll)
        longlist(k);                                    /* and print them */
    else
        shortlist(k);
    }
if(!recd)
    return;                                             /* quit if not -R */
if(work[pl - 1] != qs) {
    work[pl++] =  qs;
    work[pl] = NULL;
    }
strcat(work, "*.*");
if(find_first(work, &dta, mask))                /* else find all sub-dirs */
    do  {
        if(dta.attr & 0x10 && dta.fname[0] != '.') {
            work[pl] = 0;                             /* discard old name */
            for(z=0; dta.fname[z] != NULL; z++)
                dta.fname[z] = tolower(dta.fname[z]);
            strcat(work, dta.fname);                 /* install a new one */
            strcat(work, &qs);
            search(work);                                  /* and recurse */
            }
    } while(find_next(&dta));
return;
}
