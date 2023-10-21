#include "flex_vector.H"

void
flex_vector::overflow(int n)
{
  int old_size = buf_size;
  void** old_buf = buf;

  do
    { buf_size = calc_next_size(buf_size); }
  while(buf_size <= n);

  buf = new void*[buf_size];

  int i;
  for(i=0; i<old_size; i++) 
    buf[i] = old_buf[i];
  for(; i<buf_size; i++) 
    init(buf[i]);

  delete old_buf;
}

