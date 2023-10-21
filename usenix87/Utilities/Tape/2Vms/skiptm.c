#include <sys/types.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include "vmstape.h"

skiptm(n)
	int	n;
{
	struct	mtop	m;

	m.mt_count	= n;
	m.mt_op		= MTFSF;

	ioctl(magtape, MTIOCTOP, &m);
}
w_tapemark()
{
	struct	mtop	m;

	m.mt_count	= 1;
	m.mt_op		= MTWEOF;

	ioctl(magtape, MTIOCTOP, &m);
}
