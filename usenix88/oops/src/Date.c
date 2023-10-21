/* Date.c -- implementation of Gregorian calendar dates

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	December, 1985

Function:
	
Provides an object that contains a date, stored as a year and a
day-of-year.  This code was adapted from version 1 of the Smalltalk-80
system.

$Log:	Date.c,v $
 * Revision 1.2  88/01/16  23:38:32  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Date.h"
#include "String.h"
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "oopsIO.h"

#define	THIS	Date
#define	BASE	Object
DEFINE_CLASS(Date,Object,1,"$Header: Date.c,v 1.2 88/01/16 23:38:32 keith Exp $",NULL,NULL);

extern const int OOPS_BADMODAY,OOPS_BADDAYNAM,OOPS_BADMONAM,OOPS_BADMONTH,OOPS_BADDAY;

static const unsigned char days_in_month[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
static const dayTy first_day_of_month[12] = { 1,32,60,91,121,152,182,213,244,274,305,335 };
static const char* month_names[12] = { "January","February","March","April","May","June","July","August","September","October","November","December" };
static const char* uc_month_names[12] = { "JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE","JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER" };
static const char* week_day_names[7] = { "Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday" };
static const char* uc_week_day_names[7] = { "MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY","SUNDAY" };
static const unsigned int seconds_in_day = 24*60*60;
	
void Date::setDate(int dayCount, yearTy referenceYear)
/*
	Set the day and year of a Date object to the dayCount day
	of the referenceYear.  The dayCount may be <= 0, in which
	case the Date is that of a year prior to the referenceYear.
*/
{
	register int day = dayCount;
	register unsigned year = referenceYear;
	register unsigned yearDays;
	while (day > (yearDays = daysInYear(year))) {
		year++;
		day -= yearDays;
	}
	while (day<=0) day += daysInYear(--year);
	dy = day; yr = year;
}

Date::Date()
/*
	Construct a Date for today.
*/
{
	long clk = time(0);
	register tm* now = localtime(&clk);
	setDate(now->tm_yday+1, now->tm_year+1900);
}

Date::Date(int dayCount, yearTy referenceYear)
/*
	Construct a Date that is dayCount days after the beginning of
	the reference year.
*/
{
	setDate(dayCount,referenceYear);
}

Date::Date(int dayCount)
/*
	Construct a Date that is dayCount days after January 1, 1901.
*/	
{
	setDate(1 + (dayCount>=0 ? dayCount%1461 : -((-dayCount)%1461)),
		1901 + 4*((dayCount>=0) ? dayCount/1461 : -((-dayCount)/1461)));
}

Date::Date(dayTy day, const char* monthName, yearTy year)
/*
	Construct a Date object for the specified day, monthName, and year.
*/
{
	if (year < 100) year += 1900;
	unsigned monthNumber = numberOfMonth(monthName);
	unsigned daysInMonth  = days_in_month[monthNumber-1];
	if (monthNumber == 2 && leapYear(year)) daysInMonth++;
	unsigned firstDayOfMonth = first_day_of_month[monthNumber-1];
	if (monthNumber > 2 && leapYear(year)) firstDayOfMonth++;
	if (day<1 || day>daysInMonth) setOOPSerror(OOPS_BADMODAY,DEFAULT,this,day,monthName,year);
	setDate(day-1+firstDayOfMonth,year);
}

static void skipDelim(istream& strm)
{
	char c;
	if (!strm.good()) return;
	strm >> c;
	while (strm.good() && !isalnum(c)) strm >> c;
	strm.putback(c);
}

static const char* parseMonth(istream& strm)
/*
	Read the name of a month from the specified input stream.
*/
{
	static char month[10];
	register char* p = month;
	char c;
	skipDelim(strm);
	strm.get(c);
	while (strm.good() && isalpha(c) && (p != &month[10])) {
		*p++ = c;
		strm.get(c);
	}
	if (strm.good()) strm.putback(c);
	*p = '\0';
	return month;
}

static Date parseDate(istream& strm)
/*
	Read a date from the specified input stream in any of the following
	forms:

		<day><monthName><year>		(5 April 1982; 5-APR-82)
		<monthName><day><year>		(April 5, 1982)
		<monthNumber><day><year>	(4/5/82)
*/
{
	unsigned d,m,y;
	const char* mon;		// name of month 
	if (strm.good()) {
		skipDelim(strm);
		strm >> m;		// try to parse day or month number 
		skipDelim(strm);
		if (strm.eof()) return Date(1,1901);
		if (strm.fail()) {	// parse <monthName><day><year> 
			strm.clear();
			mon = parseMonth(strm);	// parse month name 
			skipDelim(strm);
			strm >> d;		// parse day 
		}
		else {			// try to parse day number 
			strm >> d;
			if (strm.eof()) return Date(1,1901);
			if (strm.fail()) {	// parse <day><monthName><year> 
				d = m;
				strm.clear();
				mon = parseMonth(strm);		// parse month name 
			}
			else {			// parsed <monthNumber><day><year> 
				mon = nameOfMonth(m);
			}
		}
		skipDelim(strm);
		strm >> y;
	}
	if (!strm.good()) return Date(1,1901);
	return Date(d,mon,y);
}
	
Date::Date(istream& strm)	{ *this = parseDate(strm); }

dayTy dayOfWeek(const char* dayName)
/*
	Returns the number, 1-7, of the day of the week named dayName.
*/
{
	{
		String s(dayName);
		register unsigned len = s.size();
		if (len > 2) {
			s.toUpper();
			register const char* p = s;
			for (register unsigned i =0; i<7; i++)
			if (strncmp(p,uc_week_day_names[i],len)==0) return i+1;
		}
	}
	setOOPSerror(OOPS_BADDAYNAM,DEFAULT,dayName);
	return 0;	// never executed 
}

dayTy daysInYear(yearTy year)
/*
	Returns the number of days in the specified year.
*/
{
	return leapYear(year) ? 366 : 365;
}

monthTy numberOfMonth(const char* monthName)
/*
	Returns the number, 1-12, of the month named monthName.
*/
{
	{
		String s(monthName);
		register unsigned len = s.size();
		if (len > 2) {
			s.toUpper();
			register const char* p = s;
			for (register unsigned i =0; i<12; i++)
				if (strncmp(p,uc_month_names[i],len)==0) return i+1;
		}
	}
	setOOPSerror(OOPS_BADMONAM,DEFAULT,monthName);
	return 0;	// never executed 
}

bool leapYear(yearTy year)
/*
	Returns YES if the specified year is a leap year.
*/
{
	return ((year&3) == 0 && year%100 != 0 || year%400 == 0);
}

const char* nameOfMonth(monthTy monthNumber)
/*
	Returns the name of the month numbered monthNumber.
*/
{
	if (--monthNumber >= 12) setOOPSerror(OOPS_BADMONTH,DEFAULT,monthNumber+1);
	return month_names[monthNumber];
}

const char* nameOfDay(dayTy weekDayNumber)
/*
	Returns the name of the day specified by weekDayNumber:
		Monday == 1, ... , Sunday == 7
*/
{
	if (--weekDayNumber >= 7) setOOPSerror(OOPS_BADDAY,DEFAULT,weekDayNumber+1);
	return week_day_names[weekDayNumber];
}

bool Date::operator<(Date dt)
/*
	Return YES if this Date is < the argument Date.
*/
{
	if (yr < dt.yr) return YES;
	if (yr == dt.yr && dy < dt.dy) return YES;
	return NO;
}

bool Date::operator<=(Date dt)
/*
	Return YES if this Date is <= the argument Date.
*/
{
	if (yr < dt.yr) return YES;
	if (yr == dt.yr && dy <= dt.dy) return YES;
	return NO;
}

int Date::operator-(Date dt)
/*
	Return the number of days between this Date and the argument.  The
	result is negative if the argument is later.
*/
{
	Date a(*this),b(dt);
	register bool negative = b > a;
	if (negative) {	a = b; b = *this; }
	register unsigned year = b.year();
	register int days = a.day()-b.day();
	while (year != a.year()) days += daysInYear(year++); 
	return negative ? -days : days;
}

const char* Date::nameOfMonth()	{ return ::nameOfMonth(month()); }

monthTy Date::month()
/*
	Returns the number, 1-12, of this Date's month.
*/
{
	register bool leapYear = leap();
	register unsigned firstDay;
	for (register unsigned mn =12; mn>0; mn--) {
		firstDay = first_day_of_month[mn-1];
		if (mn > 2 && leap()) firstDay++;
		if (firstDay <= dy) return mn;
	}
	setOOPSerror(OOPS_BADMONTH,DEFAULT,mn);
	return 0;	// never executed 
}

dayTy Date::firstDayOfMonth(monthTy month)
/*
	Returns the number of the day of the year that is the first day
	of the month in this Date's year.
*/
{
	register unsigned firstDay = first_day_of_month[month-1];
	if (month > 2 && leap()) firstDay++;
	return firstDay;
}

Date Date::previous(const char* dayName)
/*
	Returns the previous date whose weekday name is dayName.
*/
{
	return *this - (7 + weekDay() - dayOfWeek(dayName))%7;
}

dayTy Date::weekDay()
/*
	Return the number, 1-7, of the day of the week for this Date.
*/
{
	register unsigned yearNumber;
	register unsigned dayNumber;
	if (dy < firstDayOfMonth(3)) {
		yearNumber = yr-1;
		dayNumber = 307;
		}
	else {
		yearNumber = yr;
		dayNumber = -58-leap();
	}
	return (dayNumber + dy + yearNumber + (yearNumber/4) + (yearNumber/400) - (yearNumber/100)) % 7 + 1;
}

bool Date::between(Date min, Date max)
/*
	Return YES if this Date is <= to max and >= to min.
*/
{
	return *this >= min && *this <= max;
}

dayTy Date::dayOfMonth()
{
	return dy-firstDayOfMonth()+1;
}

Date Date::max(Date dt)
{
//	return (dt < *this) ? *this : dt;	// MASSCOMP cc bug
	if (dt < *this) return *this;
	return dt;
}

Date Date::min(Date dt)
{
//	return (dt > *this) ? *this : dt;	// MASSCOMP cc bug
	if (dt > *this) return *this;
	return dt;
}

int Date::compare(const Object& ob)
{
	int t;
	assertArgSpecies(ob,class_Date,"compare");
	if ((t=yr-((Date*)&ob)->yr) != 0) return t;
	else return (dy-((Date*)&ob)->dy);
}

Object* Date::copy()		{ return shallowCopy(); }

void Date::deepenShallowCopy()	{}

unsigned Date::hash()	{ return dy^yr; }

bool Date::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Date) && *this==*(Date*)&ob;
}

const Class* Date::species()	{ return &class_Date; }

void Date::printOn(ostream& strm)
{
	strm << form("%2d-%.3s-%.2d",
		dayOfMonth(),
		nameOfMonth(),
		yr%100);
}

void Date::scanFrom(istream& strm)	{ *this = parseDate(strm); }

Date::Date(istream& strm, Date& where)
{
	this = &where;
	strm >> dy >> yr;
}

void Date::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << dy << " " << yr << " ";
}

Date::Date(fileDescTy& fd, Date& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Date::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}
