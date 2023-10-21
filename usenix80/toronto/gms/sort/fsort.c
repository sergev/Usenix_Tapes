/*
 * This file contains the source for the file sorter (see FSORT(X)).
 * compile: cc -c -v -O fsort.c
 */

/*
 * Include files
 */

#include <path.h>	/* defines maximum length of system pathname */
#include <stat.h>	/* structure for stat and fstat system calls */
#include <stdio.h>	/* structures and defines for standard i/o library */
#include <struct.h>	/* standard PDP-11 structures */

/*
 * Parameters used by routines (should NOT be changed)
 */

#define	OK	0	/* non-error status return */
#define	ERR	(-1)	/* error status return */
#define EVER	;;	/* used in infinite for loops */

/*
 * Parameters for tuning (may be changed)
 */

#define	MRGMAX	5	/* maximum number of files to merge at once */
/* NOTE: the maximum number of files open at once is MRGMAX + 2 */

/*
 * Externals
 */

/* buffers to be used by the i/o library (sortbuf uses the remaining core) */
static	char	inbuf[BUFSIZ];	/* buffer for the input file */
static	char	outbuf[BUFSIZ];	/* buffer for the current output file */
static	char	mrgbuf[MRGMAX][BUFSIZ];	/* buffers for the merge files */
/* global variables in which the arguments to fsort are stored */
static	char	*file;		/* pointer to name of file to be sorted */
static	(*compar)();		/* pointer to the sort comparison routine */
static	unsigned width;		/* width of an element to sort */
/* i/o pointers for various input and output files */
static	FILE	*input;		/* i/o pointer for the input file */
static	FILE	*output;	/* i/o pointer for the current output file */
static	FILE	*mrgput[MRGMAX];/* i/o pointers for the merge files */
/* global variables associated with the sort buffer */
static	char	*sortbuf;	/* pointer to the sort buffer */
static	unsigned bufsize;	/* size of the sort buffer */
/* global variables associated with temporary files */
static	char	tmpnam[PATHLEN];/* holds temporary file name */
static	char	*gettmp();	/* returns character pointer */
static	numtmp;			/* number of temporary files */
/* global variables intended to be visible outside of fsort */
char	*fsortdir = {		/* pointer to directory name in which */
	"/tmp"			/* to create temporary files */
};

/*
 * Functions
 */

/*
 * Fsort sorts the file passed as its argument according to the 
 * comparison routine.  Each element is assumed to be width bytes wide.
 */

fsort(fil,wid,cmp)
char	*fil;
int	(*cmp)();
unsigned wid; {
	register i;

	file = fil;
	width = wid;
	compar = cmp;
/* set up the temporary file names */
	numtmp = 0;
	if(settmp() == ERR) {
		return(ERR);
	}
/* set up the sort buffer */
	if(getcore() == ERR) {
		cleanup();
		return(ERR);
	}
/* open the sort file for reading */
	if(infile() == ERR) {
		cleanup();
		return(ERR);
	}
/* sort the sort file */
	if(sort() == ERR) {
		cleanup();
		return(ERR);
	}
/* if there were no temporary files used, then return OK */
	if(numtmp == 0) {
		cleanup();
		return(OK);
	}
/* otherwise merge the temporary files */
	for(i = 0;(i + MRGMAX) < numtmp;i += MRGMAX) {
		if(tmpfile() == ERR) {
			cleanup();
			return(ERR);
		}
		if(merge(i,i + MRGMAX) == ERR) {
			cleanup();
			return(ERR);
		}
	}
/* do the final merge into the sort file */
	if(outfile() == ERR) {
		cleanup();
		return(ERR);
	}
	if(merge(i,numtmp) == ERR) {
		cleanup();
		return(ERR);
	}
/* successful completion of merge, return OK */
	cleanup();
	return(OK);
}

/*
 * Sort does the actual sorting of the sort file.
 */

static	sort() {
	register char	*p;
	register unsigned diff;

	for(EVER) {
/* fill up the sort buffer from the sort file */
		for(p = sortbuf;(p + width) <= &sortbuf[bufsize];p += width) {
			if(fread(p,width,1,input) != 1) {
				break;
			}
		}
/* if an error occurred while reading, return ERR */
		if(ferror(input)) {
			return(ERR);
		}
/* if there is no more, then close the input and return OK */
		if(p == sortbuf) {
			fclose(input);
			return(OK);
		}
/* sort the sort buffer using QSORT(III) */
		diff = p - sortbuf;
		qsort(sortbuf,diff / width,width,compar);
/* if there is only one file, write directly to final output (no merge) */
		if((numtmp == 0) && feof(input)) {
			if(outfile() == ERR) {
				return(ERR);
			}
		}
/* otherwise open a new temporary file for output */
		else {
			if(tmpfile() == ERR) {
				return(ERR);
			}
		}
/* write out the sorted buffer */
		if(fwrite(sortbuf,width,diff / width,output) != (diff / width)) {
			return(ERR);
		}
/* close the output file */
		fclose(output);
	}
}

/*
 * Merge merges a series of temporary files together.
 */

static	merge(low,high) {
	register char	c;
	register char	*ap,*bp;
	int	i,j,k,first;
	FILE	*iop;

	for(i = 0;i < (high - low);i++) {
/* open a temporary file for reading */
		mrgput[i] = fopen(gettmp(i + low),"r");
		if(mrgput[i] == NULL) {
			return(ERR);
		}
/* set the buffer */
		setbuf(mrgput[i],mrgbuf[i]);
/* read the first record into the merge buffer */
		if(fread(&sortbuf[i * width],width,1,mrgput[i]) != 1) {
/* the first read should never fail, so return ERR if it does */
			return(ERR);
		}
/* now sort it into place */
		for(j = i;((j - 1) >= 0) && ((*compar)(&sortbuf[j * width],&sortbuf[(j - 1) * width]) < 0);j--) {
			ap = &sortbuf[j * width];
			bp = &sortbuf[(j - 1) * width];
			for(k = width;k;k--) {
				c = *ap;
				*ap++ = *bp;
				*bp++ = c;
			}
			iop = mrgput[j];
			mrgput[j] = mrgput[j - 1];
			mrgput[j - 1] = iop;
		}
	}
/* merge until all the temporary files are exhausted */
	first = 0;
	for(EVER) {
/* find the first available line for output */
		for(i = first;i < (high - low);i++) {
			if(mrgput[i] != NULL)  {
				break;
			}
/* skip over this record next time */
			first++;
		}
/* the input files are exhausted so return OK */
		if(i == (high - low)) {
/* truncate all the input files */
			for(i = low;i < high;i++) {
				fclose(fopen(gettmp(i),"w"));
			}
/* close the output file */
			fclose(output);
			return(OK);
		}
/* write out the record just found */
		if(fwrite(&sortbuf[i * width],width,1,output) != 1) {
			return(ERR);
		}
/* replace the record from the input */
		if(fread(&sortbuf[i * width],width,1,mrgput[i]) != 1) {
/* if there was an error reading, return ERR */
			if(ferror(mrgput[i])) {
				return(ERR);
			}
/* otherwise, it was the end of file */
			fclose(mrgput[i]);
			mrgput[i] = NULL;
/* don't sort this record into place */
			continue;
		}
/* now sort the new record into place */
		for(j = i;((j + 1) < (high - low)) && ((*compar)(&sortbuf[j * width],&sortbuf[(j + 1) * width]) > 0);j++) {
			ap = &sortbuf[j * width];
			bp = &sortbuf[(j + 1) * width];
			for(k = width;k;k--) {
				c = *ap;
				*ap++ = *bp;
				*bp++ = c;
			}
			iop = mrgput[j];
			mrgput[j] = mrgput[j + 1];
			mrgput[j + 1] = iop;
		}
	}
}

/*
 * Settmp sets up the prototype temporary file name.
 */

static	settmp() {
	register char	*p;
	register FILE	*tp;
	struct	stat	statbuf;

/* copy the directory path */
	strcpy(tmpnam,fsortdir);
/* append the temporary file prototype name */
	p = tmpnam;
	while(*p++);
	strcpy(--p,"/stm@aa");
/* now find a valid name (change @ to a letter) */
	while(*p++);
	while(*--p != '@');
	for(*p = 'a';*p <= 'z';(*p)++) {
		if(stat(tmpnam,&statbuf) == OK) {
			continue;
		}
		tp = fopen(tmpnam,"w");
		if(tp != NULL) {
			break;
		}
	}
/* the temporary file could not be created so return ERR */
	if(*p > 'z') {
		return(ERR);
	}
/* otherwise, close the file and return OK */
	fclose(tp);
	return(OK);
}

/*
 * Gettmp returns a pointer to the temporary file specified by number.
 */

static	char	*gettmp(number)
register number; {
	register char	*p;

	p = tmpnam;
	while(*p++);
	p--;
	*--p = (number % 26) + 'a';
	*--p = (number / 26) + 'a';
	return(tmpnam);
}

/*
 * Getcore grabs core from the operating system to use as the sort buffer.
 */

static	getcore() {
	register unsigned coretop;

/* set sortbuf to point to the base of the data */
	sortbuf = sbrk(0);
/* now grab some core */
	for(coretop = 0160000;coretop >= sortbuf;coretop -= 0020000) {
		if(brk(coretop) != ERR) {
			break;
		}
	}
/* if there was no core available, return ERR */
	if(coretop < sortbuf) {
		return(ERR);
	}
/* if there is not enogh room to do the merge, return ERR */
	bufsize = coretop - sortbuf;
	if(bufsize < (MRGMAX * width)) {
		return(ERR);
	}
/* otherwise return OK */
	return(OK);
}

/*
 * Infile sets up the sort file for input.
 */

static	infile() {
/* open the file for reading */
	input = fopen(file,"r");
/* if the open failed, return ERR */
	if(input == NULL) {
		return(ERR);
	}
/* otherwise set the buffer and return OK */
	setbuf(input,inbuf);
	return(OK);
}

/*
 * Outfile sets up the sort file for output.
 */

static	outfile() {
/* open the file for writing */
	output = fopen(file,"w");
/* if the open failed, return ERR */
	if(output == NULL) {
		return(ERR);
	}
/* otherwise set the buffer and return OK */
	setbuf(output,outbuf);
	return(OK);
}

/*
 * Tmpfile sets up a new temporary file for output.
 */

static	tmpfile() {
/* open the new file for writng */
	output = fopen(gettmp(numtmp),"w");
/* if the open failed, return ERR */
	if(output == NULL) {
		return(ERR);
	}
/* otherwise set the i/o buffer and return OK */
	setbuf(output,outbuf);
	numtmp++;
	return(OK);
}

/*
 * Cleanup closes all files, removes all temporary files, and frees all
 * allocated core.
 */

static	cleanup() {
	register i;
	register FILE	**fpp;

/* close all files which could possibly still be open */
	if(input != NULL) {
		fclose(input);
	}
	if(output != NULL) {
		fclose(output);
	}
	for(fpp = mrgput;fpp < &mrgput[MRGMAX];fpp++) {
		if(*fpp != NULL) {
			fclose(*fpp);
		}
	}
/* if there were no temporary files, one was still created (see gettmp) */
	if(numtmp == 0) {
		numtmp = 1;
	}
/* unlink all the temporary files */
	for(i = 0;i < numtmp;i++) {
		unlink(gettmp(i));
	}
/* now free up the core that was grabbed by getcore */
	brk(sortbuf);
}
