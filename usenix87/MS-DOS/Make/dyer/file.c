#include <stdio.h>
#include "make.h"


/*
 * Return file-node for 'fname'.
 * If it doesn't exist, then create one.
 */
FILENODE *filenode(fname)
char *fname;
{
	FILENODE *f, *afnode(), *gfile();

	if((f = gfile(fname)) == NULL)
		f = afnode(fname);
	return f;
}


/*
 * Add a dependency to the node 'fnd'.
 * 'fnd' will depend on 'fname'.
 */
addfile(fnd, fname)
FILENODE *fnd;
char *fname;
{
	NODE *n;
	FILENODE *f;

	if(fnd == NULL)			/* punt if no root file */
	{
		fprintf(stderr, "No current root, can't add dependency '%s'\n", 
fname);
		return;
	}

	f = filenode(fname);
	if((n = (NODE *)malloc(sizeof(NODE))) == NULL) allerr();
	n->nnext = fnd->fnode;
	fnd->fnode = n;
	n->nfile = f;
}


/*
 * Add a line of method-text to the node 'fnode'.
 */
addmeth(fnode, methtext)
FILENODE *fnode;
char *methtext;
{
	int len;
	char *new;

	if(fnode == NULL || methtext == NULL) return;

	len = strlen(methtext) + 2;
	if(fnode->fmake == NULL)
	{
		if((fnode->fmake = (char *)malloc(1)) == NULL) allerr();
		*(fnode->fmake) = 0;
	}
	len += strlen(fnode->fmake);

/* Lattice C doesn't have 'realloc()', so this kludges around it: */
	if((new = (char *)malloc(len)) == NULL) allerr();
	strcpy(new, fnode->fmake);
	free(fnode->fmake);
	fnode->fmake = new;

	strcat(fnode->fmake, methtext);
	len = strlen(fnode->fmake);
	if(len && fnode->fmake[len - 1] != '\n')
		strcat(fnode->fmake, "\n");
}


/*
 * Get a filenode for the file called 'fn'.
 * Returns NULL if the node doesn't exist.
 */
FILENODE *gfile(fn)
char *fn;
{
	FILENODE *f;

	for(f = froot; f != NULL; f = f->fnext)
		if(!strcmp(fn, f->fname)) return f;
	return NULL;
}


/*
 * Alloc space for a new file node.
 */
FILENODE *afnode(name)
char *name;
{
	FILENODE *f;

	for(f=froot; f; f=f->fnext)
		if(!strcmp(name, f->fname)) return f;

	if((f = (FILENODE *)malloc(sizeof(FILENODE))) == NULL) allerr();
	if((f->fname = (char *)malloc(strlen(name)+1)) == NULL) allerr();
	strcpy(f->fname, name);
	f->fmake = NULL;
	f->fnode = NULL;
	f->fdate = NULL;
	f->fflag = 0;

	f->fnext = froot;
	froot = f;
	return f;
}


/*
 * Print dependency tree.
 */
prtree()
{
	FILENODE *f;
	NODE *n;

	for(f = froot; f != NULL; f = f->fnext)
	{
		printf("%s%s%s (%u, %u)\n",
			f->fname,
			(f->fflag & ROOTP) ? " (root)" : "",
			(f->fflag & REBUILT) ? " (rebuilt)" : "",
			(f->fdate != NULL) ? (f->fdate)->ds_high : 0,
			(f->fdate != NULL) ? (f->fdate)->ds_low : 0);
		if(f->fmake != NULL)
			printf("%s", f->fmake);
		for(n = f->fnode; n != NULL; n = n->nnext)
			printf("\t%s\n", (n->nfile)->fname);
		puts("");
	}
}
