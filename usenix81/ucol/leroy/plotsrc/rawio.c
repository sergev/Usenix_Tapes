#define PMODE 0644

static int uarray[50];
static int umax = -1;
static int bpoint[4];
static int nbbyt[4];

ropen_(name,type,unit,dum)	/*  open a raw io unit  */
			/*  name = file name  */
			/*  type = 0 for read */
			/*       = 1 for write */
			/*       = 2 for read/write (long integer) */
			/*  unit = returned unit number (long integer)  */
			/*  note:  if type = 1 then a new file is created */
			/*         otherwise it is assumed that the file  */
			/*         previously exists  */
			/*         dum is a dummy argument and need not */
			/*         be included in the fortran call  */
char name[];
long *unit;
long *type;
long dum;
{
	extern int uarray[50];
	extern int umax;
	extern int bpoint[4];
	extern int nbbyt[4];
	char cname[80];
	int fd;
	int ty;
	int i;

	i = 0;
	while (i < 80) {
		if (name[i] == ' ')  goto OUT;
		if (name[i] == '\0')  goto OUT;
		if (name[i] == '\n')  goto OUT;
		cname[i] = name[i];
		++i;
		}
OUT:	cname[i] = '\0';
	ty = *type;
	if (ty != 1) fd = open(cname, ty);
	if (ty == 1) fd = creat(cname, PMODE);
	*unit = fd;
	if (fd < 0)  printf("ropen: can't open/create %s\n", cname);
	if (fd >= 0) {
		umax = umax + 1;
		uarray[fd] = umax;
		nbbyt[umax] = 0;
		bpoint[umax] = 0;
		}
}

rseek_(unit, offset, origin)	/*  dummy lseek see UNIX manual */

long *unit;
long *offset;
long *origin;
{
	extern int bpoint[4];
	extern int nbbyt[4];
	extern int uarray[50];
	int u;
	int orig;
	int fd;

	orig = *origin;
	fd = *unit;
	lseek(fd, *offset, orig);
	u = uarray[fd];
	nbbyt[u] = 0;
	bpoint[u] = 0;
}

rread_(unit, nbytes, list, end)	/*  raw disk read  */
				/*  unit = unit no.  */
				/*  nbytes = no. of bytes to read  */
				/*  list = starting address of read xfer */
				/*  end = end of file flag  */
				/*        equal to the actual number of  */
				/*        bytes transfered   */

long *unit;
long *nbytes;
char list[];
long *end;
{
	int fd;
	int n;
	int nb;

	fd = *unit;
	nb = *nbytes;
	n = read(fd, list, nb);
	*end = n;
}

rwrite_(unit, nbytes, list)	/*  raw disk write  */
				/*  see routine rread for details  */
long *unit;
long *nbytes;
char list[];
{
	int fd;
	int nb;

	fd = *unit;
	nb = *nbytes;
	write(fd, list, nb);
}
rclose_(unit)		/*  closes out unit  */
			/*  it seems that this must be called before the */
			/*  end of a fortran program in order to avoid  */
			/*  an error exit  */
long *unit;
{
	int fd;

	fd = *unit;
	close(fd);
}
