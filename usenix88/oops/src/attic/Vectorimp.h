#ifndef VECTORIMPH
#define VECTORIMPH

#include "Vector.h"

#define Vectorimplement(classname,type)					\
classname::classname(int size)						\
{									\
	sz = size;							\
	if (size<=0) AllocSizeErr();					\
	else v = new type[sz];						\
}									\
									\
classname::classname(const classname& a)				\
{									\
	register i = a.sz;						\
	sz = i;								\
	v = new type[i];						\
	register type* vv = &v[0];					\
	register type* av = &a.v[0];					\
	while (i--) *vv++ = *av++;					\
}									\
									\
void classname::operator=(const classname& a)				\
{									\
	if (v != a.v) {							\
		delete v;						\
		v = new type[sz=a.sz];					\
		register i = a.sz;					\
		register type* vv = &v[0];				\
		register type* av = &a.v[0];				\
		while (i--) *vv++ = *av++;				\
	}								\
}									\
									\
bool classname::operator==(const classname& a)				\
{									\
	if (sz != a.sz) return NO;					\
	register int i = sz;						\
	register type* vv = &v[0];					\
	register type* av = &a.v[0];					\
	while (i--) if (*vv++ != *av++) return FALSE;			\
	return YES;							\
}									\
									\
type& classname::at(int i)	{ return this->operator[](i); }		\
									\
int classname::capacity()	{ return sz; }				\
									\
bool classname::isEqual(const Object& a)				\
{									\
	return a.isSpecies(VECTOR_CLASS(classname)) && *this==*(classname*)&a;\
}									\
									\
const Class* classname::species()	{ return &VECTOR_CLASS(classname); }	\
									\
void classname::reSize(int newsz)					\
{									\
	if (newsz<=0) AllocSizeErr();					\
	type* newv = new type[newsz];					\
	register i = (newsz<=sz) ? newsz:sz;				\
	register type* vp = &v[0];					\
	register type* np = &newv[0];					\
	while (i--) *np++ = *vp++;					\
	delete v;							\
	v = newv;							\
	sz = newsz;							\
}									\
									\
obid classname::shallowCopy()						\
{									\
	shouldNotImplement("shallowCopy");				\
	return 0;							\
}									\
									\
void classname::deepenShallowCopy()					\
{									\
	register type* av = &v[0];					\
	register i = sz;						\
	v = new type[i];						\
	register type* vv = &v[0];					\
	while (i--) *vv++ = *av++;					\
}									\
									\
int classname::size()	{ return sz; }					\
									\
static int typeCmp(type&, type&);					\
									\
void classname::sort()							\
{									\
	qsort(v,sz,sizeof(type),typeCmp);				\
}									\
									\
static void classname/**/_reader(istream& strm, Object& where)		\
{									\
	new classname(strm,*(classname*)&where); 			\
}									\
									\
extern const int OOPS_ALLOCSIZE,OOPS_INDEXRANGE;			\
									\
void classname::AllocSizeErr()						\
{									\
	setOOPSerror(OOPS_ALLOCSIZE,DEFAULT,this,className());		\
}									\
void classname::IndexRangeErr()						\
{									\
	setOOPSerror(OOPS_INDEXRANGE,DEFAULT,this,className());		\
}									\

#endif
