#define D_NATSTAT
#define D_SHPSTR
#include "empdef.h"

getmyship(n, str)
int n;
struct shpstr *str;
{

	if (getship(n, str) == -1 || ((cnum != str->shp_own) && (nstat != STAT_GOD))) {
		printf("%d is not your ship.\n", n);
		return (-1);
	}
	return (0);
}
