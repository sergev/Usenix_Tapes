#ifndef	RECTANGLE_H
#define	RECTANGLE_H

/* Rectangle.h -- declarations for class Rectangle

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

$Log:	Rectangle.h,v $
 * Revision 1.3  88/02/04  13:00:02  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:40:51  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"
#include "Point.h"
#include <math.h>

extern const Class class_Rectangle;
overload Rectangle_reader;

class OrderedCltn;

class Rectangle: public Object {
	Point tl;		// top left corner (origin)
	Point br;		// bottom right corner (corner)
protected:
	Rectangle(fileDescTy&, Rectangle&);
	Rectangle(istream&, Rectangle&);
	friend	void Rectangle_reader(istream& strm, Object& where);
	friend	void Rectangle_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Rectangle(int left=0, int top=0, int height=0, int width=0);
	Rectangle(Point, Point);
	Point	origin()		{ return tl; }
	Point	origin(const Point& p)	{ return tl = p; }
	Point	corner()		{ return br; }
	Point	corner(const Point& p)	{ return br = p; }
	Point	topLeft()		{ return tl; }
	Point	topCenter()		{ return Point((br.x()-tl.x())/2,tl.y()); }
	Point	topRight()		{ return Point(br.x(),tl.y()); }
	Point	rightCenter()		{ return Point(br.x(),(br.y()-tl.y())/2); }
	Point	bottomRight()		{ return br; }
	Point	bottomCenter()		{ return Point((br.x()-tl.x())/2,br.y()); }
	Point	bottomLeft()		{ return Point(tl.x(),br.y()); }
	Point	leftCenter()		{ return Point(tl.x(),(br.y()-tl.y())/2); }
	Point	center()		{ return Point((br.x()-tl.x())/2,(br.y()-tl.y())/2); }
	Point	extent()		{ return Point(br.x()-tl.x(),br.y()-tl.y()); }
	int	area()			{ return (br.x()-tl.x())*(br.y()-tl.y()); }
	int	width()			{ return br.x()-tl.x(); }
	int	height()		{ return br.y()-tl.y(); }
	bool	operator==(const Rectangle&);
	bool	operator!=(const Rectangle&);
	Rectangle operator&&(const Rectangle&);	// intersection 
	Rectangle operator||(const Rectangle&);	// union 
	void	operator+=(const Point&);	// translate 
	void	operator-=(const Point&);
	OrderedCltn& areasOutside(const Rectangle&);
	bool	contains(const Point&);
	bool	contains(const Rectangle&);
	bool	intersects(const Rectangle&);
	void 	moveTo(const Point&);
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);	// equality test 
	virtual void printOn(ostream& strm);
	virtual const Class* species();
};

#endif
