/* doalloc.c: memory allocations which exit upon error */

#include <stdio.h>
#ifndef NULL
#define NULL ((char *) 0)
#endif

/* act like calloc, but return only if no error */
char *DoRealloc(ptr,size)
    char *ptr;
    unsigned size;
{
    extern char *realloc();
    char *p;

    if ((p=realloc(ptr, size)) == NULL) {
	fprintf(stderr, "memory allocation (realloc) error");
	exit(1);
    }
    return (p);
}


/* act like malloc, but return only if no error */
char *DoMalloc(size)
    unsigned size;
{
    extern char *malloc();
    char *p;

    if ((p=malloc(size)) == NULL) {
	fprintf(stderr, "memory allocation (malloc) error");
	exit(1);
    }
    return (p);
}

