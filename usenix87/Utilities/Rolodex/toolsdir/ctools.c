/* -*- Mode: C; Package: (CTOOLS C) -*- */

#include <ctype.h>
#include <stdio.h>

#ifdef BSD42
#include <strings.h>
#endif

#include "ctools.h"

/* miscellaneous fairly primitive routines that deal with characters, */
/* strings, memory, simple input and pathnames. */
 

/* Author:  JP Massar */
/* Thinking Machines Corporation */

/* Included routines:

   emalloc
   anewstr
   
   copy
   fill
   
   to_upper_if_lower
   to_lower_if_upper
   
   buffconcat
   nbuffconcat
   
   slcompare
   slge_compare
   nocase_compare
   
   strfind
   strncfind
   strsearch
   strncsearch
   
   yes_or_no_check
   
   remove_excess_blanks
   ip_string_trim
   string_trim
   string_upcase
   string_downcase
   
   all_digits
   all_whitespace
   all_uppercase
   all_lowercase
   all_alphabetic
   all_alphanumeric
   all_ascii
   
   str_to_pos_int
   
   sreverse
   ip_sreverse
        
   temp_path
   perm_path
   make_path_numeric_extension
   make_path
   just_filename
   
   read_yes_or_no
   getline
   getlines
   ngetlines
   getfile
   ngetfile
   read_file_into_buffer
   efopen

   check_int
   check_string
   
*/
   

extern char *malloc();


char *emalloc (space) int space;

/* allocate 'space' bytes, die if we have run out of memory. */

{
  char *rval;        
  if (space < 0) {
     fprintf(stderr,"Fatal error: argument to emalloc < 0\n");
     exit(-1);
  }
  if (0 == (rval = malloc((unsigned) space))) {
     fprintf(stderr,"Fatal error:  No more memory\n");
     exit(-1);
  }
  return(rval);
}  


char *anewstr (astring) char *astring;

/* allocate space for and then copy a string.  Returns pointer to */
/* new string. */

{        
  char *newstr;
  newstr = emalloc(strlen(astring)+1);
  strcpy(newstr,astring);
  return(newstr);
}


copy (dest,src,n)

  /* copy n bytes */

  register char *dest,*src;
  register int n;

  { register int j = 0;
    while (j++ < n) *dest++ = *src++; 
  }
 

fill (addr,ch,n)

  /* fill n sequential bytes with 'ch' */

  register char *addr;
  register char ch;
  register int n;
  
  { register int j = 0;
    while (j++ < n) *addr++ = ch;
  }


to_upper_if_lower (ch)

  char ch;

  { return(islower(ch) ? toupper(ch) : ch); }


to_lower_if_upper (ch)

  char ch;

  { return(isupper(ch) ? tolower(ch) : ch); }


buffconcat (buffer,s1,s2) 

  /* concatenate two null terminated strings into a buffer. */

  char *buffer, *s1, *s2;
  
  { while (*s1 != '\0') *buffer++ = *s1++;
    while (*s2 != '\0') *buffer++ = *s2++;
    *buffer = '\0';
  }


nbuffconcat (buffer,n,s1,s2,s3,s4,s5,s6)

  /* concatenates up to 6 strings into a buffer.  Returns -1 if n */
  /* is not reasonable, otherwise returns 0. */

  char *buffer;
  int n;
  char *s1,*s2,*s3,*s4,*s5,*s6;

{
  register char *b;
  register char *s;
  int i;
  b = buffer;
  if (n < 1 || n > 6) return(-1);
  for (i = 1; i <= 6; i++) {
      if (i > n) break;
      switch (i) {
        case 1 : s = s1; break;
        case 2 : s = s2; break;
        case 3 : s = s3; break;
        case 4 : s = s4; break;
        case 5 : s = s5; break;
        case 6 : s = s6; break;
      }
      while (*s != '\0') *b++ = *s++;
  }
  *b = '\0';
  return(0);
}


slcompare (s1,l1,s2,l2)

  /* compare strings with possible nulls in them given their lengths */
  /* only returns EQUAL (0) or NOT EQUAL (-1) */

  char *s1;
  int l1;
  char *s2;
  int l2;

  { int j;
    if (l1 != l2) return(-1);
    j = 0;
    while (j++ < l1) 
      if (*s1++ != *s2++) return(-1);
    return(0);
  }

  
slge_compare (s1,l1,s2,l2)

  /* returns -1 if s1 < s2; 1 if s2 < s1; 0 if s1 = s2 */
  /* ignores nulls in the strings */

  char *s1;
  int l1;
  char *s2;
  int l2;

  { int j,len;
    j = 0;
    len = l2 > l1 ? l1 : l2;
    while (j++ < len) {
      if (*s1 != *s2) 
         return((*s1 < *s2) ? -1 : 1);
      s1++;   
      s2++;
    }  
    return((l2 == l1) ? 0 : ((l1 < l2) ? -1 : 1));
  }

nocase_compare (s1,l1,s2,l2)

  /* treats nulls as normal characters.  Returns same as slge_compare */

  char *s1;
  int l1;
  char *s2;
  int l2;

  { int j,len,ch1,ch2;
    j = 0;
    len = l2 > l1 ? l1 : l2;
    while (j++ < len) {
      ch1 = to_upper_if_lower(*s1++);
      ch2 = to_upper_if_lower(*s2++);
      if (ch1 != ch2) {
         return((ch1 < ch2) ? -1 : 1);
      }
    }  
    return((l2 == l1) ? 0 : ((l1 < l2) ? -1 : 1));
  }


char *strfind(s1,s2,fast)  
  
  register char *s1;
  char *s2;
  Bool fast;
  
{  
  register int len1,len2;
  len2 = strlen(s2);
  if (fast) {
     while (*s1 != '\0')
       if (0 == strncmp(s1++,s2,len2)) return(s1-1);
  }  
  else {
     len1 = strlen(s1);
     while (len1 >= len2) {
       if (0 == strncmp(s1++,s2,len2)) return(s1-1);
       len1--;
     }
  }
  return(0);
}     


char *strncfind(s1,s2,fast)

  register char *s1;
  char *s2;
  Bool fast;
  
{  
  register int len1,len2;
  len2 = strlen(s2);
  if (fast) {
     while (*s1 != '\0')
       if (0 == nocase_compare(s1++,len2,s2,len2)) return(s1-1);
  }
  else {
     len1 = strlen(s1);
     while (len1 >= len2) {
       if (0 == nocase_compare(s1++,len2,s2,len2)) return(s1-1);
       len1--;
     }
  }
  return(0);
}  

  
char *strsearch(s1,s1len,s2,s2len)

  /* do a substring search without noticing nulls */
  /* finds s2 in s1, returns pointer into s1 or 0 */

  register char *s1, *s2;
  register int s1len,s2len;
  
  {  register char *pc;
     register char *bound;
     register char *pctemp;
     register char *s2temp;
     register int j;

     bound = s1 + s1len - s2len;
     for (pc = s1; pc <= bound; pc++) {
         pctemp = pc;
         s2temp = s2;
         for (j = 0; j < s2len; j++)
             if (*pctemp++ != *s2temp++) goto not_here;
         return(pc);       
         not_here :
         continue;
     }    
     return(0);
}


char *strncsearch(s1,s1len,s2,s2len)

  /* do a substring search without noticing nulls */
  /* finds s2 in s1, returns pointer into s1 or 0 */
  /* case independent */

  register char *s1, *s2;
  register int s1len,s2len;
  
  {  register char *pc;
     register char *bound;
     register char *pctemp;
     register char *s2temp;
     register int j;
     char ch1, ch2;

     bound = s1 + s1len - s2len;
     for (pc = s1; pc <= bound; pc++) {
         pctemp = pc;
         s2temp = s2;
         for (j = 0; j < s2len; j++) {
             ch1 = *pctemp++;
             ch2 = *s2temp++;
             if (to_upper_if_lower(ch1) != to_upper_if_lower(ch2))
                goto not_here;
         }
         return(pc);       
         not_here :
         continue;
     }    
     return(0);
}


int remove_excess_blanks (newstring,oldstring) 

  /* it is assumed that newstring is as long as oldstring if necessary */

  char *newstring,*oldstring;

{
  int count = 0;
  int space_found = 0;

  /* skip over all blanks at beginning */
  
  if (*oldstring == ' ') {
     while (*oldstring == ' ') oldstring++;
  }

  while (*oldstring != '\0') {
        if (space_found && *oldstring == ' ') {
           oldstring++;
           continue;
        }
        space_found = (*oldstring == ' ');
        *newstring++ = *oldstring++;
        count++;
  }

  *newstring = '\0';
  if (count > 0 && *(newstring - 1) == ' ') {
     count--;
     *(newstring - 1) = '\0';
  }

  return(count);

}

int ip_string_trim (oldstring,trimchars,pretrim,posttrim)

  char *oldstring, *trimchars;
  Bool pretrim,posttrim;

{
  Bool trim = T;
  char *np = oldstring, ch;
  int len;
  
  if (pretrim) {
     while (trim && ('\0' != (ch = *np))) {
       trim = (0 != index(trimchars,ch));
       if (trim) np++;
     }
     strcpy(oldstring,np);
  }
  if (1 >= (len = strlen(oldstring)) && pretrim) return(len);
  if (posttrim) {
     np = oldstring + len - 1;
     while (T) {
       ch = *np;
       trim = (0 != index(trimchars,ch));
       if (trim) *np == '\0';
       if (!trim || np == oldstring) break;
       np--;
     }
  }
  return(strlen(oldstring));
}
  
int string_trim (newstring,oldstring,trimchars,pretrim,posttrim)

  char *newstring, *oldstring, *trimchars;
  Bool pretrim, posttrim;
  
{  
  strcpy(newstring,oldstring);
  return(ip_string_trim(newstring,trimchars,pretrim,posttrim));
}

char *string_upcase (astring) char *astring;
{
  while (*astring) {
    *astring = to_upper_if_lower(*astring);
    astring++;
  }
}

char *string_downcase (astring) char *astring;
{
  while (*astring) {
    *astring = to_lower_if_upper(*astring);
    astring++;
  }
}


yes_or_no_check (astring) char *astring;

/* returns 1 if yes, 0 if no, -1 if neither */
/* works for 'Y' 'YES' 'NO' 'N' in any capitalization */

{  
  int len;
  len = strlen(astring);
  if (len == 0 || len > 3) return(-1);
  if (0 == nocase_compare(astring,len,"YES",3) || 
      0 == nocase_compare(astring,len,"Y",1))
     return(1);
  if (0 == nocase_compare(astring,len,"NO",2) || 
      0 == nocase_compare(astring,len,"N",1))
     return(0);
  return(-1);
}


Bool all_digits (astring) char *astring;

/* test whether every character is a digit (0-9) */

{
  while (*astring != '\0') 
    if (!isdigit(*astring++)) return(F);
  return(T);
}


Bool all_whitespace (astring) char *astring;

/* test whether every character is a blank or a tab */

{
  register char ch;
  while ((ch = *astring++) != '\0') {
    if (ch == ' ' || ch == '\t') continue;
    return(F);
  }
  return(T);
}

Bool all_uppercase(astring) char *astring;
{
  register char ch;
  while ((ch = *astring++) != '\0') {
    if (!isupper(ch)) return(F);
  }
  return(T);
}

Bool all_lowercase(astring) char *astring;
{
  register char ch;
  while ((ch = *astring++) != '\0') {
    if (!islower(ch)) return(F);
  }
  return(T);
}

Bool all_alphabetic(astring) char *astring;
{
  register char ch;
  while ((ch = *astring++) != '\0') {
    if (!isalpha(ch)) return(F);
  }
  return(T);
}

Bool all_ascii(astring) char *astring;
{
  register char ch;
  while ((ch = *astring++) != '\0') {
    if (!isascii(ch)) return(F);
  }
  return(T);
}

Bool all_alphanumeric(astring) char *astring;
{
  register char ch;
  while ((ch = *astring++) != '\0') {
    if (!isalnum(ch)) return(F);
  }
  return(T);
}

int str_to_pos_int (astring,low,high) char *astring; int low,high;

  /* returns -1 if *astring is not composed of digits. */
  /* returns -2 if the integer is out of range. */
  /* treats all digit strings as decimal. */

{
  int value,len,maxlen,j;
  maxlen = strlen(MAXINTSTR);
  len = strlen(astring);
  if (!all_digits(astring)) return(-1);
  if (len > maxlen) return(-2);
  if (len == maxlen) {
     if (1 == strcmp(astring,MAXINTSTR)) return(-2);
  }
  for (j = 0; j < len-1; j++) {
      if (*astring != '0') break;
      astring++;
  }
  sscanf(astring,"%d",&value);
  if (value < low || value > high) return(-2);
  return(value);
}


int sreverse (buffer,astring) char *buffer, *astring;
{
  register int last = strlen(astring);
  buffer[last--] = '\0';
  while (last >= 0) buffer[last--] = *astring++;
}

char * ip_sreverse (astring) char *astring;
{
  register int last = strlen(astring) - 1;
  register int first = 0;
  register char ch;
  while (first < last) {
    ch = astring[first];
    astring[first++] = astring[last];
    astring[last--] = ch;
  }
  return(astring);
}



static char pathbuffer[PATH_MAXPATHLEN];


char *temp_path (dir,filename) char *dir; char *filename;

{
  return(make_path(dir,filename,"",F));
}


char *perm_path (dir,filename) char *dir; char *filename;

{
  return(make_path(dir,filename,"",T));
}


char *make_path_numeric_extension (dir,filename,extension,perm)

  char *dir, *filename;
  int extension;
  Bool perm;

{
  char buffer[20];
  sprintf(buffer,"%d",extension);
  return(make_path(dir,filename,buffer,perm));
}


char *make_path (dir,filename,extension,perm)

  char *dir, *filename, *extension;
  Bool perm;

{
  char *rval;
  if (!perm && (strlen(dir) + 1 + strlen(filename) + strlen(extension) + 1 >=
                PATH_MAXPATHLEN)) {
     return((char *) 0);
  }
  nbuffconcat(pathbuffer,4,dir,"/",filename,extension);
  if (!perm) return(pathbuffer);
  rval = emalloc(strlen(pathbuffer) + 1);  
  strcpy(rval,pathbuffer);
  return(rval);
}


char *just_filename (path,new,perm) char *path; Bool new,perm;

{
  char *fnp,*rval;
  fnp = (0 == (fnp = rindex(path,'/'))) ? path : fnp + 1;
  if (!new) return(fnp);
  if (!perm) {
     strcpy(pathbuffer,fnp);
     return(pathbuffer);
  }
  else {
     rval = emalloc(strlen(fnp) + 1);
     strcpy(rval,fnp);
     return(rval);
  }
}



read_yes_or_no (iport,oport,prompt,helpstring,quitstring)

  /* prints prompt, then reads from port until it gets 'Y', 'N', 'YES' or */
  /* 'NO' (case independently).  If helpstring and/or quitstring are not */
  /* "" or (char *) 0 then if the user types in one of those ANSWER_HELP */
  /* or ANSWER_QUIT are returned, otherwise ANSWER_NO or ANSWER_YES are */
  /* eventually returned. */

  FILE *iport, *oport;
  char *prompt, *helpstring, *quitstring;

{
  char buffer[20],buffer2[20];
  int bl,hl,ql,len;
  
  buffer[19] = '\0';
  
  while (T) {
        
    fprintf(oport,"%s",prompt);
    switch (len = getline(iport,buffer,20)) {
      case (AT_EOF) :
        return(ANSWER_EOF);
        break;
      case (TOO_MANY_CHARS) :
        break;
      default :
        if (0 == (bl = remove_excess_blanks(buffer2,buffer))) break;
        switch (yes_or_no_check(buffer2)) {
          case (0) :
            return(ANSWER_NO);
          case (1) :
            return(ANSWER_YES);
          case (-1) :
            if (helpstring != (char *) 0 && (hl = strlen(helpstring)) > 0) {
               if (0 == nocase_compare(buffer2,bl,helpstring,hl)) {
                  return(ANSWER_HELP);
               }
            }
            if (quitstring != (char *) 0 && (ql = strlen(quitstring)) > 0) {
               if (0 == nocase_compare(buffer2,bl,quitstring,ql)) {
                  return(ANSWER_QUIT);
               }
            }
            break;
        }   
        break;
    }
   
    fprintf(oport,"Please answer 'YES' or 'NO'\n");
    continue;
   
  }
    
}


int getline (iport,buffer,buflen) FILE *iport; char *buffer; int buflen;

  /* reads a line into buffer.  Does not put the '\n' into buffer. */
  /* Returns AT_EOF if at end of file when called.  If it encounters */
  /* end of file after reading at least one character, the eof is treated */
  /* as if it were a newline.   Returns TOO_MANY_CHARS if more than */
  /* buflen - 1 characters are read before encountering a newline. */        
  /* In this case exactly buflen - 1 characters are read. */
  /* The last character read is always follwed by a '\0'. */
  /* if successful getline returns the number of characters read exclusive */
  /* of a terminating newline or eof. */

{
  int ch;
  char *bptr = buffer;
  int nchars = 0;
  
  if (buflen <= 0) return(TOO_MANY_CHARS);
  
  while (T) {
    switch (ch = getc(iport)) {
      case (EOF) :
      case ('\n') :
        if (ch == EOF && nchars == 0) return(AT_EOF);
        *bptr = '\0';
        return(nchars);
      default :
        if (++nchars == buflen) { 
           *bptr = '\0';
           ungetc(ch,iport);
           return(TOO_MANY_CHARS);
        }
        *bptr++ = ch;
    }
    
  }
    
}


int getlines (fp,n,ptr_lines,linebuf,maxlinelen)

  /* See documentation for getfile below */

  FILE *fp;
  int n;
  char ***ptr_lines;
  char *linebuf;
  int maxlinelen;

{
  int len;
  char *line;
  if (0 > (len = getline(fp,linebuf,maxlinelen))) {
     if (len == AT_EOF) {
        *ptr_lines = (char **) emalloc(n * sizeof(char **));
        return(n);
     }
     else {
        return(TOO_MANY_CHARS);
     }
  }
  else {
     line = emalloc(len+1);
     strcpy(line,linebuf);
     len = getlines(fp,n+1,ptr_lines,linebuf,maxlinelen);
     if (len == TOO_MANY_CHARS) return(TOO_MANY_CHARS);
     (*ptr_lines)[n] = line;
     return(len);
  }
}


int getfile (filename,ptr_lines,linebuf,maxlinelen)

  /* read in a file as an array of character strings */
  /* 'maxlinelen+1' is the maximum length a line of the file is allowed */
  /* to be.  'linebuf' must be at least 'maxlinelen+1' characters long. */
  /* Returns the number of lines in the file (and therefore the number */
  /* of entries in *ptr_lines) if successful.  Returns IOERROR if it */
  /* could not open the file to read from. Returns TOO_MANY_CHARS if */
  /* it encounters a line longer than 'maxlinelen' characters. */

  /* Space for each line is malloc'ed as it is read in and the text for */
  /* the jth line is stored in (*ptr_lines)[j] */

  char *filename;
  char ***ptr_lines;
  char *linebuf;
  int maxlinelen;

{
  FILE *fp;
  int nlines;
  if (NULL == (fp = fopen(filename,"r"))) return(IOERROR);
  nlines = getlines(fp,0,ptr_lines,linebuf,maxlinelen);
  fclose(fp);
  return(nlines);
}


int ngetlines (fp,n,ptr_lines,linebuf,maxlinelen)

  /* See documentation for ngetfile below */

  FILE *fp;
  int n;
  char ***ptr_lines;
  char *linebuf;
  int maxlinelen;

{
  int len;
  int nlines = 0;
  *ptr_lines = (char **) emalloc(n * sizeof(char **));
  while (T) {
    if (0 > (len = getline(fp,linebuf,maxlinelen))) {
       if (len == AT_EOF) {
          return(nlines);
       }
       else {
          return(TOO_MANY_CHARS);
       }
    }
    else {
       if (++nlines > n) {
          return(TOO_MANY_LINES);
       }
       (*ptr_lines)[nlines-1] = anewstr(linebuf);
    }
  }
}



int ngetfile (n,filename,ptr_lines,linebuf,maxlinelen)

  /* Same as getfile except that at most n lines will be read. */
  /* If it attempts to read more than n lines, TOO_MANY_LINES will */
  /* be returned. */

  int n;
  char *filename;
  char ***ptr_lines;
  char *linebuf;
  int maxlinelen;

{
  FILE *fp;
  int nlines;
  if (NULL == (fp = fopen(filename,"r"))) return(IOERROR);
  nlines = ngetlines(fp,n,ptr_lines,linebuf,maxlinelen);
  fclose(fp);
  return(nlines);
}


extern int read_file_into_buffer (

       filename,ptr_lines,maxlines,buffer,buflen,linebuffer,linebuflen

    )
       
  char *filename; 
  char ***ptr_lines;
  int maxlines;
  char *buffer;
  int buflen;
  char *linebuffer;
  int linebuflen;

  /* *ptr_lines should be an array of character string pointers maxlines */
  /* big.  buffer should be an array of characters buflen long.  The routine */
  /* reads lines using getline and stores them into buffer, terminating each */
  /* with a null.  A pointer to the nth line read is stored in *ptr_lines[n] */
  /* Returns IOERROR if it cannot open the file for reading, TOO_MANY_LINES */
  /* if more than maxlines were read in, TOO_MANY_CHARS if buffer is */
  /* filled before end of file is reached, and LINE_TOO_LONG is any line is */
  /* longer than linebuflen.  Returns number of lines read in if successful. */
  
{  
  FILE *fp;
  int linecount,charcount,len;
  char *bp;
  char **lines;
  
  if (NULL == (fp = fopen(filename,"r"))) return(IOERROR);
  linecount = 0;
  charcount = 0;
  bp = buffer;
  lines = *ptr_lines;
  
  while (T) {
        
    if (0 > (len = getline(fp,linebuffer,linebuflen))) {
       fclose(fp);
       if (len == AT_EOF) {
          return(linecount);
       }
       else {
          return(LINE_TOO_LONG);
       }
    }
    
    if (linecount >= maxlines) {
       fclose(fp);
       return(TOO_MANY_LINES);
    }
    
    charcount += len;
    if (charcount >= buflen) {
       fclose(fp);
       return(TOO_MANY_CHARS);
    }
    
    strcpy(bp,linebuffer);
    lines[linecount++] = bp;
    bp += (len + 1);
  
  }
  
}
  
extern char *efopen (filename,mode) char *filename; char *mode;

  /* The routine simply calls fopen with the same arguments, but prints a */
  /* reasonable error message and calls exit if the call to fopen fails. */

{
  FILE *fp;
  if (NULL == (fp = fopen(filename,mode))) {
     fprintf(stderr,"Could not open %s, mode: %s\n",filename,mode);
     perror("Reason: ");
     exit(1);
  }
  return((char *) fp);
}



extern int record_fseek (fp,rnum,fromwhere,rsize,hdrsize)

  FILE *fp;
  long rnum;
  int fromwhere;
  int rsize;
  int hdrsize; 

{
  if (fromwhere == 0) {
     return(fseek(fp,(long) ((rnum - 1)*rsize + hdrsize),0));
  }
  else {
     return(fseek(fp,(long) (rnum*rsize),fromwhere));
  }
}


Bool check_string (s,minlen,maxlen) char *s; long minlen,maxlen;
{
  long len;
  if (s == 0) return(F);
  len = strlen(s);
  return (len >= minlen && len <= maxlen);
}

