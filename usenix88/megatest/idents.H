#ifndef IDENTS_INCLUDED
#define IDENTS_INCLUDED

#include "Assoc.H"


/* Underscores are valid, Case matters. (Mega-Pascal/C/C++) */
class Idents_UC: public Assoc
{ int hash( char*);
  int equiv( char*,  char*);

 public:
  Idents_UC(int struct_size, int initial_size = 4):(struct_size,initial_size)
    {}
};


/* Underscores are not valid. Case does not matter.  (Standard Pascal) */
class Idents_uc: public Assoc
{ int hash( char*);
  int equiv( char*,  char*);

 public:
  Idents_uc(int struct_size, int initial_size = 4):(struct_size,initial_size)
    {}
};

/* Underscores are valid, but "transparent glue" when not in the first
** position. Case does not matter.  E.g. "foo_bar" is equivalent to
** "FooBar". But "_foo" is not equivalent to "foo".
**
** This scheme passes the "telephone test":  You can read programs to
** people over the telephone, without constantly saying, "all caps",
** "underscore", etc..  I wish this were standard practice.
*/
class Idents_Tc: public Assoc  // T for telephone.
{ int hash( char*);
  int equiv( char*,  char*);

 public:
  Idents_Tc(int struct_size, int initial_size = 4):(struct_size,initial_size)
    {}
};


/* Underscores are not valid.  Case matters.  (Modula II) */
class Idents_uC: public Assoc
{ int hash( char*);
  int equiv( char*,  char*);

 public:
  Idents_uC(int struct_size, int initial_size = 4):(struct_size,initial_size)
    {}
};


/* Underscores are valid. Case does not matter. */
class Idents_Uc: public Assoc
{ int hash( char*);
  int equiv( char*,  char*);

 public:
  Idents_Uc(int struct_size, int initial_size = 4):(struct_size,initial_size)
    {}
};

#endif IDENTS_INCLUDED
