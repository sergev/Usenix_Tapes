#define D_FILES
#include	"empdef.h"

prdate()
{
	long	tvec;
	char	*ctime();

	time(&tvec);
	printf(ctime(&tvec));
	return;
}
