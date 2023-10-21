/*
 * Ref class implementation.  A Ref is a reference to a Persistent object.
 */

#include <InterViews/Graphic/ref.h>

void Ref::Warning (const char* msg) {
    fflush(stdout);
    fprintf(stderr, "warning: %s\n", msg);
}

void Ref::Panic (const char* msg, int n) {
    fflush(stdout);
    fprintf(stderr, msg, n);
    fprintf(stderr, "\n");
    exit(2);
}

Persistent* Ref::ref () {
    if (uid == INVALIDUID) {
	return (Persistent*) nil;
    } else if (refto != nil && !inMemory() && !TheManager->Find( this ) ) {
	Panic( "unable to find object ", uid );
	return (Persistent*) nil;	// so the compiler won't squawk
    } else {
	return refto;
    }
}

Persistent* Ref::refObjects () {
    if (uid == INVALIDUID) {
	return (Persistent*) nil;
    } else if (refto != nil && !inMemory() && !TheManager->Retrieve( this ) ) {
	Panic( "unable to find object ", uid );
	return (Persistent*) nil;	// so the compiler won't squawk
    } else {
	return refto;
    }
}

void Ref::unref () {
    if (inMemory() && refto != nil) {
	Persistent* obj = refto;
	uid = refto->getUID();
	
	if (obj->GetCluster() == obj) { // obj can't be nil here
	    setClusterBit();		// it is the head of a cluster
	} else {
	    resetClusterBit();
	    	// it's not the head of a cluster. Note that if GetCluster
		// returns nil (i.e., the object is part of whatever
		// cluster is currently being read/written), it's not
		// considered the head of a cluster
	}
    } else if (refto == nil) {
	uid = INVALIDUID;
    }
}

