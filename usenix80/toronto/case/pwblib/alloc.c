#
char alloc__[] "~|^`alloc.c:	1.2";
/*
	Alloc/free based on those in C library at one time.
	Also, zero() zeroes memory (first arg must be on word
	boundary) and freeall() frees all memory allocated.
	Second arg to alloc() if present causes allocated memory
	to be zeroed.

	For documentation see alloc(III) dated 3/1/74.
*/

#define	logical	char *

struct fb {
	logical	size;
	char	*next;
};

int	freelist[] {
	0,
	-1,
};
logical	slop	2;
logical frstbrk;

xalloc(asize,z)
logical asize, z;
{
	register logical size;
	register logical np;
	register logical cp;

	if((size = asize) == 0)
		return(0);
	size =+ 3;
	size =& ~01;
	for(;;) {
		cp = freelist;
		while((np = cp->next) != -1) {
			if(np->size>=size) {
				if(size+slop >= np->size)
					cp->next = np->next;
				else {
					cp = cp->next = np+size;
					cp->size = np->size - size;
					cp->next = np->next;
					np->size = size;
				}
				if(nargs()==2) return(zero(&np->next,size-2));
				else return(&np->next);
			}
			cp = np;
		}
		asize = size<1024? 1024: size;
		if((cp = sbrk(asize)) == -1) fatal("out of space (55)");
		if(frstbrk == 0) frstbrk = cp;
		cp->size = asize;
		xfree(&cp->next);
	}
}

xfree(aptr)
char *aptr;
{
	register logical ptr;
	register logical cp;
	register logical np;

	ptr = aptr-2;
	cp = freelist;
	if(cp->next < 0)
		return;
	while((np = cp->next) < ptr)
		cp = np;
	if(ptr+ptr->size == np) {
		ptr->size =+ np->size;
		ptr->next = np->next;
		np = ptr;
	} else
		ptr->next = np;
	if(cp+cp->size == ptr) {
		cp->size =+ ptr->size;
		cp->next = ptr->next;
	} else
		cp->next = ptr;
}


xfreeall()
{
	if(frstbrk)
		brk(frstbrk);
	freelist[0] = frstbrk = 0;
	freelist[1] = -1;
}


zero(x,b)
int *x, b;
{
	register int *p, *w;

	w = x + ((b+1)>>1);
	for(p=x; p<w; p++) *p = 0;
	return(x);
}
