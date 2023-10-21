/* Array.c.m4 -- template for implementations of generic array objects

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

m4 macro template for the .c files for arrays of the specified
fundamental type: char, int, short, long, unsigned, float, and double.
For example, to generate the implementation of an array of chars:

	m4 Array.c.m4 Arraychar.p >Arraychar.c

The type-specific part of the implementation is in the file
Arraychar.p, for example.

WARNING -- Make changes to the .p or .m4 files, not to the .c file
they generate.

$Log:	Array_c.m4,v $
Revision 1.3  88/02/04  12:51:59  keith
Make Class class_CLASSNAME const.
Make destructor non-inline.

Revision 1.2  88/01/16  23:37:10  keith
Remove pre-RCS modification history

*/

`#define' NEW(type,size)	((type*)malloc(sizeof(type)*(size)))

define(ARRAYIMPLEMENT,
$1::$1(unsigned size)
{
	sz = size;
	if (size==0) AllocSizeErr();
	else v = NEW($2,sz);
}

$1::$1(const $1& a)
{
	register unsigned i = a.sz;
	sz = i;
	v = NEW($2,i);
	register $2* vv = &v[0];
	register $2* av = &a.v[0];
	while (i--) *vv++ = *av++;
}

$1::~$1()	{ DELETE(v); }

void $1::operator=(const $1& a)
{
	if (v != a.v) {
		DELETE(v);
		v = NEW($2,sz=a.sz);
		register unsigned i = a.sz;
		register $2* vv = &v[0];
		register $2* av = &a.v[0];
		while (i--) *vv++ = *av++;
	}
}

bool $1::operator==(const $1& a)
{
	if (sz != a.sz) return NO;
	register unsigned i = sz;
	register $2* vv = &v[0];
	register $2* av = &a.v[0];
	while (i--) if (*vv++ != *av++) return NO;
	return YES;
}

unsigned $1::capacity()	{ return sz; }

bool $1::isEqual(const Object& a)
{
	return a.isSpecies(class_$1) && *this==*($1*)&a;
}

const Class* $1::species()	{ return &class_$1; }

void $1::reSize(unsigned newsz)
{
	if (newsz<=0) AllocSizeErr();
	v = REALLOC(v,newsz);
	sz = newsz;
}

void $1::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	register $2* av = &v[0];
	register unsigned i = sz;
	v = NEW($2,i);
	register $2* vv = &v[0];
	while (i--) *vv++ = *av++;
}

unsigned $1::size()	{ return sz; }

void $1::sort()
{
	qsort(v,sz,sizeof($2),$2Cmp);
}

extern const int OOPS_ALLOCSIZE;
extern const int OOPS_INDEXRANGE;

void $1::AllocSizeErr()
{
	setOOPSerror(OOPS_ALLOCSIZE,DEFAULT,this,className());
}
void $1::IndexRangeErr()
{
	setOOPSerror(OOPS_INDEXRANGE,DEFAULT,this,className());
}
)
