/* Point.c  -- implementation of X-Y coordinates

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
	August, 1985

Function:
	
A Point represents an x-y coordinate pair.  By convention, Point(0,0) is
the top left corner of the display, with x increasing to the right and y
increasing to the bottom.

$Log:	Point.c,v $
 * Revision 1.2  88/01/16  23:40:28  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Point.h"
#include "oopsIO.h"

#define	THIS	Point
#define	BASE	Object
DEFINE_CLASS(Point,Object,1,"$Header: Point.c,v 1.2 88/01/16 23:40:28 keith Exp $",NULL,NULL);

Point Point::max(Point p)
{
	return Point(MAX(xc,p.xc),MAX(yc,p.yc));
}

Point Point::min(Point p)
{
	return Point(MIN(xc,p.xc),MIN(yc,p.yc));
}

bool Point::isEqual(const Object& p)
{
	return p.isSpecies(class_Point) && *this==*(Point*)&p;
}

const Class* Point::species()	{ return &class_Point; }

unsigned Point::hash()	{ return xc^yc; }

int Point::compare(const Object& p)
{
	int t;
	assertArgSpecies(p,class_Point,"compare");
	if ((t=yc-((Point*)&p)->yc) != 0) return t;
	else return (xc-((Point*)&p)->xc);
}

Object* Point::copy()		{ return shallowCopy(); }

void Point::deepenShallowCopy()	{}

void Point::printOn(ostream& strm)
{
	strm << "(" << xc << " @ " << yc << ")";
}

Point::Point(istream& strm, Point& where)
{
	this = &where;
	strm >> xc >> yc;
}

void Point::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << xc << " " << yc << " ";
}

Point::Point(fileDescTy& fd, Point& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Point::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}
