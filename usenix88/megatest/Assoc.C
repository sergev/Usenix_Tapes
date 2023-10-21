
#include "Assoc.H"

//  This is a ASSOCIATIVE ARRAY class, which maps arbitrary things to
//  structures of arbitary size.  There is also an ITERATOR CLASS for 
//  the associative arrays.
//
//  C.f.  C++ Programming Language book, sections 2.3.10 and 6.7.
//
//            

// Author: Dave Jones

//
//  NOTES:
//
//  Here's how it works.  We keep an array of Pairs, 
//     { char* key; VOID* contents }
//
//  A Pair is valid if and only if the contents is not null.
//  If contents is not null, it points to a struct of the given size.
//  We hash into the array to find keys, using a rehash function for
//  collision avoidance.  If you look up a previously invalid key,
//  we create a valid Pair for it.  By default, the first long int of a 
//  new field is zeroed, so a user may use it to determine whether or not an
//  array element is a new one or not.  Users may specify some other
//  initialization routine. (It's "virtual".)
//
//  To save space, the user may invalidate entries with the procedure remove().
//
//  When the array approaches half full, we double its size.
//
//  WARNING:  These algorithms are quite tricky. They are built for speed,
//  not clarity.  If you change something, you may very well introduce a bug 
//  which will not manifest itself except in very infrequent situations,
//  and which will be impossible to track down.  Fore-warned is skeptical,
//  but fore-armed.
//
//  For example, the algorithms will not work unless the table size is a 
//  power of two, and the table is kept strictly less than half full.
//  In particular, the lookup routine only provably terminates
//  because for a given pos. int. M, the sequence
//
//        s(0) = 0
//
//       s(i+1) = ((s(i)+1)*3) mod 2**M
//
//  repeats after exactly 2**(M-1) steps.  I have proved this true
//  for all pos. ints. M, but my proof is very inelegant.  You can
//  prove it true for small M, say less than 33, with a simple computer
//  program.  I have done that also, just to be on the safe side.
//



static Assoc_entry* new_table(int);
static int round_up_to_pwr2(int);


Assoc::Assoc( int struct_size, int initial_size)
     : heap(struct_size)
{
  size = round_up_to_pwr2(initial_size);
  num_entries = 0;
  max_entries = size/2 - 1;
  mask = size - 1;  // Binary arithmetic. x mod size == x & (size-1)
  hash_table = new_table(size);
}





Assoc::~Assoc()
{
  Assoc_iterator next(*this);
  Assoc_entry* bucket;

  while(bucket=next())
    delete_key(bucket->key);

  delete hash_table;
}




VOID*
Assoc::lookup( char* key )
{
  // The lookup may add an entry, so..
  if ( num_entries >= max_entries ) overflow();
  
  register int bucket_number = HASH(key);
  register Assoc_entry * bucket;

  while(1)
    {

      bucket = hash_table + bucket_number;

      if ( bucket->contents == 0 )
	{ 
	  bucket->key = copy_key(key);
	  bucket->contents = (VOID*)heap.alloc();
	  init(bucket->contents);
	  num_entries++;
	  break;    // <====== created new entry
	}
      
      if ( !equiv( bucket->key, key) )
	{ 
	  bucket_number = REHASH(bucket_number);
	  continue; // <====== search some more (collision)
	}

      break;        // <====== found old Assoc_entry

    }
  
  return bucket->contents;

}



void
Assoc::overflow()
{
  Assoc_entry* old_hash = hash_table;
  int old_size = size;

  max_entries = size - 1;
  size = size * 2;
  mask = size - 1;
  hash_table = new_table(size);
  
  /* Take everything that was in the old table, and put it
  ** into the new one.
  */

  register int recno;
  for (recno = 0; recno < old_size; recno++)
    { Assoc_entry *mem = old_hash + recno;
      if ( mem->key )
	{ 
	  register int bucket_number = HASH(mem->key);
	  while(1)
	    {
	      register Assoc_entry* bucket = hash_table + bucket_number;
	      if ( bucket->contents == 0 )
		{ 
		  bucket->key = mem->key;
		  bucket->contents = mem->contents;
		  break; // <==== copied it
		}

	      // search some more
	      bucket_number = REHASH(bucket_number);
	      
	    }
	}
      
    }

  delete old_hash;
}





void
Assoc::remove( char* key )
{
  register int  bucket_number  = HASH(key);
  register struct Assoc_entry * bucket;

  // Find old entry and remove it.
  while(1)
    { bucket = hash_table+bucket_number;
       
       if ( bucket->contents == 0 )
	 return;  // <===== nothing to delete

       if ( equiv(bucket->key, key ))
	 { // found the condemned item
	   delete_key(bucket->key);
	   heap.free(bucket->contents);
	   bucket->contents = 0;
	   num_entries-=1;
	   break; // <====== clobbered it
	 }
       // collision
       bucket_number = REHASH(bucket_number);
     }
  
  // Entries that might have been "bumped down the rehash chain"
  // when they collided with the now defunct Assoc_entry, may now be misplaced.
  // So we remove them, then reinsert them.   This is the trickiest part.
  while(
        bucket_number = REHASH(bucket_number),
	bucket = hash_table + bucket_number,
	bucket->contents != 0
       )
    { // check for bumpee
      register char* key = bucket->key;
      register int new_bucket_number = HASH(key);

      if(new_bucket_number != bucket_number) // unnecessary test (accelerator)
	{ // remove and reinsert
	  VOID* contents = bucket->contents;
	  bucket->contents = 0;  // remove it.
	  
	  while(1)
	    {
	      Assoc_entry* new_bucket = hash_table + new_bucket_number;
	      
	      if ( new_bucket->contents == 0 )
		{ 
		  new_bucket->key = key;
		  new_bucket->contents = contents;
		  break;    // <====== reinserted entry
		}
	      
	      new_bucket_number = REHASH(new_bucket_number);
	    }

	}// end remove and reinsert
    } // end check for bumpee

  return;

} // end Assoc::remove()
  



Assoc_entry*
Assoc_iterator::operator()()
{
  for (; i < cs->size; i++)
    if ( cs->hash_table[i].contents)
      { 
	return &cs->hash_table[i++];
      }
  i = 0;  // Table is exhausted. We will start the sequence anew next time.
  return 0;
}


int
Name_table::hash(register  char* key)
{
  /* I looked at the assembly language generated for various hashing
  ** algorithms, (Sun-3, release 3.4), and this one won.
  */
  register int retval = 0;
  while(*key) retval += retval + (unsigned char)(*key++);
  return retval;
}

void
Name_table::delete_key(char* key)
{ delete key; }

char*
Name_table::copy_key(char* key)
{ return strcpy( new char[strlen(key)], key ); }

int
Name_table::equiv(char* key1, char* key2)
{ return strcmp(key1, key2) == 0; }




Assoc_entry* Assoc::new_table(int size)
{
  Assoc_entry* table = new Assoc_entry [size];
  int i = 0;
  for(i = 0; i < size; i++) table[i].contents = 0;
  return table;
}

static int round_up_to_pwr2(int initial_size)
{
  const int SMALLEST = 4;
  int size = SMALLEST;

  while(initial_size > SMALLEST)
    { size *= 2;
      initial_size = ( initial_size + 1)/2;
    }
  return size*2;
}
