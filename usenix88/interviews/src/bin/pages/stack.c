/*
 * Stack
 */

#include "stack.h"

struct StackEntry {
    void * value;
    StackEntry * above;
    StackEntry * below;

    StackEntry() { value = nil; above = nil; below = nil; }
    StackEntry( void * v ) { value = v; above = nil; below = nil; }
    StackEntry( StackEntry& t ) { value = t.value; above = nil; below = nil; }
};

Stack::Stack () {
    top = nil;
}

Stack::~Stack() {
    while ( top != nil ) {
	top = top->below;
	delete top->above;
    }
}

void Stack::Push( void * v ) {
    StackEntry * entry = new StackEntry( v );
    if ( top != nil ) {
	entry->below = top;
	top->above = entry;
    }
    top = entry;
}

void Stack::Push( int i ) {
    StackEntry * entry = new StackEntry( (void*)i );
    if ( top != nil ) {
	entry->below = top;
	top->above = entry;
    }
    top = entry;
}

void Stack::Pop( void*& v ) {
    if ( top != nil ) {
	v = top->value;
	top = top->below;
	if ( top != nil ) {
	    delete top->above;
	    top->above = nil;
	}
    } else {
	v = nil;
    }
}

void Stack::Pop( int & i ) {
    if ( top != nil ) {
	i = (int)top->value;
	top = top->below;
	if ( top != nil ) {
	    delete top->above;
	    top->above = nil;
	}
    } else {
	i = 0;
    }
}

void Stack::Peek( void*& v ) {
    if ( top != nil ) {
	v = top->value;
    } else {
	v = nil;
    }
}

void Stack::Peek( int & i ) {
    if ( top != nil ) {
	i = (int)top->value;
    } else {
	i = 0;
    }
}

void Stack::Drop() {
    void * dummy;
    Pop( dummy );
}

void Stack::Dup() {
    void * dummy;
    Peek( dummy );
    Push( dummy );
}

void Stack::Swap() {
    void * dummy1;
    void * dummy2;
    Pop( dummy1 );
    Pop( dummy2 );
    Push( dummy1 );
    Push( dummy2 );
}

boolean Stack::IsEmpty() {
    return top==nil;
}
