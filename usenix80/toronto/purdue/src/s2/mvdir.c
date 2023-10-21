#

/*
 *      mvdir -- move directory around within a filesystem
 *
 * This program can be used to move a directory and all of its subdirectories
 * from one branch of the tree to another (within the same filesystem).  It
 * performs several tests to attempt to prevent undesirable moves such as
 * a directory to its own subdirectory, a directory into a file, directories
 * owned by someone else, etc.  The code which actually does the move is
 * relatively straightforward, and is executed only if all of the tests
 * succeeed.
 *
 * Directories which have incorrect "." and ".." entries should be illegal
 * as the source or destination for the move.  Linked directories introduce
 * problems, and the program attempts to do its best to cope with these.
 *
 * Pathnames ending in "." or ".." for the source directory (existing
 * directory pathname) are illegal.  "." and ".." should be avoided since
 * the "." and ".." entries in the moved directory are changed.
 *
 * For use by only system staff, compile and set mode 755.
 * For use by all users, compile and set owner to "root", mode 4755.
 */

struct inode {
  int   dev;           /* +0: major, minor device of i-node */
  int   inumber;       /* +2 */
  int   flags;         /* +4: see below */
  char  nlinks;        /* +6: number of links to file */
  char  uid;           /* +7: user ID of owner */
  char  gid;           /* +8: group ID of owner */
  char  size0;         /* +9: high byte of 24-bit size */
  int   size1;         /* +10: low word of 24-bit size */
  int   addr[8];       /* +12: block numbers or device number */
  int   actime[2];     /* +28: time of last access */
  int   modtime[2];    /* +32: time of last modification */
} srci, junki;

int uid;


#define IFDIR   040000

main(argc, argv)
char **argv;
{

	char orig[200], *eorg;          /* Source pathname */
	char prnt[200], *eprt;          /* Pathname of parent of dest */
	char dest[200], *edst;          /* Pathname of destination */
	char *copy();
	int inum;			/* Scratch variable */
	register struct inode *ip;      /* Always points to "junki" */
	register char *lnp;             /* Moves around a bit */

	ip = &junki;                    /* "junki" is a scratch buffer */

	if (argc != 3)
		errxit("Syntax is \"%s src dst\"", *argv);

	if ((uid = getuid()) && geteuid())
		errxit("Not super-user");

	/* Duplicate arguments to program so they can be manipulated */

	eorg = copy(*++argv, orig);     /* Source pathname */
	edst = copy(*++argv, dest);     /* Destination pathname */
	eprt = copy(*argv, prnt);       /* Parent of destination (soon) */


	/* Get last part of source pathname */

	for (lnp = eorg; --lnp >= orig && *lnp != '/';);
	if (*++lnp == '.' && ((lnp[1] == '.' && !lnp[2]) || !lnp[1]))
		errxit("Illegal source directory -- %s", orig);


	/* "stat" both the source and destination pathnames.  If the
	 * source is inode 1 or doesn't exist, take an error exit.  If
	 * destination exists, append the last part of the source name
	 * and try again.  If that name exists, exit.
	 */

	if (stat(orig, &srci) < 0)
		errxit("Can't \"stat\" %s", orig);
	if (srci.inumber == 1)
		errxit("%s is root of a filesystem", orig);
	if (!(srci.flags & IFDIR))
		errxit("%s is not a directory", orig);

	if (stat(dest, ip) >= 0){
		if (!(ip->flags & IFDIR))
			errxit("%s is not a directory", dest);
		*edst++ = '/';
		edst = copy(lnp, edst);
		if (stat(dest, ip) >= 0)
			errxit("%s exists", dest);
	}



	/* Check access permissions on the parent of the source dir */

	copy("/..", eorg);
	if (stat(orig, ip) < 0)
		errxit("Can't \"stat\" %s", orig);
	if (!(ip->flags & IFDIR))
		errxit("%s not a directory!!", orig);

	if (!access(ip))
		errxit("No permission in %s", orig);

	*eorg = 0;


	/* Form the name of the parent of the source directory
	 * and compare its inumber with the own obtained above.
	 * If they do not match, something fishy is going on,
	 * so exit.
	 */

	inum = ip->inumber;
	for (lnp = copy(orig, prnt); --lnp >= prnt && *lnp != '/';);
	*++lnp = '.';
	*(eprt = ++lnp) = 0;
	if (stat(prnt, ip) < 0)
		errxit("Can't \"stat\" %s", prnt);
	if (ip->inumber != inum)
		errxit("Bad \"..\" entry in %s", orig);


	/* Now, take off names from the end of the parent pathname,
	 * and check each level of the pathname back to the beginning
	 * so see if it actually refers to the directory to be moved
	 * by a combination of ".." entries.  If so, exit.
	 */


	lnp =- 2;
	*lnp = 0;			/* Strip out previous "." entry */
	while(lnp > prnt){
		while(*--lnp != '/' && lnp > prnt);
		if (lnp == prnt && *lnp == '/') lnp[1] = 0;
			else if (lnp > prnt) *lnp = 0;
				else {
					*prnt = '.';
					prnt[1] = 0;
				}
		if (stat(prnt, ip) < 0)
			errxit("Can't \"stat\" %s", prnt);
		if (!(ip->flags & IFDIR))
			errxit("%s is not a directory!!", prnt);
		if (ip->inumber == srci.inumber)
			errxit("Recursive pathname -- %s", orig);
	}


	/* Form the name of the parent of the destination directory
	 * and check access permissions in it.
	 */

	for (lnp = copy(dest, prnt); --lnp >= prnt && *lnp != '/';);
	*++lnp = '.';
	*(eprt = ++lnp) = 0;

	if (stat(prnt, ip) < 0)
		errxit("Can't \"stat\" %s", prnt);
	if (!(ip->flags & IFDIR))
		errxit("%s not a directory!!", prnt);
	if (!access(ip))
		errxit("No permission in %s", prnt);

	if (srci.dev != ip->dev)
		errxit("Source, destination on different filesystems");


	/* Make sure that the source and the parent of the destination
	 * are not the same thing.
	 */

	if (ip->inumber == srci.inumber)
		errxit("Destination is subdirectory of source");


	/* Now, check if the destination is a subdirectory of the source.
	 * To do this, traceback the destination directory from the parent
	 * obtained just above until inode 1 is reached.  At each level
	 * compare the inode number with that of the source directory.
	 * If they match, exit.  If the end of the buffer "prnt" is
	 * reached and inode 1 has not yet been located, there probably
	 * is a bad ".." entry in one of the directories which is
	 * complicating things.  (It is possible, though unlikely,
	 * that nesting has occurred to a very deep level -- a pathname
	 * of 100 characters, however, would still allow 33 levels
	 * of nesting before this gives up.)
	 *
	 * There is another weakness to this test which is very
	 * difficult to fix.  If a parent of the destination is
	 * linked to several different directories (only super-user
	 * can do this, supposedly), it is possible that the ".."
	 * entry refers to one of the other parent directories.
	 * In this case, it is possible to move a directory to one
	 * of its own subdirectories.  However, while the ".." entry
	 * will now be circular, the damage can be undone because
	 * the links from other directories are still around.
	 */

	if (ip->inumber != 1)          /* Unless already at inode 1 */
		while(1){
			lnp = copy("/..", lnp);
			if (stat(prnt, ip) < 0)
				errxit("Can't \"stat\" %s", prnt);
			if (!(ip->flags & IFDIR))
				errxit("%s not a directory!!", prnt);
			if (ip->inumber == 1) break;
			if (ip->inumber == srci.inumber)
				errxit("Destination is subdirectory of source");
			if (lnp > &prnt[196])
				errxit("Too much nesting on destination");
		}

	*eprt = 0;


	/* Now for the actual move.  First, disable interrupts,
	 * quits, and hangups and nice up to -5 to keep from
	 * being disturbed and to work quickly.
	 */

	signal(1,1);
	signal(2,1);
	signal(3,1);
	nice(-5);


	/* Next, link the source and destination directories
	 * together.  If an error occurs, merely exit -- nothing
	 * has been done.
	 */

	if (link(orig, dest) < 0)
		errxit("Can't link");


	/* Next, unlink the parent name ".." in the destination
	 * directory (now the same as the source).  If an erro
	 * occurs here, unlink the newly-formed link and exit.
	 */

	copy("/..", edst);
	if (unlink(dest) < 0){
		printf("Can't unlink %s", dest);
		*edst = 0;
		unlink(dest);
		exit();
	}


	/* Finally, link ".." to the new parent and delete the
	 * source name from its parent.  By now, an error will
	 * be very hard to process, so hope it works ok.
	 */

	if (link(prnt, dest) < 0)
		printf("Can't link %s to %s -- contact system staff\n",
			prnt, dest);
	if (unlink(orig) < 0)
		printf("Can't unlink %s -- contact system staff\n",
			orig);
	exit();

}

char *copy(p, q)
char *p, *q;
{
	/* Copy string starting at "p" to buffer starting at "q"
	 * until zero byte is copied.  Return address of terminating
	 * zero byte.
	 */

	while(*q++ = *p++);
	return(--q);
}

errxit(s, a){

	/* Print error message and terminate execution */

	printf(s, a);
	putchar('\n');
	exit();
}

access(p)
struct inode *p;
{

	/* Determine access permission on directory whose
	 * "stat" buffer is located at "p".  This routine
	 * assumes 16-bit user-id field and returns 1 if
	 * user is super-user, user owns the directory, or
	 * user has read-write-execute permission on it;
	 * returns 0 otherwise.
	 */


	register struct inode *ip;

	if (!uid) return(1);
	if (7 == (ip->flags & 7)) return(1);
	if (uid == ((ip->gid)&0377) + (((ip->uid)<<8)&0177400))
		return(1);
	return(0);
}
