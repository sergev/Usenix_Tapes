/* ObjectId.c -- member functions of class ObjectId

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: {decvax!}seismo!elsie!cecil!keith
	October, 1985

Function:
	
Provides an object that contains an obid (Object pointer).

Modification History:
	
7-Jan-86	K. E. Gorlen

1.  Add ObjectId::species().

2.  Modify ObjectId::isEqual to use species().

*/

#include "ObjectId.h"

#define	THIS	ObjectId
#define	BASE	Object
DEFINE_CLASS(ObjectId,Object,1,NULL);

bool ObjectId::isEqual(Object& ob)
{
	return ob.isSpecies(class_ObjectId) && (id == ((ObjectId*)&ob)->value());
}

Class* ObjectId::species()	{ return &class_ObjectId; }

int ObjectId::compare(Object& ob)
{
	return ob.compare(*id);
}

int ObjectId::hash()	{ return (int)id; }
	
void ObjectId::printOn(ostream& strm)
{
	strm << *id;
}

static void ObjectId_reader(istream& strm, Object& where)
{
	ObjectId* temp = new ObjectId(strm,*(ObjectId*)&where);
}

ObjectId::ObjectId(istream& strm, ObjectId& where)
{
	this = &where;
	id = readFrom(strm);
}

void ObjectId::storer(ostream& strm)
{
	BASE::storer(strm);
	id->storeOn(strm);
}
