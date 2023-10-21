#include "macros.h"

get_rand(x, y)
int x, y;
{
	int s,r;

	two_sort(x, y);

	r = rand()>>3;
	r = (r % ((y-x)+1)) + x;
	s = (int) r;
	return(s);
}

rand_percent(percentage)
{
	return(get_rand(1, 100) <= percentage);
}
