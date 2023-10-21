/* -*- Mode: C; Package: (CTOOLS C) -*- */

#ifndef Bool
#define Bool int
#endif

#ifndef T
#define T 1
#endif

#ifndef F
#define F 0
#endif

#ifndef MAXINT
#define MAXINT 2147483647
#define MAXINTSTR "2147483647"
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 80
#endif

extern char *emalloc();

  /* int space; */
  /* space must be greater than 0 */
  /* Causes executution to halt with a 'Fatal error' message if memory */
  /* cannot be allocated, otherwise returns pointer to malloc-ed space */

extern char *anewstr();

  /* char *astring; */
  /* emalloc's space and copies astring into that space.  Returns pointer */
  /* to new string. */


extern int copy();

  /* char *dest, *src; int n; */
  /* copies exactly n bytes. */
  /* return value undefined.  Use only as procedure. */

extern int fill();

  /* char *addr, ch; int n; */
  /* copies ch into n consecutive bytes. */
  /* return value undefined.  Use only as procedure. */

extern int to_upper_if_lower();

  /* char ch;  Returns possibly upper-cased value. */

extern int to_lower_if_upper();

  /* char ch;  Returns possibly lower-cased value. */

extern int buffconcat();

  /* char *buffer, *s1, *s2; */
  /* s1 and s2 must be null terminated.  Buffer must be at least */
  /* strlen(s1) + strlen(s2) + 1 characters long.  Buffer is null */
  /* terminated upon completion. */

  /* return value undefined.  Use only as procedure. */

extern int nbuffconcat();

  /* char *buffer; int n; char *s1,*s2,*s3,*s4,*s5,*s6; */
  /* all the strings must be null terminated.  Buffer must be big enough */
  /* to hold the null terminated result.  0 < n < 7 .. 
  /* returns -1 if n is out of range, otherwise 0 */

extern int slcompare();

  /* char *s1; int l1; char *s2; int l2 */
  /* does not stop if it encounters a null character. */
  /* returns 0 if equal, -1 if not equal */

extern int slge_compare();

  /* char *s1; int l1; char *s2; int l2 */
  /* does not stop if it encounters a null character. */
  /* returns 0 if equal, -1 if s1 < s2, 1 if s1 > s2 */

extern int nocase_compare();

  /* char *s1; int l1; char *s2; int l2 */
  /* does not stop if it encounters a null character. */
  /* returns 0 if equal, -1 if s1 < s2, 1 if s1 > s2  case independently. */

extern char * strfind();

  /* char *s1; char *s2; int fast; */
  /* finds s2 as a substring of s1.  s1 and s2 are null terminated. */
  /* returns 0 if not found, otherwise pointer into s1 to first matching */
  /* character. */

  /* WARNING:  will access off the end of s1 in the interest of efficiency */
  /* if 'fast' is non-zero. */

extern char * strncfind();

  /* char *s1; char *s2; int fast; */
  /* finds s2 as a substring of s1 case independently.  s1 and s2 are */
  /* null terminated. */
  /* returns 0 if not found, otherwise pointer into s1 to first matching */
  /* character. */

  /* WARNING:  will access off the end of s1 in the interest of efficiency */
  /* if 'fast' is non-zero. */

extern char * strsearch();

  /* char *s1; int l1; char *s2; int l2 */
  /* finds s2 as a substring of s1.  Does not stop if it encounters a null. */
  /* returns pointer into s1, otherwise (char *) 0 if search fails */
  /* case dependent */

extern char * strncsearch();

  /* char *s1; int l1; char *s2; int l2 */
  /* finds s2 as a substring of s1. */
  /* returns pointer into s1, otherwise (char *) 0 if search fails */
  /* case independent */

extern int remove_excess_blanks();

  /* char *newstring, *oldstring; */
  /* newstring must be long enough to hold the result, which may be as */
  /* long as oldstring.  oldstring must be null terminated. */
  /* an excess blank is any blank before the first non-blank character, */
  /* any blank after the last non-blank character, and any blank immediately */
  /* following a blank. */
  /* returns length of newstring */

extern int yes_or_no_check();

  /* char *astring; */
  /* returns 1 for yes, 0 for no, -1 for neither. */
  /* astring must be one of "YES", "Y", "NO", "N" in any capitalization. */


/* These routines return T if every char satisfies a certain condition. */
/* These returns all returns T if given a null string. */

extern Bool all_digits();
extern Bool all_whitespace();
extern Bool all_uppercase();
extern Bool all_lowercase();
extern Bool all_alphabetic();
extern Bool all_alphanumeric();
extern Bool all_ascii();


extern int str_to_pos_int();

  /* char *astring; int low,high; */
  /* low must be >= 0. */
  /* returns -1 if *astring is not composed of digits. */
  /* returns -2 if the integer is out of range. */
  /* despite its name, 0 can be returned as a legitimate value. */
  /* treats all digit strings as decimal. */


extern int sreverse();

  /* char *buffer; char *astring; */
  /* puts the characters of astring in reverse order into buffer. */
  /* buffer must be at least as long as astring + 1. */
  /* buffer is null terminated when done. */
  /* No return value.  Use only as procedure. */

extern char *ip_sreverse();

  /* char *astring; */
  /* Returns astring with its characters reversed. */
  /* reversal is done in place. */



#define PATH_MAXPATHLEN 256

char *temp_path();

/*
  char *dir; char *filename;

  Returns a pointer to a character string containing the string
  <dir>/<filename>.  The pointer points to a buffer which will may get
  overwritten if any functions in this package are subsequently called.
  0 is returned if the pathname would exceed PATH_MAXPATHLEN-1 chars.
*/


char *perm_path();

/*
  char *dir; char *filename;

  Same as temp_path, except the pathname string is malloc'ed and is thus
  permanent unless specifically freed by the user.  Further, no limit
  on the size of the path is made.
*/


char *make_path();

/*
  char *dir; char *filename; char *extension; Bool perm;

  Creates <dir>/<filename><extension> .  The string returned is permanent
  or not depending on 'perm'.  If perm is not true, 0 will be returned if
  the resulting path exceeds PATH_MAXPATHLEN-1 chars.
*/


char *make_path_numeric_extension();

/*
  char *dir; char *filename; int extension; Bool perm;

  Same as make_path except that extension is first converted into a
  string using sprintf.
*/


char *just_filename();

/*  
  char *path; Bool new; Bool perm;

  Given a path of the form /<x>/<y>/<z> returns <z>.  If new is not set
  then a pointer into the original input string is returned.  If new is
  set a copy is returned, either permanent or not depending on perm.
*/


#define ANSWER_NO 0
#define ANSWER_YES 1
#define ANSWER_HELP 2
#define ANSWER_QUIT 3
#define ANSWER_EOF 4

#define AT_EOF -1
#define TOO_MANY_CHARS -2
#define IOERROR -3
#define TOO_MANY_LINES -4
#define LINE_TOO_LONG -5

extern read_yes_or_no ();

  /* FILE *iport, *oport; char *prompt; char *helpstring; char *quitstring; */

  /* prints prompt, then reads from iport until is gets 'Y', 'N', 'YES' or */
  /* 'NO' (case independently).  If helpstring and/or quitstring are not */
  /* "" or (char *) 0 then if the user types in one of those ANSWER_HELP */
  /* or ANSWER_QUIT are returned, otherwise ANSWER_NO or ANSWER_YES are */
  /* eventually returned. */


extern int getline ();

  /* FILE *iport; char *buffer; int buflen; */

  /* reads a line into buffer.  Does not put the '\n' into buffer. */
  /* Returns AT_EOF if at end of file when called.  If it encounters */
  /* end of file after reading at least one character, the eof is treated */
  /* as if it were a newline.   Returns TOO_MANY_CHARS if more than */
  /* buflen - 1 characters are read before encountering a newline. */        
  /* In this case exactly buflen - 1 characters are read. */
  /* The last character read is always follwed by a '\0'. */
  /* if successful getline returns the number of characters read exclusive */
  /* of a terminating newline or eof. */


extern int getlines();

  /* FILE *fp; int n; char ***ptr_lines; char *linebuf; int maxlinelen; */
  /* See documentation for getfile below */
  /* If called, 'n' must have a value 0. */

extern int getfile();

  /* char *filename; char ***ptr_lines; char *linebuf; int maxlinelen; */

  /* read in a file as an array of character strings */
  /* 'maxlinelen+1' is the maximum length a line of the file is allowed */
  /* to be.  'linebuf' must be at least 'maxlinelen+1' characters long. */
  /* Returns the number of lines in the file (and therefore the number */
  /* of entries in *ptr_lines) if successful.  Returns IOERROR if it */
  /* could not open the file to read from. Returns TOO_MANY_CHARS if */
  /* it encounters a line longer than 'maxlinelen' characters.  

  /* Space for each line is malloc'ed as it is read in and the text for */
  /* the jth line is stored in (*ptr_lines)[j] */

  /* Only works for fairly small files as it recurses its way through the */
  /* file and does a lot of malloc-ing.  Use read_file_into_buffer or */
  /* ngetfile for large files. */

extern int ngetlines();

 /* FILE *fp; int n; char ***ptr_lines; char *linebuf; int maxlinelen; */
 /* Same as getlines, except at most 'n' lines will be read.  Returns */
 /* TOO_MANY_LINES if more than 'n' lines are present. */

extern int ngetfile();

 /* int n; char *filename; char ***ptr_lines; char *linebuf; int maxlinelen; */
 /* See ngetlines above. */

extern int read_file_into_buffer();

  /* char *filename; 
     char ***ptr_lines;
     int maxlines;
     char *buffer;
     int buflen;
     char *linebuffer;
     int linebuflen;
  */

  /* *ptr_lines should be an array of character string pointers maxlines */
  /* big.  buffer should be an array of characters buflen long.  The routine */
  /* reads lines using getline and stores them into buffer, terminating each */
  /* with a null.  A pointer to the nth line read is stored in *ptr_lines[n] */
  /* Returns IOERROR if it cannot open the file for reading, TOO_MANY_LINES */
  /* if more than maxlines were read in, TOO_MANY_CHARS if buffer is */
  /* filled before end of file is reached, and LINE_TOO_LONG is any line is */
  /* longer than linebuflen.  Returns number of lines read in if successful. */

extern char *efopen();  

  /* char *filename; char *mode */

  /* Actually returns a (FILE *), so one must cast the return value to this */
  /* type.  It doesn't return a (FILE *) explicitly because then to include */
  /* this file one would have to include <stdio.h> explicitly before it. */
  /* The routine simply calls fopen with the same arguments, but prints a */
  /* reasonable error message and calls exit if the call to fopen fails. */


extern int record_fseek();

  /* FILE *fp; long rnum; int fromwhere; int rsize; int hdrsize; */

  /* Assumes a file is divided into fixed length records with a fixed length */
  /* header (possibly 0 bytes).  Performs a fseek which moves to the start */
  /* of a given record.  Record numbers begin with 1. */

  /* Returns what fseek returns. */

  /* 'rnum' is either relative or absolute, depending on 'fromwhere' which */
  /* corresponds to the 'ptrname' argument of fseek. */


Bool check_string();

  /* char *str; long minlen; long maxlen; */

  /* Returns T if str is not 0 and has a length between minlen and maxlen */
  /* inclusived, otherwise returns F. */


#ifndef check_int
#define check_int(i,minval,maxval) ((i) >= (minval) && (i) <= (maxval))
#endif
