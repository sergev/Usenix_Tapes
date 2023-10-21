/*
 *	ncode.c --  a random text constructor
 *	pixar!good
 *	based on a story by pixar!mark
 */

#include <stdio.h>

#define DATALEN		4096	/* max number of lines allowed in input file */
#define BUFSIZ		2048	/* max number of chars allowed in a line */

char *data[DATALEN]; /* array of pointers to strings from file, one line each */

char *malloc();
char *rindex();
long random();

struct gstruct {
	char *name;	/* points to name of a group in data[] */
	int count;	/* how many elements of data[] belong to this group */
	int index;	/* index of element of data[] where group starts */
};
struct gstruct groups[DATALEN];
int ngroups;			/* number of elements in groups[] */
int findex;			/* number of elements loaded into data[] */

main (ac,av)
int ac;
char *av[];
{
	char *prog, *fname = 0;
	int howmany = 1;	/* times through main loop */
 	char *groupname = "CODE";

	prog = rindex(*av,'/');			/* name of program */
	prog = ( prog == NULL ) ? *av : ++prog ;
	ac--;av++;
	while(ac && **av == '-'){
		if (strcmp(*av,"-n") == 0){
			ac--;av++;
			howmany = atoi(*av);
			if (howmany <= 0){
				fprintf(stderr,
					"%s: -n: need positive integer\n",prog);
				exit(1);
			}
			ac--;av++;
		} else if (strcmp(*av,"-g") == 0){
			ac--;av++;
			groupname = *av;	/* use instead of "CODE" */
			if (! groupname ){
				fprintf(stderr,
					"%s: -g: need group name\n",prog);
				exit(1);
			}
			ac--;av++;
		} else {
			printf(
			"Usage %s [-n n ] [-g groupname] codefile\n",prog);
			exit(0);
		}
	}
	if (!ac){
		fprintf(stderr,
			"Usage %s [-n n ] [-g groupname] codefile\n",prog);
		exit(1);
	}
	fname = *av;

	findex = 0;
	if( init(fname) != 0 ){
		fprintf(stderr,"%s: init error\n",prog);
		exit(1);
	}
	if ( scan() != 0 ){
		fprintf(stderr,"%s: scan error\n",prog);
		exit(1);
	}
	srandom(getpid());	/* seed the number generator */
	while ( howmany ){
		expand(groupname,strlen(groupname));
		howmany--;
	}
}

init(fname)
char *fname;
{
	FILE *fp;
	char buf[BUFSIZ];
	char *s;

	/*
	 *	Read whole file into data[]
	 */
	fp = fopen(fname,"r");
	if ( fp == NULL ) {
		perror(fname);
		return 1;
	}
	while ( fgets(buf,BUFSIZ,fp) != NULL ){
		if ( findex > DATALEN ){
			fprintf(stderr,"init: findex reached %d\n",findex);
			return 1; /* no point in going on */
		}
		for(s=buf;*s!='\0';s++)		/* nuke newline */
			if(*s=='\n')
				 *s='\0';
		if( buf[0] == '#' && (strncmp(buf,"#include",8)==0) ){
			for(s= &buf[8]; *s!='\0' && (*s==' '||*s=='\t');s++)
				;	/* skipping white space */
			if (init(s) != 0){ /* expects a path name */
				return 1;
			}
			continue;	/* don't put in the #include line! */
		}
		data[findex]=(char *)malloc(strlen(buf)+1);
		if (data[findex] != NULL)
			strcpy(data[findex], buf);
		else {
			printf("init: bad malloc\n");
			return 1;	/* something went wrong with malloc */
		}
		findex++;
	}
	fclose(fp);
	return 0;
}

/*
 *	Scan data[] marking and counting groups
 */
scan()
{
	register i, gcnt, gindex;

	/* special case: first line always a group name */
	groups[0].name = data[0];
	groups[0].index = 0;
	ngroups = 1;
	i = 1;
	gindex = 0;
	gcnt = 0;
	while ( i < findex ){
		if ( data[i][0] == '%' ){
			groups[gindex].count = gcnt;
			gcnt = 0;		/* close out prev group */
			ngroups++;
			i++;		/* start next group */
			gindex++;
			groups[gindex].name = data[i];
			groups[gindex].index = i;
		}else{
			gcnt++;
		}
		i++;
	}
	return 0;
}

expand(s,lim)
char s[];
int lim;
{
	register i, j, k, done, n, r;

	i = j = 0;
	while ( s[i] != 0 && i < lim ){
		done = 0;
		while ( ! done && j <= lim ){
			if ( isawhite(s[j]) ){
				/* chase down remaining white space */
				for (k=j; k<=lim && s[k] && isawhite(s[k]);k++){
					;
				}
				n = isagroup(&s[i], j-i);
				if ( n >= 0 ){
					r = (groups[n].index + 1
						+ rnd(groups[n].count));
					expand( data[r], strlen(data[r]));
					outstring(&s[j], k-j);
				} else {
					outstring(&s[i], k-i);
				}
				done++;
				i = j = k; /* should be on next word, if any */
			}
			j++;
		}
	}


}

/*
 *	Return index into groups[] array if a group name, -1 if just a word
 */
isagroup(s,lim)
char s[];
int lim;
{
	register i;
	static char gbuf[BUFSIZ];

	strncpy(gbuf,s,lim);
	gbuf[lim] = '\0';	/* strncpy might not do this */
	for(i=0; i<=ngroups; i++ ){
		if (groups[i].name && strcmp(gbuf,groups[i].name) == 0){
			return i;	/* hit */
		}
	}
	return -1;	/* fail */
}

/*
 * 	Output string, handling splices
 */
outstring(s,lim)
char s[];
int lim;
{
	register i = 0;

	while ( s[i] != '\0' && i < lim ){
		switch (s[i]){
		case '|':
				break;	/* splice: no output */
		case '\\':
				putchar('\n');
				break;
		default:
				putchar(s[i]);
				break;
		}
		i++;
	}
}

/*
 *     Return random number 0 to limit
 */
rnd(limit)
int limit;
{
	int i;

	if (limit > 0){
		return (random() % limit);
	}
	return 0;	/* better than a floating exception if lim == 0 */
}

/*
 *	Return 1 if one of our "white" characters.  A white character is
 *	any character which can bound a group name, so punctuation marks
 *	are included.
 */
isawhite(c)
char c;
{
	if (	c == '\0' ||		/* traditional white space */
		c == ' ' ||
		c == '\t' ||
		c == '|' ||		/* "splice" character */
		c == '\\' ||		/* becomes a newline */
		c == '.' ||		/* common punctuation */
		c == '-' ||
		c == ':' ||
		c == ';' ||
		c == ',' ||
		c == '!' ||
		c == '?' ||
		c == '[' ||
		c == ']' ||
		c == '{' ||
		c == '}' ||
		c == '(' ||
		c == ')' ||
		c == '\'' ||
		c == '\"' ||
		c == '`'
	)
		return 1;
	return 0;
}
