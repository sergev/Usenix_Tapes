#define	D_TRTSTR
#define	D_FILES
#include	"empdef.h"

gettre(number)
int	number;
{
	long	lseek();

	lseek(trtf, (long)number * sizeof(trty), 0);
	if( read(trtf, &trty, sizeof(trty)) != sizeof(trty) ) return(-1);
	return(0);
}

puttre(number)
int	number;
{
	long	lseek();

	lseek(trtf, (long)number * sizeof(trty), 0);
	write(trtf, &trty, sizeof(trty));
}
