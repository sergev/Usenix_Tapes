/*---- p2c.h ------------------------------------------------------
Defines and Global Variable for the Pascal to C translator
3/25/87 Daniel Kegel (seismo!rochester!srs!dan)
-------------------------------------------------------------------*/

#define MAXTOKLEN 2048	/* maximum token length */
    /* Note: even comments are jammed into a token; that's why this is big. */

typedef struct {	/* holds keywords, operators, etc. */
    char str[MAXTOKLEN];
    int kind;		/* code from table of wnodes */
} token;

typedef struct {
  int  ktype;		/* the meaning of the keyword */
  char *pname;		/* the Pascal name of the keyword */
  char *cname;		/* the C      name of the keyword */
} wnode;

	/* Allocate or Reallocate n 'type' items */
#define MALLOC(type, n) \
	((type *) DoMalloc((unsigned) sizeof(type) * (n)))
#define REALLOC(ptr, type, n) \
	((type *) DoRealloc((char *)ptr, (unsigned) sizeof(type) * (n)))

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef boolean
#define boolean int
#endif

/*--- The Global Variable ---------*/
token cTok;		/* current token from scanner */

