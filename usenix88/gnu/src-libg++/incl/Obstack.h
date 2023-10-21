// This may look like C code, but it is really -*- C++ -*-
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


#ifndef OBSTACKH
#define OBSTACKH

#include <std.h>

class Obstack
{
  class _obstack_chunk
  {
    friend class    Obstack;
    char*           limit;
    _obstack_chunk* prev;
    char            contents[4];
  };

  long	          chunksize;
  _obstack_chunk* chunk;
  char*	          objectbase;
  char*	          nextfree;
  char*	          chunklimit;
  int             alignmentmask;

  void  _free(void* obj);
  void  newchunk(int size);

public:
        Obstack(int size = 4092, int alignment = 4);   // 4092 = 4096 - 4

        ~Obstack();

  void* base();
  void* next_free();
  int   alignment_mask();
  int   chunk_size();
  int   size();
  int   room();

  void  grow(const void* data, int size);
  void  grow(const void* data, int size, char terminator);
  void  grow(const char* s);
  void  grow(char c);
  void  grow_fast(char c);
  void  blank(int size);
  void  blank_fast(int size);

  void* finish();
  void* finish(char terminator);

  void* copy(const void* data, int size);
  void* copy(const void* data, int size, char terminator);
  void* copy(const char* s);
  void* copy(char c);
  void* alloc(int size);

  void  free(void* obj);

};

#endif // OBSTACKH
