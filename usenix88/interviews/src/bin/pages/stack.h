/*
 * Stack
 */

#ifndef stack_h
#define stack_h

#include <InterViews/defs.h>

class StackEntry;

class Stack {
    StackEntry * top;
public:
    Stack();
    ~Stack();

    void Push( void* );
    void Push( int );

    void Pop( void*&  );
    void Pop( int& );

    void Peek( void*& );
    void Peek( int& );

    void Drop();
    void Dup();
    void Swap();

    boolean IsEmpty();
};

#endif
