/*
 * Interface to associative Cache class.
 */

#ifndef cache_h
#define cache_h

#include "ref.h"
#include <InterViews/defs.h>

static const UID DIRTYBITMASK = 0x80000000;

class CacheEntry;

class Cache {
    friend class ObjectMan;
protected:
    int size;
    CacheEntry** hashTable;

    boolean isDirty(Ref uid) { return (uid.uid & DIRTYBITMASK) != 0; }
	// the value of a pointer tag contains the dirty bit
	// (note that we don't do the same for the reverse mapping)
    void touch(Ref& uid) { uid.uid = uid.uid | DIRTYBITMASK; }
    void clean(Ref& uid) { uid.uid = uid.uid & ~DIRTYBITMASK; }

    int hash(Ref tag) { clean(tag); return tag.uid % size; }
	// clean and dirty tags must hash to the same value
    boolean tagsEqual(Ref tag1, Ref tag2);  
	// a quick & dirty check; returns false if one of the tags is
	// is inMemory and the other isn't
public:
    Cache(int size);
    ~Cache();

    boolean IsDirty(Ref ptr);		// we don't check to make sure
    void Touch(Ref ptr);		// ptr is really in memory, so
    void Clean(Ref ptr);		// you'd better make sure it is!
    void TouchAll();
    void CleanAll();

    void Set(Ref tag, Ref val);
    void Unset(Ref tag);
    Ref Get(Ref tag);
    
    boolean Flush();			// does a Save on valid pointers
};

inline boolean Cache::tagsEqual (Ref tag1, Ref tag2) {
    if (tag1.inMemory() != tag2.inMemory()) {
	return false;
    } else if (!tag1.inMemory()) {
	tag1.resetClusterBit();
	tag2.resetClusterBit();
	return tag1.uid == tag2.uid;
    } else {
	return tag1.refto == tag2.refto;
    }
}

#endif
