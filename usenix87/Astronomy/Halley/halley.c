/*  halley.c:  A Halley Ephemeris Translated from Basic to C       */
/*             by Jeff Greenwald.  No rights reserved.             */

#include <stdio.h>
#include <math.h>
#include <ctype.h>

#define COSTR  "Comet Halley"
#define PH     1986.11
#define PL     170.011
#define AN     58.1453
#define PY     76.0081
#define SM     17.9435
#define EO     .967267
#define IO     162.239
#define PI     3.14159

main()
{
double Y, M, Z, S, X, DS, B, N, K, E, Q, U, Y1, V, V1, L, R, F, F2;
double F1, I, P, P1, T, T1, C, J, H, R1, U1, U2, Q1, Q2, Q3, Q4, Q5;
double E1, L1, M1, Y2, B1, G, H1, I1, J1, N1, W, K1, G1, D1, D2, R3;
double R2, D, K9, MO, MA, W1;

char c;
double dtoper(), degadj(), round(), floor();

/*  OPENING MESSAGE                                                */

printf ("       %s\n",COSTR);
printf ("------------------------------\n");
printf ("     Ephemeris For Dates\n");
printf ("     Between 1946 and 2026\n\n");
printf ("       Original BASIC version\n");
printf ("       by Roger Browne\n\n");
printf ("       Translated to C by\n");
printf ("       Jeff Greenwald\n");

/*  ENTER DATE                                                     */

for(;;) {
	do      {
		printf ("INPUT YEAR:\n");
		scanf  ("%lf",&Y);
		getchar();
		}

	while (Y < 1946 || Y > 2026);
	do      {
		printf ("INPUT MONTH\n");
		scanf  ("%lf",&M);
		getchar();
		}

	while (M < 1 || M > 12);
	do      {
		printf ("INPUT DAY\n");
		scanf  ("%lf",&D);
		getchar();
		}

	while (D < 1 || D > 31);

/*  CALCULATE COMET POSITION                                       */
        X = PH;
	if (Y >= 1986) Z = 1984, S = 0;
	if (Y < 1986) Z = 1988, S = 1;
	N = dtoper(Y,Z,S,X,M,D);
	DS = N;
	B = (360/PY) * (N/365.25);
	K = B;
	K = degadj(K);
	B = (K * PI)/180;
	E = B;
	Y1 = EO;

	for (;;) {
		Q = E - (Y1 * sin(E)) - B;
		if (fabs(Q) <= .000017) break;
		else    {
			U = Q / (1 - (Y1 * cos(E)));
			E = E - U;
			continue;
			}
		 }
	V = (sqrt ((1 + Y1) / (1 - Y1)) * tan (E/2));
	V = 2 * atan (V);
	V1 = (V * 180) / PI;
	L = V1 + PL;
	R = SM * (1 - (Y1 * Y1)) / (1 + Y1 * cos (V));
	F = L - AN;
	F2 = IO;
	F1 = (F * PI) / 180;
	F2 = (F2 * PI) / 180;
	I = sin (F1) * sin (F2);
	I = atan (I / sqrt(-I * I + 1));
	P = atan (tan (F1) * cos (F2));
	P1 = (P * 180) / PI + AN;

	if (F >= 90 && F <= 270) P1 += 180;
	if (P1 < 0) P1 += 360;
	P = (P1 * PI) / 180;
	R2 = R * cos (I);

/*  CALCULATE EARTH POSITION                                       */
        X = 1975;
        if (Y >= X)  Z = 1972, S = 0;
        if (Y < X)   Z = 1976, S = 1;
        N = dtoper(Y,Z,S,X,M,D);
        T = (360 / 365.25) * (N / 1.00004);
        T = degadj(T);
        T1 = (T * PI) / 180;
        C = .01672;
	J = T + (360 / PI) * C * sin(T1 - .051943);
	J += 99.5343;

	if (J > 360)  J -= 360;
	if (J < 0)    J += 360;

        H = ((J - 102.51044) * PI) / 180;
	R1 = (1 - C * C) / (1 + C * cos(H));

/*  ECLIPTIC COORDINATES                                           */
        U1 = ((P1 - J) * PI) / 180;
        U2 = ((J - P1) * PI) / 180;
        if (R2 < R1)
                {
                Q3 = R2 * sin(U2);
                Q3 = Q3 / (R1 - (R2 * cos(U2)));
                Q3 = atan(Q3);
                Q2 = (Q3 * 180) / PI + P1;
                }
	else    {
		Q1 = R1 * sin(U1);
		Q1 = Q1 / (R2 - (R1 * cos(U1)));
		Q1 = atan(Q1);
		Q2 = (Q1 * 180) / PI + P1;
                }

	if (Q2 > 360) Q2 -= 360;
	if (Q2 < 0)   Q2 += 360;
	Q4 = (Q2 * PI) / 180;
	Q5 = R2 * tan(I) * sin(Q4 - P);
	Q5 = Q5 / (R1 * sin(U1));
        Q5 = atan(Q5);

/*  CONVERT TO EQUATORIAL COORDINATES                              */
        E1 = .40893064;
        L1 = sin(Q5) * cos (E1);
        L1 = L1 + (cos(Q5) * sin(E1) * sin(Q4));
        M1 = atan(L1 / sqrt(-L1 * L1 + 1));
        Y2 = (M1 * 180) / PI;
	B1 = tan(Q4) * cos(E1);
        B1 = B1 - ((tan(Q5) * sin(E1)) / cos(Q4));
        G = atan(B1);
        H1 = (G * 180) / PI;
        I1 = floor(Q2 / 90);
        J1 = floor(H1 / 90);

        if (I1 - J1 == 4 || I1 - J1 == 1) H1 += 360;
        if (I1 - J1 == 2 || I1 - J1 == 3) H1 += 180;
        if (I1 - J1 == -4) H1 += 360;
        if (I1 - J1 == -2) H1 -= 180;

        N1 = H1 / 15;
        W = floor((N1 - floor(N1)) * 60 + .5);
        if (W == 60) N1 = N1 + 1, W = 0;
        K1 = fabs(Y2);
	W1 = floor((K1 - floor(K1)) * 60 + .5);
        if (W1 == 60)  K1 = K1 + 1, W1 = 0;

	G1 = floor(K1);
	if (Y2 < 0 && G1 < 1)  W1 = -W1;
	D1 = R1 * R1 + R2 * R2;
	D1 = D1 - (2 * R1 * R2 * cos(U1));
	D2 = sqrt(D1);
	R3 = D2 / cos(I);
	R = round(R);
	K9 = R3 / 10;
	K9 = round(K9);
	R3 = K9 * 10;
	MO = 4.1;
	N = 3.1;
	if (DS < 0) MO = 5, N = 4.44;

	MA = MO + 5 * .4343 * log(R3);
	MA = MA + N * 2.5 * .4343 * log(R);
	MA = (floor(10 * MA)) / 10;
	if (Y2 < 0)  G1 = -G1;

/*  OUTPUT ROUTINE                                                 */
printf ("\n");
printf ("------------------------------\n");
printf ("Data For %s\n",COSTR);
printf ("DATE:  M/D/Y = %g/%g/%g\n",M,D,Y);
printf ("Days to perihelion:  %g\n",floor(DS));
printf ("\n\n");
printf ("Coordinates:\n");
printf ("  RA:  %g HRS %g MIN \n",floor(N1),W);
printf ("  DEC:  %g DEG %g MIN \n",G1,W1);
printf ("\n\n");
printf ("Distances:\n");
printf ("  Comet To Sun:  %g AU\n",R);
printf ("  Comet To Earth:  %g AU\n",R3);
printf ("\n");
printf ("Predicted Mag.:  %g\n",MA);
printf ("------------------------------\n");
printf ("Enter X <cr> To Terminate\n");
printf ("Or Enter Any Other Key <cr> To Enter Another Date\n");


c = getchar();
if (isupper(c))
  c = tolower(c);
if (c != 'x' )  continue;
else break;
	}

/*  END MAIN                                                        */
exit(0);
}

/*  SUBROUTINE "DTOPER":  Days to perihelion                        */
double dtoper (Y,Z,S,X,M,D)
double Y,Z,S,X,M,D;

{
double A,A1,M2,N;
A = (Y - Z)/4;
A1 = floor(A + S);
N = 365 * ((Y - X) + S) + A1;

if (floor(A) == A)
	{
        if ((M == 2 && D < 29) || M == 1)  N = N - 1;
        }

if (M <= 2)
        {
        M2 = M - 1;
        M2 *= 31;
        }
        else
                {
                M2 = M + 1;
                M2 = floor(30.6 * M2) - 63;
                }

N = N + M2 + D - 365 * S;
return(N);
}

/*  SUBROUTINE "DEGADJ":  Adjust to a value from 0 to 360 degrees   */
double degadj(K)
double K;
{
while (K < 0)
        {
        K += 360;
        if (K >= 0)  return(K);
        else  continue;
        }

while (K > 360)
	{
	K -= 360;
	if (K <= 360)  return(K);
	else  continue;
	}

return(K);
}

/*  SUBROUTINE "ROUND":  Rounding function                          */
double round(K9)
double K9;

{
K9 = K9 * 1000;
K9 = floor(K9 + .5);
K9 = K9 / 1000;
return(K9);
}
