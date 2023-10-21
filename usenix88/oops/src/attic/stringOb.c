#include "String.h"

obid stringOb(const char* s)
{
	return new String(s);
}
