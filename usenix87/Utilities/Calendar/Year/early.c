#include <stdio.h>


/**********************************************************************
*
* unknown_calendar() -- ignorance place holder
*
*     This is for countries that I don't know about.  It is used for
*   the Muslim, Celtic, Chinese, Mayan, and other calendars.
*
**********************************************************************/
int unknown_calendar(year, change_year)
    int year;
    int change_year;
{
    return(-1);
}


/**********************************************************************
*
* julian() -- return length of year based on Julius Caesar/Sosigenes rule
*
*     Actually could go back to 45 BC, but why?
*     Calling routine allows no years before 1 AD 
*
**********************************************************************/
int julian(year, change_year)
    int year;
    int change_year;
{
    int length = 365;


    if ((year & 03) == 0)
	length = 366;

    if (year == change_year)
	switch (change_year / 100)
	{
	case 20:
		/* 2000 - 2099 */
		/* 2000 is a leap year; fall through */
	case 19:
		/* 1900 - 1999 */
		return(length-13);
	case 18:
		/* 1800 - 1899 */
		return(length-12);
	case 17:
		/* 1700 - 1799 */
		return(length-11);
	case 16:
		/* 1600 - 1699 */
		/* 1600 is a leap year, fall through */
	case 15:
		/* 1500 - 1599 */
		return(length-10);
	}

    return(length);
}


/**********************************************************************
*
* belgium_early() -- handle problem caused by the change to Gregorian
*                    overlapping two years
*
**********************************************************************/
int belgium_early(year, change_year)
    int year;
    int change_year;
{
    /* the days 12/25/1582 - 1/5/1583 were dropped */
    if (year == change_year || year == change_year-1)
        return(360);

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
*  canada_early() -- deal with conflict between French rules and
*                    British rules for dates
*
*   France changed to Gregorian 12/10/1582 (Julian)
*   Britain changed to Gregorian 09/05/1752 (Julian)
*   therefore, Canada changed on ??/??/????
*
*   Some guesses can be made based on the individual provinces/territories:
*
*     Alberta                  British rules likely apply
*     British Columbia         British rules definitely apply
*     Manitoba                 likely British rules (def. British after 1763)
*     New Brunswick            France and British alternated control
*     Newfoundland             contested until 1713 when became British
*     Nova Scotia              British after 1713
*     Ontario                  French first then British after 1759
*     Prince Edward Island     follows Ontario rules (?)
*     Quebec                   Britain gained in 1763 with Treaty of Paris
*     Saskatchewan             British rules apply
*     Yukon Territory          first Russian then British after 1840 (?)
*
**********************************************************************/
int canada_early(year, change_year)
    int year;
    int change_year;
{
    /* calculate using British change year for Canada's */
    return(julian(year, change_year));
}


/**********************************************************************
*
* china_early() -- previous to using the Gregorian calendar, China
*                  used a 12 (ordinary year) or 13 (full year) month
*                  calendar that operated in 60 year cycles
*                  Years began with the lunar month that had the sun
*                  enter the zodiacal sign Aquarius
*
*                  Jesuit missionaries tried to reform the calendar
*                  when they arrived in the 1600s.  I haven't found
*                  out the full ramifications of that attempt.  What I
*                  have seen is that the calendar was again changed by
*                  the Chinese after that; this introduced some errors.
*
*                  Now, what that means for converting years is
*                  a lot of work...
*
**********************************************************************/
int china_early(year, change_year)
    int year;
    int change_year;
{
    /* punt */
    return(-1);
}



/**********************************************************************
*
* france_early() -- deal with France's use of the Republic calendar
*                   for the years 1793-1805
*
*    The Revolutionary Calendar was a base 10 calendar.  That is, it
*  had 360 days arranged in 12 months of 30 days plus 5 or 6 unnumbered
*  days that were added to the end of the last month.  Each day was 10
*  hours of 100 minutes of 100 seconds long.
*
*    It was rejected after over a decade of use mainly because of its
*  opposition from religious groups who did not approve of weeks that
*  were seven days long.
*
**********************************************************************/
int france_early(year, change_year)
    int year;
    int change_year;
{
    /* Republic --> Gregorian */
    if (year == 1806)
	return(365);	/* ? */

    /* Republic Calendar -- follows Julian leap-year rule */
    if (year > 1793  ||  year < 1806)
    {
	if ((year & 03) == 0)
	    return(366);
	else
	    return(365);
    }

    /* Gregorian --> Republic */
    if (year == 1793)
	return(365);

    /* Gregorian */
    if (year > 1582  ||  year < 1793)
    {
	if ((year&0x3)==0 && ((year&0xF)==0 || year%100))
	   return(366);
	else
	   return(365);
    }

    /* Julian --> Gregorian */
    if (year == 1582)
	return(365-10);		/* dropped the days 12/10 - 12/20 */

    /* Julian */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* german_early() -- deal with problem of country adopting the
*                   Gregorian calendar in parts
*
*  Gregorian calendar was adopted in overlapping parts:
*      10/16/1583   Bavaria
*      11/14/1583   the Catholic population
*       3/01/1682   Strassburg
*      11/15/1699   the Protestant population
*      12/12/1700   Utrecht
*
*  I have the last date (1700) in the table for the entire country.
*  If you want more accuracy in dealing with the partial changes,
*  make modifications to the country table and the following code.
*
**********************************************************************/
int german_early(year, change_year)
    int year;
    int change_year;
{
    /* deal with change-over date */
    if (year == change_year)
	return(366-11);			/* 1700 is a leap year */

    /* else, do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}



/**********************************************************************
*
* greece_early() -- country did not change all at one time
*
*    The changes that occurred are:
*
*        07/15/1916  Calendar change adopted by all except...
*        09/30/1923  ...the Greek Church, which finally accepted it
*
*    The table uses the latter date.  Hack this if you don't like it.
*    A warning:  one reference seemed to indicated (it wasn't clear if
*    it had been just proposed or it was accepted) that Greece is using a
*    modified Julian calendar that has two leap centuries out of *nine*.
*    If this is true, then dates after 2800 will be different from
*    the Gregorian calendar.
*
**********************************************************************/
int greece_early(year, change_year)
    int year;
    int change_year;
{
    /* do change-over year */
    if (year == change_year)
	return(365-13);		/* 1923 was not a leap year */

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* iran_early() -- uses the Monarchic Calendar
*
*  I don't know how if or when Gregorian was used.  The Muslim
*  calendar may still be used (it is a 12 month calendar with a 30
*  year cycle;  eleven days are added over the cycle) for daily or
*  religious or even Government functions.
*
**********************************************************************/
int iran_early(year, change_year)
    int year;
    int change_year;
{
    return(-1);
}


/**********************************************************************
*
* japan_early() -- deal with Japanese lunar calendar
*
*   Unfortunately, I have been unable to find any detailed information
*   on the calendar used prior to the adoption of the Gregorian calendar.
*
*   If anyone has some info, I'd like to receive it.
*
**********************************************************************/
int japan_early(year, change_year)
    int year;
    int change_year;
{
    return(-1);
}


/**********************************************************************
*
* netherlands_early() -- deal with scattered adoption
*
*  Adoption of the Gregorian calendar went by the following cities:
*     12/15/1582   Holland, Zeeland, Brabant, Vlaandern
*     06/30/1700   Gelerland
*     11/30/1700   Utrecht, Overijisol
*     12/31/1700   Friesland, Groningen
*     01/12/1701   Entire country consistent
*
**********************************************************************/
int netherlands_early(year, change_year)
    int year;
    int change_year;
{
    /* Use date of total country adoption -- 1701 */
    if (year == change_year)
	return(365 - 11);

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* poland_early() -- deal with country's partial adoption dates
*
*  Dates for adoption are
*      11/01/1582   Catholics (and Protestants?) adopt
*      03/18/1918   Russian Poland changes (Civil War split)
*      05/??/1923   Orthodox members adopt
*
**********************************************************************/
int poland_early(year, change_year)
    int year;
    int change_year;
{
    /* Use date of total country adoption -- 1923 */
    if (year == change_year)
	return(365 - 13);

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* sweden_early() -- deal with bouncing leap day
*
*  Sweden took a half-step towards Gregorian but then retreated (think
*  of it as sort of a single partner polka).
*
*       1700 -- made this year a non-leap year by dropping Feb 29th
*       1712 -- went back to Julian calendar by making a Feb 30th !!
*  2/18/1753 -- adopted Gregorian
*
**********************************************************************/
int sweden_early(year, change_year)
    int year;
    int change_year;
{
    if (year == change_year)
	return(365 - 11);

    if (year == 1700)
	return(365);

    if (year == 1712)
	return(367);

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* switzerland_early() -- deal with scattered adoption in country
*
*  Districts and their adoption dates (I had conflicting sources for
*  the spelling of the district names)
*
*  Catholic districts 
*     01/22/1584  Fribourg, Lucerne, Schwyz, Solothurn, Unterwalden, Uri, Zug
*     01/17/1597  Appenzell
*     03/01/1656  Valais (part did early in 1622)
*
*  Protestant districts 
*     01/01/1701  Baselstradt, Bern, Biel, Cargous, Geneva, Neuchatel,
*                 Schaffhausen, Thurgan, and Zurich
*     12/20/1723  Appenzell
*     01/12/1724  Glarus, St. Galen
*     02/17/1812  Grisons
*
**********************************************************************/
int switzerland_early(year, change_year)
    int year;
    int change_year;
{
    /* Use date of total country adoption -- 1724 (a leap year) */
    if (year == change_year)
	return(366 - 11);

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* turkey_early() -- deal with scattered adoption of new calendar
*
*  Adoption seemed to depend on regional background
*
*      1908  people with a European heritage
*      1917  people with a Asian heritage (might be 1914)
*
**********************************************************************/
int turkey_early(year, change_year)
    int year;
    int change_year;
{
    /* Use date of total country adoption -- 1917 */
    if (year == change_year)
	return(365 - 13);

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* usa_early() -- deal with state differences
*
*  The USA, for the most part, followed Great Britain's lead in the
*  adoption of the the Gregorian calendar in 1752.  However, not all
*  current states were part of the "country" at that time.  An easy
*  lie is to say that all parts of the yet-to-be USA changed over on
*  that date.  States like California were not even settled until
*  about 1770.
*
*  The exceptions to the 1752 rule:
*
*  Alaska was owned by Russia until 1867. (see Russia's rules)
*  Hawaii was an independent country that adopted Gregorian in 1893(?).
*
*  The non-states that operate under US protection are likewise exceptions
*  to the rule (luckily, some were not inhabited or we'd have to hypnotise
*  the native population to change their memory of previous calendars:-):
*
*  American Samoa became possesion 1899.
*  Baker, Howland, and Jarvis Islands became possesions 1857(?).
*  Panama Canal Zone was a possesion 1903-1979 (?).
*  Canton and Enderbury Islands - 1939.
*  Great Corn and Little Corn - leased from Nicaraugua for 99 years in 1914.
*  Guam - obtained in 1898, lost to Japan 1941, regained in 1944.
*  Johnston Island - came with Hawaii
*  Midway Islands - 1867
*  Phillipine Islands - was US territory 1898-1946
*  Puerto Rico - Spain ceded to US in 1898
*  Trust Territories of the Pacific (approx 2000 islands in W. Pacific)-1947
*  Virgin Islands - bought from Denmark in 1917
*  Wake Island - got from Spain 1898, Japan took 1941, regained in 1945
*
**********************************************************************/
int usa_early(year, change_year)
    int year;
    int change_year;
{
    /* this is for the majority of the cases,  change if desired */
    if (year == change_year)	/* 1752 */
	return(366 - 11);

    /* do normal Julian rules */
    if ((year & 03) == 0)
	return(366);
    else
	return(365);
}


/**********************************************************************
*
* ussr_early() -- deal with scattered adoption of new calendar
*
* The changing calendar for the Soviet Union reflects a country
* that was undergoing great changes.  The dates of change are:
*
*   1/1/1918 -- Western part changes to Gregorian
*   2/5/1920 -- Eastern part changes to Gregorian
*       1929 -- Entire country changed to a 5 day week and new calendar
*       1932 -- Entire country changed to a 6 day week and new calendar
*  6/27/1940 -- Chucked the non-standard calendar and returned to Gregorian
*
**********************************************************************/
int ussr_early(year, change_year)
    int year;
    int change_year;
{
    /* the above is the present and total knowledge I have on the */
    /* calendars used....  So, I'll punt and return(-1);          */
    return(-1);
}
