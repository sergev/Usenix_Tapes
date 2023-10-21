#include "simplex.h"

enter(fname)
char *fname;
{
     register int   i, j;

     printf("SIMPLEX Optimization --- 'C' Version\n\n");
     printf("Accessing file: %s\n\n", fname);

     fscanf(fpdata, "%d", &maxiter);
     printf("maximum number of iterations = %d\n\n", maxiter);

     printf("Start Coordinates: ");
     for (i=0 ; i<M ; i++) {
          fscanf(fpdata, "%F", &simp[0][i]);
          if ((i+1) % LW == 0)
               printf("\n");
          printf(" %e", simp[0][i]);
     }
     printf("\n\n");

     printf("Start Steps: ");
     for (i=0 ; i<M ; i++) {
          fscanf(fpdata, "%F", &step[i]);
          if ((i+1) % LW == 0)
               printf("\n");
          printf(" %e", step[i]);
     }
     printf("\n\n");

     printf("Maximum Error: ");
     for (i=0 ; i<N ; i++) {
          fscanf(fpdata, "%F", &maxerr[i]);
          if ((i+1) % LW == 0)
               printf("\n");
          printf(" %e", maxerr[i]);
     }
     printf("\n\n");

     printf("DATA\n");
     printf("      X             Y\n");
     np = 0;
     while (!feof(fpdata)) {
          for (j=0 ; j<NVPP ; j++) {
               if (fscanf(fpdata, "%F", &data[np][j]) == EOF)
                    break;
               printf("  %e", data[np][j]);
          }
          np++;
          printf("\n");
     }
     np--;

}
