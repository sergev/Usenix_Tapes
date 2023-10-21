#ifndef lint
static char rcsid[]="$Header: ofiles.c,v 1.1 85/05/03 06:49:19 cspencer Exp $";
#endif lint
#include <sys/local.h>	/* DCIEM local mod. Delete at other sites */
#include <sys/param.h>
#define KERNEL
#include <sys/file.h>
struct file file[1];	/* to satisfy "extern" reference in <sys/file.h> */
#undef KERNEL
#include <sys/inode.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <a.out.h>
#include <sys/stat.h>
#include <pwd.h>
#ifdef FSTAB
#include <fstab.h>
#endif FSTAB
#include <stdio.h>


#define CDIR	01
#define OFILE	02
#define RDIR	04


long eseek();
int nproc;		/* number of entries in proc table 		*/
int swplo;		/* lowest block number on swap device		*/
int mem;		/* fd for /dev/mem				*/
int swap;		/* fd for /dev/swap				*/
long procbase;

int pids_only = 0;	/* if non-zero, only output process ids	*/

char *progname;
struct nlist nl[] = {
#define X_PROC		0
	{"_proc"},
#define X_NPROC 	1
	{"_nproc"},
#define X_SWPLO		2
	{"_swplo"},
	{""}
};


main(argc, argv)
int 	argc;
char	*argv[];
{

	struct inode *i,*getinode();
	struct stat s;
	struct user *u, *getuser();
	struct proc p;
	register int filen, flags, procn;
	register char *filename, *fsname;
#ifdef FSTAB
	struct fstab *fs, *getfsfile(), *getfsspec();
#endif FSTAB

	progname = argv[0];

	if(argc == 1) {
		fprintf(stderr,"usage: %s [-p] files\n", progname);
		exit(1);
	}

	/* check for -p flag */
	if(strcmp(argv[1],"-p") == 0) {
		pids_only++;
		--argc;
		++argv;
	}

	if((mem = open("/dev/mem", 0)) < 0)
		error("can't open /dev/mem. ");
	if((swap = open("/dev/swap", 0)) < 0) 
		error("can't open /dev/swap. ");
	getsyms();

	while(--argc) {
		++argv;
#ifdef FSTAB
		/* assume arg is  a filesystem */
		if((fs = getfsfile(*argv)) != NULL) {
			fsname = *argv;
			filename = fs->fs_spec;
		}
		/* maybe it's the device name for a filesystem */
		else if ((fs = getfsspec(*argv)) != NULL) {
			filename = *argv;
			fsname = fs->fs_file;
		}
		/* probably a regular file */
		else
#endif FSTAB
			{
			filename = *argv;
			fsname = "";
		}
		if(stat(filename, &s)) {
			fprintf(stderr,"can't stat %s. ",filename);
			perror("");
			continue;
		}
		if(! pids_only) {
			printf("%s\t%s\n", filename,fsname);
			printf("%-8.8s\tpid\ttype\tcmd\n", "user");
		}
		for(procn = 0; procn < nproc; procn++){
			procslot(procn, &p);
			flags = 0;
			if(p.p_stat == 0 || p.p_stat == SZOMB)
				continue;
			u = getuser(&p);
			if ( u == (struct user *)NULL)
				continue;
			i = getinode(u->u_rdir, "rdir");
			if(check(&s,i))
				flags |= RDIR;

			i = getinode(u->u_cdir, "cdir");
			if(check(&s,i))
				flags |= CDIR;

			for(filen = 0; filen < NOFILE; filen++) {
				struct file f;

				if(u->u_ofile[filen] == NULL)
					continue;

				eseek(mem,(long)u->u_ofile[filen],0,"file");
				eread(mem,(char *)&f, sizeof(f), "file");

				if(f.f_count > 0) {
#ifdef MASCOT
					i = getinode((char *)f.f_typ.f_in.f_inode, "file");
#else
					i = getinode((char *)f.f_inode, "file");
#endif MASCOT
					if(check(&s,i))
						flags |= OFILE;
				}
			}
			if(flags) gotone(u,&p,flags);
		}
		if(! pids_only)
			printf("\n");
	}
}		

/*
 * print the name of the user owning process "p" and the pid of that process
 */
gotone(u,p,f)
struct user *u;
struct proc *p;
int f;
{
	register struct passwd *pw;
	struct passwd *getpwuid();

	/* only print pids and return */
	if(pids_only) {
			printf("%d ",p->p_pid);
			fflush(stdout);
			return;
	}
	pw = getpwuid(u->u_uid);
	if(pw)
		printf("%-8.8s\t", pw->pw_name );
	else
		printf("(%d)\t", u->u_uid);
	printf("%d\t", p->p_pid);
	if(f & OFILE) putchar('f');	/* proc has a file 		   */	
	else putchar(' ');
	if(f & CDIR)  putchar('p');	/* proc's current dir is on device */
	else putchar(' ');
	if(f & RDIR)  putchar('r');	/* proc's root dir is on device	   */
	else putchar(' ');
	printf("\t");
	printf("%-14.14s", u->u_comm);
	printf("\n");
}

/*
 * is inode "i" on device "s"? returns TRUE or FALSE 
 */
check(s, i)
struct stat *s;
struct inode *i;
{
	if ((s->st_mode & S_IFMT) == S_IFBLK && s->st_rdev == i->i_dev)
			return(1);

	if ((s->st_mode & S_IFMT) == S_IFCHR && s->st_rdev == i->i_dev)
			return(1);

	if((s->st_dev == i->i_dev) && (s->st_ino == i->i_number))
			return(1);

	return(0);
}


/* 
 *	getinode - read an inode from from mem at address "addr"
 * 	      return pointer to inode struct. 
 */
struct inode *getinode(ip,s)
struct inode *ip;
char *s;
{
	static struct inode i;

	eseek(mem, (long)ip, 0, "inode");
	eread(mem, (char *)&i, sizeof(i), "inode");
	return(&i);
}



/* 
 * get user structure for proc "p" from core or swap
 * return pointer to user struct
 */
struct user *
getuser(p)
struct proc *p;
{
	int file;
	long addr;
	static struct user user;

	/* easy way */
	if ((p->p_flag & SLOAD) == 0) {
		addr = (long)(p->p_addr + swplo) << 9;
		file = swap;
		}
	else	{
		addr = (long)p->p_addr<<6;
		file = mem;
		}

	lseek(file, addr, 0);
	if (read(file, (char *)&user, sizeof(struct user))==0){
		fprintf(stderr, "error: can't get swapped user page\n");
		return (struct user *)NULL;
		}
	return(&user);
}

/*
 * read with error checking
 */
eread( fd, p, size, s)
int fd;
char *p;
int size;
char *s;
{
	int n;
	char buf[100];
	if(( n =  read(fd, p, size)) != size){
		sprintf(buf, "read error for %s. ", s);
		error(buf);
	}
	return(n);
}

/*
 * seek with error checking
 */
long eseek(fd, off, whence, s)
int fd;
long off;
int whence;
char *s;
{
	long lseek();
	long ret;
	char buf[100];

	if(( ret = lseek(fd, off, whence)) != off) {
		sprintf(buf, "seek for %s failed, wanted %o, got %o. ",
			s, off, ret);
		error(buf);
	}
	return(ret);
}

/*
 * print mesg "s" followed by system erro msg. exit.
 */
error(s)
char *s;
{
	if (s)
		fprintf(stderr,s);
	perror("");
	exit(1);
}

/*
 * get some symbols form the kernel
 */
getsyms()
{
	register i;

	nlist("/unix", nl);

	for(i = 0; i < (sizeof(nl)/sizeof(nl[0]))-1; i++)
		if(nl[i].n_value == 0) {
			if (i == X_NPROC)
				continue;

			fprintf(stderr,"%s: can't nlist for %s.\n",
				progname, nl[i].n_value);
			exit(1);
		}
	procbase = (unsigned)nl[X_PROC].n_value;
	if (nl[X_NPROC].n_value == 0)
		nproc = NPROC;
	else {
		eseek(mem, (long)nl[X_NPROC].n_value, 0, "nproc");
		eread(mem, &nproc, sizeof(nproc), "nproc");
	}
	eseek(mem, (long)nl[X_SWPLO].n_value, 0, "swplo");
	eread(mem, &swplo, sizeof(swplo), "swplo");
	return;
}
			

/*
 * read proc table entry "n" into buffer "p"
 */
procslot(n, p)
int n;
struct proc *p;
{
	eseek(mem, procbase + (long)(n * sizeof(struct proc)), 0, "procslot");
	eread(mem, (char *)p, sizeof(struct proc), "proc");
	return;
}
