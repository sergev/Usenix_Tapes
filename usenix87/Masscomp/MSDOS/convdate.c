/*
 * convdate(), convtime()
 */

/*
 * convert MSDOS directory datestamp to ASCII
 */

char *
convdate(date_high, date_low)
unsigned date_high;
unsigned date_low;
{
/*
 *	    hi byte     |    low byte
 *	|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
 *      | | | | | | | | | | | | | | | | |
 *      \   7 bits    /\4 bits/\ 5 bits /
 *         year +80      month     day
 */
	static char buffer[9];
	unsigned char year, month_hi, month_low, day;

	year = (date_high >> 1) + 80;
	month_hi = (date_high & 0x1) << 3;
	month_low = date_low >> 5;
	day = date_low & 0x1f;
	sprintf(buffer, "%2d-%02d-%02d", month_hi+month_low, day, year);
	return(buffer);
}

/*
 * Convert MSDOS directory timestamp to ASCII
 */

char *
convtime(time_high, time_low)
unsigned time_high;
unsigned time_low;
{
/*
 *	    hi byte     |    low byte
 *	|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
 *      | | | | | | | | | | | | | | | | |
 *      \  5 bits /\  6 bits  /\ 5 bits /
 *         hour      minutes     sec*2
 */
	static char buffer[7];
	char am_pm;
	unsigned char hour, min_hi, min_low;

	hour = time_high >> 3;
	am_pm = (hour >= 12) ? 'p' : 'a';
	if (hour > 12)
		hour = hour -12;
	if (hour == 0)
		hour = 12;
	min_hi = (time_high & 0x7) << 3;
	min_low = time_low >> 5;
	sprintf(buffer, "%2d:%02d%c", hour, min_hi+min_low, am_pm);
	return(buffer);
}
