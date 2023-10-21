#

/*
 *	Checkmate
 *
 *	Original by Greg Ordy.
 *	Modified by Bill Shannon.
 */

#define		MAXPLAYER	9
#define		DEAD		-1
#define		NOT_HERE	-2
#define		ALIVE		0

/* keys for direction control */
#define		UP		'8'
#define		DOWN		'2'
#define		LEFT		'4'
#define		RIGHT		'6'

/* x and y dimensions, minus 1 */
#define		XMAX		78
#define		YMAX		23

/* number of lines in /etc/ttys */
#define		NTTYS		31

#define		spaces		"                                                                                "

struct
{
	char	tty;
	char	dir;
	int	fd;
	int	status;
	char	name[10];
	int	curx;
	int	cury;
	int	mode;
	int	type;
}
	player[MAXPLAYER];

struct {
	char	enabled;
	char	ttyname;
	char	ttytype;
	char	newline;
} tty[NTTYS];

int arg[3];	// for stty

char	screen[80][24];

int	nplayers;
char	*bp;

char	devname[]	{"/dev/tty "};
char	lock[]		{"/tmp/chklock"};

int onintr();

main(argc, argv)
char *argv[];
{
	char		c, buffer[80];
	register int	i, x, y;
	extern int	atoi();
	int		lfd, j, z, tvec[2];
	int		efd;
	
	if ((lfd = open(lock, 0)) > 0)
	{
		/* slave */
		close(lfd);
		devname[8] = ttyn(0);
		chmod(devname, 0666);
		signal(2, onintr);
		signal(3, onintr);
		pause();
	}
	else
	{
		/* master */
		lfd = creat(lock, 0644);
		z = getpid();
		write(lfd, &z, 2);
		close(lfd);
	}

	if ((efd = open("/etc/ttys", 0)) < 0)
	{
		printf("can't open /etc/ttys\n");
		exit(8);
	}
	read(efd, tty, sizeof tty);
	close(efd);

	time(tvec);
	srand(tvec[1]);
	
	bp = argv[1];
	for (i = 0; i < MAXPLAYER && *bp != 0; i++)
	{
		c = *bp++;
		player[i].tty = c;
		devname[8] = c;
		if ((player[i].fd = open(devname, 1)) < 0)
		{
			printf("Cannot open /dev/tty%c\n", c);
			exit();
		}
	}
	nplayers = i;
	
	for (i = 0; i < nplayers; i++)
		write(player[i].fd, "\007Checkmate countdown in 15 seconds!!\n", 37);
	sleep(15);
	
	for (i = 0; i < nplayers; i++)
	{
		close(player[i].fd);
		devname[8] = player[i].tty;
		if ((player[i].fd = open(devname, 2)) < 0)
		{
			write(1, "Player ", 7);
			write(1, &player[i].tty, 1);
			write(1, " won't play\n", 12);
			player[i].status = NOT_HERE;
		}
		else
		{
			gtty(player[i].fd, arg);
			player[i].mode = arg[2];
			arg[2] = 0160;		// odd parity, raw, map CR
			stty(player[i].fd, arg);
			for (j = 0; j < NTTYS; j++)
				if (tty[j].ttyname == player[i].tty)
				switch (tty[j].ttytype) {
				case '2': player[i].type = 2; break;
				case '4': player[i].type = 3; break;
				}
			player[i].tty = player[i].tty - 040;
			player[i].status = ALIVE;
		}
	}
	
	for (x = 10; x > 0; x--)
	{
		for (i = 0; i < nplayers; i++)
		if (player[i].status != NOT_HERE)
		{
			z = x / 10;
			if  (z) {
				z += '0';
				write(player[i].fd, &z,  1);
			}
			z = x % 10 + '0';
			write(player[i].fd, &z,  1);
			write(player[i].fd, "\n",  1);
		}
		sleep(1);
	}
	
	for (i = 0; i < nplayers; i++) 
	if (player[i].status != NOT_HERE)
	{
		eras(player[i].fd, player[i].type);
		curs(0, 0, player[i].fd, player[i].type);
		write(player[i].fd, "**********************************************************************************", XMAX);
		curs(0, YMAX, player[i].fd, player[i].type);
		write(player[i].fd, "**********************************************************************************", 79);
		for (z = 0; z < YMAX; z++)
		{
			curs(0, z, player[i].fd, player[i].type);
			write(player[i].fd, "*", 1);
			curs(XMAX, z, player[i].fd, player[i].type);
			write(player[i].fd, "*", 1);
		}
		x = player[i].curx = rand()%70 + 4;
		y = player[i].cury = rand()%16 + 4;
		screen[x][y] = i + 1;
		switch(rand()%4)
		{
			case  0: player[i].dir = UP;  break;
			case  1: player[i].dir = DOWN; break;
			case  2: player[i].dir = LEFT; break;
			case  3: player[i].dir = RIGHT; break;
		}
	}
	
	for (j = 0; j < nplayers; j++)
	if (player[j].status != NOT_HERE)
	{
		for (i = 0; i < nplayers; i++)
		{
			x = player[i].curx;
			y = player[i].cury;
			curs(x, y, player[j].fd, player[j].type);
			write(player[j].fd, &player[i].tty,  1);
		}
	}
	
	loop:
	j = 0;
	z = 0;
	for (i = 0; i < nplayers; i++)
		if (player[i].status == ALIVE) { z = i; j++;}
	if (j == 1 && nplayers > 1)
	{
		for (i = 0 ; i < nplayers; i++)
		if (player[i].status != NOT_HERE)
		{
			curs(0, YMAX, player[i].fd, player[i].type);
			write(player[i].fd, spaces, XMAX);
			curs(0, YMAX, player[i].fd, player[i].type);
			write(player[i].fd, "WINNER: Player ",  15);
			write(player[i].fd, &player[z].tty,  1);
			write(player[i].fd, "\n", 1);
		}
		done();
	}
	if (j == 0 && nplayers == 1) done();
	for (i = 0; i < nplayers; i++)
	{
		if (player[i].status != ALIVE) continue;
		x = player[i].curx;
		y = player[i].cury;
		if (!empty(player[i].fd))
		{
			read(player[i].fd, &c, 1);
			switch(c)
			{
				case    UP:
				case  DOWN:
				case  LEFT:
				case RIGHT:
				player[i].dir = c;
			}
		}
		switch(player[i].dir)
		{
			case    UP: y--; break;
			case  DOWN: y++; break;
			case  LEFT: x--; break;
			case RIGHT: x++; break;
		}
		if (screen[x][y] != 0 || x == 0 || y == 0 || x == XMAX || y == YMAX)
		{
			player[i].status = DEAD;
			c = '+';
		}
		else
		{
			c = player[i].tty;
			player[i].curx = x;
			player[i].cury = y;
			screen[x][y] = i + 1;
		}
		for (j = 0; j < nplayers; j++)
		if (player[j].status != NOT_HERE)
		{
			curs(x, y, player[j].fd, player[j].type);
			write(player[j].fd, &c, 1);
		}
	
	}
	nap(30);
	goto loop;
}


done()
{
	register int i;


	for (i = 0; i < nplayers; i++)
	if (player[i].status != NOT_HERE)
	{
		gtty(player[i].fd, arg);
		arg[2] = player[i].mode;
		stty(player[i].fd, arg);
	}
	exit(0);
}

_cleanup()
{
	unlink(lock);
}

onintr()
{
	chmod(devname, 0622);
	exit(0);
}
