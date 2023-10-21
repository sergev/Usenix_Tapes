#
/*
 * C library -- alloc/free
 */

#define	logical	char *

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

int aftrace;
char *aftop -1;
char *afbottom 0;

struct fb {
	logical	size;
	char	*next;
};

int	freelist[] {
	0,
	-1,
};
logical	slop	2;

int afnfree 0;
int afnused 0;

alloc(asize)
logical asize;
{
	register logical size;
	register logical np;
	register logical cp;

	if ((size = asize) == 0)
		return(0);
	if(size < 0)
		error("workspace exceeded");
	size =+ 3;
	size =& ~01;
	for (;;) {
		cp = freelist;
		while ((np = cp->next) != -1) {
			if (np->size>=size) {
				if (size+slop < np->size) {
					cp = cp->next = np+size;
					cp->size = np->size - size;
					np->size = size;
				}
				cp->next = np->next;
				afnused =+ np->size;
				afnfree =- np->size;
if(aftrace)
printf("alloc:	%d\tf%d\tu%d\n", np->size, afnfree, afnused);
				return(&np->next);
			}
			cp = np;
		}
		asize = size<1024? 1024: size;
		if ((cp = sbrk(asize)) == -1) {
			error("workspace exceeded");

		}
		aftop = min(aftop, cp);
		afbottom = max(afbottom, cp+asize);
		cp->size = asize;
		afnused =+ asize;
		free(&cp->next);
	}
}

free(aptr)
char *aptr;
{
	register logical ptr;
	register logical cp;
	register logical np;

	if(afbottom <= aptr || aftop > aptr){
		printf("[free botch: !(%o <= %o < %o)]\n", aftop, aptr, afbottom);
		return;
	}
	ptr = aptr-2;
	cp = freelist;
if(aftrace)
printf("free:	%d\tf%d\tu%d\n", ptr->size, afnfree, afnused);
	afnfree =+ ptr->size;
	afnused =- ptr->size;
	while ((np = cp->next) < ptr)
		cp = np;
	if (ptr+ptr->size == np) {
		ptr->size =+ np->size;
		ptr->next = np->next;
		np = ptr;
	} else
		ptr->next = np;
	if (cp+cp->size == ptr) {
		cp->size =+ ptr->size;
		cp->next = ptr->next;
	} else
		cp->next = ptr;
}
