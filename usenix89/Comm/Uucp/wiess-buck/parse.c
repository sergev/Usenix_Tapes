#include "nodes.h"
#include <sys/types.h>
#include <sys/dir.h>


char *strchr();
int thresh;

main(argc, argv)
char *argv[];
int argc;
{
	int i;

	thresh = yyparse1("DAILY");
	if (argc > 1) for (i = 1; i < argc; i++) parse(argv[i]);
	else read_dir();
	fprintf(stderr, "Saving info...                          \n");
	site[0].s_neigh = next_node() - 1;
	neightab[0] = total_neigh;
	save();
	fprintf(stderr,"Total # of sites = %d\n", site[0].s_neigh);
	fprintf(stderr,"Total # of neighbors = %d\n", total_neigh);
}
parse(name)
char *name;
{
	FILE *fp;
	char *tmp, *ctmp;
	char *ptr, *ptr2, *ptr3;
	char buf[512], file[256];
	register int current;
	char sitebuf[16], dest[16], prio[256];

	sprintf(file,"%s%s",PATH,name);
#ifdef VERBOSE
	fprintf(stderr,"Parsing %s            \r",file);
#else
	fprintf(stderr,".");
#endif
	fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr,"Could not open %s\n",file);
		return;
	}
	while (fgets(buf, 510, fp) != NULL) {
		if (buf[0] == '#') continue;
		if (strchr(buf,'=')) {
			if (strchr(buf, '{')) {
				if (network(fp, buf) == -1) {
					fclose(fp);
					return;
				}
				continue;
			}
			while ((buf[0] != '#') && (tmp = fgets(buf, 510, fp)));
			if (tmp == NULL) {
				fclose(fp);
				return;
			}
			else continue;
		}
		for (ptr = buf; isspace(*ptr); ptr++);
		if (*ptr == '\0') continue;
		ptr = buf;
		ptr2 = sitebuf;
		ptr3 = &sitebuf[15];
		while(!isspace(*ptr) && *ptr != '(' && *ptr != '\t' && *ptr != '\n' && *ptr != '#')
			if (ptr2 < ptr3)	*ptr2++ = *ptr++;
			else ptr++;
		if (*ptr == '\n' || *ptr == '#') {
newline:
			if (fgets(buf, 510, fp) == NULL) {
				fprintf(stderr,"Error in file %s\n",name);
				fprintf(stderr,"Site without a destination???\n");
				fclose(fp);
				return;
			}
			ptr = buf;
		}
		*ptr2 = '\0';
		if ((ctmp = strchr(sitebuf,'!')) != NULL) *ctmp = '\0';
		current = lookup(sitebuf, 1);
again:
		while (isspace(*ptr) && *ptr != '\n' ) ptr++;
		if (*ptr == '\n' || *ptr == '#') goto newline;
		if (*ptr == '=') {
			while ((buf[0] != '#') && (tmp = fgets(buf, 510, fp)));
			if (tmp == NULL) {
				fclose(fp);
				return;
			}
			ptr = buf;
		}
		ptr2 = dest;
		ptr3 = &dest[15];
		while (!isspace(*ptr) && *ptr != '(' && *ptr != ',' &&  *ptr != '#')
			if (ptr2 < ptr3)	*ptr2++ = *ptr++;
			else ptr++;
		while(*ptr == ' ' || *ptr == '\t') *ptr++;
		*ptr2 = '\0';
		if (*ptr == '\n' || *ptr == '#') {
			strcpy(prio,"DEFAULT");
			add_neigh(current, dest, prio);
			if (yyparse1(prio) < thresh)
				 strcpy(prio, "DAILY");
			add_neigh(lookup(dest, 1), site[current].s_name, prio);
			continue;
		}
		if (*ptr == ',') {
			strcpy(prio,"DEFAULT");
			add_neigh(current, dest, prio);
			if (yyparse1(prio) < thresh)
				 strcpy(prio, "DAILY");
			add_neigh(lookup(dest, 1), site[current].s_name, prio);
			goto nxt;
		}
		ptr++;
		ptr2 = prio;
		while (*ptr != ')' || (*(ptr+1) == '/' || *(ptr+1) == '*'))
			*ptr2++ = *ptr++;
		ptr++;
		*ptr2 = '\0';
		add_neigh(current, dest, prio);
		if (yyparse1(prio) < thresh)
			 strcpy(prio, "DAILY");
		add_neigh(lookup(dest, 1), site[current].s_name, prio);
nxt:
		while (*ptr != ',' && *ptr != '\n' && *ptr != '#') ptr++;
		if (*ptr == ',') {
			while(isspace(*++ptr));
			if (*ptr != '\0') goto again;
			if (fgets(buf, 510, fp) != NULL) {
				for (ptr = buf; isspace(*ptr); ptr++);
				if (*ptr == '\0' ||  *ptr == '#') {
					while ((buf[0] == '#') && 
					    (tmp = fgets(buf, 510, fp)));
					if (tmp == NULL) {
						goto eof;
					}
					continue;
				}
				else {
					ptr = buf;
					goto again;
				}
			} else break;
		}
	}
eof: 	
	fclose(fp);
}
/* this routine calls "parse" for every file in a directory */
/* (PATH) which has a "." in its name (ie usa.ny) */
read_dir()
{
	DIR *dirp;
	struct direct *dp;

	dirp = opendir(PATH);
	if (dirp == NULL) {
		fprintf(stderr, "Could not open \"%s\"\n", PATH);
		exit(0);
	}
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
		if (!(strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")))
			continue;
		else {
			if (strchr(dp->d_name,'.') != NULL) {
				parse(dp->d_name);
			}
		}
	closedir(dirp);
}
/* this routine parses networks */
int
network(file, ptr)
FILE *file;
char *ptr;
{
	char sitebuf[16], dest[16], prio[64];
	char buf[510];
	char *ptr1;
	char *ptr2;
	int check;
	register int current;

net:
	check = 0;
	/* get site name */
	ptr1 = sitebuf;
	ptr2 = &sitebuf[15];
	while(*ptr != ' ' && *ptr != '=')
		if (ptr1 < ptr2) *ptr1++ = *ptr++;
		else ptr++;
	*ptr1 = '\0';
	current = lookup(sitebuf, 1);
	site[current].s_isnet = '1';

	/* let's start parsing the neighbors */
	ptr1 = strchr(ptr, '{');
	if (ptr1 == NULL) {
		fprintf(stderr, "There used to be a '{' in this string.\n");
		exit(0);
	}
nxt:   
	while (!isalpha(*++ptr1) && *ptr1 != '\n' && *ptr1 != '}');
	if (*ptr1 == '\n') {
		ptr1 = fgets(buf, 510, file);
		if (ptr1 == NULL) {
			fprintf(stderr,"EOF unexpected in routine \"network\"\n");
			exit(0);
		}
	}
	else if (*ptr1 == '}') {
loop:          
		ptr = strchr(ptr1, '(');
		if (check++ > 3) {
			fprintf(stderr, "Something is wrong - routine \"network\"\n");
			fprintf(stderr, "bye...\n");
			exit(0);
		}
		if (ptr == NULL) {
			ptr1 = fgets(buf, 510, file);
			if (ptr1 == NULL) {
				fprintf(stderr,"E0F unexpected in routine \"network\"\n");
				exit(0);
			}
			ptr = strchr(ptr1, '(');
			if (ptr == NULL) goto loop;
		}
		/* ok, lets find the frequency */
		ptr++;
		ptr1 = prio;
		while (*ptr != ')' || (*(ptr+1) == '/' || *(ptr+1) == '*'))
			*ptr1++ = *ptr++;
		*ptr1 = '\0';
		fix(current, yyparse1(prio));
nextn:
		ptr = fgets(buf, 510, file);
		if (ptr == NULL) return(-1);
		if (buf[0] != '#' && strchr(buf, '{'))  {
			goto net;
		}
		else if (buf[0] == '#' && (buf[1] == 'N' || buf[1] == 'S'))
			return;
		else goto nextn;
	}
	else {
		ptr = dest;
		while (*ptr1 != ',' && *ptr1 != ' ' && *ptr1 != '\n' &&
			 *ptr1 != '}' && *ptr1 != '"')
			*ptr++ = *ptr1++;
		*ptr = '\0';
		add_neigh(current, dest, TMPVAL);
		add_neigh(lookup(dest, 1), sitebuf, NETVALS);
		ptr1--;
	}
	goto nxt;
	/* NOTREACHED */
}
/* this routines changes the cost between 2 sites */
fix(sitenum, newpri)
int sitenum, newpri;
{
	struct nptr *nptr;

	nptr = site[sitenum].s_un.s_next;
	for(;nptr->n_next != NULL; nptr = nptr->n_next)
		if (nptr->n_freq == yyparse1(TMPVAL)) nptr->n_freq = newpri;
}
