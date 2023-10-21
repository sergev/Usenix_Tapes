#define	D_FILES
#include	"empdef.h"

getmysect(i, j, selup)
int i, j, selup;
{

	if ((getsect(i, j, selup) != -1) && (owner != 0)) {
		return (0);
	} else {
		printf("%d,%d is not your sector\n", i, j);
		return (-1);
	}
}
