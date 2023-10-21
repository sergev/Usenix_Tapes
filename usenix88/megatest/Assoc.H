#ifndef NAME_TABLE_INCLUDED
#define NAME_TABLE_INCLUDED

#include <string.h>
#include "heap.H"
#include <generic.h>

// Author: Dave Jones

struct VOID   // structure of unknown size and content
{ char* pointer;  // Assoc's make this null
};

class Assoc_entry
{  
  friend class Assoc;
  friend class Assoc_iterator;
  char* key;
  VOID* contents;
 public:
  inline const char* index() { return key; }
  inline const VOID* value() { return contents; }
};


class Assoc  // generic associative array
{
  virtual int hash( char* whatever ) { return (int) whatever; }

  friend class Assoc_iterator;
  Heap heap;
  Assoc_entry* hash_table;  
  int size;
  int max_entries;
  int mask;
  int num_entries;
  inline int HASH( char* key) { return (hash(key)) & mask;}
  inline int REHASH(int num)  { return ((((num)+1)*3) & mask); }
  void overflow();
  Assoc_entry* new_table(int);

 public:
  
  virtual char* copy_key(char* key) { return key; }
  virtual void delete_key(char* key) {}
  virtual int equiv( char* key1,  char* key2) { return key1 == key2; }
  virtual void init( VOID *member ) { member->pointer = 0; }

  Assoc( int struct_size, int initial_size = 4 );

  ~Assoc();

  VOID* lookup(char*);
  VOID& operator[] (char* key) { return *Assoc::lookup(key); }
  void remove( char * );

  inline int entries( ) { return num_entries; }

};

class Name_table : public Assoc
{
  int hash( char* );
  
public:
  char* copy_key(char *);
  void delete_key(char*);
  int equiv(char*, char*);
  Name_table(int struct_size, int initial_size = 4) : 
     (struct_size, initial_size) {}

};


// CAVEAT
// Do not use assoc::operator[] or assoc::remove() while an assoc_iterator
// is active, or risk having the iterator miss some entries.

class Assoc_iterator  
{
  Assoc* cs;
  int i;
 public:
  Assoc_iterator(Assoc& s) { cs = &s; i = 0; }
  Assoc_entry* operator() ();
};


#define NAME_TABLE(_name_,_type_) \
class name2(_name_,_table) : public Name_table \
{ \
  public: \
    name2(_name_,_table) () : (sizeof(_type_)) {} \
    _type_& operator[] (char* key) \
      { return *((_type_*)Name_table::lookup(key));} \
}; \
\
class name2(_name_,_entry) : public Assoc_entry \
{ \
  public: \
    inline const _type_* value() { return (_type_*)(Assoc_entry::value()); } \
};  \
\
class name2(_name_,_iterator): public Assoc_iterator \
{ \
  public: \
    name2(_name_,_entry)* operator() () \
      { return (name2(_name_,_entry)*) Assoc_iterator::operator() (); } \
}; 
#endif NAME_TABLE_INCLUDED
