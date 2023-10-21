#include <stdio.h>
#include "af.h"


int affd ;			/* artfile file descriptor */
struct afheader afhd ;		/* artfile header */


genafopen(file, mode)
      char *file, *mode ;
      {
      int omode = mode[1] ? 2 : 0 ;

      if ((affd = open(file, omode)) < 0
       && (sleep(2), affd = open(file, omode)) < 0)
            xerror("can't open %s", file) ;
      if (read(affd, (char *)&afhd, sizeof(afhd)) != sizeof afhd)
            xerror("can't read data base header") ;
      if (afhd.af_magic != AF_MAGIC)
            xerror("bad data base file") ;
      if (afhd.af_version != AF_VERSION)
            xerror("Version of article data base is wrong") ;
}
