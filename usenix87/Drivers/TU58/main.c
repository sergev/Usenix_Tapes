/*
 * tu58 - Emulate a TU58 over a serial line
 *
 *	Copyright (C) Rockefeller Univ. Dept. of Neurobiology
 *
 * Usage: tu58 [-options] serial_device file0 [file1]
 * Options:	-s####	set line speed
 *		-r[#]	readonly drive
 *		-c[#]	create files
 *		-i[#]	initialize RT-11 directory
 */

#ifdef	PWBTTY

#include <sys/sgtty.h>
#define	sg_ispeed	sg_ispd
#define	sg_ospeed	sg_ospd
#define	sg_flags	sg_flag
#define	sgttyb		sgtty

#else

#include <sgtty.h>

#endif

#include "tu58.h"
#include <stdio.h>
#include "debug.h"

 int r_device = 0;			/* Default to stdin */
 int w_device = 1;			/* Default to stdout */
 int speed = B2400;			/* Default line speed */
 struct sgttyb osg[2];

main(c, v)
int c; char **v;
{
	register int i;
	register char *s;

	/*
	 * init
	 */

	fileinit();

	/*
	 * parse command line for flags
	 */

	while (c > 1 && *(s = v[1]) == '-' && s[1]) {
		v++;
		c--;
		switch (s[1]) {
		case 's':
			i = baud(atoi(&s[2]));
			if (i < 0)
				error("Bad speed: %s", &s[2]);
			else
				speed = i;
			break;
		case 'r':
			readonly(s);
			break;
		case 'c':
			createflag(s);
			break;
		case 'i':
			initrtflag(s);
			break;
		default:
			error("Bad option: %s\n", s);
		}
	}

	/* 
	 * Open the files
	 */

	if (c < 3 || c > (2+NTU58))
		error("Usage: tu58 [-options] serial_device file0 [file1]");

	if (strcmp("-", v[1]))  {
		root();
		if( (r_device = w_device = open(v[1], 2)) < 0 )
			error("No serial line");
		notroot();
	}

	c -= 2;
	v += 2;
	for (i = 0; i < c; i++) 
		if( fileopen(v[i], 2, 0644) < 0 )
			error("Can't open %s nohow\n",v[i]);

#ifdef DEBUG
	/*
	 * open debug file
	 */
	db_open(r_device);
#endif
	
	/*
	 * setup serial lines and play tu58
	 */

	init_device(r_device);
	if (r_device != w_device)
		init_device(w_device);

	tu58();

	restore_device(r_device);
	if (r_device != w_device)
		restore_device(w_device);
#ifdef DEBUG
	db_close();
#endif
}

/*
 * set and reset serial lines
 */

init_device(fd)
int fd;
{
	static int sgx = 0;
	struct sgttyb sg;

	if (gtty(fd, &osg[sgx]))
		error("Bad serial device");
	sg = osg[sgx];
	if (speed >= 0)
		sg.sg_ospeed = sg.sg_ispeed = speed;
	sg.sg_flags &= ~(CBREAK|ECHO|XTABS);
	sg.sg_flags |= (RAW);
	stty(fd, &sg);
	sgx++;
}

restore_device(fd)
int fd;
{
	static int sgx = 0;

	stty(fd, &osg[sgx++]);
}

 int baud_map[] = {	19200,	EXTA,	9600,	B9600,	4800,	B4800,
			2400,	B2400,	1200,	B1200,	600,	B600,
			300,	B300,	200,	B200,	150,	B150,
			110,	B110,	75,	B75,	50,	B50,
			0,	B0,	-1 };

baud(spd)
register int spd;
{
	register int *p;

	for (p = baud_map; *p != -1; p++)
		if (*p++ == spd)
			return *p;
	return -1;
}

error(a, b, c, d, e)
char *a, *b, *c, *d, *e;
{
	fprintf(stderr, "tu58: ");
	fprintf(stderr, a, b, c, d, e);
	fprintf(stderr, "\n");
	exit(-1);
}

int uid, euid;

root()
{
	euid = geteuid();
	uid = getuid();
	setreuid(euid,-1);
}

notroot()
{
	setreuid(uid,euid);
}
