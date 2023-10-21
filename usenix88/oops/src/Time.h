#ifndef	TIME_H
#define	TIME_H

/* Time.h -- declarations for class Time

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

$Log:	Time.h,v $
 * Revision 1.3  88/02/04  13:00:26  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:46  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

typedef unsigned short hourTy;
typedef unsigned short minuteTy;
typedef unsigned short secondTy;
typedef unsigned long clockTy;

extern const Class class_Time;
overload Time_reader;

class Date;

class Time: public Object {
	clockTy sec;			/* seconds since 1/1/1901 */
	bool isDST();
	Time localTime();
protected:
	Time(fileDescTy&,Time&);
	Time(istream&,Time&);
	friend	void Time_reader(istream& strm, Object& where);
	friend	void Time_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Time();				// current time 
	Time(clockTy s)			{ sec = s; }
	Time(hourTy h, minuteTy m, secondTy s =0, bool dst =NO);
	Time(const Date&, hourTy h =0, minuteTy m =0, secondTy s=0, bool dst =NO);
	operator Date();
	bool	operator<(Time time)	{ return sec < time.sec; }
	bool	operator<=(Time time)	{ return sec <= time.sec; }
	bool	operator>(Time time)	{ return sec > time.sec; }
	bool	operator>=(Time time)	{ return sec >= time.sec; }
	bool	operator==(Time time)	{ return sec == time.sec; }
	bool	operator!=(Time time)	{ return sec != time.sec; }
	friend Time operator+(Time t, long s)	{ return Time(t.sec+s); }
	friend Time operator+(long s, Time t)	{ return Time(t.sec+s); }
	long	operator-(Time t)	{ return sec - t.sec; }
	Time	operator-(long s)	{ return Time(sec-s); }
	void	operator+=(long s)	{ sec += s; }
	void	operator-=(long s)	{ sec -= s; }
	bool	between(Time a, Time b)	{ return *this >= a && *this <= b; }
	hourTy	hour();			// hour in local time 
	hourTy	hourGMT();		// hour in GMT 
	minuteTy minute();		// minute in local time 
	minuteTy minuteGMT();		// minute in GMT 
	secondTy second();		// second in local time or GMT 
	clockTy	seconds()		{ return sec; }
	Time	max(Time);
	Time	min(Time);
	virtual int compare(const Object&);
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual const Class* species();
};

#endif /* TIMEH */
