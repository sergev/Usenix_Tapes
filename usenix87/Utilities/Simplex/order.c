#include "simplex.h"

order()
{
     register int   i, j;

     for (j=0 ; j<N ; j++)
          for (i=0 ; i<N ; i++) {
               if (simp[i][j] < simp[l[j]][j])
                    l[j] = i;
               if (simp[i][j] > simp[h[j]][j])
                    h[j] = i;
          }
}
