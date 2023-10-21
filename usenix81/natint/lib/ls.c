struct dirent {
	int     inode;
	char    name[14];
	};

int     ls_all;

/*
 * Return number of directory entries
 * in a directory.
 */

ndirent(dir) char *dir;{
	struct dirent dbuf[32];
	register struct dirent *d;
	register int i, n;
	int fd;

	if((fd=open(dir,0))<0) return -1;
	for(n=0; (i=read(fd, &dbuf[0], sizeof dbuf))>0; )
		for(i=/ 16, d= &dbuf[0]; i>0; i--, d++)
			if(d->inode && (ls_all || d->name[0]!='.')) n++;
	close(fd);
	return n; }

/*
 * List directory entries.
 * Each pointer must point to a buffer >=15 bytes.
 * Number of entries is returned.
 */

ls(dir,ep,max) char *dir, *ep[];{
	struct dirent dbuf[32];
	register struct dirent *d;
	register int i, n;
	int fd, scmp();

	if((fd=open(dir,0))<0) return -1;
	for(n=0; (i=read(fd, &dbuf[0], sizeof dbuf))>0; )
		for(i=/ 16, d= &dbuf[0]; i>0; i--, d++)
			if(d->inode && n<max && (ls_all || d->name[0]!='.')){
				copy(&d->name[0],ep[n],14);
				ep[n][14]= 0;
				n++; }
	qsort(&ep[0],n,2,&scmp);
	close(fd);
	return n; }

static scmp(sp1,sp2) char **sp1, **sp2;{
	return scompare(*sp1,*sp2); }
