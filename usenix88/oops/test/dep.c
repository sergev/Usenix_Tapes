/* Test Object dependencies

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

Function:
	
Modification History:
	
$Log:	dep.c,v $
 * Revision 1.2  88/02/04  13:37:54  keith
 * Make extern Class class_CLASSNAME const.
 * 
 * Revision 1.1  88/01/17  22:24:34  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: dep.c,v 1.2 88/02/04 13:37:54 keith Exp $";

#include "Point.h"
#include "OrderedCltn.h"

extern const Class class_MyPoint;
overload MyPoint_reader;

class MyPoint : public Point {
protected:
	MyPoint(fileDescTy&,MyPoint&) {}
	MyPoint(istream&,MyPoint&) {}
	friend	void MyPoint_reader(istream& strm, Object& where);
	friend	void MyPoint_reader(fileDescTy& fd, Object& where);
public:
	MyPoint(int newx, int newy) : (newx,newy) {}
	~MyPoint()	{ release(); }
	int x()		{ return Point::xc; }
	int y()		{ return Point::yc; }
	int x(int newx)	{ Point::xc = newx; changed(); return Point::xc; }
	int y(int newy)	{ Point::yc = newy; changed(); return Point::yc; }
	virtual Class* isA();
};

DEFINE_CLASS(MyPoint,Point,1,"$Header: dep.c,v 1.2 88/02/04 13:37:54 keith Exp $",NULL,NULL);

extern const Class class_PointView;
overload PointView_reader;

class PointView : public Object {
	Point* p;
protected:
	PointView(fileDescTy&,PointView&) {}
	PointView(istream&,PointView&) {}
	friend	void PointView_reader(istream& strm, Object& where);
	friend	void PointView_reader(fileDescTy& fd, Object& where);
public:
	PointView(Point& newp)	{ p = &newp; p->addDependent(*this); update(*p,*nil); }
	~PointView()		{ p->removeDependent(*this); }
	virtual Class* isA();
	virtual void update(const Object&, const Object&);
	virtual void printOn(ostream& strm);
};

DEFINE_CLASS(PointView,Object,1,"$Header: dep.c,v 1.2 88/02/04 13:37:54 keith Exp $",NULL,NULL);

void PointView::update(const Object& p, const Object& /*unused*/)
{
	cout << p.className() << " changed: " << p << "\n";
	cout.flush();
}

void PointView::printOn(ostream& strm)
{
	strm << className() << *p << "\n";
}

main()
{
	cout << "Test Object Dependencies\n";
	MyPoint p(0,0);
	PointView* v = new PointView(p);
	cout << p.dependents() << "\n";
	p.x(1); p.y(2);		/* should print changes */
	delete v;
	cout << p.dependents() << "\n";
	p.x(3); p.y(4);		/* should not print changes */
	v = new PointView(p);
	p.x(5); p.y(6);		/* should print changes */
	p.release();
	cout << p.dependents() << "\n";
	p.x(7); p.y(8);		/* should not print changes */
}
