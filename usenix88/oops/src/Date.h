#ifndef	DATE_H
#define	DATE_H

/* Date.h -- declarations for Gregorian calendar dates

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

$Log:	Date.h,v $
 * Revision 1.3  88/02/04  12:58:50  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:38:37  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

typedef unsigned short dayTy;
typedef unsigned short monthTy;
typedef unsigned short yearTy;

extern const Class class_Date;
overload Date_reader;

dayTy	dayOfWeek(const char* dayName);
dayTy	daysInYear(yearTy year);
monthTy	numberOfMonth(const char* monthName);
bool	leapYear(yearTy year);
const char*	nameOfMonth(monthTy monthNumber);
const char*	nameOfDay(dayTy weekDayNumber);

class Date: public Object {
	dayTy dy;		// day of year 
	yearTy yr;		// year 
	void setDate(int,yearTy);
protected:
	Date(fileDescTy&,Date&);
	Date(istream&);			// read date from stream 
	friend	void Date_reader(istream& strm, Object& where);
	friend	void Date_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Date();				// current date 
	Date(int dayCount);
	Date(int dayCount, yearTy referenceYear);
	Date(dayTy newDay, const char* monthName, yearTy newYear);
	Date(istream&,Date&);
	bool	operator<(Date);
	bool	operator<=(Date);
	bool	operator>(Date date)	{ return date < *this; }
	bool	operator>=(Date date)	{ return date <= *this; }
	bool	operator==(Date date)	{ return dy == date.dy && yr == date.yr; }
	bool	operator!=(Date date)	{ return dy != date.dy || yr != date.yr; }
	friend Date operator+(Date dt, int dd)	{ return Date(dt.dy+dd, dt.yr); }
	friend Date operator+(int dd, Date dt)	{ return Date(dt.dy+dd, dt.yr); }
	int	operator-(Date dt);
	Date	operator-(int dd)	{ return Date(dy-dd, yr); }
	void	operator+=(int dd)	{ setDate(dy+dd, yr); }
	void	operator-=(int dd)	{ setDate(dy-dd, yr); }
	bool	between(Date, Date);
	dayTy	day()			{ return dy; }
	dayTy	dayOfMonth();
	dayTy	firstDayOfMonth()	{ return firstDayOfMonth(month()); }
	dayTy	firstDayOfMonth(monthTy month);
	bool	leap()			{ return leapYear(yr); }
	Date	max(Date);
	Date	min(Date);
	monthTy	month();
	const char*	nameOfMonth();
	Date	previous(const char* dayName);
	dayTy	weekDay();
	yearTy	year()			{ return yr; }
	virtual int compare(const Object&);
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual void scanFrom(istream& strm);
	virtual const Class* species();
};

#endif
