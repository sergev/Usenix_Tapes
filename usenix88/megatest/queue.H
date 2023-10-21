#ifndef QUEUE_DEFD
#define QUEUE_DEFD

#include "heap.H"


class Queue_list
{
  
friend class Queue_iterator;
friend class Queue;
  
  Queue_list* cdr;
  void* car;
  
};


class Queue
{

  Queue_list* tail;
  Queue_list* head;
  int size;
  friend class Queue_iterator;

 public:
  Queue() { size = 0; tail=0; head=0; }
  ~Queue();

  void* push(void*);
  void* pop();
  void* append(void*);

  inline  
  void* first()
    { return(size==0)? 0 : head->car; }

  inline  
  void* last()
    { return (size==0) ? 0:  tail->car; }
  
  inline int length() { return size; }
};

class Queue_iterator
{
  Queue* lq;
  Queue_list* rest;

 public:
  Queue_iterator(Queue& q) { lq=&q; rest = q.head; }
  void* operator() ();
  int done() { return rest==0; }
				 
};

#endif QUEUE_DEFD
