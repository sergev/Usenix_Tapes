#include <stdio.h>

struct country_years
{
    short  greg_switch;		/* year according to Christian era  */
    int    (*weird_years)();	/* function pointer for early years */
};


#include "year.tbl"



/**********************************************************************
*
*  days_in_year() -- give number of days for particular year and country
*
*     N.B. countries and their borders have changed a bit over time ...
*
*     returns:
*        -1      for unknown countries or bad years
*     # of days  for good years of countries in table
*
**********************************************************************/
int
days_in_year(year, country_code)
    register int   year;
    register int   country_code;
{
    register int switch_year;


    /* valid country code? */
    if (country_code < 0  ||  country_code > MAX_COUNTRY)
	return(-1);

    /* check the farthest boundaries; there is no year 0 */
    /* NOTE: individual country functions may handle less */
    if (year <= 0  ||  year > 9999)
	return(-1);

    /* if year is after Gregorian calendar change, use Gregorian rule */
    switch_year = year_table[country_code].greg_switch;
    if (year > switch_year)
	if ((year&0x3)==0 && ((year&0xF)==0 || year%100))
	    return(366);
	else
	    return(365);
    
    /* else, need to handle years before and during calendar change */
    return((*year_table[country_code].weird_years)(year, switch_year));
}
