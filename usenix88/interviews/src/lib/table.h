/*
 * Object association table.
 */

#ifndef table_h
#define table_h

#include <InterViews/defs.h>

typedef void* TableIndex;
typedef void* TableValue;

class Table {
    struct TableEntry {
	TableIndex index;
	TableValue value;
	TableEntry* chain;
    };

    int size;
    TableEntry** first, ** last;
    TableEntry* Start (TableIndex i) { return first[(int)i & size]; }
    TableEntry** StartAddr (TableIndex i) { return &first[(int)i & size]; }
public:
    Table(int);
    ~Table();
    void Insert(TableIndex, TableValue);
    boolean Find(TableValue&, TableIndex);
    void Remove(TableIndex);
};

extern Table* assocTable;

#endif
