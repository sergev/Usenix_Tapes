/* RefList class */

#ifndef reflist_h
#define reflist_h

#include "ref.h"
#include <InterViews/defs.h>

class RefList : public Ref {
protected:
    RefList* next;
    RefList* prev;
public:
    RefList() : () { next = this; prev = this; }
    RefList(UID u) : (u) { next = this; prev = this; }
    RefList(Persistent* p) : (p) { next = this; prev = this; }
    ~RefList() {
	uid = INVALIDUID;
        next = (RefList*) -2;
        prev = (RefList*) -3;
    }
    void SetRef(Ref r);

    boolean IsEmpty() { return next == this; }
    void Append(RefList*);
    void Prepend(RefList*);
    void Remove(RefList*);
    void Delete(Ref);
    RefList* Find(Ref);
    RefList* First() { return next; }
    RefList* Last() { return prev; }
    RefList* End() { return this; }
    RefList* Next() { return next; }
    RefList* Prev() { return prev; }

    boolean Write(File*);
    boolean Read(File*);
    boolean WriteObjects(File*);
    boolean ReadObjects(File*);

    RefList* operator[](int count);
};

inline void RefList::SetRef (Ref r) {
    uid = r.uid;
}

inline void RefList::Append (RefList* e) {
    prev->next = e;
    e->prev = prev;
    e->next = this;
    prev = e;
}

inline void RefList::Prepend (RefList* e) {
    next->prev = e;
    e->prev = this;
    e->next = next;
    next = e;
}

inline void RefList::Remove (RefList* e) {
    e->prev->next = e->next;
    e->next->prev = e->prev;
    e->prev = (RefList*) -1;
    e->next = (RefList*) -1;
}

inline void RefList::Delete (Ref p) {
    register RefList* e;

    e = Find(p);
    if (e != nil) {
	Remove(e);
	delete e;
    }
}

#endif
