/*
 * Silicon disk driver.
 *
 * Copyright 1987 by Mike Parker.
 * You may do anything you like with this as long as you don't:
 *	- claim you wrote any part of it that you didn't (ie, you may
 *	   of course claim any modifications you've made, but not any
 *	   of my code); nor
 *	- change or delete this copyright notice.
 * Note also that I do not take any responsibility for anything this code
 * may do.  I have tried to make it useful but I can't promise anything.
 *
 * If you find any bugs I'd appreciate hearing about them, especially
 * if you also fix them.  Write to mouse@mcgill-vision.uucp.
 */

/* Configuration. */

/* What spl... do we use for total lockout?  (We assume splx() un-locks.) */
#define lockout() splhigh() /* works on 4.3BSD derivatives for the VAX */
/* #define lockout() spl7() /* works on Sun UNIX 3.0 */

/* Where is mount.h? */
#define UFS_UFSMOUNT /* ../ufs/ufsmount.h: works on mtXinu 4.3+NFS */
/* #define H_MOUNT /* ../h/mount.h: works on 4.3 */
/* #define UFS_MOUNT /* ../ufs/mount.h: works on Sun UNIX 3.0 */

#include "memdisk.h"

#if NMEMDISK > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/ioctl.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/time.h"
#include "../h/kernel.h"
#include "../h/errno.h"
#ifdef UFS_UFS_MOUNT
#include "../ufs/ufsmount.h"
#endif
#ifdef H_MOUNT
#include "../h/mount.h"
#endif
#ifdef UFS_MOUNT
#include "../ufs/mount.h"
#endif

int max_md_size = (2048+512) * 1024; /* 2.5Mb */

static struct memdiskinfo {
	 int busy;
	 int live;
	 int unit;
	 int nbytes;
	 char *data; } memdiskinfo[NMEMDISK];
static int didinit = 0;

#define OKUNIT(u) (((u)>=0)&&((u)<NMEMDISK))
#define LIVEUNIT(u) (OKUNIT(u)&&memdiskinfo[u].live)

memdiskopen(dev,flag)
dev_t dev;
int flag;
{
 int unit;
 int s;

#ifdef lint
 flag = flag;
#endif
 unit = minor(dev);
 if (! OKUNIT(unit))
  { return(ENXIO);
  }
 s = lockout();
 if (! didinit)
  { for (unit=0;unit<NMEMDISK;unit++)
     { register struct memdiskinfo *mdi;
       mdi = &memdiskinfo[unit];
       mdi->busy = 0;
       mdi->live = 0;
       mdi->unit = unit;
     }
    didinit = 1;
  }
 splx(s);
 return(0);
}

memdiskstrategy(bp)
struct buf *bp;
{
 int unit;
 register struct memdiskinfo *mdi;
 int off;
 int nblk;
 int s;

 unit = minor(bp->b_dev);
 if (! LIVEUNIT(unit))
  { bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }
 mdi = &memdiskinfo[unit];
 s = lockout();
 if (mdi->busy)
  { splx(s);
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }
 off = dbtob(bp->b_blkno);
 if ( (off < 0) ||
      (off+bp->b_bcount > mdi->nbytes) )
  { splx(s);
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }
 if (bp->b_flags & B_READ)
  { bcopy(mdi->data+off,bp->b_un.b_addr,bp->b_bcount);
  }
 else
  { bcopy(bp->b_un.b_addr,mdi->data+off,bp->b_bcount);
  }
 splx(s);
 iodone(bp);
}

memdiskread(dev,uio)
dev_t dev;
struct uio *uio;
{
 static struct buf b[NMEMDISK];
 int unit = minor(dev);

 if (! LIVEUNIT(unit))
  { return(ENXIO);
  }
 return(physio(memdiskstrategy,&b[unit],dev,B_READ,minphys,uio));
}

memdiskwrite(dev,uio)
dev_t dev;
struct uio *uio;
{
 static struct buf b[NMEMDISK];
 int unit = minor(dev);

 if (! LIVEUNIT(unit))
  { return(ENXIO);
  }
 return(physio(memdiskstrategy,&b[unit],dev,B_WRITE,minphys,uio));
}

memdiskioctl(dev,cmd,data,flag)
dev_t dev;
int cmd;
caddr_t data;
int flag;
{
 int unit;
 int rv;
 int nb;
 struct memdiskinfo *mdi;
 int s;

 unit = minor(dev);
#ifdef lint
 flag = flag;
#endif
 if (! OKUNIT(unit))
  { return(ENXIO);
  }
 mdi = &memdiskinfo[unit];
 rv = 0;
 switch (cmd)
  { case LIOC_MEM_SET:
       nb = * (int *) data;
       if ( (nb < 0) ||
	    (nb != dbtob(btodb(nb))) ||
	    (nb > max_md_size) )
	{ rv = EINVAL;
	  break;
	}
       if (unit_mounted(unit))
	{ rv = EBUSY;
	  break;
	}
       s = lockout();
       if (mdi->busy)
	{ splx(s);
	  return(EALREADY);
	}
       mdi->busy = 1;
       splx(s);
       if (mdi->live)
	{ wmemfree(mdi->data,mdi->nbytes);
	  mdi->live = 0;
	}
       if (nb > 0)
	{ mdi->nbytes = nb;
	  mdi->data = wmemall(memall,nb);
	  if (mdi->data == 0)
	   { rv = ENOMEM;
	   }
	  else
	   { mdi->live = 1;
	   }
	}
       mdi->busy = 0;
       break;
    default:
       rv = ENOTTY;
       break;
  }
 return(rv);
}

memdiskdump()
{
 return(ENXIO);
}

memdisksize(dev)
dev_t dev;
{
 int unit = minor(dev);

 if (! LIVEUNIT(unit))
  { return(-1);
  }
 return(btodb(memdiskinfo[unit].nbytes));
}

static int unit_mounted(unit)
int unit;
{
 struct mount *mp;
 int s;
 dev_t dev;

 s = spl4(); /* as long as ipl > 0, (u)mounts can't happen */
 for (mp= &mounttab[0];mp<&mounttab[NMOUNT];mp++)
  { if (mp->m_bufp != NULL)
     { dev = mp->m_dev;
       if ( (bdevsw[major(dev)].d_strategy == memdiskstrategy) &&
	    (minor(dev) == unit) )
	{ splx(s);
	  return(1);
	}
     }
  }
 splx(s);
 return(0);
}

#endif
