#include "dis.h"

#define HTSIZE 0x1000			/* Power of 2 */
#define HTMASK (HTSIZE-1)

struct hashslot {
	int addr;			/* The key */
	struct ref_chain *ref;		/* Who references it */
	char *name;			/* The symbolic name (if it has one) */
};

struct   hashslot hashtbl[HTSIZE];	/* the hash table */

struct hashslot *
hash(loc, allocate)
int loc;
int allocate;
{
	int probes;
	register struct hashslot *hp;

	hp = &hashtbl[loc & HTMASK];
	probes = 0;

	while (probes< HTSIZE) {
		if (hp->addr == loc)
			return(hp);
		if (hp->name == NULL && hp->ref == NULL) {
			if (allocate) {
				hp->addr = loc;
				return(hp);
			} else {
				return(NULL);
			}
		}
		hp++;
		if (hp == &hashtbl[HTSIZE])
			hp = &hashtbl[0];
		probes++;
	}

	crash("Hash table full");
}

save_ref(refer, refee) 
int refer;
int refee;
{
	struct ref_chain *rc;
	struct hashslot *hp;

	rc = (struct ref_chain *)emalloc(sizeof(*rc));
	rc->who = refer;
	hp = hash(refee, 1);
	rc->next = hp->ref;
	hp->ref = rc;
}

save_name(loc, name)
int loc;
char *name;
{
	struct hashslot *hp;

	hp = hash(loc, 1);
	hp->name = name;
}

struct ref_chain *
get_ref(loc)
{
	struct hashslot *hp;

	hp = hash(loc, 0);
	if (!hp) 
		return(NULL);
	return(hp->ref);
}

char *
get_name(loc)
{
	struct hashslot *hp;

	hp = hash(loc, 0);
	if (!hp) 
		return(NULL);
	return(hp->name);
}
