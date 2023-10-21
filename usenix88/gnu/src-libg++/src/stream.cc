/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of GNU CC.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU CC General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU CC, but only under the conditions described in the
GNU CC General Public License.   A copy of this license is
supposed to have been given to you along with GNU CC so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  
*/

#include <stream.h>
#include <ctype.h>

ostream::ostream(const char* filename, io_mode m, access_mode a)
:(filename, m, a) {}

ostream::ostream(const char* filename, const char* m)
:(filename, m) {}

ostream::ostream(int filedesc, io_mode m = io_writeonly)
:(filedesc, m) {}

ostream::ostream(FILE* fileptr)
:(fileptr) {}

istream::istream(const char* filename, io_mode m, access_mode a) 
:(filename, m, a) { tied_to = 0; }

istream::istream(const char* filename, const char* m)
:(filename, m) { tied_to = 0; }

istream::istream(int filedesc, io_mode m = io_readonly, ostream *t = 0)
:(filedesc, m) { tied_to = t; }

istream::istream(FILE* fileptr)
:(fileptr) { tied_to = 0; }

istream& istream::operator>>(whitespace&)
{
  char ch;
  while((File::get(ch)) && isspace(ch));
  if (good())
    unget(ch);
  return *this;
}

/*
 * predeclared streams
 */

istream  cin(stdin);
ostream  cout(stdout);
ostream  cerr(stderr);

whitespace WS;

#define SHOULD_BE_INLINE


SHOULD_BE_INLINE void* ostream::operator void*()
{ 
  return (fail() || bad())? 0 : this ; 
}

SHOULD_BE_INLINE ostream& ostream::operator<<(char   c)
{ 
  put(c);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(short  n)
{ 
  form("%d",(int)n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(unsigned short n)
{ 
  form("%u",(unsigned)n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(int    n)
{ 
  form("%d",n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(unsigned int n)
{ 
  form("%u",n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(long   n)
{ 
  form("%ld",n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(unsigned long n)
{ 
  form("%lu",n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(float  n)
{ 
  form("%g",(double)n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(double n)
{ 
  form("%g",n);
  return *this;
}

SHOULD_BE_INLINE ostream& ostream::operator<<(const char* s)
{ 
  put(s);
  return *this;
}

SHOULD_BE_INLINE void*    istream::operator void*()
{ 
  return (fail() || bad())? 0 : this ; 
}

SHOULD_BE_INLINE istream& istream::operator>>(char&   c)
{ 
  get(c);
  return *this;
}

SHOULD_BE_INLINE istream& istream::operator>>(short&  n)
{ 
  scan("%hd", &n); 
  return *this;
}

SHOULD_BE_INLINE istream& istream::operator>>(unsigned short& n)
{ 
  scan("%hd", &n); 
  return *this;
}

SHOULD_BE_INLINE istream& istream::operator>>(int&    n)
{ 
  scan("%d",  &n); 
  return *this;
}

SHOULD_BE_INLINE istream& istream::operator>>(unsigned int& n)
{ 
  scan("%d",  &n); 
  return *this;
}

SHOULD_BE_INLINE istream& istream::operator>>(long&   n)
{ 
  scan("%ld", &n); 
  return *this; 
}

SHOULD_BE_INLINE istream& istream::operator>>(unsigned long& n)
{ 
  scan("%ld", &n); 
  return *this;
}

SHOULD_BE_INLINE istream& istream::operator>>(float&  n)
{ 
  scan("%f",  &n); 
  return *this; 
}

SHOULD_BE_INLINE istream& istream::operator>>(double& n)
{ 
  scan("%lf", &n); 
  return *this; 
}

SHOULD_BE_INLINE istream& istream::operator>>(char*   s)
{ 
  scan("%s",   s); 
  return *this;
}

