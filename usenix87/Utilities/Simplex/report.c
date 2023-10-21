#include "simplex.h"

report()
{
     register int   i, j;
     double         y, dy, sigma;

     printf("\nProgram exited after %d iterations.\n\n", niter);

     printf("The final simplex is:\n");
     for (j=0 ; j<N ; j++) {
          for (i=0 ; i<N ; i++) {
               if ((i+1) % LW == 0)
                    printf("\n");
               printf("  %e", simp[j][i]);
          }
          printf("\n");
     }
     printf("\n\n");

     printf("The mean is:");
     for (i=0 ; i<N ; i++) {
          if ((i+1) % LW == 0)
               printf("\n");
          printf("  %e", mean[i]);
     }
     printf("\n\n");

     printf("The estimated fractional error is:");
     for (i=0 ; i<N ; i++) {
          if ((i+1) % LW == 0)
               printf("\n");
          printf("  %e", error[i]);
     }
     printf("\n\n");

     printf("   #         X              Y             Y''             DY\n");
     sigma = 0.0;
     for (i=0 ; i<np ; i++) {
          y = f(mean, data[i]);
          dy = data[i][1] - y;
          sigma += (dy*dy);
          printf("%4d  ", i);
          printf("%13e  %13e  ", data[i][0], data[i][1]);
          printf("%13e  %13e\n", y, dy);
     }
     printf("\n");
     sigma = sqrt(sigma);
     printf("The standard deviation is %e\n\n", sigma);
     sigma /= sqrt((double) (np-M));
     printf("The estimated error of the function is %e\n\n", sigma);
}
