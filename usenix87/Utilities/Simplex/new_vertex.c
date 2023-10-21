#include "simplex.h"

new_vertex()
{
     register int   i;

     printf(" --- %4d", niter);
     for (i=0 ; i<N ; i++) {
          simp[h[N-1]][i] = next[i];
          printf("  %e", next[i]);
     }
     printf("\n");
}
