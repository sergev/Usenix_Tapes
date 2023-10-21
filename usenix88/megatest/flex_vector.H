#ifndef FLEX_VECTOR_DEFD
#define FLEX_VECTOR_DEFD

struct flex_vector
{
  void** buf;           /* Currently allocated buffer.          */
  int buf_size;         /* Size currently allocated.            */

  /* Strategy for expanding size of buffer. */
  virtual int calc_next_size(int n)
    { if(n==0) return 4; else return n*2; }

  /* Initializes each slot in a vector. */
  virtual void init(void*& ptr) { ptr = (void*)0; }

  /* Called when buffer overflows. */
  void overflow(int n);

public:

  int lub;              /* Least upper bound of accessed slots. */

  inline flex_vector( int init_buf_size = 0)
    {
      lub = 0;
      buf_size = init_buf_size;
      if(buf_size > 0)
	buf = new void*[buf_size];
      else
	buf = 0;
    }
  
  inline ~flex_vector()
    {
      delete buf;
    }
  
  inline void*&  operator[] (int n)
    { 
      if( lub <= n ) lub = n + 1;
      if( n >= buf_size ) overflow(n);
      return buf[n];
    }

};

#endif FLEX_VECTOR_DEFD
