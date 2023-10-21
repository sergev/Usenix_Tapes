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

	if ((f = gfile(fname)) == (FILENODE *) NULL)
		f = afnode(fname);
	return f;
}


/*
 * Add a dependency to the node 'fnd'.
 * 'fnd' will depend on 'fname'.
 */
FILENODE *addfile(fnd, fname)
FILENODE *fnd;
char *fname;
{
	NODE *n;
	FILENODE *f;

    if (fnd == (FILENODE *) NULL) {		/* punt if no root file */
        fputs("No current root, can't add dependency '", stderr);
	fputs(fname, stderr);
	fputs("%s'\n", stderr);
	return (FILENODE *) NULL;
    }

    f = filenode(fname);
    if ((n = (NODE *)calloc(1, sizeof(NODE))) == (NODE *) NULL) allerr();
    n->nnext = fnd->fnode;
    fnd->fnode = n;
    n->nfile = f;
    return f;
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

	if (fnode == (FILENODE *) NULL || methtext == NULL) return;

	len = strlen(methtext) + 2;
	if (fnode->fmake == NULL) {
	    if ((fnode->fmake = (char *)calloc(1, 1)) == NULL) allerr();
	    *(fnode->fmake) = '\0';
	}
	len += strlen(fnode->fmake);

/* Lattice C doesn't have 'realloc()', so this kludges around it: */
	if ((new = (char *)calloc(1, len)) == NULL) allerr();
	strcpy(new, fnode->fmake);
	free(fnode->fmake);
	fnode->fmake = new;

	strcat(fnode->fmake, methtext);
	len = strlen(fnode->fmake);
	if (len && fnode->fmake[len - 1] != '\n')
		strcat(fnode->fmake, "\n");
}

/*
 * Add token to the parent list.  Return the pointer to the new parent.
 * If token is already on the parent list, simply return the pointer found.
 */
FILENODE *addtolist(token, list)
char *token;
NODE **list;
{
    NODE *search, *newnode;
    
    for (search = *list; search != (NODE *) NULL; search = search->nnext) {
        if (!strcmp (search->nfile->fname, token)) 
        return search->nfile;
    };
    /* token not found so far... add it to list */
    if ((newnode=(NODE *)calloc(1,sizeof(NODE))) == (NODE *)NULL) allerr();
    search = *list;
    *list = newnode;
    newnode->nnext = search;
    if ((newnode->nfile = (FILENODE *)calloc(1, sizeof(FILENODE))) == (FILENODE *) NULL) allerr();

    if ((newnode->nfile->fname = (char *)calloc(1, strlen(token)+1)) == NULL) allerr();
    strcpy(newnode->nfile->fname, token);
    newnode->nfile->parent = (FILENODE *) NULL;
    return newnode->nfile;
}

static NODE *parentlist = (NODE *) NULL;

FILENODE *addparent(token)
char *token;
{
    FILENODE *addtolist();

    return addtolist(token, &parentlist);
}

#ifdef FUNNYLIBS
isonlibrary(f)				/* return SUCCEED if f is a library. */
FILENODE *f;				/* set f->fdate to parent's date */
{
    if (f->fflag & LIBRARY) {
        getdate(f->parent);
        f->fdate = f->parent->fdate;
	return(SUCCEED);
    }
    return FAILURE;
}
#else
/*
 * Add file node fnd to library list.
 */
static FILENODE *librarylist = (FILENODE *) NULL;

addtolibrary(fnd)
FILENODE *fnd;
{
    NODE *n;

    if (librarylist == (FILENODE *) NULL)  {
        if ((librarylist = (FILENODE *) calloc(1, sizeof(FILENODE))) == (FILENODE *) NULL)
            allerr();
        librarylist->fnode = (NODE *) NULL;
    }
    if ((n = (NODE *)calloc(1, sizeof(NODE))) == (NODE *) NULL) allerr();
    n->nnext = librarylist->fnode;
    librarylist->fnode = n;
    n->nfile = fnd;
}

/*
 * Return SUCCEED if filenode f is a direct descendant of a library;
 * set f->fdate to parent's time.
 */

isonlibrary(f)
FILENODE *f;
{
    NODE *lib, *dep;

#if DEBUG
printf("Searching for: %s\n", f->fname);
#endif
    if (librarylist == (FILENODE *)  NULL) return FAILURE;
    for (lib = librarylist->fnode; lib != (NODE *) NULL; lib = lib->nnext) {
        for (dep = lib->nfile->fnode; dep != (NODE *) NULL; dep = dep->nnext) {
#if DEBUG
printf("Examining: %s\n", dep->nfile->fname);
#endif
            if (f == dep->nfile) {                /* found it!! */
#if DEBUG
printf("Found %s depend on %s\n", dep->nfile->fname, lib->nfile->fname);
#endif
                f->fdate = lib->nfile->fdate;        /* update time */
                return SUCCEED;
            }
        }
    }
    return FAILURE;
}
#endif

isanarchive(f)				/* return SUCCEED if f is an archive */
FILENODE *f;				/* set f->fdate to date in parent's */
{					/* archive directory */
	DATE getarchdate();

    if (f->fflag & ARCHIVE) {
        f->fdate = getarchdate(f->parent->fname, f->fname);
	return(SUCCEED);
    }
    return FAILURE;
}

NODE  *deletelist = (NODE *) NULL;

extract(f)
FILENODE *f;
{
	FILENODE *addtolist();
#if DEBUG
printf("Extracting %s for archivehood.\n", f->fname);
#endif
    if (f->fflag & ARCHIVE) {
#if DEBUG
printf("Copying %s for archivehood.\n", f->fname);
#endif
#ifndef NOREALEXTRACT
        copyfile(f->parent->fname, f->fname);
#endif
	(void) addtolist(f->fname, &deletelist);  /* delete f->fname at end of run */
	return(SUCCEED);
    }
    return FAILURE;
}

cleanuparchives()
{
    NODE *search;

#if DEBUG
printf("Inside cleanuparchives.\n");
#endif

    for (search = deletelist; search != (NODE *)NULL; search = search->nnext) {
        fputs("Purging ", stdout);
	puts(search->nfile->fname);
#ifndef NOREALDELETE
        unlink (search->nfile->fname);
#endif
    };
}


/*
 * Get a filenode for the file called 'fn'.
 * Returns (FILENODE *) NULL if the node doesn't exist.
 */
FILENODE *gfile(fn)
char *fn;
{
	FILENODE *f;

	for (f = froot; f != (FILENODE *) NULL; f = f->fnext)
		if (!strcmp(fn, f->fname)) return f;
	return (FILENODE *) NULL;
}


/*
 * Alloc space for a new file node.
 */
FILENODE *afnode(name)
char *name;
{
	FILENODE *f;

	for (f=froot; f; f=f->fnext)
		if (!strcmp(name, f->fname)) return f;

	if ((f = (FILENODE *)calloc(1, sizeof(FILENODE))) == (FILENODE *) NULL) allerr();

	if ((f->fname = (char *)calloc(1, strlen(name)+1)) == NULL) allerr();
	strcpy(f->fname, name);

	if ((f->fmake = (char *)calloc(1, 1)) == NULL) allerr();
	*(f->fmake) = '\0';

	f->fdate = (DATE) NULL;

	f->fnode = (NODE *) NULL;
	
	f->parent = (FILENODE *) NULL;

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

	for (f = froot; f != (FILENODE *) NULL; f = f->fnext)
	{
	    fputs(f->fname, stdout);
	    fputs((f->fflag & ROOTP) ? " (root)" : "", stdout);
	    fputs((f->fflag & REBUILT) ? " (rebuilt)" : "", stdout);
	    fputs((f->fflag & LIBRARY) ? " (library)" : "", stdout);
	    fputs((f->fflag & EXTRACT) ? " (extracted)" : "", stdout);
	    fputs((f->fflag & ARCHIVE) ? " (archive)" : "", stdout);
	    printdate(f->fdate);
	    if (f->parent != (FILENODE *) NULL) {
	        fputs("Parent is: ", stdout);
		fputs(f->parent->fname, stdout);
		fputc('\n', stdout);
		fputs("Parental Date:", stdout);
	        printdate(f->parent->fdate);
	    }
		if (f->fmake != NULL)
		    puts(f->fmake);
		puts("Dependents: ");
		for (n = f->fnode; n != (NODE *) NULL; n = n->nnext) {
		    fputc('\t', stdout);
		    puts((n->nfile)->fname);
		}
		fputc('\n', stdout);
	}
}
