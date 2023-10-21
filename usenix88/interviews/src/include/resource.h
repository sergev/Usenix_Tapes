/*
 * References to resource objects are counted for convenient sharing.
 * As a double-check for debugging, we also keep a pointer to the
 * object itself inside the object.
 */

#ifndef resource_h
#define resource_h

class Resource {
    Resource* self;
    unsigned refcount;
public:
    Resource () { self = this; refcount = 1; }
    ~Resource () {
	--refcount;
	if (refcount == 0) {
	    self = (Resource*) -2;
	} else {
	    this = 0;
	}
    }
    void Reference () { ++refcount; }
    int BadRef () { return self != this; }
    int LastRef () { return refcount == 1; }
};

#endif
