/*
**	GOTST -- Random GO player (for testing referee)
*/
#include	<stdio.h>
#include	"godef.h"

main()
{
	char buf[128];
	int omove, tmove;
	int ocolor, tcolor;

	setbuf(stdout, 0);
	srand((int) time(0));
	fgets(buf, sizeof buf, stdin);
	if (strcmp(buf, "black\n") == 0) {
	    ocolor = BLACK;
	    tcolor = WHITE;
	} else if (strcmp(buf, "white\n") == 0) {
	    ocolor = WHITE;
	    tcolor = BLACK;
	} else {
	    fprintf(stderr, "Huh?  Expected `black' or `white', got `%s'\n",
	     buf);
	    fputs("resign\n", stdout);
	    exit(1);
	}
	bdinit(&b);                           /* initialize board contents */
	omove = tmove = 0;
	switch (ocolor) {
	case WHITE:
	    for (;;) {
		fgets(buf, sizeof buf, stdin);
		tmove = ctos(buf);
		if (tmove == RESIGN || (tmove == PASS && omove == PASS))
		    break;
		makemove(tcolor, tmove);
	case BLACK:
		omove = pickmove(ocolor);
		printf("%s\n", stoc(omove));
		if (omove == RESIGN || (tmove == PASS && omove == PASS))
		    break;
		makemove(ocolor, omove);
	    }
	}
}

pickmove(ocolor)
{
	register int i, x, y, omove;
	char buf[128], oc;

	for (i = 100; --i > 0; ) {
	    x = (rand() % BSZ) + 1;
	    y = (rand() % BSZ) + 1;
	    omove = x * BSZ2 + y;
	    if (legality(ocolor, omove) == LEGAL)
		return(omove);
	}
	return(PASS);
}

log(str)
char	*str;
{
	fputs(str, stderr);
}
