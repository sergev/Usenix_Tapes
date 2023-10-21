/*
 * writestat - write name and stats to statfile for cfs
 */

#include "cfs.h"

writestat (fp, name, sbuf)
FILE   *fp;
char   *name;
struct stat *sbuf;
{
    struct stat dummy;

    fwrite (name, strlen (name) + 1, 1, fp);
    fwrite (sbuf, sizeof (dummy), 1, fp);
}
