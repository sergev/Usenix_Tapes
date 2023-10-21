#include "simplex.h"

first()
{
     register int   i, j;

     printf("Starting Simplex\n");
     for (j=0 ; j<N ; j++) {
          printf(" simp[%d]", j+1);
          for (i=0 ; i<N ; i++) {
               if ((i+1) % LW == 0)
                    printf("\n");
               printf("  %e", simp[j][i]);
          }
          printf("\n");
     }
     printf("\n");
}
