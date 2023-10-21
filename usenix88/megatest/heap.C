#include "heap.H"
#include "Assoc.H"

// Everything is calculated in "heap-units", which is the amount of space
// required for an arbitrary pointer.
//
// Each block of packets is linked to the previous ones through the
// first "heap-unit". The head of the list is "cache".
//
// All free packets are similarly linked. The head of the list is
// "next_free".
//


Heap::Heap(unsigned int size, unsigned int init_size )
{
  // calculate size of element in heap-units.
  packet_size = (size + (sizeof(heap_unit) - 1)) / sizeof(heap_unit);
  cache = 0;
  num_elements = init_size;
  next_free = 0;
}

void Heap::underflow()
{
  heap_unit* more = cache;
  cache = new heap_unit[packet_size*num_elements + 1]; // one extra for link
  cache->next = more;  // This is the link.
  heap_unit* freebee = cache + 1;  // First free element is 1 beyond link.

  int i;
  for(i=0; i<num_elements; i++)
    { free(freebee);  // This is "our" free, not C-library's free.
      freebee += packet_size;
    }

  // If we run out again, we'll get this many more.
  num_elements = next_load(num_elements);

}

Heap::~Heap()
{
  while(cache)
    { heap_unit* more = cache->next;
      delete cache;
      cache = more;
    }
}

