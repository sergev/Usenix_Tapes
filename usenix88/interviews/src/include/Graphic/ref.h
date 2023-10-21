/*
 * Interface to Ref (persistent object reference) base class.
 */

#ifndef ref_h
#define ref_h

#include "file.h"
#include "objman.h"
#include "persistent.h"
#include <stdio.h>

const UID CLUSTERBITMASK = 0x80000000;
const UID INMEMORYBITMASK = 0x1;

class Ref {
    friend class Cache;
    friend class ObjectMan;
protected:
    union {			    // distinguished in that a UID has lsb == 1
	Persistent* refto;
	UID uid;
    };

    void Warning(const char*);
    void Panic(const char*, int);
    
    boolean inMemory();
    boolean isCluster();	    // checks cluster bit (msb UID); it's set
				    // if object is a head of a cluster
				    // (doesn't check if ref is inMemory!)
    void setClusterBit();
    void resetClusterBit();
    inline UID getUID();	    // removes cluster bit

    Persistent* ref();		    // returns object (possibly faulting it in)
    Persistent* refObjects();	    // seekless ref() for faulting in 
				    // consecutive objects
    void unref();		    // converts ref to uid w/cluster bit
public:
    Ref () { uid = INVALIDUID; }
    Ref (UID u) { uid = u; }
    Ref (Persistent* obj)
	{ refto = (obj == nil) ? (Persistent*) INVALIDUID : obj; }
    
    Persistent* operator()();
    boolean Valid();		    // true if ref is non-nil/non-INVALIDUID

    boolean operator==(Ref r);
    boolean operator!=(Ref r);

    boolean Write(File*);	    // write uid + cluster bit
    boolean Read(File*);	    // read uid + cluster bit
    boolean WriteObjects(File*);    // write object if not head of a cluster
    boolean ReadObjects(File*);	    // read object if not head of a cluster
};

inline boolean Ref::inMemory () {
    return (uid & INMEMORYBITMASK) == 0;    // test inMemory bit
}

inline boolean Ref::isCluster () {
    return (uid & CLUSTERBITMASK) != 0;	
}

inline void Ref::setClusterBit () {
    uid = uid | CLUSTERBITMASK;
}

inline void Ref::resetClusterBit () {
    uid = uid & ~CLUSTERBITMASK;
}

UID Ref::getUID () {
    return inMemory() && (refto!=nil) ? refto->getUID() : uid&~CLUSTERBITMASK;
}

inline Persistent* Ref::operator() () {
    return ref();
}

inline boolean Ref::Valid () {
    return uid != INVALIDUID && refto != nil;
}

inline boolean Ref::operator== (Ref r) {
    if (r.inMemory() && inMemory()) {
	return r.refto == refto;

    } else if (!r.inMemory() && !inMemory()) {
	return (r.uid & ~CLUSTERBITMASK) == (uid & ~CLUSTERBITMASK) ;

    } else if (!r.Valid() && !Valid()) {
	return true;

    } else if (r.Valid() != Valid()) {
	return false;

    } else {
	return r.getUID() == getUID();
    }
}

inline boolean Ref::operator!= (Ref r) {
    return ! (*this == r);
}

inline boolean Ref::Write (File* f) {
    unref();
    return f->Write(uid);
}

inline boolean Ref::Read (File* f) {
    return f->Read(uid);
}

inline boolean Ref::WriteObjects (File* f) {
    if (isCluster()) {
	return true;	    // if it's the head of a cluster, we're not
			    // responsible for writing it out
    } else if ( inMemory() || TheManager->IsCached(this) ) {
	return refto->writeObjects(f);
    } else if (uid != INVALIDUID) {
	Warning("object within a cluster not in memory prior to write");
	return false;
    }
}

inline boolean Ref::ReadObjects (File*) {
    if (!isCluster()) {
	(void*) refObjects();
    }
    return true;
}

#endif
