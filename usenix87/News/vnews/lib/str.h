/* string declarations */

char *index(), *rindex(), *sindex(), *strpbrk();
char *ckmalloc(), *savestr(), *nsavestr();
#define scopy(from, to)	strcpy(to, from)
#define equal(s1, s2)	(strcmp(s1, s2) == 0)
