/* stat.c Feb 1987 - main, sizecheck, printit, statit
 *
 * stat - a program to perform what the stat(2) call does.  
 *
 * usage: stat [-] [-all] -<field> [-<field> ...] [file1 file2 file3 ...]
 *
 * where   <field> is one of the struct stat fields without the leading "st_".
 *	   The three times can be printed out as human times by requesting
 *	   -Ctime instead of -ctime (upper case 1st letter).
 *	   - in av[1] means take the file names from stdin.
 *
 * output: if only one field is specified, that fields' contents are printed.
 *         if more than one field is specified, the output is
 *	   file	filed1: f1val, field2: f2val, etc
 *
 * written: Larry McVoy, (mcvoy@rsch.wisc.edu)  
 *  
 * modified: Tom Christiansen on Sunday, 29 Mar 87 -- 13:02:49 CDT
 *  	-- give correct exit stati
 *	-- give perror() messages for bad stats
 *  	-- give correct file types
 *  	-- give user, group, and other read/write/exec info
 */

# include	<stdio.h>
# include	<sys/types.h>
# include	<sys/stat.h>
# define	addr(x)		(u_char*)(&sbuf.x)
# define	size(x)		sizeof(sbuf.x)
# define	equal(s, t)	(!strcmp(s, t))
# define	careful()	sizecheck()	/**/
# define	MAXPATH		500
# define	LS_ADDS_SPACE	/* AT&T Unix PC, ls prints "file[ */]" */
				/* This makes stat fail. */

# ifndef S_IFSOCK		/* sys v doesn't do this, I guess */
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;
# else
#   define BSD
# endif

struct stat sbuf;
extern int  errno;
char* 	    ctime();

struct field {
    char* f_name;	/* field name in stat */
    u_char* f_addr;	/* address of the field in sbuf */
    u_short f_size;	/* size of the object, needed for pointer arith */
    u_short f_print;	/* show this field? */
} fields[] = {
    { "dev",		addr(st_dev),		size(st_dev),		0, },
    { "ino",		addr(st_ino),		size(st_ino),		0, },
    { "mode",		addr(st_mode),		size(st_mode),		0, },
    { "nlink",		addr(st_nlink),		size(st_nlink),		0, },
    { "uid",		addr(st_uid),		size(st_uid),		0, },
    { "gid",		addr(st_gid),		size(st_gid),		0, },
    { "rdev",		addr(st_rdev),		size(st_rdev),		0, },
    { "size",		addr(st_size),		size(st_size),		0, },
    { "Atime",		addr(st_atime),		size(st_atime),		0, },
    { "Mtime",		addr(st_mtime),		size(st_mtime),		0, },
    { "Ctime",		addr(st_ctime),		size(st_ctime),		0, },
    { "atime",		addr(st_atime),		size(st_atime),		0, },
    { "mtime",		addr(st_mtime),		size(st_mtime),		0, },
    { "ctime",		addr(st_ctime),		size(st_ctime),		0, },
# ifdef BSD
    { "blksize", 	addr(st_blksize),	size(st_blksize),	0, },
    { "blocks",		addr(st_blocks),	size(st_blocks),	0, },
# endif
    { NULL,		  0,			0, },
};
    
main(ac, av)
    char** av;
{
    register i, j, nprint = 0, files = 0;
    char     buf[MAXPATH];
    int      ret, from_stdin = 0;

    careful();

    if (equal(av[i = 1], "-"))
	i++, from_stdin++;

    for ( ; i<ac; i++)  {
	if (av[i][0] == '-')  {
	    if (equal("-all", av[i]))
		for (j=0; fields[j].f_name; j++)
		    nprint++, fields[j].f_print++;
	    for (j=0; fields[j].f_name; j++) 
		if (equal(fields[j].f_name, &av[i][1]))
		    nprint++, fields[j].f_print++;
	}
	else 
	    files++;
    }
    if (!nprint) {
	fprintf(stderr, "usage: %s [-] [-all] -field[s] file1[s]\n", av[0]);
	exit(0);
    }
    for (i=1; i<ac; i++)
	if (av[i][0] != '-')  
	    ret |= statit(av[i], nprint, files);

    if (from_stdin)
	while (gets(buf)) {
# ifdef LS_ADDS_SPACE
	    buf[strlen(buf)-1] = NULL;
# endif
	    ret |= statit(buf, nprint, 2);
	}
    exit(ret);
}

/*------------------------------------------------------------------30/Jan/87-*
 * statit(file, nprint, files) - do the work
 *----------------------------------------------------------------larry mcvoy-*/
statit(file, nprint, files)
    char* file;
{
    register ret, j;
	
    if (ret = stat(file, &sbuf)) {
	perror(file);
        return ret;
    } else {
	register first = 1;

	for (j=0; fields[j].f_name; j++) {
	    if (fields[j].f_print) {
		if (first) {
		    if (files > 1)
			printf("%-14s ", file);
		    first = 0;
		}
		else
		    printf("  ");
		printit(&sbuf, &fields[j], nprint);
	    }
	}
	printf("\n");
    }
    return 0;
}

/*------------------------------------------------------------------30/Jan/87-*
 * printit(sb, f, n) - print the field
 *
 * Inputs    -> (struct stat*), (struct field*), (int)
 *
 * Results   -> Displays the field, with special handling of weird fields like
 *		mode and spare4.  The mode field is dumped in octal, followed
 *		by one or more of the S_IF<X> and/or S_I<X> values.  The spare4
 *		field is dumped as two u_longs.  All other fields are dumped
 *		as u_char, u_short, or u_long, as apporpriate.
 *----------------------------------------------------------------larry mcvoy-*/
#define MODE(x) ((x->st_mode) & ~S_IFMT) 
#define TYPE(x) ((x->st_mode) &  S_IFMT) 

#define S_IOREAD	(S_IREAD >> 6)
#define S_IOWRITE	(S_IWRITE >> 6)
#define S_IOEXEC	(S_IEXEC >> 6)
#define S_IGREAD	(S_IREAD >> 3)
#define S_IGWRITE	(S_IWRITE >> 3)
#define S_IGEXEC	(S_IEXEC >> 3)
#define S_IUREAD	S_IREAD
#define S_IUWRITE	S_IWRITE
#define S_IUEXEC	S_IEXEC

#define IS_MODE(FIELD) if ((MODE(sb) & S_I/**/FIELD) == S_I/**/FIELD) printf(mask,"FIELD")
#define IS_TYPE(FIELD) if ((TYPE(sb)) == S_IF/**/FIELD) printf(mask,"FIELD")

printit(sb, f, n)
    register struct stat* sb;
    register struct field* f;
    int n;
{
    static char mask[] = ", %s";

    if (n > 1)
	printf("%s: ", f->f_name);

    if (equal(f->f_name, "mode")) {
	printf("%07o", sb->st_mode);


	IS_TYPE(DIR);

# ifdef S_IFFIFO
	IS_TYPE(FIFO);
# endif
	IS_TYPE(CHR);
	IS_TYPE(BLK);
	IS_TYPE(REG);

# ifdef S_IFSOCK
	IS_TYPE(SOCK);
# endif

	IS_MODE(SUID);
	IS_MODE(SGID);
	IS_MODE(SVTX);
	IS_MODE(UREAD);
	IS_MODE(UWRITE);
	IS_MODE(UEXEC);
	IS_MODE(GREAD);
	IS_MODE(GWRITE);
	IS_MODE(GEXEC);
	IS_MODE(OREAD);
	IS_MODE(OWRITE);
	IS_MODE(OEXEC);
    }

    /* times in human form, uppercase first letter */
    else if (equal("Ctime", f->f_name)) 
	printf("%.24s", ctime(&sb->st_ctime));
    else if (equal("Mtime", f->f_name))
	printf("%.24s", ctime(&sb->st_mtime));
    else if (equal("Atime", f->f_name))
	printf("%.24s", ctime(&sb->st_atime));
    else if (equal("ctime", f->f_name)) 
	printf("%u", sb->st_ctime);
    else if (equal("mtime", f->f_name))
	printf("%u", sb->st_mtime);
    else if (equal("atime", f->f_name))
	printf("%u", sb->st_atime);
# ifdef S_IFSOCK
    else if (equal("spare4", f->f_name))
	printf("%u %u", sb->st_spare4[0], sb->st_spare4[1]);
# endif
    else
	switch (f->f_size) {
	    case 1: printf("%u", *(u_char*)f->f_addr);
		    break;
	    case 2: printf("%u", *((u_short*)f->f_addr));
		    break;
	    case 4: printf("%u", *((u_long*)f->f_addr));
		    break;
	    default:fprintf(stderr, "\nError: bad field size %d (%s)\n", 
			    f->f_size, f->f_name);
		    break;
	}
}

sizecheck()
{
    if (sizeof(char) != 1)
	fprintf(stderr, "warning: char != 1\n");
    if (sizeof(short) != 2)
	fprintf(stderr, "warning: short != 2\n");
    if (sizeof(long) != 4)
	fprintf(stderr, "warning: long != 4\n");
}
