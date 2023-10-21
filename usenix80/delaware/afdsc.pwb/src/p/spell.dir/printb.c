#
#include "bbuf.h"

/* printb - c version of printf 
 * %d -- print d as a signed decimal number.
 * %l -- print long integer.
 * %o -- octal number
 * %s -- print s as a string.
 * %t -- time and date (no parameter required)
 * %#1.#2(option) as described in the UNIX manual, the first
 *        number describes the minimum field width, the second
 *        describes the maximum parameter conbersion width.
 */

struct bufr prbbuf;

int prnbn1, prnbn2;
printb (fmt,args) char *fmt; {
    register char  *s;
    register int  *ap;
    register int c;
    int x, y;
    int  tvec[2];

    if (prbbuf.bu_fd == 0) initb (1, &prbbuf);
    ap = &args;                 /*set ap to start of parms*/
    while(1)  {
        while((c = *fmt++) != '%') {
            if(c == '\0') {
                flushb (&prbbuf);
                return;
            }
            putbc (c, &prbbuf);
        }
        c = *fmt++;
        prnbn1 = 0; prnbn2 = 1000;      /* start with width params off */
        while (c <= '9' && c >= '0') {
                prnbn1 = prnbn1*10 + c - '0';
                c = *fmt++;
        }
        if (c == '.') {
                c = *fmt++;
                prnbn2 = 0;
                while (c <= '9' && c >= '0') {
                        prnbn2 = prnbn2*10 + c - '0';
                        c = *fmt++;
                }
        }
        switch (c) {
        case 'd':               /*decimal conbersion*/
            x = *ap++;
            if(x<0)  {
                x = -x;
                if(x<0)  {      /*minus infinity*/
                    prntbs ("-32768");
                    goto fill;
                    }
                prntbp ('-');
            }
            printd(x);
            goto fill;
        case 'l':
            x = *ap++;
            y = *ap++;
            printbl (x,y);
            goto fill;
        case 't':               /*print date and time*/
            time(tvec);
            s = ctime(tvec);
            s[24] = 0;
            goto scom;
        case 's':
            s = *ap++;
        scom:
            while(c = *s++)
                prntbp (c);
            goto fill;
        case 'c':
            prntbp (*ap++);
            goto fill;
        case '%':
            prntbp ('%');
            goto fill;
        case 'o':
            printo(*ap++);
            goto fill;
        default:
            prntbp (c);
            continue;

        fill:
            while (prnbn1-- > 0) putbc (' ', &prbbuf);
    }
}
}

/*print n in decimal; n must be non-negative*/
printd(n)
int n;
{
    register int  a, b;
    if (a=n/10)
        printd(a);
    b = n%10 + '0';
    prntbp (b);
}

/*print n in octal*/
printo(n)
int n;
{
    register int a,b;
    if (a = (n>>3)&017777)
        printo (a);
    b = (n&7) + '0';
    prntbp (b);
}

prntbp (c) int c; {
        if (--prnbn2 < 0) return;
        prnbn1--;
        putbc (c, &prbbuf);
}

prntbs (s) char *s; {
        register int c;
        while (c = *s++) prntbp (c);
}

/* print l as long integer */
prntbln (l)
long l;
{
    long a;
    register int b;
    a = l/10;
    if (a)
        prntbln(a);
    b = l%10 + '0';
    prntbp (b);
}

/* put - sign on neg long integers */
printbl (l)
long l;
{
    if (l < 0) {
        l = -l;
        prntbp ('-');
    }
    prntbln (l);
}
