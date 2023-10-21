/* demo.c, lzh, 10/07/85 */

/*
 * LOADABLE Demo Device driver 
 *
 * This device will continuously read back the last 1024 bytes written to it.
 */

#if NDEMO > 0
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/ioctl.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../sun/psl.h"
#include "../sundev/mbvar.h"

#define DEMOUNIT(dev) (minor(dev))
int demostrategy();
int demo_is_open = 0;
char demobuf[1024];

extern struct mb_device *ldinfo[];
#define demoinfo ldinfo

struct buf rdemobuf[NDEMO];

demoopen(dev, flag)
dev_t dev;
int flag;
{
    if (demo_is_open)
	return(ENXIO);
    return(0);
}

democlose(dev)
dev_t dev;
{
    demo_is_open = 0;
}

demoread(dev, uio)
dev_t dev;
struct uio *uio;
{
    if (DEMOUNIT(dev) >= NDEMO)
	return(ENXIO);
    return(physio(demostrategy, &rdemobuf[DEMOUNIT(dev)], dev, B_READ,
		  minphys, uio));
}

demowrite(dev, uio)
dev_t dev;
struct uio *uio;
{
    if (DEMOUNIT(dev) >= NDEMO)
	return(ENXIO);
    return(physio(demostrategy, &rdemobuf[DEMOUNIT(dev)], dev, B_WRITE,
		  minphys, uio));
}

demostrategy(bp)
register struct buf *bp;
{
    register int i;
    char *cp = bp->b_un.b_addr;

    for (i = 0; i < bp->b_bcount; i++)
	if (bp->b_flags & B_READ)
	    *cp++ = demobuf[i%1024];
	else
	    demobuf[i%1024] = *cp++;
    iodone(bp);
}

demointr()
{
    return(0);
}
#endif
