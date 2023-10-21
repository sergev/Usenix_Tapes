#include "nodes.h"

struct nptr *calloc();
char *strchr();

/* return the next available node in the array "sitetab" */
next_node()
{
	static short cur;
	if (++cur >= MAXSITE) {
		fprintf(stderr,"Not enough site nodes.\n");
		fprintf(stderr,"Fix \"MAXSITE\" in \"nodes.h\"\n");
		fprintf(stderr, "you might want to change \"NHASH\" too??? \n");
		exit(0);
	}
	return(cur);
}

/* Allocate a node for a neighbor */
struct nptr *
new_neigh()
{
	struct nptr *ptr;
	ptr = calloc(1, sizeof(struct nptr));
	if (ptr == NULL) {
		fprintf(stderr,"calloc failed in \"node.c\"\n");
		exit(0);
	}
	return(ptr);
}

/* append neighbor to linked list if he does not already exist */
#define NEXT ptr->n_next
#define TEMP ptr->n_index
add_neigh(current, nbor, prio)
char *nbor, *prio;
int current;
{

	struct nptr *ptr;
	char *tmp;
	int xxx;

	if ((tmp = strchr(nbor,'!')) != NULL) *tmp = '\0';
	ptr = site[current].s_un.s_next;
	if (strcmp(site[current].s_name, nbor) == 0) {
		return;
	}
	if (ptr == NULL) {
		ptr = site[current].s_un.s_next = new_neigh();
		ptr->n_index = lookup(nbor, 1);
		ptr->n_freq = yyparse1(prio);
		site[current].s_neigh++;
		total_neigh++;
	}
	else {
		for(;(NEXT != NULL) && strcmp(site[TEMP].s_name, nbor);
			ptr = NEXT);
		if (strcmp(site[TEMP].s_name, nbor) == 0) {
			if (ptr->n_freq > (xxx = yyparse1(prio)))
				ptr->n_freq = xxx;
		} else {
			NEXT = new_neigh();
			NEXT->n_index = lookup(nbor, 1);
			NEXT->n_freq = yyparse1(prio);
			site[current].s_neigh++;
			total_neigh++;
		}
	}
}

/* see if a site exists. if not (and flag == 1), add him to "sitetab" */
/* and return where he hashes to in "hashtab" (0 == notfound) */
lookup(ptr, flag)
char *ptr;
int flag;
{
	int hval;
	int sitenum;

	hval = hash(ptr);
	if (sitenum = hashtab[hval]) {
		for (;(site[sitenum].s_next != 0) && (strcmp(ptr, site[sitenum].s_name));
			sitenum = site[sitenum].s_next)
							;
		if (strcmp(ptr, site[sitenum].s_name)) {
			if (flag == 0) return(0);
			site[sitenum].s_next = next_node();
			sitenum = site[sitenum].s_next;
			strcpy(site[sitenum].s_name, ptr);
		}
		return (sitenum);
	} else {
		if (flag == 0) return(0);
		hashtab[hval] = next_node();
		strcpy(site[hashtab[hval]].s_name, ptr);
		return(hashtab[hval]);
	}
}
/* save all the arrays so we don't have to recompute them every time */
save()
{
	FILE *fp1;
	struct nptr *nptr;
	int offset, siteptr, count;
	int error_check;
	int retval;

	count = 0;
	for (siteptr = 1; siteptr < MAXSITE; siteptr++) {
		offset = (count + 1);
		nptr = site[siteptr].s_un.s_next;
		error_check = site[siteptr].s_neigh;
		for (;nptr != NULL; nptr = nptr->n_next) {

			if (++count == MAXNEIGH) {
				fprintf(stderr,"Neighbor table is to small;\n");
				fprintf(stderr,"Change \"MAXNEIGH\" in \"nodes.h\"\n");
				fprintf(stderr, "it is currently: %d\n", MAXNEIGH);
				fprintf(stderr, "Make it (%d) + 100 = %d.\n", total_neigh, total_neigh + 100);
				exit(0);
			}
			if ((count - offset) == error_check) {
				count--;
				fprintf(stderr, "Neighbor count mis-match.\n");
				fprintf(stderr, "Site: %s   #_of_neigh should be: %d\n",
					site[siteptr].s_name, error_check);
				fprintf(stderr, "Next neighbor is: %s\n", 
					site[(nptr->n_next->n_index)].s_name);
				break;
			}
			neightab[count] = nptr->n_index;
			weighttab[count] = nptr->n_freq;
		}
		site[siteptr].s_un.s_noff = offset;
	}
	fp1 = fopen(HASHPATH, "w");
	if (fp1 == NULL) {
		fprintf(stderr,"Cannot open %s\n",HASHPATH);
		exit(0);
	}
	retval = fwrite(hashtab, sizeof(hashtab[0]), NHASH, fp1);
	if (retval != NHASH) {
		fprintf(stderr, "Could not write \"hashtab\" to \"%s\"\n", HASHPATH);
   		exit(0);
	}
	fclose(fp1);
	if (chmod(HASHPATH, MODE) == -1) {
		fprintf(stderr,"Could not change %s to mode %d\n",HASHPATH, MODE);
	}
	fp1 = fopen(SITEPATH, "w");
	if (fp1 == NULL) {
		fprintf(stderr,"Cannot open %s\n",SITEPATH);
		exit(0);
	}
	retval = fwrite(site, sizeof(struct site), MAXSITE, fp1);
	if (retval != MAXSITE) {
		fprintf(stderr,"Could not write \"sitetab\" to \"%s\"\n", SITEPATH);
		exit(0);
	}
	fclose(fp1);
	if (chmod(SITEPATH, MODE) == -1) {
		fprintf(stderr,"Could not change %s to mode %d\n",SITEPATH, MODE);
	}
	fp1 = fopen(NEIGHPATH, "w");
	if (fp1 == NULL) {
		fprintf(stderr, "Cannot open %s\n", NEIGHPATH);
		exit(0);
	}
	retval = fwrite(neightab, sizeof(neightab[0]), MAXNEIGH, fp1);
	fclose(fp1);
	if (chmod(NEIGHPATH, MODE) == -1) {
		fprintf(stderr,"Could not change %s to mode %d\n",NEIGHPATH, MODE);
	}
	if (retval != MAXNEIGH) {
		fprintf(stderr,"Could not write \"neightab\" to %s\n", NEIGHPATH);
		exit(0);
	}
	fp1 = fopen(WEIGHPATH, "w");
	if (fp1 == NULL) {
		fprintf(stderr, "Cannot open %s\n", WEIGHPATH);
		exit(0);
	}
	retval = fwrite(weighttab, sizeof(weighttab[0]), MAXNEIGH, fp1);
	fclose(fp1);
	if (chmod(WEIGHPATH, MODE) == -1) {
		fprintf(stderr,"Could not write \"neightab\" to %s\n", WEIGHPATH);
	}
	if (retval != MAXNEIGH) {
		fprintf(stderr,"Could not write \"weighttab\" to %s\n", WEIGHPATH);
		exit(0);
	}
}
