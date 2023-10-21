
/* hostname.c		January 12, 1987	
 * Identical to Berkeley "hostname"  program.
 * 	hostname [name]
 * If name is not given, the current hostname is displayed on stdout.
 * Otherwise, the hostname is set to "name".  Needless to say, the
 * second form requires root privilege.
 */

#include <stdio.h>
#include <sys/utsname.h>
#include <a.out.h>
#include <fcntl.h>

#define NODE_LEN	9
#define KERNEL		"/unix"
#define KMEM		"/dev/kmem"

struct nlist k[] = {
#define UTSNAME		0
   { "_utsname" },
   { 0 }
};

main(argc, argv)
	register int argc;
	register char *argv[];
{
	struct utsname name_buf;
	register int kmem;

	/* Get the current name */
	uname(&name_buf);

	if  (argc < 2) {
		/* Display host name */
		printf ("%s\n", name_buf.nodename);
	} else {
		/* Set host name */

		strncpy (name_buf.nodename, argv[1], NODE_LEN-1); /* Put new name in structure */

		if (nlist(KERNEL, k) != 0) {	/* Find where utsname is */
			perror(KERNEL);
			exit(1);
		}

		if ((kmem = open(KMEM, O_RDWR)) == -1) { /* Open kernel memory */
			perror(KMEM);
			exit(2);
		}

		if (lseek(kmem, (long) k[UTSNAME].n_value, 0) == -1L) {	/* Position us to utsname in kernel */
			perror(KMEM);
			exit(3);
		}

		if (write(kmem, (char *) &name_buf, sizeof name_buf) == -1) { /* Write new utsname structure */
			perror(KMEM);
			exit(4);
		}

		close(kmem);
	}
}
