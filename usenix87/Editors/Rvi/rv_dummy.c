#include "rv.h"

#ifndef USG
erasechar()
{
	return CTRL(H);
}

killchar()
{
	return CTRL(U);
}
#endif
