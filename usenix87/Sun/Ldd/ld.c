/* ld.c, lzh, 01/31/86 */

/*
 * Kernel support for loadable device drivers
 */
#include "ld.h"
#if NLD > 0

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/errno.h"
#include "../sundev/mbvar.h"
#include "../sundev/lddefs.h"

int ldnone(), ldprobe(), ldintr();

int ld_magic = LD_MAGIC;

struct ld_dev ld_dev;

char ld_driver[LD_SIZE];

struct mb_device *ldinfo[NLD];

struct mb_driver lddriver = {
    ldprobe, 0, 0, 0, 0, ldintr, LD_DEV_SIZE, "ld", ldinfo, 0, 0, 0,
};

ldprobe(reg)
caddr_t reg;
{
#ifndef DEMO
    if (poke(reg + LD_PROBE_OFFSET, 0)) 
	return(0);
#endif
    return(LD_DEV_SIZE);
}

ldintr()
{
    if (ld_dev.ld_intr != 0)
	return((*ld_dev.ld_intr)());
    else
	return(0);
}

ldopen(dev, flags)
register dev_t dev;
register flags;
{
    if (ld_dev.ld_open != 0)
	return((*ld_dev.ld_open)(dev, flags));
    else
	ld_error("open");
}

ldclose(dev)
register dev_t dev;
{
    if (ld_dev.ld_close != 0)
	return((*ld_dev.ld_close)(dev));
    else
	ld_error("close");
}

ldread(dev, uio)
register dev_t dev;
register struct uio *uio;
{
    if (ld_dev.ld_read != 0)
	return((*ld_dev.ld_read)(dev, uio));
    else
	ld_error("read");
}

ldwrite(dev, uio)
register dev_t dev;
register struct uio *uio;
{
    if (ld_dev.ld_write != 0)
	return((*ld_dev.ld_write)(dev, uio));
    else
	ld_error("write");
}

ldioctl(dev, cmd, data, flag)
register dev_t dev;
register int cmd;
register caddr_t data;
register int flag;
{
    if (ld_dev.ld_ioctl != 0)
	return((*ld_dev.ld_ioctl)(dev, cmd, data, flag));
    else
	ld_error("ioctl");
}

ldmmap(dev, off, prot)
register dev_t dev;
register off_t off;
register int prot;
{
    if (ld_dev.ld_mmap != 0)
	return((*ld_dev.ld_mmap)(dev, off, prot));
    else
	ld_error("mmap");
}

ldstrategy(bp)
register struct buf *bp;
{
    if (ld_dev.ld_strategy != 0)
	return((*ld_dev.ld_strategy)(bp));
    else
	ld_error("strategy");
}

ldsize(dev)
register dev_t dev;
{
    if (ld_dev.ld_size != 0)
	return((*ld_dev.ld_size)(dev));
    else
	ld_error("size");
}

lddump(dev, addr, blkno, nblk)
register dev_t dev;
register caddr_t addr;
register daddr_t blkno, nblk;
{
    if (ld_dev.ld_dump != 0)
	return((*ld_dev.ld_dump)(dev, addr, blkno, nblk));
    else
	ld_error("dump");
}

static
ld_error(s)
char *s;
{
    printf("ld0: %s routine called, but not defined!\n", s);
}

#endif
