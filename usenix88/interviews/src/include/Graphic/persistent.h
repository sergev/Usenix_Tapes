/*
 * Interface to Persistent and related base classes.
 */

#ifndef persistent_h
#define persistent_h

#include "classes.h"
#include <InterViews/defs.h>

class ObjectMan;
class Persistent;
class ListObject;

extern ObjectMan* TheManager;	    // the well-known global object manager

class File;

typedef unsigned UID;
#define INVALIDUID	1
			    // INVALIDUID & OBJMANUID must be consecutive uids
#define OBJMANUID	3   

class Persistent {		    // basic (persistent) Persistent
    friend class Ref;
    friend class ObjectMan;
protected:
    virtual UID getUID();

    void Warning(const char*);
    void Panic(const char*);
    void Fatal(const char*);

    virtual boolean write(File*);
    virtual boolean read(File*) { return true; }
    virtual boolean writeObjects(File*);
    virtual boolean readObjects(File*) { return true; }
    virtual boolean initialize() { return true; }
public:
    virtual ClassId GetClassId() { return PERSISTENT; }
    virtual boolean IsA(ClassId id) { return GetClassId() == id; }

    Persistent();
    ~Persistent();

    virtual Persistent* GetCluster() { return this; }
	// returns head of cluster.  If GetCluster is redefined to return
	// nil, then the object is assumed to be within the "current"
	// cluster, i.e., it is a part of whatever cluster is currently
	// being written out or read in.  If the object is Saved explicitly,
	// a warning is issued and the object is NOT saved.

    virtual boolean Save();
    virtual boolean IsDirty();
    virtual void Touch();
    virtual void Clean();
};

#endif
