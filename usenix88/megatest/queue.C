#include "queue.H"


// All Queue_lists will come from this private heap.  MUCH quicker.

static Heap heap(sizeof(Queue_list));


void* Queue::push(void* cont)
{
  size++;
  Queue_list* old_head = head;
  head = (struct Queue_list*)heap.alloc(); 
  head->cdr = old_head;
  head->car = cont;
  if(!tail) tail = head;
  return cont;
}

void* Queue::pop()
{
  if(size == 0) return 0;
  size--;
  void* cont = head->car;
  Queue_list* condemned = head;
  head = head->cdr;
  heap.free(condemned);
  if(size==0) tail = 0;
  return cont;
}

void* Queue::append(void *cont)
{
  size++;
  Queue_list* old_tail = tail;
  tail = (struct Queue_list*)heap.alloc();
  tail->cdr=0;
  tail->car=cont;
  if(old_tail)
    old_tail->cdr = tail;
  if(head==0)head = tail;
  return cont;
}


void* Queue_iterator::operator() ()
{
  if(done()) { rest = lq->head;  return 0; }
  void* retval = rest->car;
  rest=rest->cdr;
  return retval;
}

Queue::~Queue()
{
  while(head)
    { Queue_list* condemned = head;
      head = head->cdr;
      heap.free(condemned);
    }
}
