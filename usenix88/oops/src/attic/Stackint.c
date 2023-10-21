#define ARRAY_SEP_PRINT(s)	s << "\t"
#define ARRAY_ELEM_HASH(h,x)	h^=(int)(x)
#include "Stackint.h"
DEFINE_CLASS(Stackint,Arrayint,1);
implement(Stack,int)
