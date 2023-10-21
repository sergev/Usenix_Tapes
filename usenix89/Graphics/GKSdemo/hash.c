#include "mongo.h"

#define HASHSIZE 100

static struct nlist *hashtab[HASHSIZE];


	/* fill the hashtable with NULL's */

table_init()
{
	int i;
	
	for (i = 0; i < HASHSIZE; i++)
		hashtab[i] = NULL;
}


	/* form a hash value */

hash(s)
char *s;
{
	int hashval;

	for (hashval = 0; *s != '\0'; )
		hashval += *s++;
	return(hashval % HASHSIZE);
}


	/* return a pointer to the struct containing the string
	   s in the table, or NULL if none is found. */

struct nlist *
lookup(s)
char *s;
{
	struct nlist *np;

	for (np = hashtab[hash(s)]; np != NULL; np = np->next)
		if (strcmp(s, np->name) == 0)
			return(np);
	return(NULL);
}

	/* install a new entry into the hash table, or return NULL
	   if there is no more room */ 

struct nlist *
install(name, func)
char *name;
PFI func;
{
	struct nlist *np, *lookup();
	char *malloc(), *strsave();
	int hashval;

	if ((np = lookup(name)) == NULL) {
		np = (struct nlist *) malloc (sizeof(struct nlist));
		if (np == NULL)
			return(NULL);
		if ((np->name = strsave(name)) == NULL)
			return(NULL);
		hashval = hash(name);
		np->next = hashtab[hashval];
		hashtab[hashval] = np;
	}
	np->func = func;
	return(np);
}
		
	/* return a pointer to a copy of the given string, or NULL
	   if there is no more free memory */

char *
strsave(s)
char *s;
{
	char *p, *malloc();

	if ((p = malloc(strlen(s))) == NULL)
		return(NULL);
	strcpy(p, s);
	return(p);
}	
