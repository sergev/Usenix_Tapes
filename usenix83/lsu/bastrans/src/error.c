#include "bastrans.h"
error(t,s)
int t;
char s[CLEN];
{
	switch (t) {

		case OS:
			printf("\nOut of space...too many BASIC lines\n");
			break;
		case USE:
			printf("Useage: bastrans {BASIC source file}\n");
			break;
		case FRE:
			printf("Can't read %s\n",s);
			break;
		case FNB:
			printf("%s is not a BASIC program\n",s);
			break;
		case FWE:
			printf("Open failed on %s\n",s);
			break;
		case IFTH:
			printf("Missing THEN in IF:\n\t%s\n",s);
			break;
		case PPF:
			printf("Can't parse PRINT statement in:\n\t%s\n",s);
			break;
		default:
			printf("\nUnrecognized error in bastran\n");
			break;
	}
	exit(0);
}
