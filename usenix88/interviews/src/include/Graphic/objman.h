/*
 * Interface to ObjectMan (object manager) class.
 */

#ifndef objectman_h
#define objectman_h

#include "classes.h"
#include "persistent.h"
#include <InterViews/defs.h>

class Cache;
class File;
class Ref;
class RefList;

typedef long ObjOffset;

extern char* OBJMAP_POSTFIX;
extern char* OBJSTORE_POSTFIX;

#define INVALIDOFFSET 1
#define NAMESIZE 100

class ObjectMan : public Persistent {
protected:
    void (*userInitializer)(RefList*);
    Persistent* (*userCreator)(ClassId id);

    UID lastuid;		// last UID I allocated
    ObjOffset lastuidOffset;	// spot where lastuid is written in objStore
    ObjOffset currentOffset;    // offset of last Persistent calling Write

    char filename [NAMESIZE];   // we'll gag if names get too big
    Cache* objCache;
    File* objMap, *objStore;

    RefList* root;

    ObjOffset getOffset(UID uid) { return (uid >> 1) * sizeof(ObjOffset); }
    boolean seek(UID uid);	// moves to proper position in objStore
    UID nextUID() { return lastuid += 2; }
    boolean read(File*);
    boolean write(File*);
public:
    ClassId GetClassId() { return OBJECTMAN; }
    boolean IsA(ClassId id) { return GetClassId()==id || Persistent::IsA(id); }

    ObjectMan(
	char* filename, 
	void (*userInitializer)(RefList*) =nil,
	Persistent* (*userCreator)(ClassId) =nil
    );
	// The parameter userInitializer must point to a function that
	// creates primordial root objects needed by your application,
	// and installs them in the passed RefList.
	// It will be called only when the application is run
	// for the FIRST TIME.  Henceforth, such objects will be read
	// from the persistent object store.

	// The parameter userCreator must point to a function that
	// can create an instance of ANY persistent object used in the
	// application, given a ClassId.
	// Typically it will deal explicitly with Classes derived
	// specifically for this application, and call library-defined 
	// functions for Classes taken from libraries.

    ObjectMan();
    ~ObjectMan();
    
    RefList* GetRoot() { return root; }

    Persistent* Create(ClassId id);
    UID GetUID(Persistent*);
	// returns a unique UID for the Persistent
	// looks in cache first, allocates new UID if not found
	
    boolean Invalidate(Persistent*);
	// sets the objStore offset associated with the object in the
	// objMap to INVALIDOFFSET; useful when deallocating an object
	// once and for all

    boolean IsCached(Ref*);
	// looks in the cache for the referenced object; returns false if
	// not found, true otherwise.  Puts the object into Ref as a side
	// effect
    boolean Find(Ref*);
	// put Persistent * into Ref, maybe seeking for and reading Persistent
	// from disk and return true if all went well
    boolean Retrieve(Ref*);
	// reads Persistent blindly from disk (no seek) and return true
	// if all went well.  Assumes object referred to by *ref is
	// NOT in memory.  Used when reading clustered objects,
	// for which no offset exists in the objMap since clustered objects
	// are written consecutively

    boolean Store(Persistent*);	    // find new (disk) space for Persistent
    boolean Update(Persistent*);    // update objMap for Persistent

    boolean ObjectIsDirty(Persistent*);
    void ObjectTouch(Persistent*);
    void ObjectClean(Persistent*);
};

#endif
