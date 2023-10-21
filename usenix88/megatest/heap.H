#ifndef HEAP_DEFD
#define HEAP_DEFD


class heap_unit  
{ friend class Heap;
  heap_unit* next;
};

class Heap  // implements heap of packets of a fixed size
{
  virtual int next_load(int n) // Strategy determines size of subsequent
    { return n*2; }            //   block, calculated from size of 
			       //   previous block.

  int packet_size;      // in units of sizeof(heap_unit)
  int num_elements;     // number of packets to allocate in next block
  heap_unit* cache;     // queue of all blocks of packets
  heap_unit* next_free; // free store of packets
  void underflow();     // called when free store is depleted

public:

  Heap(unsigned int     /* packet-size in bytes */, 
       unsigned int = 4 /* number of packets in first block */);

  ~Heap();

  inline void* 
  alloc()
    { heap_unit* retval;
      if(next_free == 0) underflow();
      retval=next_free;  next_free = next_free->next;
      return (void*)retval;
    }

  inline void
  free(void* packet)
    {
      heap_unit *more = next_free;
      next_free = (heap_unit*)packet;
      ((heap_unit*)packet)->next = more;
    }
}; 


#endif HEAP_DEFD
