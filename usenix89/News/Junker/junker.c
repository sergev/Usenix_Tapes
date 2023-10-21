/****************************/
/*      JAMIE'S JUNKER      */
/* "All the News that Fits" */
/*       Version 1.2        */
/*            --            */
/*     main program and     */
/*   associated functions   */
/****************************/

/***********************************************************
 *
 * Usage: junker <newsdir>
 *
 * Algorithm: reads standard input for newsgroup information.
 * Treats lines beginning with a hash (#) as comments.  For each
 * line of the form
 *      <newsgroup> <window> <byteshr>
 * - constructs newsgroup directory from <newsdir> and
 *   <newsgroup> (e.g. /usr/spool/news/net/rec/nude)
 * - does a scandir on that newsgroup, sorting by article number
 * - finds which article was the first to come in since
 *   <window> hours ago
 * - computes #bytes left to limit = window * byteshr
 * - for that article and all with article number higher than it,
 *   - approximates the article size limit as (#bytes left to
 *     limit) / (#articles left to consider)
 *   - if size of article is greater than size limit,
 *     - creates smaller-size version of the file with the middle
 *       sliced out, preserving as much of header as possible
 *     - unlinks old version of file
 *     - renames new file to name of old file
 *     - relinks any articles said to be linked to old version by
 *       the article's Xref line
 *     - returns new article size
 *   - subtracts article size from #bytes left to limit
 *
 * Output: echoes input and prints a short summary of what was done
 * to vulnerable newsgroups
 *
 * Author: Jamie Andrews, Dept. of Computer Science, University
 * of British Columbia
 *
 ***********************************************************/

#include<stdio.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/dir.h>
#include<sys/file.h>
#include<sys/time.h>
#include<sys/stat.h>

#ifdef debugging
#define debug(flag_b, pars) if (flag_b) printf pars ; else
#endif
#ifndef debugging
#define debug(flag_b, pars)
#endif

#define TRUE		 -1
#define FALSE		  0
#define MAX_LINE_LEN	512
#define MAX_HEADER_LINES 20

typedef struct direct *direct_p_t;

typedef int boolean;


char g_sitename_ca[MAX_LINE_LEN+1];	/* Name of site (from Path line) */
char g_newsdir_ca[MAXNAMLEN+1];		/* Runtime news directory name   */


#ifndef BSD

/*
 * the BSD routine index is called strchr on Bell standard systems
 */
#define index(s, c) strchr(s, c)

/*
 * scandir is found only on BSD systems
 * Public-domain scandir written by Barry Brachman, June 1986
 *
 * Scan the given directory and make a list of pointers to direct structures
 * for each (selected) directory entry
 * One pass is made through the directory since the number of entries may
 * change between passes
 * If s_fun is not NULL, it is the address of a function that is passed
 * a filename and returns non-zero if the filename is to be included in the
 * list
 * If c_fun is not NULL then the list is sorted and c_fun is used as the
 * comparison function for qsort()
 * Return the number of entries in the list or -1 if dirname is not a directory
 * or cannot be read or if memory cannot be allocated
 *
 * Algorithm for allocating pointers based on one written by
 * Chris Torek <chris@umcp-cs.uucp> appearing in net.sources.bugs May 1986
 */
scandir(dirname, names, s_fun, c_fun)
char *dirname;
struct direct *(**names);
int (*s_fun)(), (*c_fun)();
{
	register int n, nleft, nnames, size;
	register DIR *dirp;
	struct direct **base, **d, *dentry;

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);

	nnames = 10;
	base = (struct direct **) malloc((unsigned) (nnames * sizeof(*base)));
	if (base == NULL) {
		closedir(dirp);
		return(-1);
	}
	nleft = nnames;
	d = base;
	n = 0;

	while ((dentry = readdir(dirp)) != NULL) {
		if (s_fun != NULL && (s_fun)(dentry->d_name) == 0)
			continue;

		if (--nleft < 0) {
			nleft = nnames - 1;	/* we're about to use one */
			nnames <<= 1;		/* double number of pointers */
			base = (struct direct **) realloc((char *) base,
					(unsigned) (nnames * sizeof(*base)));
			if (base == NULL) {
				closedir(dirp);
				return(-1);
			}
			d = base + nleft + 1;
		}
		size = DIRSIZ(dentry);
		if ((*d = (struct direct *) malloc(size)) == NULL) {
			closedir(dirp);
			return(-1);
		}
		bcopy(dentry, *d++, size);
		n++;
	}
	closedir(dirp);

	if (c_fun != NULL)
		qsort(base, n, sizeof(struct direct *), c_fun);

	/*
	 * If there are many entries in a directory it is possible to
	 * allocate much more space than is necessary.  If you are willing to
	 * trade speed for space, define MAX_OVER_ALLOC to be the maximum
	 * amount of space (in bytes) that you are willing to waste and
	 * uncomment the following code
	 */

/*
	if ((nnames - n) * sizeof(*base)) > MAX_OVER_ALLOC)
		base = (struct direct **) realloc((char *) base,
				(unsigned) (n * sizeof(*base)));
*/

	*names = base;
	return(n);
}

alphasort(p, q)
register struct direct **p, **q;
{

	return(strcmp((*p)->d_name, (*q)->d_name));
}
#endif BSD


/*
 * Initializing of arguments, generic setup message
 */
init(argc, argv)
  int argc;
  char *argv[];
{
  long now_l;

  char *now_pca;

  if (argc != 2) {
    printf("Usage: junker newsdir\n");
    exit(1);
  }

  strcpy(g_newsdir_ca, argv[1]);

  now_l = time(0);
  now_pca = ctime(&now_l);
  printf("Junker run: %s", now_pca);
}

/*
 * Selection routine for scandir/qsort.  Returns 1 iff all characters
 * in filename pointed to by direct_p are digits.
 */
int
only_articles(direct_p)
  struct direct *direct_p;
{
  char *char_p;

  char_p = direct_p->d_name;
  while (*char_p && isdigit(*char_p))
      ++char_p;

  if (*char_p) /* stopped on nondigit */
    return 0;
  else /* all digits */
    return 1;
}


/*
 * Comparison routine for scandir/qsort.  Sorts article files
 * pointed to by directX_pp by article number.
 */
int
by_article_no(direct1_pp, direct2_pp)
  struct direct **direct1_pp, **direct2_pp;
{
  long artno1_i, artno2_i;

  sscanf((*direct1_pp)->d_name, "%ld", &artno1_i);
  sscanf((*direct2_pp)->d_name, "%ld", &artno2_i);
  if (artno1_i < artno2_i)
    return -1;
  else if (artno1_i > artno2_i)
    return 1;
  else
    return 0;
}


/*
 * Edits a string to replace all occurrences of '.' and ':' by '/';
 * therefore effectively converts a string of the form <newsgroup>
 * or <newsgroup>:<article_no> to the end of a directory or file name.
 */
cnv_to_fname(char_p)
  char *char_p;
{
  while (*char_p) {
    if (*char_p == '.' || *char_p == ':')
      *char_p = '/';
    char_p++;
  }
}


/*
 * Takes a string containing a 3-letter month name abbreviation and
 * returns a month number (0-11, like the time routines) based on it.
 */
int
month_no(mon_ca)
  char mon_ca[];
{
  switch (mon_ca[0]) {
  case 'J': /* Jan, Jun, Jul */
    if (mon_ca[1] == 'a')
      return 0;
    else if (mon_ca[2] == 'n')
      return 5;
    else
      return 6;
  case 'F': /* Feb */
    return 1;
  case 'M': /* Mar, May */
    if (mon_ca[2] == 'r')
      return 2;
    else
      return 4;
  case 'A': /* Apr, Aug */
    if (mon_ca[1] == 'p')
      return 3;
    else
      return 7;
  case 'S': /* Sep */
    return 8;
  case 'O': /* Oct */
    return 9;
  case 'N': /* Nov */
    return 10;
  default:  /* Dec */
    return 11;
  }
}


/*
 * first_vuln and associated functions
 */
#include "first.c"


/*
 * check_files and associated functions
 */
#include "check.c"


/*
 * Frees storage allocated by scandir.
 */
free_direct(direct_pa, no_files_i)
  direct_p_t direct_pa[];
  int no_files_i;
{
  int i;

  for (i=0; i<no_files_i; i++)
    free(direct_pa[i]);

  free(direct_pa);
}


/*
 * Main program
 */
main(argc, argv)
  int argc;
  char *argv[];
{
  int window_hrs_i,
      bytes_hr_i,
      no_files_i,
      first_vuln_i;

  char input_ca[MAX_LINE_LEN+1],
       newsgroup_ca[MAXNAMLEN+1],
       dirname_ca[MAXNAMLEN+1];

  direct_p_t *direct_pa_p;

  init(argc, argv);

  while (gets(input_ca)) {
    printf("> %s\n", input_ca);

    if (input_ca[0] != '#') { /* not comment; input line */
      sscanf(input_ca,
             " %s %d %d",
             newsgroup_ca,
             &window_hrs_i,
             &bytes_hr_i);

      cnv_to_fname(newsgroup_ca);
      sprintf(dirname_ca, "%s/%s", g_newsdir_ca, newsgroup_ca);
      no_files_i
        = scandir(dirname_ca,
                  &direct_pa_p,
                  only_articles,
                  by_article_no);
      if (no_files_i < 0)
        printf("Warning: scandir error on %s", dirname_ca);

      if (no_files_i > 0) { /* are files in directory */
        first_vuln_i
          = first_vuln(dirname_ca,
		       direct_pa_p,
		       no_files_i,
		       window_hrs_i);

        if (first_vuln_i < no_files_i) /* some files vulnerable */
          check_files(dirname_ca,
		      direct_pa_p,
		      first_vuln_i,
		      no_files_i,
		      window_hrs_i*bytes_hr_i);
        else
          printf("  no files within window");

        free_direct(direct_pa_p, no_files_i);
      }
    }
  }
}

