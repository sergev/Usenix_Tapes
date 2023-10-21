#
#include "../param.h"
#include "../systm.h"
#include "../l_user.h"
#include "../reg.h"
#include "../l_file.h"
#include "../inode.h"

/*
 * read system call
 */
read()
{
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi, writei, or pipe code.
 */
rdwr(mode)
{
	register *fp, m, *ip;
	int blkio, err, arg1;
	long l;

	m = mode;
	err = 0;
	arg1 = u.u_arg[1];
	blkio = 1;
	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if((fp->f_flag&m) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = u.u_arg[0];
	u.u_count = arg1;
	u.u_segflg = 0;
	ip = fp->f_inode;
	if(fp->f_flag&FPIPE) {
		if(m==FREAD)
			readp(fp); else
			writep(fp);
	} else {
		u.u_offset = fp->f_offset;
		if((ip->i_mode & IFMT) == IFCHR) {
			blkio = 0;
			if(ip->i_addr->i_cflag == ICFLAG) {	/* contig. file */
				u.u_offset =+ (ip->i_addr->i_cbase)<<9;
				/* l = distance from current file offset to eof */
				l = (ip->i_addr->i_clast + 1)<<9;
				l =- u.u_offset;
				if(u.u_count > l) {
					if(m == FWRITE)
						err = EBADF;
					arg1 = u.u_count = l;
				}
				if((u.u_offset&0777) || (u.u_count&0777))
					blkio++;
			}
		}
		if(m==FREAD)
			readi(ip, blkio); else
			writei(ip, blkio);
		fp->f_offset =+ (arg1 - u.u_count);
	}
	u.u_ar0[R0] = arg1-u.u_count;
	if(u.u_error == 0)
		u.u_error = err;
}

/*
 * open system call
 */
open()
{
	register *ip;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	u.u_arg[1]++;
	open1(ip, u.u_arg[1], 0, 1);
}

/*
 * creat system call
 */
creat()
{
	extern uchar;

	creat1(namei(&uchar, 1), 1);
}

creat1(ip, oflag)
register *ip;
{
	if(ip == NULL) {
		if(u.u_error)
			return;
		if ((u.u_cdir->i_nlink == 0) && (fubyte (u.u_arg[0]) != '/'))
		  {
			u.u_error = ENOENT;
err:			iput (u.u_pdir);
			return;
		  }
		ip = maknode(u.u_arg[1]&07777&(~ISVTX));
		if (ip==NULL)
		  goto err;
		open1(ip, FWRITE, 2, oflag);
	} else
		open1(ip, FWRITE, 1, oflag);
}

/* creat for contiguous files
 * the name has been played with and is in u.u_dbuf
 */
c_creat()
{
	extern schar;

	u.u_dirp = u.u_dbuf;
	creat1(namei(&schar, 1), 0);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 * Open file if oflag == 1
 */
open1(ip, mode, trf, oflag)
int *ip;
{
	register struct file *fp;
	register *rip, m;
	int i;

	rip = ip;
	m = mode;
	if(trf != 2) {
		if(m&FREAD)
			access(rip, IREAD);
		if(m&FWRITE) {
			access(rip, IWRITE);
			if((rip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if(u.u_error)
		goto out;
	if(trf)
		itrunc(rip);
	prele(rip);
	if(!oflag) {
		u.u_ar0[R1] = rip;
		return;
	}
	if ((fp = falloc()) == NULL)
		goto out;
	fp->f_flag = m&(FREAD|FWRITE);
	fp->f_inode = rip;
	i = u.u_ar0[R0];
	openi(rip, m&FWRITE);
	if(u.u_error == 0)
		return;
	u.u_ofile[i] = NULL;
	fp->f_count--;

out:
	iput(rip);
}

/*
 * close system call
 */
close()
{
	register *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	u.u_ofile[u.u_ar0[R0]] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
seek()
{
	long n;
	register *fp, t, *ip;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	ip = fp->f_inode;
	if(fp->f_flag&FPIPE) {
		u.u_error = ESPIPE;
		return;
	}
	t = u.u_arg[1];
	n = u.u_arg[0];
	if(t > 2) {
		n =<< 9;
		if(t == 3)
			n.hiword =& 0777;
	} else {
		if(t==0)
			n.hiword = 0;
	}
	switch(t) {

	case 1:
	case 4:
		n =+ fp->f_offset;
		break;

	default:
		n.hiword =+ ip->i_size0&0377;
		n =+ ip->i_size1;

	case 0:
	case 3:
		;
	}
	if (n.hiword & ~0777)
		u.u_error = EFBIG;
	else
		fp->f_offset = n;
}

/*
 * Tell -- discover offset of file R/W pointer
 */
tell()
{
	register struct file *fp;

	if (fp = getf(u.u_ar0[R0])) {
		u.u_ar0[R1] = fp->f_offset.loword;	/* fixed- jrn */
		u.u_ar0[R0] = fp->f_offset.hiword;
	}
}

/*
 * link system call
 */
link()
{
	register *ip, *xp;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if(ip->i_nlink >= 127) {
		u.u_error = EMLINK;
		goto out;
	}
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	/*
	 * unlock to avoid possibly hanging the namei
	 */
	ip->i_flag =& ~ILOCK;
	u.u_dirp = u.u_arg[1];
	xp = namei(&uchar, 1);
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
	}
	if(u.u_error)
		goto out;
	if(u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	wdir(ip);
	ip->i_nlink++;
	ip->i_flag =| IUPD;

out:
	iput(ip);
}

/*
 * mknod system call
 */
mknod()
{
	register *ip;
	extern uchar;

	if(suser()) {
		ip = namei(&uchar, 1);
		if(ip != NULL) {
			u.u_error = EEXIST;
			goto out;
		}
	}
	if(u.u_error)
		return;
	ip = maknode(u.u_arg[1]);
	if (ip==NULL)
		return;
	ip->i_addr[0] = u.u_arg[2];

out:
	iput(ip);
}

/*
 * sleep system call
 * not to be confused with the sleep internal routine.
 */
sslep()
{
	char *d[2];

	spl6();
	d[0] = time[0];
	d[1] = time[1];
	dpadd(d, u.u_ar0[R0]);

	while(dpcmp(d[0], d[1], time[0], time[1]) > 0) {
		if(dpcmp(tout[0], tout[1], time[0], time[1]) <= 0 ||
		   dpcmp(tout[0], tout[1], d[0], d[1]) > 0) {
			tout[0] = d[0];
			tout[1] = d[1];
		}
		sleep(tout, PSLEP);
	}
	spl0();
}

/*
 * access system call
 */
saccess()
{
	extern uchar;
	register svuid, svgid;
	register *ip;

	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	ip = namei(&uchar, 0);
	if (ip != NULL) {
		if (u.u_arg[1]&(IREAD>>6))
			access(ip, IREAD);
		if (u.u_arg[1]&(IWRITE>>6))
			access(ip, IWRITE);
		if (u.u_arg[1]&(IEXEC>>6))
			access(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}
