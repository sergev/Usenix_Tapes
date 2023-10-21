/*
 * Unless the newsgroup is asked for explicitly, delete it from the
 * subscription list.
 */

#include <stdio.h>
#include "config.h"
#include "str.h"

makehimask(sublist, ngrp)
      register char *sublist ;
      char *ngrp ;
      {
      char c, *p ;
      char *index() ;

      for (;;) {
            if (prefix(sublist, ngrp)
             && (c = sublist[strlen(ngrp)]) == '\0' || c == ',')
                  return ;
            if ((p = index(sublist, ',')) == NULL)
                  break ;
            sublist = p + 1 ;
      }
      while (*sublist)
            sublist++ ;
      if (sublist[-1] != ',')
            *sublist++ = ',' ;
      *sublist++ = '!' ;
      scopy(ngrp, sublist) ;
}
