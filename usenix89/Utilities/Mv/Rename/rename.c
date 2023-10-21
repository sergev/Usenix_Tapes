/*
 Project:	rename
 Module:	main
 File:		rename.c	$Revision$
 Synopsis:	rename [-i] file [...] [prefix]^=[suffix]
 Option:
	-i : interactive mode.  Query before erasing a file
 Machine:	vax 780
 Author: Gilles Chartrand
 Purpose:
	Rename many files by adding a suffix and/or prefix to each.

 Date:	Jan. 1986
*/

#include <stdio.h>
#include <strings.h>
#include <sys/file.h>

#define XBREAK	'^'
#define RBREAK	'='
#define MAX_LEN	40
#define ERROR(me)	fprintf(stderr, "%s\n", me) ;

typedef	enum {unknown, expand, replace} op_type_t ;
typedef enum {false, true} bool_t ;

extern int	rename() ;	/* libc function to rname files */
static void	multi_exp() ;	/* expand file names */
static void	multi_rep() ;	/* replace substring in file */
static void	my_rename() ;
static char	*sindex() ;	/* find a substring in a string */

bool_t		i_flag ;

void
main(argc, argv)
char	*argv[] ;
int	argc ;
{
	char		*str1, *str2, *scan ;
	op_type_t	op_type ;

	if (strcmp(argv[1], "-i") == 0) {
	    i_flag = true ;
	    argv ++ ;
	    argc -- ;
	}

	if (argc < 3) {
	    ERROR("usage: rename [-i] file [...] [str1]<^|=>[str2]") ;
	    exit(1) ;
	}

	str1 = "" ;
	str2 = "" ;
	op_type = unknown ;

	scan = argv[argc - 1] ;
	if (scan[0] == XBREAK) {
	    op_type = expand ;
	    str2 = scan + 1 ;

	} else {

	    str1 = scan ;
	    while (*scan != '\0' && *scan != XBREAK && *scan != RBREAK) scan++ ;
	    if (*scan == '\0') {
		ERROR("rename: a ^ or a = must appear in the last argument");
		exit(1) ;
	    }
	    str2 = scan + 1 ;
	    if (*scan == XBREAK)	op_type = expand ;
	    else			op_type = replace ;

	    *scan = '\0' ;
	}

	if ((strlen(str1) + strlen(str2)) == 0) {
	    ERROR("rename: one of str1 or str2 must be given") ;
	    exit(1) ;
	}

	if (op_type == expand)	multi_exp(argv + 1, argc - 2, str1, str2) ;
	else			multi_rep(argv + 1, argc - 2, str1, str2) ;
} /* main */

/*
 Procedure: multi_exp(file_array, count, prefix, suffix)
 Parameters:
	file_array : array of files to be moved.
	count : number of files in above array.
	prefix : prefix to add to the front of the file.
	suffix : suffix to add to the front of the file.
 Description:
	Expand each of the given files to have the given prefix and suffix.
*/
static void
multi_exp(file_array, count, prefix, suffix)
char	*file_array[] ;
int	count ;
char	*prefix, *suffix ;
{
	char	*new_file ;
	int	max_len, pad, length, rc ;

	max_len = MAX_LEN ;
	new_file = (char *) malloc((unsigned int) max_len) ;
	pad = strlen(prefix) + strlen(suffix) + 1 ;

	while (count-- > 0) {
	    length = strlen(file_array[count]) + pad ;
	    if (length > max_len) {
		max_len = 2 * length ;
		free(new_file) ;
		new_file = (char *) malloc((unsigned int) max_len) ;
	    }
	    strcpy(new_file, prefix) ;
	    strcat(new_file, file_array[count]) ;
	    strcat(new_file, suffix) ;

	    my_rename(file_array[count], new_file) ;
	}
	return ;
} /* multi_exp */

/*
 Procedure: multi_rep(file_array, count, old, new)
 Parameters:
	file_array : array of files to be moved.
	count : number of files in above array.
	old : subtring to be replaced.
	new : string to replace substring.
 Description:
	call move for each of the given files.
*/
static void
multi_rep(file_array, count, old, new)
char	*file_array[] ;
int	count ;
char	*old, *new ;
{
	char	*new_file, *substr ;
	int	max_len, pad, rc ;
	int	length, offset ;

	max_len = MAX_LEN ;
	new_file = (char *) malloc((unsigned int) max_len) ;
	pad = strlen(new) - strlen(old) + 1 ;

	if (strlen(old) == 0) {
	    ERROR("rename: no substring given") ;
	    exit(1) ;
	}

	while (count-- > 0) {
	    length = strlen(file_array[count]) + pad ;
	    if (length > max_len) {
		max_len = 2 * length ;
		free(new_file) ;
		new_file = (char *) malloc((unsigned int) max_len) ;
	    }
	    substr = sindex(file_array[count], old) ;
	    if (substr == NULL) {
		fprintf(stderr, "rename: substring not found in %s\n",
			file_array[count]) ;
		continue ;
	    }
	    offset = substr - file_array[count] ;
	    strncpy(new_file, file_array[count], offset) ;
	    new_file[offset] = '\0' ;
	    strcat(new_file, new) ;
	    strcat(new_file, file_array[count] + offset + strlen(old)) ;

	    my_rename(file_array[count], new_file) ;
	}
	return ;
} /* multi_rep */

/*
 Procedure:	my_rename(old, new) ;
 Parameters:
	old = old file name
	new = new file name
 Description:
	call rename and give appropriate error messages.
	if the -i option was given then make sure new doesn't exist.
 Method:
	rename(2), perror(3), and access(2)
*/
static void
my_rename(old, new)
char	*old, *new ;
{
	char	reply[80] ;
	int	rc ;

	if ((i_flag) && (access(new, F_OK) == 0)) {
	    fprintf(stderr, "Ok to replace %s?", new) ;
	    scanf("%s", reply) ;
	    if (reply[0] != 'y') return ;
	}
	rc = rename(old, new) ;
	if (rc != 0) perror(old) ;

	return ;
}

/*
 Function:	sindex(master, sub)
 Parameters:
	master - string in which sub is to be found
	sub - string to look for in master
 Description:
	return a substring pointer to master or NULL if the substring is
	not contained.
*/
static char *
sindex(master, sub)
char *master, *sub ;
{
	char	*mptr, *sptr, *ptr ;
	int	mlen, slen ;

	mlen = strlen(master) ;
	slen = strlen(sub) ;
	if (mlen < slen) return (NULL) ;
	if (slen == 0) return(NULL) ;

	mptr = master ;
	while (strlen(mptr) >= slen) {
	    while (*mptr != sub[0]) {
		if (*mptr == '\0') return (NULL) ;
		mptr ++ ;
	    }
	    sptr = sub ;
	    ptr = mptr ;
	    while (*ptr++ == *sptr++) {
		if (*sptr == '\0') return (mptr) ;
	    }
	    mptr++ ;
	}
	return (NULL) ;
} /* sindex */
