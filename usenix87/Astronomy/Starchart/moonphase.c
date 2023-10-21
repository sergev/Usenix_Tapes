/****************************************************************************
 pom.c

     Phase of the Moon. Calculates the current phase of the moon.
     Based on routines from `Practical Astronomy with Your Calculator',
        by Duffett-Smith.
     Comments give the section from the book that particular piece
        of code was adapted from.

     -- Keith E. Brandt  VIII 1984

 ****************************************************************************/

#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#define PI         3.141592654
#define EPOCH   1983
#define EPSILONg 279.103035     /* solar ecliptic long at EPOCH */
#define RHOg     282.648015     /* solar ecliptic long of perigee at EPOCH */
#define e          0.01671626   /* solar orbit eccentricity */
#define lzero    106.306091     /* lunar mean long at EPOCH */
#define Pzero    111.481526     /* lunar mean long of perigee at EPOCH */
#define Nzero     93.913033     /* lunar mean long of node at EPOCH */

main()  {

double dtor();
double adj360();
double potm();

long *lo = (long *) calloc (1, sizeof(long)); /* used by time calls */
struct tm *pt; /* ptr to time structure */

double days;   /* days since EPOCH */
double phase;  /* percent of lunar surface illuminated */
double phase2; /* percent of lunar surface illuminated one day later */
int i = EPOCH;

time (lo);  /* get system time */
pt = gmtime(lo);  /* get ptr to gmt time struct */
cfree(lo);

/* calculate days since EPOCH */
days = (pt->tm_yday +1) + ((pt->tm_hour + (pt->tm_min / 60.0)
       + (pt->tm_sec / 3600.0)) / 24.0);
while (i < pt->tm_year + 1900)
   days = days + 365 + ly(i++);

phase = potm(days);
printf("The Moon is ");
if ((int)(phase + .5) == 100) {
   printf("Full\n");
   }
else if ((int)(phase + 0.5) == 0) 
   printf("New\n");
else if ((int)(phase + 0.5) == 50)  {
   phase2 = potm(++days);
   if (phase2 > phase)
      printf("at the First Quarter\n");
   else 
      printf("at the Last Quarter\n");
   }
else if ((int)(phase + 0.5) > 50) {
   phase2 = potm(++days);
   if (phase2 > phase)
      printf("Waxing ");
   else 
      printf("Waning ");
   printf("Gibbous (%1.0f%% of Full)\n", phase);
   }
else if ((int)(phase + 0.5) < 50) {
   phase2 = potm(++days);
   if (phase2 > phase)
      printf("Waxing ");
   else
      printf("Waning ");
   printf("Crescent (%1.0f%% of Full)\n", phase);
   }
}

double potm(days)
double days;
{
double N;
double Msol;
double Ec;
double LambdaSol;
double l;
double Mm;
double Ev;
double Ac;
double A3;
double Mmprime;
double A4;
double lprime;
double V;
double ldprime;
double D;
double Nm;

N = 360 * days / 365.2422;  /* sec 42 #3 */
adj360(&N);

Msol = N + EPSILONg - RHOg; /* sec 42 #4 */
adj360(&Msol);

Ec = 360 / PI * e * sin(dtor(Msol)); /* sec 42 #5 */

LambdaSol = N + Ec + EPSILONg;       /* sec 42 #6 */
adj360(&LambdaSol);

l = 13.1763966 * days + lzero;       /* sec 61 #4 */
adj360(&l);

Mm = l - (0.1114041 * days) - Pzero; /* sec 61 #5 */
adj360(&Mm);

Nm = Nzero - (0.0529539 * days);     /* sec 61 #6 */
adj360(&Nm);

Ev = 1.2739 * sin(dtor(2*(l - LambdaSol) - Mm)); /* sec 61 #7 */

Ac = 0.1858 * sin(dtor(Msol));       /* sec 61 #8 */
A3 = 0.37 * sin(dtor(Msol));

Mmprime = Mm + Ev - Ac - A3;         /* sec 61 #9 */

Ec = 6.2886 * sin(dtor(Mmprime));    /* sec 61 #10 */

A4 = 0.214 * sin(dtor(2 * Mmprime)); /* sec 61 #11 */

lprime = l + Ev + Ec - Ac + A4;      /* sec 61 #12 */

V = 0.6583 * sin(dtor(2 * (lprime - LambdaSol))); /* sec 61 #13 */

ldprime = lprime + V;                /* sec 61 #14 */

D = ldprime - LambdaSol;             /* sec 63 #2 */

return (50 * (1 - cos(dtor(D))));    /* sec 63 #3 */
}

ly(yr)
int yr;
{
/* returns 1 if leapyear, 0 otherwise */
return (yr % 4 == 0 && yr % 100 != 0 || yr % 400 == 0);
}

double dtor(deg)
double deg;
{
/* convert degrees to radians */
return (deg * PI / 180);
}

double adj360(deg)
double *deg;
{
/* adjust value so 0 <= deg <= 360 */
do if (*deg < 0)
   *deg += 360;
else if (*deg > 360)
   *deg -= 360;
while (*deg < 0 || *deg > 360);
}


