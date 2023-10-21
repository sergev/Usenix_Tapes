/*LINTLIBRARY*/
int      percent_d(x) int      x; { return x; }
int      percent_o(x) int      x; { return x; }
int      percent_x(x) int      x; { return x; }
long     percent_D(x) long     x; { return x; }
long     percent_O(x) long     x; { return x; }
long     percent_X(x) long     x; { return x; }
double   percent_e(x) double   x; { return x; }
double   percent_f(x) double   x; { return x; }
double   percent_g(x) double   x; { return x; }
double   percent_E(x) double   x; { return x; }
double   percent_F(x) double   x; { return x; }
double   percent_G(x) double   x; { return x; }
unsigned percent_u(x) unsigned x; { return x; }
int      percent_c(x) int      x; { return x; }
char *   percent_s(x) char *   x; { return x; }

int      percent_star(x) int   x; { return x; }

int *      percent_d_ptr(x)  int *      x; { return x; }
int *      percent_o_ptr(x)  int *      x; { return x; }
int *      percent_x_ptr(x)  int *      x; { return x; }
short *    percent_hd_ptr(x) short *    x; { return x; }
short *    percent_ho_ptr(x) short *    x; { return x; }
short *    percent_hx_ptr(x) short *    x; { return x; }
long *     percent_D_ptr(x)  long *     x; { return x; }
long *     percent_O_ptr(x)  long *     x; { return x; }
long *     percent_X_ptr(x)  long *     x; { return x; }
float *    percent_e_ptr(x)  float *    x; { return x; }
float *    percent_f_ptr(x)  float *    x; { return x; }
float *    percent_g_ptr(x)  float *    x; { return x; }
double *   percent_E_ptr(x)  double *   x; { return x; }
double *   percent_F_ptr(x)  double *   x; { return x; }
double *   percent_G_ptr(x)  double *   x; { return x; }
unsigned * percent_u_ptr(x)  unsigned * x; { return x; }

int *      percent_c_ptr(x)  int *      x; { return x; }
char *     percent_bkt(x)    char *     x; { return x; }

/* NOTE: not all C compilers support unsigned long! - If your compiler rejects
 * the following lines, replace "unsigned long" with just "long"
 */
unsigned long   percent_U(x)     unsigned long   x; { return x; }
unsigned long * percent_U_ptr(x) unsigned long * x; { return x; }
