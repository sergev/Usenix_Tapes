#include "nodes.h"

FILE *fp1;
int tos, prcost;
int built, maybe, new;
char buf[NAMESIZE];
char stack[64][NAMESIZE];

main(argc, argv)
char **argv;
int argc;
{
	int i;
	i = 0;
	if (argc > 1) {
		if (argv[1][0] == '-') {
			while(argv[1][++i]) {
				if (argv[1][i] == 'd')
					prcost++;
				else if (argv[1][i] == 'n')
					new++;
				else fprintf(stderr,"illegal option \"-%c\"\n",
					argv[1][i]);
			}
		*++argv;
		argc--;
		}
	}
	fprintf(stderr,"Loading Data...\n");
	fp1 = fopen(HASHPATH, "r");
	if (fp1 == NULL) {
		fprintf(stderr, "Could not open %s\n", HASHPATH);
		exit(0);
	}
	if (fread (hashtab, sizeof(hashtab[0]), NHASH, fp1) <= 0) {
		fprintf(stderr, "Read of %s failed\n", HASHPATH);
		exit(0);
	}
	fclose(fp1);
	fp1 = fopen(NEIGHPATH, "r");
	if (fp1 == NULL) {
		fprintf(stderr, "Could not open %s\n", NEIGHPATH);
		exit(0);
	}
	if (fread (neightab, sizeof(neightab[0]), MAXNEIGH, fp1) <= 0) {
		fprintf(stderr, "Read of %s failed\n", NEIGHPATH);
		exit(0);
	}
	fclose(fp1);
	fp1 = fopen(WEIGHPATH, "r");
	if (fp1 == NULL) {
		fprintf(stderr, "Could not open %s\n", WEIGHPATH);
		exit(0);
	}
	if (fread (weighttab, sizeof(weighttab[0]), MAXNEIGH, fp1) <= 0) {
		fprintf(stderr, "Read of %s failed\n", WEIGHPATH);
		exit(0);
	}
	fclose(fp1);
	if (!new) {
		fp1 = fopen(TREEPATH, "r");
		if (fp1 == NULL) {
			fprintf(stderr,"Could not open %s, let me rebuild the tree.\n", TREEPATH);
			new++;
		}
		else {
			fread(site, sizeof(struct site), 1, fp1);
			if (argc > 1) {
				if (strcmp(argv[1], site[0].s_name) == 0)  {
					rewind(fp1);
					fread(site, sizeof(struct site), MAXSITE, fp1);
					built++;
				}
				else {
					fclose(fp1);
				}
			}
			else if (strcmp(site[0].s_name, HOME) == 0) {
				rewind(fp1);
				fread(site, sizeof(struct site), MAXSITE, fp1);
				built++;
			}
			else {
				fclose(fp1);
				new++;
			}
		}
	}
	if (new || !built) {
		fp1 = fopen(SITEPATH, "r");
		if (fp1 == NULL) {
			fprintf(stderr, "Could not open %s\n", SITEPATH);
			exit(0);
		}
		if (fread (site, sizeof(struct site), MAXSITE, fp1) <= 0) {
			fprintf(stderr, "Read of %s failed\n", SITEPATH);
			exit(0);
		}
	}
	fclose(fp1);
	fprintf(stderr,"\n");
	if (argc > 1) {
		make_tree(*++argv);
	}
	else make_tree(HOME);
	if (!built) savetree();
	while(1) {
		printf("Dest: ");
		if (gets(buf) == NULL) break;
		if (buf[0] == '\0') continue;
		i = lookup(buf, 0);
		if (i == 0) {
			fprintf(stderr, "%s does not exist.\n", buf);
			continue;
		}
		/*
		for(sp = &site[i]; sp != &site[0]; sp = &site[sp->s_parent]) {
			printf("%s!", sp->s_name);
		}
		printf("%s\n", sp->s_name);
*/
		printf("Best Route: ");
		tos = 0;
		pr_path(&site[i]);
		if (prcost) pr_cost(stack);
		printf("\n\n");
	}
}
/* this routine prints out the neighbors of a site. */
/* it was used for debugging at one point and is */
/* used no more.				  */
get_neigh(ptr)
char *ptr;
{
	int sitenum, count, nptr;

	sitenum = lookup(ptr, 0);
	if (sitenum == 0) {
		fprintf(stderr, "%s does not exist.\n", ptr);
		return;
	}
	if (!site[sitenum].s_neigh) {
		printf("%s has no neighbors.\n", site[sitenum].s_name);
		return;
	} else printf("%s:\n", site[sitenum].s_name);
	nptr = site[sitenum].s_un.s_noff;
	count = site[sitenum].s_neigh;
	while (count--) printf("\t%s\n",site[neightab[count+nptr]].s_name);
	printf("\n");
}
/* build the tree (this takes lots of time) */
make_tree(ptr)
char *ptr;
{
	register struct site *sp;
	register int numsite, i;
	register short *pi, *pw;
	int minloc, mincost, n_nodes, us;
	short *pl;

	if (built) return;
	maybe++;
	strcpy(site[0].s_name, ptr);
	us = lookup(ptr, 0);
	if (us == 0) {
		fprintf(stderr,"Ummm, Do we exist? \n");
		fprintf(stderr,"I can't find \"%s\" in the hash table!\n",
		    ptr);
		exit(0);
	}
	fprintf(stderr,"Building a tree is not easy...\n");
#ifdef VERBOSE
	fprintf(stderr,"       <= nodes in tree\r");
#endif
	for (i = 1; i <= site[0].s_neigh; i++) {
		site[i].s_cost = BIG;
	}
	site[us].s_cost = -2;
	for (pi = &neightab[site[us].s_un.s_noff],
	    pw = &weighttab[site[us].s_un.s_noff],
	    pl = pi + site[us].s_neigh; pi < pl;  pw++, pi++) {
		sp = &site[*pi];
#ifdef DEBUG
		fprintf(stderr, "setting site %d %s to %d\n", *pi, sp->s_name, *pw);
#endif
		sp->s_cost = *pw;
		sp->s_parent = us;
	}
	numsite = n_nodes = site[0].s_neigh-1;
	while (n_nodes--) {
#ifdef VERBOSE
		fprintf(stderr,"%-d\r",numsite - n_nodes);
#endif
		mincost = BIG;
		minloc = -2;
		for (i = 1; i <= site[0].s_neigh; i++) {
			sp = &site[i];
			if ((sp->s_cost >= 0) && (sp->s_cost < mincost)) {
				mincost = sp->s_cost;
				minloc = i;
			}
		}
		if (minloc == -2) {
			fprintf(stderr, "\nCould not connect %d sites.\n", n_nodes+1);
			return;
		}
		for (pi = &neightab[site[minloc].s_un.s_noff],
		    pw = &weighttab[site[minloc].s_un.s_noff],
		    pl = pi + site[minloc].s_neigh; pi < pl;  pw++, pi++) {
			sp = &site[*pi];
			if (mincost + *pw < sp->s_cost) {
				sp->s_cost = *pw + mincost;
				sp->s_parent = minloc;
			}
		}
		site[minloc].s_cost = -2;
	}
}
/* print out the path. since the path is reversed
   (dest to home), this routine is recursive to
   compensate.					*/
pr_path(ptr)
struct site *ptr;
{
	if (ptr->s_parent == 0) {
		 printf("%s",ptr->s_name);
		strcpy(stack[tos++], ptr->s_name);
	}
	else {
		pr_path(&site[ptr->s_parent]);
		if ((ptr->s_isnet != '1') || !strcmp(ptr, buf))
			 printf("!%s",ptr);
		strcpy(stack[tos++], ptr);
	}
}
/* save the tree */
savetree()
{
	int retval;
	if (!new) {
		fprintf(stderr, "Do want to save the tree (deflt = no)? ");
		gets(buf);
		if (!(buf[0] == 'y' || buf[0] == 'Y')) return;
	}
	fprintf(stderr, "Saving the tree.\n");
	fp1 = fopen(TREEPATH, "w");
	if (fp1 == NULL) {
		fprintf(stderr, "Cannot open %s\n",TREEPATH);
		exit(0);
	}
	retval = fwrite(site, sizeof(struct site), MAXSITE, fp1);
	if (retval != MAXSITE) {
		fprintf(stderr,"Could not write \"site\" to %s\n", TREEPATH);
	}
	fclose(fp1);
	chmod(TREEPATH, MODE);
	fprintf(stderr,"Tree has been saved\n");
}
/* also used for debugging - prints out the cost
   between sites (Networks appear as a "1")	*/
pr_cost(ptr)
char ptr[][NAMESIZE];
{
	FILE *fp;
	int i, offset, current, nptr;

	tos--;
	printf("\n");
	i = offset = 0;
	if (tos == 0) {
		return;
	}
	fp = fopen(SITEPATH, "r");
	if (fp == NULL) {
		fprintf(stderr,"Could not open %s.\n", SITEPATH);
		return;
	}
top:
	current = lookup(ptr[i], 0);
	if (current == 0) {
		fprintf(stderr,"Could not find %s in the table.\n", ptr[i]);
		return;
	}
	nptr = site[current].s_un.s_noff;
	for(offset = 0;
		(site[current].s_neigh > offset) &&
		(strcmp(site[neightab[nptr + offset]].s_name, ptr[i + 1]) != 0);
			offset++);
	if ((site[current].s_neigh <= offset)) {
		fprintf(stderr, "Could not find cost between %s & %s\n",
			ptr[i], ptr[i+1]);
		return;
	}
	if (weighttab[nptr + offset] == NETVAL) 
		printf("NET!");
	else
		printf("%d!", weighttab[nptr + offset]);
	i++;
	if (!(i < tos)) {
		printf("you're_there");
		return;	
	}
	else goto top;
}
