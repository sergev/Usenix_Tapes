#include <stdio.h>
#include "arc.h"

/*	split up a file name (subroutine for makefnam)
 *
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
*/

static INT _makefn(source,dest)
unsigned char *source;
unsigned char *dest;
{
 INT j;

    setmem (dest, 17, 0);	/* clear result field */
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
/*	make a file name using a template
*/

char *makefnam(rawfn,template,result)
unsigned char *rawfn;			/* the original file name */
unsigned char *template;		/* the template data */
unsigned char *result;			/* where to place the result */
{
  unsigned char et[17],er[17];

  _makefn(template,et);
  _makefn(rawfn,er);
  *result=0;			/* assure no data */
  strcat(result,er[0]?er:et);
  strcat(result,er[3]?er+3:et+3);
  strcat(result,er[12]?er+12:et+12);
  return ((char *)&result[0]);
}

INT freedir(dirs)
register struct direct **dirs;
{
	register INT ii;

	if(dirs == (struct direct **)0)
		return(-1);
	for(ii = 0; dirs[ii] != (struct direct *)0; ii++)
		free(dirs[ii]);
	free(dirs);
	return(0);
}

#if MSDOS
#include <dir.h>

INT alphasort(dirptr1, dirptr2)
struct direct **dirptr1, **dirptr2;
{
	return(strcmp((*dirptr1)->d_name, (*dirptr2)->d_name));
}

#endif
