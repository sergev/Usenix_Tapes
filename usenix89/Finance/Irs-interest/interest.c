
/*************************************************************************
 * interest.c
 *
 * IRS Interest Rate Calculations
 *	based on IRS Interest Rate Table
 *
 *  by John M. Ritter
 *     Allied-Signal, Inc.
 *     Corporate Tax Department
 *     Morristown, NJ
 *
 *  The author and Allied-Signal, Inc. make no warranty as to the design,
 *  capability, capacities, or suitability for use of this program. The
 *  program is distributed on an "as is" basis without warranty. The
 *  program may be freely distributed and used provided this message
 *  remains intact.
 *
 *************************************************************************/

#define		ROWS		17	/* number of rows in interest table */
#define		SIMPLE		0	/* simple interest calculation */
#define		COMPOUND	1	/* compound interest calculation */
#define		YEAR_END	87	/* year table ends */
#define		DAY_END		181	/* julian day of YEAR_END */
#include	<stdio.h>

static int day_tab[2][13] = {		/* Days in each month */
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, /* un-leap year */
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, /* leap year */
};

static int int_tab[ROWS][4] = {		/* interest rate table */
/*    epoch - 06/30/75 */  {   1,  6,   SIMPLE, 0},
/* 07/01/75 - 12/31/75 */  { 184,  6,   SIMPLE, 0},
/* 01/01/76 - 01/31/76 */  { 215,  9,   SIMPLE, 1},
/* 02/01/76 - 12/31/76 */  { 550,  7,   SIMPLE, 1},
/* 01/01/77 - 01/31/78 */  { 946,  7,   SIMPLE, 0},
/* 02/01/78 - 12/31/79 */  {1645,  6,   SIMPLE, 0},
/* 01/01/80 - 01/31/80 */  {1676,  6,   SIMPLE, 1},
/* 02/01/80 - 12/31/80 */  {2011, 12,   SIMPLE, 1},
/* 01/01/81 - 01/31/82 */  {2407, 12,   SIMPLE, 0},
/* 02/01/82 - 12/31/82 */  {2741, 20,   SIMPLE, 0},
/* 01/01/83 - 06/30/83 */  {2922, 16, COMPOUND, 0},
/* 07/01/83 - 12/31/83 */  {3106, 11, COMPOUND, 0},
/* 01/01/84 - 12/31/84 */  {3472, 11, COMPOUND, 1},
/* 01/01/85 - 06/30/85 */  {3653, 13, COMPOUND, 0},
/* 07/01/85 - 12/31/85 */  {3837, 11, COMPOUND, 0},
/* 01/01/86 - 06/30/86 */  {4018, 10, COMPOUND, 0},
/* 07/01/86 - 06/30/87 */  {4383,  9, COMPOUND, 0},
};

main()
{
	int ddd[2], month[2], day[2], yyddd[2], year[2];
	int no_days, offset, row;
	int days, counter;
	double interest, tinterest, nprinciple, principle;

	tinterest = 0.0;

	printf("Enter start date (mm/dd/yy): ");
	indate(&month[0], &day[0], &year[0]);
	printf("  Enter end date (mm/dd/yy): ");
	indate(&month[1], &day[1], &year[1]);
	printf("            Enter principle: ");
	scanf("%f", &principle);
	nprinciple = principle;

	ddd[0] = to_ddd(year[0], month[0], day[0]);
	yyddd[0] = (year[0] * 1000) + ddd[0];
	ddd[1] = to_ddd(year[1], month[1], day[1]);
	yyddd[1] = (year[1] * 1000) + ddd[1];

	no_days = diffdays(year[0], ddd[0], year[1], ddd[1]);
	if (no_days != EOF) {
		printf("Calculating interest for %d days.\n", no_days);
	} else {
		printf("Start date must be before end date.\n");
		exit(1);
	}

	offset = int_tab[ROWS - 1][0] - diffdays(year[0], ddd[0], YEAR_END, DAY_END);
	for (row = 0; row < ROWS; ++row)
		if (int_tab[row][0] > offset)
			break;

	days = int_tab[row][0] - offset;
	if (no_days > days) {
		intcalc(days, row, &principle, &interest);
		tinterest += interest;
		no_days -= days;
		if (row == 9)
			principle += tinterest;
		++row;
	}

	while (no_days > 0) {
		if (no_days > (int_tab[row][0] - int_tab[row - 1][0])) {
			days = int_tab[row][0] - int_tab[row - 1][0];
			intcalc(days, row, &principle, &interest);
			tinterest += interest;
			no_days -= days;
			if (row == 9)
				principle += tinterest;
			++row;
		} else {
			intcalc(no_days, row, &principle, &interest);
			tinterest += interest;
			no_days = 0;
		}
	}
	printf("\nprinciple = $%-12.2f\n", nprinciple);
	printf(" interest = $%-12.2f\n", tinterest);
	printf("    total = $%-12.2f\n", nprinciple + tinterest);
	exit(0);
}

/*************************************************************************
 * diffdays()
 *	find the number of days between two dates
 *************************************************************************/
diffdays(yy1, ddd1, yy2, ddd2)
int yy1, ddd1, yy2, ddd2;
{
	int duration = 0;

	if (yy1 > yy2)			/* can't go back in time yet */
		return(EOF);

	if (yy1 == yy2)			/* same year - just subtract */
		if (ddd1 > ddd2)	/* don't want to go back, but could */
			return(EOF);
		else
			return(ddd2 - ddd1);

	duration += ddd2;		/* add current year days */
	duration += (365 - ddd1);	/* add starting year days */
	if (isleap(yy1))
		duration += 1;		/* adjust for leap year */
	++yy1;				/* ready to start at January */

	while (yy1 < yy2) {		/* add complete years */
		if (isleap(yy1))
			duration += 366;
		else
			duration += 365;
		++yy1;
	}
	return(duration);
}

/*************************************************************************
 * indate()
 *	get 3 integers between slashes
 *************************************************************************/
indate(month, day, year)
int *month, *day, *year;
{
	scanf("%d/%d/%d", month, day, year);
	if (*month < 1 || *month > 12) {
		printf("Invalid month!\n");
		exit(1);
	}
	if (*day < 1 || *day > day_tab[isleap(*year)][*month]) {
		printf("Invalid day!\n");
		exit(1);
	}
}

/*************************************************************************
 * intcalc()
 *	calculate interest on principle for "days" using
 *	interest rate table
 *************************************************************************/
intcalc(days, row, principle, interest)
int days, row;
double *principle, *interest;
{
	double daily_rate, tint;
	int i;

	if (int_tab[row][3] == 0)
		daily_rate = (int_tab[row][1] / 365.0) / 100.0;
	else
		daily_rate = (int_tab[row][1] / 366.0) / 100.0;

	if (int_tab[row][2] == SIMPLE)
		*interest = days * daily_rate * *principle;

	if (int_tab[row][2] == COMPOUND) {
		*interest = 0.0;
		for (i = 0; i < days; ++i) {
			tint = daily_rate * *principle;
			*principle += tint;
			*interest += tint;
		}
	}
}

/*************************************************************************
 * isleap()
 *	is "year" a leap year?
 *************************************************************************/
isleap(year)
int year;
{
	year += 1900;
	if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0)
		return(1);
	else
		return(0);
}

/*************************************************************************
 * to_ddd()
 *	return julian day of year for date
 *************************************************************************/
to_ddd(year, month, day)
int year, month, day;
{
	int i;

	for (i = 1; i < month; i++)
		day += day_tab[isleap(year)][i];
	return(day);
}
