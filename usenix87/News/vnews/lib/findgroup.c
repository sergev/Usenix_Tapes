#include "defs.h"
#include "str.h"
#include "newsrc.h"


struct ngentry *
findgroup(name)
      register char *name ;
      {
      register struct ngentry *ngp ;
      extern int maxng ;

      for (ngp = ngtable; ngp < ngtable + MAXGROUPS ; ngp++) {
            if (ngp->ng_name && equal(ngp->ng_name, name))
                  return ngp ;
      }
      return 0 ;
}
