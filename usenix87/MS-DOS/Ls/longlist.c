#include <stdio.h>
#include "intdefs.h"

/* longlist - list everything about files in two columns */

struct llst {               /* structure to hold file information */
    unsigned char *fattr;   /* file attribute pointer */
    long size;              /* file size */
    int day;                /* the day of creation */
    int mnum;               /* month number */
    int yr;
    int hh;                 /* creation times */
    int mm;
    int ap;                 /* am or pm */
    } l;

longlist(k)
int k;          /* total number to list */
{

register int i, m, n;
int cdate;
unsigned char *mon, *mname();

cdate = gcdate();                         /* get current date (in months) */
if(colm)
    n = k;                                    /* set for 1 column listing */
else
    n = (k + 1)/2;                             /* or for 2 column listing */
for(i=0; i < n; i++){
    for(m = 0; (m+i) < k; m += n) {     
        fill(i+m, &l);                             /* fill llst structure */
        mon = mname(l.mnum);                      /* conv month # to name */

#ifdef MSC
        if(tsc){
            cprintf("%s%7ld  %2d %s ", l.fattr, l.size, l.day, mon);
            if(cdate >= (l.yr * 12 +l.mnum) + 12)
                cprintf(" %4d  ", l.yr);         /* print year if too old */
            else 
                cprintf("%2d:%02d%c ", l.hh, l.mm, l.ap);    /* else time */
            }
        else
#endif
            {
            fprintf(stdout, "%s%7ld  %2d %s ", l.fattr, l.size, l.day, mon);
            if(cdate >= (l.yr * 12 +l.mnum) + 12)
                fprintf(stdout, " %4d  ", l.yr);
            else 
                fprintf(stdout, "%2d:%02d%c ", l.hh, l.mm, l.ap);
            }
        putname(i+m);
        if(m+n < k)
            fputs(BAR, stdout);                   /* double bar separator */
        }
    fputc(endlin(), stdout);
    }
return;
}

/* fill - fill long list structure with file information */

fill(i, ll)
register int i;
struct llst *ll;
{
register int j, k;
static unsigned char fbuf[16][4] = {
    "---",
    "--r",
    "-h-",
    "-hr",
    "s--",
    "s-r",
    "sh-",
    "shr",
    "d--",
    "d-r",
    "dh-",
    "dhr",
    "d--",
    "d-r",
    "dh-",
    "dhr"
    };

if((obuf[i].oattr & 0x10) && obuf[i].oname[0] != '.')
    ll->size = clsize;                    /* if directory, use block size */
else
    ll->size = obuf[i].osize;                       /* else use file size */
j = (obuf[i].oattr & 0x10) ? 8 : 0;               /* set file attr offset */
ll->fattr = fbuf[(obuf[i].oattr & 0x07) + j];   /* point to symbolic attr */
ll->day = obuf[i].odate & 0x1F;
ll->mnum = (obuf[i].odate >> 5) & 0x0F;
ll->yr = (obuf[i].odate >> 9) + 1980;
k = obuf[i].otime >> 5;                                /* this is a mess */
ll->mm = k & 0x3f;
ll->ap = ((ll->hh = k >> 6) >= 12) ? 'p' : 'a';
if(ll->hh > 12)
    ll->hh -= 12;
if(ll->hh == 0)
    ll->hh = 12;
return;
}
