
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 */

#include <stdio.h>
#define maxlen 8

char rword[20];
char vow[] = {'a','e','i','o','u','y'};
char con[] = {'t','n','s','h','r','d','l','b','c','f','g','j','k',
                'm','p','w','v','z','x','q'};
char dip[][2] = {'t','h','s','h','s','s','r','d','q','u',
                   'l','l','r','n','s','p','s','t','r','z'};
char *fword()
{
	char nl, a, b, c, d;

	nl = (rand() % maxlen + rand() % maxlen)/2 + 1;
	for (b = 0; b < nl; ){
		switch (rand() % 8) {
			case 0 :
			case 1 :
			case 2 :
			case 3 :
			case 4 :
			case 5 :
			case 6 :
				rword[b++] = con[abs(rand() % 20 + rand() %
					20 - 20)]; 
				break;
			case 7:
				c = abs(rand() % 10 + rand() % 10 - 10);
				rword[b++] = dip[c][0];
				rword[b++] = dip[c][1];
				break;
		}
		rword[b++] = vow[abs(rand() % 6 + rand() % 6 - 6) ];
	}
	if (rand() % 10 < 7)rword[b++] = con[rand() % 20];
	rword[b++]='\0';
	return (rword);
}


main() {

	char *fword();

	srand(getpid());
	printf ("%s\n",fword());
}

