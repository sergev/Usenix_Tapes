#include "nodes.h"
hash(key)
char *key;
{
	unsigned int hval;
	hval = 0;
	while (*key) {
		hval = (hval + hval) + ((*key << 7) ^ (*key * 13));
		key++;
	}
	return(hval % NHASH);
}
