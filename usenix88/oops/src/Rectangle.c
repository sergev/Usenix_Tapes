/* Rectangle.c -- implementation of class Rectangle

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
	September, 1985

Function:
	
A Rectangle is defined by two points: an origin, which specifies the top
left corner, and corner, which specifies the bottom right corner.

$Log:	Rectangle.c,v $
 * Revision 1.2  88/01/16  23:40:42  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Rectangle.h"
#include "OrderedCltn.h"
#include "oopsIO.h"

#define	THIS	Rectangle
#define	BASE	Object
DEFINE_CLASS(Rectangle,Object,1,"$Header: Rectangle.c,v 1.2 88/01/16 23:40:42 keith Exp $",NULL,NULL);

Rectangle::Rectangle(int left, int top, int height, int width)
{
	tl = Point(left,top);
	br = Point(left+width,top+height);
}

Rectangle::Rectangle(Point o, Point c)
{
	tl = o;
	br = c;
}

bool Rectangle::operator==(const Rectangle& r)
{
	return (tl==((Rectangle*)&r)->tl && br==((Rectangle*)&r)->br);
}

bool Rectangle::operator!=(const Rectangle& r)
{
	return (tl!=((Rectangle*)&r)->tl || br!=((Rectangle*)&r)->br);
}

Rectangle Rectangle::operator&&(const Rectangle& r)
{
	return Rectangle(tl.max(((Rectangle*)&r)->tl), br.min(((Rectangle*)&r)->br));
}

Rectangle Rectangle::operator||(const Rectangle& r)
{
	return Rectangle(tl.min(((Rectangle*)&r)->tl),
		br.max(((Rectangle*)&r)->br));
}

void Rectangle::operator+=(const Point& p)
{
	tl += p;
	br += p;
}

void Rectangle::operator-=(const Point& p)
{
	tl -= p;
	br -= p;
}

OrderedCltn& Rectangle::areasOutside(const Rectangle& r)
{
	OrderedCltn& areas = *new OrderedCltn(4);
	short yOrigin, yCorner;
	if ((tl > ((Rectangle*)&r)->br) || (((Rectangle*)&r)->tl > br)) {
		areas.add(*this);	// rectangles do not intersect 
		return areas;
	}
	if (r.tl.y() > tl.y())
		areas.add(*new Rectangle(tl, Point(br.x(), yOrigin = r.tl.y())));
	else	yOrigin = tl.y();
	if (r.br.y() < br.y())
		areas.add(*new Rectangle(Point(tl.x(), yCorner = r.br.y()), br));
	else	yCorner = br.y();
	if (r.tl.x() > tl.x())
		areas.add(*new Rectangle(Point(tl.x(), yOrigin), Point(r.tl.x(), yCorner)));
	if (r.br.x() < br.x())
		areas.add(*new Rectangle(Point(r.br.x(), yOrigin), Point(br.x(), yCorner)));
	return areas;
}

bool Rectangle::contains(const Point& p)
{
	return (tl <= p) && (p <= br);
}

bool Rectangle::contains(const Rectangle& r)
{
	return (contains(((Rectangle*)&r)->tl) && contains(((Rectangle*)&r)->br));
}

bool Rectangle::intersects(const Rectangle& r)
{
	if (tl.max(((Rectangle*)&r)->tl) < br.min(((Rectangle*)&r)->br)) return YES;
	return NO;
}

void Rectangle::moveTo(const Point& p)
/*
	Move this Rectangle so its origin is at p.
*/
{
	br += p-tl;
	tl = p;
}

Object* Rectangle::copy()		{ return shallowCopy(); }

void Rectangle::deepenShallowCopy()	{}

unsigned Rectangle::hash()	{ return tl.hash()^br.hash(); }

bool Rectangle::isEqual(const Object& r)
{
	return r.isSpecies(class_Rectangle) && *this==*(Rectangle*)&r;
}

const Class* Rectangle::species()	{ return &class_Rectangle; }

void Rectangle::printOn(ostream& strm)
{
	tl.printOn(strm);
	strm << " corner: ";
	br.printOn(strm);
}

Rectangle::Rectangle(istream& strm, Rectangle& where)
{
	this = (Rectangle*)&where;
	readFrom(strm,"Point",tl);
	readFrom(strm,"Point",br);
}

void Rectangle::storer(ostream& strm)
{
	BASE::storer(strm);
	tl.storeOn(strm);
	br.storeOn(strm);
}

Rectangle::Rectangle(fileDescTy& fd, Rectangle& where)
{
	this = &where;
	readFrom(fd,"Point",tl);
        readFrom(fd,"Point",br);
}

void Rectangle::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
        tl.storeOn(fd);
	br.storeOn(fd);
}
