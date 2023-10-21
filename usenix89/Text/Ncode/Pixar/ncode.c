/*
 *	ncode.c --  a random text constructor
 *	pixar!good
 *	based on a program by pixar!mark
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DATALUMP	8192	/* increment size of data[] by this as needed */
#define BIGBUF		4096	/* nice, roomy buffer for group name checks */

char **data;	/* array of pointers to elements in bucket */
int dindex;	/* number of elements in data */
int datasize;	/* how many elements would fit in data[] is right now */

char *malloc();
char *realloc();
char *rindex();
char *fgets();
long random();

struct gstruct {
	char *name;	/* points to name of a group in data[] */
	int count;	/* how many elements of data belong to this group */
	int index;	/* index of element of data where group starts */
};
struct gstruct *groups; /* where the group structs live, or is that obvious? */
int ngroups;		/* number of elements in groups */

main (ac,av)
int ac;
char *av[];
{
	char *prog;		/* name of this program */
	char *fname = 0;
	int loopcnt = 1;	/* times through main loop */
 	char *groupname = "CODE";

	prog = rindex(*av,'/');
	prog = ( prog == NULL ) ? *av : ++prog ;
	ac--;av++;
	while(ac && **av == '-'){
		if (strcmp(*av,"-n") == 0){
			ac--;av++;
			loopcnt = atoi(*av);
			if (loopcnt <= 0){
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

	/*
	 *	Make some room to start, and increment by DATALUMP as needed
	 */
	datasize = DATALUMP;
	if ((data = (char **) malloc(datasize * sizeof(char *))) == NULL ){
		perror("main: could not malloc for data[]");
		exit(1);
	}
	dindex = 0;
	if( init(fname) != 0 ){	
		fprintf(stderr,"%s: init error\n",prog);
		exit(1);
	}
	/*
	 * This should be more than enough room for worst-case
	 */
	groups = (struct gstruct *) malloc(dindex * sizeof(struct gstruct));
	if (groups == NULL){
		perror("main: could not malloc for groups[]");
		exit(1);
	}
	if ( scan() != 0 ){
		fprintf(stderr,"%s: scan error\n",prog);
		exit(1);
	}
	srandom(a_random_number());	/* seed the number generator */
	while ( loopcnt ){
		expand(groupname,strlen(groupname));
		loopcnt--;
	}
	exit(0);
}

init(fname)
char *fname;
{
	char *bucket;	/* big array where data lives */
	char *bptr;	/* points into bucket */
	int fd;
	struct stat sbuf;
	char *s, *t;

	fd = open(fname,O_RDONLY,0);
	if ( fd < 0 ) {
		perror(fname);
		return 1;
	}
	if ( fstat(fd,&sbuf) != 0 ){
		perror(fname);
		return 1;
	}
	if ((bucket = (char *) malloc( sbuf.st_size + 1)) == NULL ){
		perror("init(): malloc() trouble");
		return 1;
	}
	/*
	 *	Read entire file into bucket[]
	 */
	if ((read(fd,bucket,sbuf.st_size)) != sbuf.st_size){
		perror("init: read error");
		return 1;
	}
	close(fd);
	/*
	 * Make first pass through memory, pointing data[] the right way
	 * and recursing as needed on #include files.
	 */
	bptr = bucket;
	while ( *bptr && (bptr <= bucket + sbuf.st_size) ){
		s = bptr; 
		while ( *s != '\0' ){
			if(*s == '\n' )
				*s = '\0';	/* nuke newline */
			else
				s++;
		}
		if( strncmp(bptr, "#include", 8) == 0 ){
			for(t = bptr + 8; *t!='\0' && (*t==' '||*t=='\t');t++)
				;	/* skipping white space */
			if (init(t) != 0){	/* RECURSES HERE */
				return 1;
			}
			bptr = t +strlen(t) + 1; /* skip the #include line */
			continue;	/* back to the top of the while loop */
		}
		/*
		 *	Make sure data[] is still big enough
		 */
		if ( dindex >= datasize){
			datasize += DATALUMP;
			if((data=(char **) realloc(data,
					datasize*sizeof(char *)))==NULL){
				perror("init: could not realloc for data[]");
				return(1);
			}
		}
		data[dindex] = bptr;	/* point it at the data */
		bptr = s + 1;	/* move bptr to the next location to fill */
		dindex++;
	}
	return 0;
}

/*
 *	Scan data[] marking and counting groups
 */
scan()
{
	register i, gcnt, gindex;

	/*
	 * special case: first line always a group name 
	 */
	groups[0].name = data[0];
	groups[0].index = 0;
	ngroups = 1;
	i = 1;
	gindex = 0;
	gcnt = 0;
	while ( i < dindex ){
		if ( data[i][0] == '%' ){
			groups[gindex].count = gcnt;
			gcnt = 0;		/* close out prev group */
			ngroups++;
			i++;			/* start next group */
			/*
			 *	If a #included file has any blank lines after
			 *	the last '%' then the group name would wind
			 *	up being '\0'.  So the first group name after
			 *	the #include won't be marked as a group name
			 *	and will thus never be expanded.  We could
			 *	cluck our tongues at the user and say he has
			 *	a bogus file and thus deserves what he gets.
			 *	But hopefully this check will just make the
			 *	program more robust.
			 */
			while ((i < dindex) && (data[i][0] == '\0')){
					i++;
			}
			gindex++;
			groups[gindex].name = data[i];
			groups[gindex].index = i;
		}else{
			gcnt++;
		}
		i++;
	}
	ngroups--;	/* The last % in the file doesn't start a new group */
	return 0;
}

/*
 *	This is where we finally do the deed.  If a string is a group name
 *	then expand() will randomly select a member of that group to
 *	replace it.  Through the miracle of recursion, a whole sentence
 *	may be passed to expand() and each word (anything bounded by what
 *	we call "white space" gets expanded.  Anything that cannot be
 *	expanded gets printed out.
 */
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
 *	Return index into groups[] array if a group name, -1 if just a word.
 *	We have to use gbuf, a seperate place, so that we can null-terminate
 *	the string where we want.  Otherwise it wouldn't know santa from
 *	santana.
 */
isagroup(s,lim)
char s[];
int lim;
{
	register i;
	static char gbuf[BIGBUF];

	strncpy(gbuf,s,lim);
	gbuf[lim] = '\0';	/* strncpy might not do this */
	for(i=0; i<ngroups; i++ ){
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
	if (limit > 0){
		return (random() % limit);
	}
	return 0;	/* better than a floating exception if lim == 0 */
}

a_random_number()
{
	struct timeval tp;
	struct timezone tzp;

	gettimeofday (&tp, &tzp);

	return((getpid() ^ tp.tv_usec) % 123456);
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
		c == ' '  ||
		c == '\t' ||
		c == '|'  ||		/* "splice" character */
		c == '\\' ||		/* becomes a newline */
		c == '.'  ||		/* common punctuation */
		c == '-'  ||
		c == ':'  ||
		c == ';'  ||
		c == ','  ||
		c == '!'  ||
		c == '?'  ||
		c == '['  ||
		c == ']'  ||
		c == '{'  ||
		c == '}'  ||
		c == '('  ||
		c == ')'  ||
		c == '\'' ||
		c == '\"' ||
		c == '`'
	)
		return 1;
	return 0;
}
