/*  ARC - Archive utility - ARCMISC

System V Version 1.0.

*/
#include <ctype.h>
#include "arc.h"

static INT _makefn(source,dest)
unsigned char *source;
unsigned char *dest;
{
    int j;

    setmem(dest, 17, 0);             /* clear result field */
    if (strlen (source) > 1 && source[1] == ':')
    for (j = 0; j < 2;)
        dest[j++] = *source++;
    for (j = 3; *source && *source != '.'; ++source)
        if (j < 11)
            dest[j++] = *source;
    for (j = 12; *source; ++source)
        if (j < 16)
            dest[j++] = *source;
}

/*    make a file name using a template
*/
char *makefnam(rawfn,template,result)
unsigned char *rawfn;                /* the original file name */
unsigned char *template;             /* the template data */
unsigned char *result;               /* where to place the result */
{
    unsigned char et[17],er[17];

    _makefn(template,et);
    _makefn(rawfn,er);
    *result=0;                       /* assure no data */
    strcat(result,er[0]?er:et);
    strcat(result,er[3]?er+3:et+3);
    strcat(result,er[12]?er+12:et+12);
    return ((char *)&result[0]);
}

/*  convert a string to upper case  */
upper(s) char *s; {
    while (*s = toupper(*s)) ++s;
}

setmem(dest,size,c) char *dest,c; INT size; {
    int i;

    for (i = 0; i < size; dest[i] = c, i++);
}

abort(s,arg1,arg2,arg3) char *s; {
    fprintf(stderr,"arc: ");
    fprintf(stderr,s,arg1,arg2,arg3);
    fprintf(stderr,"\n");
    perror("arc system:");
    exit(1);
}

rename(o, n) char *o, *n; {
    return link(o, n) || unlink(o);
}
