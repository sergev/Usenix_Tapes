#include "simplex.h"

#define until(x) while (!(x))

int       h[N], l[N];
int       np, maxiter, niter;
vector    next, mean, error, maxerr, step, simp[N];
datarow   data[MNP];
FILE      *fpdata;

main(argc, argv)
int   argc;
char  *argv[];
{
   register int   i, j, done;
   vector         center, p, q;

   if (argc != 2) {
      fprintf(stderr, "usage: simplex file_name\n", argv[1]);
      exit(1);
   }

   if ((fpdata = fopen(argv[1], "r")) == NULL) {
      fprintf(stderr, "simplex: can't open %s\n", argv[1]);
      exit(1);
   }

   enter(argv[1]);

   /* First Vertex */
   sum_residual(simp[0]);

   /* Compute offset of Vertices */
   for (i=0 ; i<M ; i++) {
      p[i] = step[i] * (sqrt((double) N) + M - 1) / (M * ROOT2);
      q[i] = step[i] * (sqrt((double) N) - 1) / (M * ROOT2);
   }

   /* All Vertices of the Starting Simplex */
   for (i=1 ; i<N ; i++) {
      for (j=0 ; j<M ; j++)
         simp[i][j] = simp[0][j] + q[j];
      simp[i][i-1] = simp[0][i-1] + p[i-1];
      sum_residual(simp[i]);
   }

   /* Preset */
   for (i=0 ; i<N ; i++) {
      l[i] = 1;
      h[i] = 1;
   }
   order();

   first();

   niter = 0;

   /* Iterate */
   do {
      /* Wish it were True */
      done = 1;
      niter++;

      /* Compute Centroid...Excluding the Worst */
      for (i=0 ; i<N ; i++)
         center[i] = 0.0;
      for (i=0 ; i<N ; i++)
         if (i != h[N-1])
            for (j=0 ; j<M ; j++)
               center[j] += simp[i][j];

      /* First Attempt to Reflect */
      for (i=0 ; i<N ; i++) {
         center[i] /= M;
         next[i] = (1.0+ALPHA) * center[i] - ALPHA * simp[h[N-1]][i];
      }
      sum_residuals(next);

      if (next[N-1] <= simp[l[N-1]][N-1]) {
         new_vertex();
         for (i=0 ; i<M ; i++)
            next[i] = GAMMA * simp[h[N-1]][i] + (1.0-GAMMA) * center[i];
         sum_residual(next);
         if (next[N-1] <= simp[l[N-1]][N-1])
            new_vertex();
      }
      else {
         if (next[N-1] <= simp[h[N-1]][N-1])
            new_vertex();
         else {
            for (i=0 ; i<M ; i++)
               next[i] = BETA * simp[h[N-1]][i] + (1.0-BETA) * center[i];
            sum_residual(next);
            if (next[N-1] <= simp[h[N-1]][N-1])
               new_vertex();
            else {
               for (i=0 ; i<N ; i++) {
                  for (j=0 ; j<M ; j++)
                     simp[i][j] = BETA * (simp[i][j] + simp[l[N-1]][j]);
                  sum_residual(simp[i]);
               }
            }
         }
      }

      order();

      /* Check For Convergence */
      for (j=0 ; j<N ; j++) {
         error[j] = (simp[h[j]][j] - simp[l[j]][j]) / simp[h[j]][j];
         if (done)
            if (error[j] > maxerr[j])
               done = 0;
      }

   } until(done || (niter == maxiter));

   /* Average Each Parameter */
   for (i=0 ; i<N ; i++) {
      mean[i] = 0.0;
      for (j=0 ; j<N ; j++)
         mean[i] += simp[j][i];
      mean[i] /= N;
   }

   report();
}
