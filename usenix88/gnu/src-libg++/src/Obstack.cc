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


#include <Obstack.h>
 

Obstack::Obstack(int size = 4092, int alignment = 4)
{
  alignmentmask = alignment - 1;
  chunk = (_obstack_chunk*) (new char[chunksize = size]);
  nextfree = objectbase = chunk->contents;
  chunklimit = chunk->limit = (char*)(chunk) + chunksize;
  chunk->prev = 0;
}

void Obstack::_free(void* obj)
{
  register _obstack_chunk*  lp;
  register _obstack_chunk*  plp;

  lp = chunk;
  while (lp != 0 && ((void*)lp > obj || (void*)(lp)->limit < obj))
  {
    plp = lp -> prev;
    delete(lp);
    lp = plp;
  }
  if (lp)
  {
    objectbase = nextfree = (char *)(obj);
    chunklimit = lp->limit;
    chunk = lp;
  }
  else if (obj != 0)
    abort();
}

void Obstack::newchunk(int size)
{
  register _obstack_chunk*	old_chunk = chunk;
  register _obstack_chunk*	new_chunk;
  register long	new_size;
  register int obj_size = nextfree - objectbase;

  new_size = (obj_size + size) << 1;
  if (new_size < chunksize)
    new_size = chunksize;

  new_chunk = chunk = (_obstack_chunk*)(new char[new_size]);
  new_chunk->prev = old_chunk;
  new_chunk->limit = chunklimit = (char *) new_chunk + new_size;

  bcopy((void*)objectbase, (void*)new_chunk->contents, obj_size);
  objectbase = new_chunk->contents;
  nextfree = objectbase + obj_size;
}

void* Obstack::finish()
{
  void* value = (void*) objectbase;
  nextfree = (char*)((int)(nextfree + alignmentmask) & ~(alignmentmask));
  if (nextfree - (char*)chunk > chunklimit - (char*)chunk)
    nextfree = chunklimit;
  objectbase = nextfree;
  return value;
}

  
#define SHOULD_BE_INLINE


SHOULD_BE_INLINE Obstack::~Obstack()
{
  _free(0); 
}

SHOULD_BE_INLINE void* Obstack::base()
{
  return objectbase; 
}

SHOULD_BE_INLINE void* Obstack::next_free()
{
  return nextfree; 
}

SHOULD_BE_INLINE int Obstack::alignment_mask()
{
  return alignmentmask; 
}

SHOULD_BE_INLINE int Obstack::chunk_size()
{
  return chunksize; 
}

SHOULD_BE_INLINE int Obstack::size()
{
  return nextfree - objectbase; 
}

SHOULD_BE_INLINE int Obstack::room()
{
  return chunklimit - nextfree; 
}

SHOULD_BE_INLINE void Obstack:: grow(const void* data, int size)
{
  if (nextfree+size > chunklimit) 
    newchunk(size);
  bcopy(data, nextfree, size);
  nextfree += size; 
}

SHOULD_BE_INLINE void Obstack:: grow(const void* data, int size, char terminator)
{
  if (nextfree+size+1 > chunklimit) 
    newchunk(size+1);
  bcopy(data, nextfree, size);
  nextfree += size; 
  *(nextfree)++ = terminator; 
}

SHOULD_BE_INLINE void Obstack:: grow(const char* s)
{
  grow((void*)s, strlen(s), 0); 
}

SHOULD_BE_INLINE void Obstack:: grow(char c)
{
  if (nextfree+1 > chunklimit) 
    newchunk(1); 
  *(nextfree)++ = c; 
}

SHOULD_BE_INLINE void Obstack:: blank(int size)
{
  if (nextfree+size > chunklimit) 
    newchunk(size);
  nextfree += size; 
}

SHOULD_BE_INLINE void* Obstack::finish(char terminator)
{
  grow(terminator); 
  return finish(); 
}

SHOULD_BE_INLINE void* Obstack::copy(const void* data, int size)
{
  grow (data, size);
  return finish(); 
}

SHOULD_BE_INLINE void* Obstack::copy(const void* data, int size, char terminator)
{
  grow(data, size, terminator); 
  return finish(); 
}

SHOULD_BE_INLINE void* Obstack::copy(const char* s)
{
  grow((void*)s, strlen(s), 0); 
  return finish(); 
}

SHOULD_BE_INLINE void* Obstack::copy(char c)
{
  grow(c);
  return finish(); 
}

SHOULD_BE_INLINE void* Obstack::alloc(int size)
{
  blank(size);
  return finish(); 
}

SHOULD_BE_INLINE void Obstack:: free(void* obj)     
{
  if (obj >= (void*)chunk && obj<(void*)chunklimit)
    nextfree = objectbase = obj;
  else 
    _free(obj); 
}

SHOULD_BE_INLINE void Obstack:: grow_fast(char c)
{
  *(nextfree)++ = c; 
}

SHOULD_BE_INLINE void Obstack:: blank_fast(int size)
{
  nextfree += size; 
}
