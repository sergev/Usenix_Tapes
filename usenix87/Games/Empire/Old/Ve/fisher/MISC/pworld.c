/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	"emp.h"
#include	<stdio.h>

FILE	*fdsec, *fdship;

main()
{
	struct	sector	sect;
	int	i, j;

	if( (fdsec = fopen("empsect", "r")) == NULL ) {
		fprintf(stderr, "Can't open empsect file\n");
		exit(1);
	}
	putchar('\n');
	for( j = -32; j <= 31; j++ ) {
		for( i = -32; i <= 31; i++ ) {
			getsec(&sect, secno(i, j));
			putchar(sect.s_des);
			putchar(' ');
		}
		putchar('\n');
	}
}
