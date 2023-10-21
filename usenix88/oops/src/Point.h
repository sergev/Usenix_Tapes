#ifndef	POINT_H
#define	POINT_H

/* Point.h -- declarations for X-Y coordinates

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

$Log:	Point.h,v $
 * Revision 1.3  88/02/04  12:59:54  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:40:30  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"
#include <math.h>

extern const Class class_Point;
overload Point_reader;

class Point: public Object {
protected:
	short xc,yc;			// x-y coordinate 
protected:
	Point(fileDescTy&,Point&);
	Point(istream&,Point&);
	friend	void Point_reader(istream& strm, Object& where);
	friend	void Point_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Point(short newx =0, short newy =0)	{ xc=newx; yc=newy; }
	short	x()			{ return xc; }
	short	x(short newx)		{ return xc = newx; }
	short	y()			{ return yc; }
	short	y(short newy)		{ return yc = newy; }
	Point	operator+(Point p)	{ return Point(xc+p.xc, yc+p.yc); }
	Point	operator-()		{ return Point(-xc,-yc); }
	Point 	operator-(Point p)	{ return Point(xc-p.xc, yc-p.yc); }
	friend Point operator*(Point p, int i) { return Point(i*p.xc, i*p.yc); }
	friend Point operator*(int i, Point p) { return Point(i*p.xc, i*p.yc); }
	int 	operator*(Point p)	{ return xc*p.xc + yc*p.yc; }
	bool	operator==(Point p)	{ return (xc==p.xc && yc==p.yc); }
	bool	operator!=(Point p)	{ return (xc!=p.xc || yc!=p.yc); }
	bool	operator<(Point p)	{ return (yc<p.yc && xc<p.xc); }
	bool	operator<=(Point p)	{ return (yc<=p.yc && xc<=p.xc); }
	bool	operator>(Point p)	{ return (yc>p.yc && xc>p.xc); }
	bool	operator>=(Point p)	{ return (yc>=p.yc && xc>=p.xc); }
	void	operator+=(Point p)	{ xc += p.xc; yc += p.yc; }
	void	operator-=(Point p)	{ xc -= p.xc; yc -= p.yc; }
	void	operator*=(int s)	{ xc *= s; yc *= s; }
	double	dist(Point p)		{ return hypot(xc-p.xc, yc-p.yc); }
	Point	max(Point);
	Point	min(Point);
	Point	transpose()		{ return Point(yc,xc); }
 	bool	isBelow(Point p)	{ return yc > p.yc; }
	bool	isAbove(Point p)	{ return yc < p.yc; }
	bool	isLeft(Point p)		{ return xc < p.xc; }
	bool	isRight(Point p)	{ return xc > p.xc; }
	virtual	int compare(const Object&);	// compare Points 
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);	// equality test 
	virtual void printOn(ostream& strm);
	virtual const Class* species();
};

#endif
