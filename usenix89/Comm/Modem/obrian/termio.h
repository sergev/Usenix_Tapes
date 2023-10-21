#include <stdio.h>
#include <sgtty.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/file.h>
#include <sys/errno.h>

typedef	unsigned char	byte;
typedef	unsigned short	word;

int	errno;	/* define this globally so that Unix can set it	*/

