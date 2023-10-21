/*
 *	ena - become super-user, but only if in correct group
 *
 *	ena allows anyone that can execute it to become super-user.
 *	security is maintained by only allowing people in group
 *	'wheel' to execute it.
 *
 *		chown root /bin/ena
 *		chgrp wheel /bin/ena
 *		chmod 4110 /bin/ena
 *
 *	Bill Shannon - 08/21/77
 */

main()
{
	setuid(0);				/* become super-user */
	execl("/bin/sh", "-", 0);		/* get us a shell */
	printf("cannot execute shell\n");	/* if error */
}
