/*
 *	Rename a file so's it will be unique under systems without
 *	extended filename size.
 *	The new name is written on the stardard output.
 *
 *	Example:
 *		To copy lots of files from a 4.?BSD system to a
 *		SystemV, SystemIII, 2.?BSD or V7 system, without
 *		having to worry about overlapping filenames
 *		(eg, so "what_a_long_filename.c" and "what_a_long_filename.h"
 *		don't get squished), just do
 *			for i in *
 *			do
 *				j=`shrink -m $i`
 *				uucp $j othersys!/stuff/$j  (or rcp, hhcp, ...)
 *			done
 *
 *	Bug:	It gets overenthusiastic and deletes characters
 *	    	unnecessarily if the filename begins with a "."
 *
 *	Author:
 *		Simon Brown
 *		Department of Computer Science, University of Edinburgh.
 */

#include <stdio.h>

/*
 *	maximum no. of "segments" allowed in a file 
 */
#define MAXSEGS 10

/*
 *	silly names for strchr/strrchr under non-usg unices
 */
#ifndef sysV
#define strchr	index
#define strrchr	rindex
#endif sysV

char *getsegment();
char *strchr(), *strrchr();

struct segment {
	char seg_string[63];
	char seg_sep;
} seg[MAXSEGS];

char defaults[] = "._+-,@~=";
char usagemsg[] = "Usage: %s [-<maxlength>] [-mnSv] [-s<separators>] filename ...\n";

int max = 14;			/* target length */
int nocheck = 0;		/* don't check for existance (-n flag) */
int chatty = 0;			/* talk a lot (-v flag) */
int silent = 0;			/* don't say anything (-s flag) */
int move = 0;			/* rename file (-m flag) */
char *file, *prefix;
int modified = 0;
char realname[512];
char *separators = defaults;

main(argc,argv)
char **argv;
{
	register int i, j;
	int nsegs, suflen, remainder, delete;
	int total;
	int zap = 0;
	register char *op;
	char *progname = argv[0];

	if (argv[1] == (char *)0){
		fprintf(stderr, usagemsg, progname);
		exit(1);
	}
	while (*++argv)		/* process command-line options... */
		if (**argv=='-')
			while (*++*argv)
				switch(**argv){
				   case '0': case '1': case '2':
				   case '3': case '4': case '5':
				   case '6': case '7': case '8':
				   case '9':	/* change target length */
					max = atoi(*argv);
					if (max<=0){
						fprintf(stderr,"%s: illegal length %s\n", argv[0], argv[1]);
						exit(1);
					}
					while (*++*argv);
					--*argv;
					break;
				   case 's':	/* change separator definition */
					separators = ++*argv;
					while (*++*argv);
					--*argv;
					break;
				   case 'n':	/* don't create a uniquely named file */
					nocheck=1;
					break;
				   case 'v':	/* talk a lot */
					chatty=1;
					break;
				   case 'm': 	/* move files */
					move=1;
					break;
				   case'S':	/* silent */
					silent=1;
					break;
				   default:
					fprintf(stderr,usagemsg,progname);
					exit(1);
				}
		else break;
	while (*argv){
		strcpy(realname, *argv);
		if (prefix=strrchr(*argv,'/')){		/* dir/.../file */
			file=prefix+1;
			*prefix = '\0';
			prefix = *argv;
		} else file = *argv;
		argv++;
		op=file;
		modified=0;
		/* split into logical parts */
		for (i=0; i<MAXSEGS && (op=getsegment(op,i)); i++);
		if (i==MAXSEGS){
			fprintf(stderr,"%s: too many segments\n", file);
			continue;
		}
		nsegs = i-1;
		if ((nsegs*2) >= max){
			fprintf(stderr,"%s: too many segments\n", file);
			continue;
		}
		suflen = strlen(seg[nsegs].seg_string);
		if (suflen>max-nsegs-1){
			if (chatty)
				fprintf(stderr,"warning: %s: suffix truncated\n", file);
			suflen = max-(2*nsegs)-1;
			seg[nsegs].seg_string[suflen-1] = '\0';
			modified=1;
		} else if (suflen+nsegs > max-nsegs-1){
			zap = nsegs - (max-suflen)/2;
			if (chatty && zap)
				fprintf(stderr,"warning: %s: %d segments removed\n", file, zap);
		}
		if (nsegs>=1){		/* complicated filename */
			for (i=zap+1; i<=nsegs; i++)
				if (seg[i].seg_sep) suflen++;
			if (seg[0].seg_sep) suflen++;
			remainder = max - suflen;
			delete = remainder/nsegs;
			total = suflen;
			for (i=zap+1; i<nsegs; i++){
				if (strlen(seg[i].seg_string) > delete)
					modified=1;
				seg[i].seg_string[delete] = '\0';
				total += delete;
			}
			if (strlen(seg[0].seg_string) > max-total)
				modified=1;
			seg[0].seg_string[max-total] = '\0';
		}
		unique(zap,nsegs,modified);
	}
}

/*
 *	get a segment from a filename.
 *	returns pointer to remainder of filename, or 0 at end of filename
 */
char *
getsegment(cp,i)
char *cp;
{
	register char *xp = seg[i].seg_string;
	while (*cp && strchr(separators,*cp)==0)
		*xp++ = *cp++;
	*xp = '\0';
	seg[i].seg_sep = *cp;
	if (*cp) cp++;
	if (seg[i].seg_string[0] || seg[i].seg_sep) return(cp);
	else return((char *)0);
}

	
/*
 *	print out the current filename in a unique form
 */
unique(zap,n,mod)
{
	register int i, j;
	int jmax[MAXSEGS];
	int xstart, xstop;

	if (ok(zap,zap||mod)) return;
	xstart = (n==0)? 0 : 1;
	xstop = (n==0)? 1 : n;
    more:
	if (xstop <= xstart){
		xstart=0;
		xstop = n+1;
	}
	for (i=xstart; i<xstop; i++)
		jmax[i] = strlen(seg[i].seg_string);
	for (j=0; j<5; j++){
		for (i=xstart; i<xstop; i++){
			if (j >= jmax[i]) continue;
			increment(seg[i].seg_string,j);
			if (ok(zap,1)) return;
		}
	}
	if (xstart>0 || xstop<n+1){	/* Desparation time: try _any_ segment! */
		xstop = -1;
		goto more;
	}
	fprintf(stderr,"Can't generate unique name for %s\n", file);
}

/*
 *	modify a character in a string (stupid algorithm)
 */
increment(str,ind)
char *str;
{
	char ch = str[ind];

	if (ch>='a' && ch<='z') ch += ('A'-'a');
	else if (ch>='A' && ch<='Z') ch += ('a'-'A');
	str[ind] = ch;
}


/*
 *	see if a filename is reasonable, and if so print it out
 *	and return 1.
 *	otherwise return 0.
 */
ok(zap,changed)
{
	char buffer[512];
	register char *cp = buffer;
	register char *xp;
	register int i;

	if (prefix){
		for (xp=prefix; *xp; )
			*cp++ = *xp++;
		*cp++ = '/';
	}
	for (xp=seg[0].seg_string; *xp; )	/* copy initial segment */
		*cp++ = *xp++;
	if (seg[0].seg_sep){
		*cp++ = seg[0].seg_sep;
		for (i=zap+1; ; i++){		/* copy rest of segments */
			for (xp=seg[i].seg_string; *xp; )
				*cp++ = *xp++;
			if (seg[i].seg_sep) *cp++ = seg[i].seg_sep;
			else if (seg[i].seg_string[0] == '\0') break;
		}
	}
	*cp = '\0';
	if (changed && nocheck==0 && access(buffer,0)==0){
		if (chatty)
			fprintf(stderr,"warning: %s already exists - trying again\n",buffer);
		return(0);
	} else {
		if (silent==0) printf("%s\n", buffer);
		if (move && strcmp(realname,buffer)){
			if (link(realname,buffer)==-1 || unlink(realname)==-1)
				fprintf(stderr,"%s: link/unlink failure\n", realname);
		}
		return(1);
	}
}


