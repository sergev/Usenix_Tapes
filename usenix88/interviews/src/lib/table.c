/*
 * Associative map between objects.
 */

#include "table.h"

extern void bzero(void*, int);

Table::Table (int n) {
    typedef TableEntry* EntryPtr;

    size = 128;
    while (size < n) {
	size <<= 1;
    }
    first = new EntryPtr[size];
    bzero(first, size * sizeof(EntryPtr));
    --size;
    last = &first[size];
}

Table::~Table () {
}

void Table::Insert (TableIndex i, TableValue v) {
    register TableEntry* e;
    register TableEntry** a;

    e = new TableEntry;
    e->index = i;
    e->value = v;
    a = StartAddr(i);
    e->chain = *a;
    *a = e;
}

boolean Table::Find (TableValue& v, TableIndex i) {
    register TableEntry* e;

    for (e = Start(i); e != nil; e = e->chain) {
	if (e->index == i) {
	    v = e->value;
	    return true;
	}
    }
    return false;
}

void Table::Remove (TableIndex i) {
    register TableEntry* e, * prev, ** a;

    a = StartAddr(i);
    e = *a;
    if (e != nil) {
	if (e->index == i) {
	    *a = e->chain;
	    delete e;
	} else {
	    do {
		prev = e;
		e = e->chain;
	    } while (e != nil && e->index != i);
	    if (e != nil) {
		prev->chain = e->chain;
		delete e;
	    }
	}
    }
}
