/**************************************************************************/
/**************************************************************************/


                  /***** Block Memory Allocator *****/


/**************************************************************************/
/**************************************************************************/

/* Author: JP Massar */

#include <stdio.h>

#include "sys5.h"

#ifdef BSD42
#include <strings.h>
#endif

#define NO_MORE_MEMORY -1

static int bytes_left;                  /* space left in current block */
static char *ptr_next_byte;             /* next free byte */
static char *ptr_space;                 /* current block */
static int chunk_size;                  /* size of block last allocated */

extern char *malloc();


int allocate_memory_chunk (space) int space;

/* malloc up a new block of memory.  Set our static variables */
/* returns NO_MORE_MEMORY if can't allocate block. */

{ 
  if (0 == (ptr_space = malloc(space))) {
        fprintf(stderr,"fatal error, no more memory\n");
        return(NO_MORE_MEMORY);
  }
  ptr_next_byte = ptr_space;
  bytes_left = space;
  chunk_size = space;
  return(0);
}


char * get_memory_chunk (size) int size;

/* allocate a small segment out of our large block of memory.  If we */
/* run out allocate another block.  Adjust our static variables. */
/* returns 0 if no more memory. */

{ char *rval;
        
  if (size > chunk_size) {
        fprintf(stderr,"attempt to allocate too large a chunk\n");
        return(0);
  }
        
  if (size > bytes_left) {
        if (NO_MORE_MEMORY == allocate_memory_chunk(chunk_size)) {
                return(0);
        }
        return(get_memory_chunk(size));
  }

  rval = ptr_next_byte;
  ptr_next_byte += size;
  bytes_left -= size;
  return(rval);
  
}


char * store_string (str,len) char *str; int len;

/* put copy of a string into storage in a memory block.  Return a pointer to */
/* the copied string.  Returns 0 if out of memory. */

{ char *ptr_space;

  if (0 == (ptr_space = get_memory_chunk(len+1))) {
        return(0);
  }
  strcpy(ptr_space,str);
  return(ptr_space);
}
