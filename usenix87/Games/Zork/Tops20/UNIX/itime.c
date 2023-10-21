#include <time.h>

/*
 * Function itime_(hour, min, sec)
 *
 *	Return the obvious to a FORTRAN program.
 */
itime_(T_h, T_m, T_s)
 int *T_h, *T_m, *T_s;
{
	long T_tme;
	struct tm *T_tp;

	time(&T_tme);
	T_tp = localtime(&T_tme);
	*T_h = T_tp->tm_hour;
	*T_m = T_tp->tm_min;
	*T_s = T_tp->tm_sec;
}

/*
 * Function idate_(year, month, day)
 *
 *	Return the obvious to a FORTRAN program
 */
idate_(D_y, D_m, D_d)
 int *D_y, *D_m, *D_d;
{
	long D_tme;
	struct tm *D_tp;

	time(&D_tme);
	D_tp = localtime(&D_tme);
	*D_y = D_tp->tm_year;
	*D_m = D_tp->tm_mon;
	*D_d = D_tp->tm_mday;
}
