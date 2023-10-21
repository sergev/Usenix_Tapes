#include <stdio.h>
#include <sys/param.h>
#include <sys/file.h>
#include <ctype.h>
#include <sys/dir.h>
#include <sys/stat.h>



/*
** This program modifies its input file(s).  It replaces all occurances
** of a specified string with a specified new string. If no replacements
** were actually done, the input file is not altered. (Its last-modification-
** date remains the same.)  It prints to stdout the names of the files in which
** replacements are done.  In the unlikely event that the system
** goes down after unlinking the old file but before linking the new one,
** there will be a copy of the new file in <filename>.<pid>, e.g.
** "/foo/bar.15782".
**
**	% replace <old_string> <new_string> <file_name> ...
**
** The method employed is somewhat complicated.  The key to understanding
** it is in the routine init_incr(). Read that first, then read replace().
** 
** I found out after I wrote it that the string-replacement algorithm is an 
** extension of a string-search algorithm named after about three people.  
** But, alas, I can't remember their names, so I can't give you a reference in
** the literature.  Sorry.
**
**           -- Dave Jones (djones)
*/



int*   incr;
char*  old_pattern;
char*  new_pattern;


main(argc, argv)
     char** argv;
     
{
  
  
  char** files;
  char*  buffered_file();
  
  if (argc < 4)
    {  fprintf(stderr,
	       "%s: ERROR.  Too few arguments (%d).\n", argv[0], argc );
       fprintf(stderr, 
	       "Usage: %s <old_string> <new_string> <file_name>...\n",
	       argv[0]);
       exit(1);
     }
  
  old_pattern = argv[1];
  new_pattern = argv[2];
  files	    = &(argv[3]);
  
  {
    int len = strlen(old_pattern);
    incr = (int*)malloc( (len+2) * sizeof (int));
    init_incr(len);
  }
  
  if (*old_pattern == '\0')
    {  fprintf(stderr, "%s: ERROR.  Old pattern string is NIL.\n", argv[0]);
       exit(1);
     }

  for ( files = &(argv[3]);  *files;  files++ )
    {
      char* temp();
      char* old_name = *files;
      char* old_file;
      char* new_name = temp(old_name);
      FILE* new_file;
      
      old_file = buffered_file(old_name);
      
      if(old_file != NULL)
	{ new_file = fopen(new_name, "w");
	  
	  if (new_file == NULL)
	    {
	      fprintf(stderr, "Cannot open temp-file. " );
	      perror(new_name);  exit(1);
	    }
	  else
	    {/* replace() returns true iff a replacement was done*/
	      
	      int change_file = 
	        replace(old_pattern, new_pattern, old_file, new_file);

	      if( ferror(new_file) || fclose(new_file)) /* sic */
		{ 
		  perror(new_name);
		  unlink(new_name);
		  exit(1);
		}

	      if(change_file)
		{ printf("%s\n", old_name);
		  unlink(old_name);
		  link(new_name, old_name );
		}

	      unlink(new_name);
	    }
	  free(old_file);
	}
      
      free(new_name);
    }
  
  exit(0);  	
  
}/* end.. main */



char*
temp( name )  /* Makes a file name for a temporary file. */
     char* name;
     
{
  char* new_name = (char*)malloc( strlen(name) + 20 );
  
  if(new_name == 0)
    { fprintf(stderr, "Out of memory\n");
      exit(1);
    }
  
  sprintf(new_name, "%s.%d", name, getpid());
  
  return new_name;
  
}

#ifdef kluge
/* The following kluge is to save a few cycles on putc().  May not be
 * transportable.
 */

#undef putc

#define putc(x,p) if(--(p)->_cnt>=0) \
                     *(p)->_ptr++=(x); \
                  else \
		    _flsbuf((unsigned)(x),p)

/* end kluge */
#endif kluge



static replacement_done;

replace(old_pattern, new_pattern, old_file, new_file)
     register char* 	old_pattern;
     char* 		new_pattern;
     register char* 	old_file;
     register FILE* 	new_file;
     
{ /* replace *************************************************************/
  register char* next_to_match = old_pattern;
  register char  next_char;
  char*		 flush_deferred_chars();
  
  replacement_done = 0;
  next_char = *old_file++;
  
  /* Invariant: next_to_match != old_pattern iff output chars are defered. */
    
  while ( next_char )
    { 
      /* Built for speed. The case in which the match fails at the 
      ** first index is the most important.
      */

      if( *next_to_match == next_char )
	{ /* assert: *next_to_match != '\0' */
	  /* Partial match.  Defer writing output. */
	  
	  next_to_match++;
	  next_char = *old_file++;
	  continue; /* <===== */
	}
      
      if( next_to_match == old_pattern)
	{ 
	  /* typical case.. match fails on first char. Write it. */
	  
	  putc(next_char, new_file);
	  next_char = *old_file++;
	  continue; /* <===== */
	}
      
      /* match succeeded,  or failed in the middle */
      next_to_match
	= flush_deferred_chars(next_to_match, new_file);
      
    } 
    
  if (next_to_match != old_pattern)
    flush_deferred_chars(next_to_match, new_file);
  
  
  return replacement_done;
  
} /* end .. replace  ******************************************************/



/**************************************************************************/
char*
flush_deferred_chars(next_to_match, new_file)
     char  *next_to_match;
     register FILE* new_file;
{  /* flush_deferred_chars */
  
  /* Copy chars to the output file from either the new pattern, or from
   * an initial segment of the old pattern.
   */
  if ( *next_to_match == '\0' )
    {
      /* Old_pattern is matched. Copy new_pattern to output file. */
      
      char* copy = new_pattern;
      while( *copy ) putc( *copy++, new_file);
      next_to_match = old_pattern;
      replacement_done = 1;
    }
  else
    { 
      /* Match has failed in middle of old_pattern. */

      register char* copy = old_pattern;
      int delta = next_to_match - old_pattern;
      int disp  = incr[delta];
      register int flush_deferred_chars = disp;

      while ( flush_deferred_chars-- ) putc(*copy++, new_file);
      
      next_to_match = old_pattern + delta - disp;
      
      
    } 
  
  
  return next_to_match;
  
  
}  /* end.. flush_deferred_chars **********************************************************/





/*
** Initializes a table (incr) which contains, for each positive integer delta
** less than the length of the old_string, the least number "place" such
** that 
**
**   substr(old_string,0,delta-place) == substr(old_string,place, delta-place)
**
** where substr(s, i, L)  is the substring of s starting at index i, and
** having length L.
**
**
** example: If old_string is "abbabbbx", then incr[6] is 3:
**
**    place = 3  delta = 6
**            |  |
**            |  |
**            V  V
**         abbabbbx
**            abbabbbx
**          ->| |<-
**
**       delta - place = 3
**
**
** The upshot of this is that when a match of "abbabbbx" fails at index
** 6, (after "abbabb"),  we will emit "abb" into the output file, and begin 
** matching at position, 3, having implicitly matched a second "abb".
**
*/

init_incr(len)
{ 
  int delta;
  
  for ( delta = 1; delta < len; delta++ )
    
    { int place = 1;
      while
	(strncmp( old_pattern, old_pattern + place, delta-place)!= 0)
	  place++;
      incr[delta] = place;
    }
  
}  /* end.. init_incr */



/*
** This procedure buffers up a named file, and returns a pointer to the
** buffer.  If something goes wrong, it returns NULL, and errno will have
** been set.
*/

char*
buffered_file(file_name)
     char* file_name;
     
{
  
  char* file_buffer;
  char  format[20];
  struct stat stat_buf;
  
  int fd = open(file_name, O_RDONLY, 0);
  
  if (fd < 0 || fstat( fd, &stat_buf) == -1)
    { perror(file_name);
      return NULL;
    }
  
  file_buffer = (char*)malloc(stat_buf.st_size + 1);
  
  if(file_buffer == 0)
    { fprintf(stderr, "Out of memory\n");
      exit(1);
    }
  
  if( read( fd, file_buffer, stat_buf.st_size ) != stat_buf.st_size )
    {
      perror(file_name);
      close(fd);
      free(file_buffer);
      return NULL;
    }
  
  file_buffer[stat_buf.st_size] = '\0';
  
  close(fd);
  
  return file_buffer;
  
} /* end.. buffered_file */
/****************************************************************************/



