/* Time.c -- implementation of class Time

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
	
Provides an object that represents a Time, stored as the number of
seconds since January 1, 1901, GMT.

$Log:	Time.c,v $
 * Revision 1.2  88/01/16  23:41:42  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Date.h"
#include "Time.h"
#include "oopsconfig.h"
#include "oopsIO.h"

#define	THIS	Time
#define	BASE	Object

#if defined(AIX) | defined(RTUATT) | defined(SYSV)

#include <time.h>
#define TIME_ZONE timezone
#define DST_OBSERVED daylight
DEFINE_CLASS(Time,Object,1,"$Header: Time.c,v 1.2 88/01/16 23:41:42 keith Exp $",NULL,NULL);

#endif

#if defined(UCB42BSD) | defined(RTUUCB) | defined(ACIS42)

#include <sys/time.h>

static long TIME_ZONE;          /* seconds west of GMT */
static int DST_OBSERVED;        /* flags U.S. daylight saving time observed */

static void inittimezone(const Class&) {
	struct timeval tval;            /* see <sys/time.h> */
	struct timezone tz;             /* see <sys/time.h> */
	gettimeofday(&tval,&tz);
	TIME_ZONE = 60*(tz.tz_minuteswest);
	DST_OBSERVED = tz.tz_dsttime;
}

DEFINE_CLASS(Time,Object,1,"$Header: Time.c,v 1.2 88/01/16 23:41:42 keith Exp $",inittimezone,NULL);

#endif

extern const int OOPS_DATERANGE,OOPS_BADTIME;

static const unsigned long seconds_in_day = 24*60*60;
static const Date refDate(0);
static const Date maxDate(49709);	/* ((2**32)-1)/seconds_in_day -1 */
	
static Time localTime(const Date& date, hourTy h=0, minuteTy m=0, secondTy s=0)
/*
	Return a local Time for the specified Standard Time date, hour, minute,
	and second.
*/
{
	if (!date.between(refDate,maxDate))
		setOOPSerror(OOPS_DATERANGE,DEFAULT,
			date.dayOfMonth(),date.nameOfMonth(),date.year());
	return Time(seconds_in_day*(date-refDate) + 60*60*h + 60*m + s);
	
}

Time::Time()
/*
	Construct a Time for this instant.
*/
{
	sec = time(0);
	sec += 2177452800L;	/* seconds from 1/1/01 to 1/1/70 */
}

Time::Time(hourTy h, minuteTy m, secondTy s, bool dst)
/*
	Construct a Time for today at the specified (local) hour, minute, and
	second.
*/
{
	sec = Time(Date(),h,m,s,dst).sec;
}


Time::Time(const Date& date, hourTy h, minuteTy m, secondTy s, bool dst)
/*
	Construct a Time for the specified (local) Date, hour, minute, and
	second.
*/
{
	sec = ::localTime(date,h,m,s).sec-3600;
	if (isDST()) {
		sec += 3600;
		if (isDST() || dst) sec -= 3600;
	}
	else {
		sec += 3600;
		if (isDST()) setOOPSerror(OOPS_BADTIME,DEFAULT,
			date.dayOfMonth(),date.nameOfMonth(),date.year(),
			h,m,s,(dst?"DST":""));
	}
	sec += TIME_ZONE;				// adjust to GMT 
}

Time::operator Date()
/*
	Convert a Time to a local Date
*/
{
//	return Date((int)(localTime().sec/seconds_in_day));	4.2 cc bug
        int daycount = localTime().sec/seconds_in_day;
	return Date(daycount);
}

hourTy Time::hour()
/*
	Return the hour of this Time in local time; i.e., adjust for
	time zone and Daylight Savings Time.
*/
{
	return localTime().hourGMT();
}

hourTy Time::hourGMT()
/*
	Return the hour of this Time in GMT.
*/
{
	return (sec % 86400) / 3600;
}

static Time beginDST(unsigned year)
/*
	Return the local Standard Time at which Daylight Savings Time
	begins in the specified year.
*/
{
	Time DSTtime(localTime(Date(31,"Mar",year).previous("Sun")+7,2));
	if (year<=1986) {
		DSTtime = localTime(Date(30,"Apr",year).previous("Sun"),2);
		if (year==1974) DSTtime = localTime(Date(6,"Jan",1974),2);
		if (year==1975) DSTtime = localTime(Date(23,"Feb",1975),2);
	}
	return DSTtime;
}

static Time endDST(unsigned year)
/*
	Return the local Standard Time at which Daylight Savings Time
	ends in the specified year.
*/
{
	Time STDtime(localTime(Date(31,"Oct",year).previous("Sun"),2-1));
	return STDtime;
}

bool Time::isDST()
/*
	Return YES if this local Standard Time should be adjusted
	for Daylight Savings Time.
*/
{
//	unsigned year = Date((unsigned)(this->sec/seconds_in_day)).year();  4.2 cc bug
        int daycount = this->sec/seconds_in_day;
	unsigned year = Date(daycount).year();
	if (DST_OBSERVED && *this >= beginDST(year) && *this < endDST(year)) 
	  return YES;
	return NO;
}

Time Time::localTime()
/*
	Adjusts this GM Time for local time zone and Daylight Savings Time.
*/
{
	Time local_time(sec-TIME_ZONE);
	if (local_time.isDST()) local_time.sec += 3600;
	return local_time;
}

minuteTy Time::minute()
/*
	Return the minute of this Time in local time; i.e., adjust
	for time zone and Daylight Savings Time.
*/
{
	return localTime().minuteGMT();
}

minuteTy Time::minuteGMT()
/*
	Return the minute of this Time in GMT.
*/
{
	return ((sec % 86400) % 3600) / 60;
}

secondTy Time::second()
/*
	Return the second of this Time.
*/
{
	return ((sec % 86400) % 3600) % 60;
}

Time Time::max(Time t)	{ return (t < *this) ? *this : t; }

Time Time::min(Time t)	{ return (t > *this) ? *this : t; }

int Time::compare(const Object& ob)
{
	assertArgSpecies(ob,class_Time,"compare");
	register clockTy t = ((Time*)&ob)->sec;
	if (sec < t) return -1;
	if (sec > t) return 1;
	return 0;
}

Object* Time::copy()		{ return shallowCopy(); }

void Time::deepenShallowCopy()	{}

unsigned Time::hash()	{ return sec; }

bool Time::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Time) && *this==*(Time*)&ob;
}

const Class* Time::species()	{ return &class_Time; }

void Time::printOn(ostream& strm)
{
	register unsigned hh = hour();
	((Date)*this).printOn(strm);
	strm << form(" %d:%02d:%02d ",
		(hh <= 12) ? hh : hh-12,
		minute(),
		second());
	if (hh < 12) strm << "am";
	else strm << "pm";
}

Time::Time(istream& strm, Time& where)
{
	this = &where;
	strm >> sec;
}

void Time::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << sec << " ";
}


Time::Time(fileDescTy& fd, Time& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Time::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}
