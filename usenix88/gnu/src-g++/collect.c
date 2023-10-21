/* Output tables which say what global initializers need
   to be called at program startup, and what global destructors
   need to be called at program termination, for GNU C++ compiler.
   Copyright (C) 1987 Free Software Foundation, Inc.
   Hacked by Michael Tiemann (tiemann@mcc.com)

This file is part of GNU CC.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU CC General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU CC, but only under the conditions described in the
GNU CC General Public License.   A copy of this license is
supposed to have been given to you along with GNU CC so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


/* This file contains all the code needed for the program `collect'.

   `collect' is run on all object files that are processed
   by GNU C++, to create a list of all the file-level
   initialization and destruction that need to be performed.
   It generates an assembly file which holds the tables
   which are walked by the global init and delete routines.
   The format of the tables are an integer length,
   followed by the list of function pointers for the
   routines to be called.

   Constructors are called in the order they are laid out
   in the table.  Destructors are called in the reverse order
   of the way they lie in the table.  */

#include "config.h"

/* For this file, some special macros.  These should be merged
   into tm.h some time.  */
#undef ASM_OUTPUT_INT
#undef ASM_OUTPUT_LABELREF

#ifndef hp9000s300
#define ASM_OUTPUT_INT(FILE,VALUE)  \
  fprintf (FILE, "\t.long %d\n", VALUE)
#define ASM_OUTPUT_PTR_INT_SUM(FILE,PTRNAME,VALUE)	\
  fprintf (FILE, "\t.long _%s+%d\n", PTRNAME, VALUE)
#define ASM_OUTPUT_LABELREF(FILE,NAME)	\
  fprintf (FILE, "\t.long _%s\n", NAME);
#else
#define ASM_OUTPUT_INT(FILE,VALUE)  \
  fprintf (FILE, "\tlong %d\n", VALUE)
#define ASM_OUTPUT_PTR_INT_SUM(FILE,PTRNAME,VALUE)	\
  fprintf (FILE, "\tlong _%s+%d\n", PTRNAME, VALUE)
#define ASM_OUTPUT_LABELREF(FILE,NAME)	\
  fprintf (FILE, "\tlong _%s\n", NAME);
#endif /* hp9000s300 */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <a.out.h>
#include <ar.h>

extern int xmalloc ();
extern void free ();

#ifndef CTOR_TABLE_NAME
#define CTOR_TABLE_NAME "__CTOR_LIST__"
#endif

#ifndef DTOR_TABLE_NAME
#define DTOR_TABLE_NAME "__DTOR_LIST__"
#endif

enum error_code { OK, BAD_MAGIC, NO_NAMELIST,
		  FOPEN_ERROR, FREAD_ERROR, FWRITE_ERROR,
		  RANDOM_ERROR, };

enum error_code process ();
enum error_code process_a (), process_o ();
enum error_code coalesce ();
void assemble_name ();

/* Files for holding the assembly code for table of constructor
   function pointer addresses and list of destructor
   function pointer addresses.  */
static FILE *outfile;

/* Default outfile name, or take name from argv with -o option.  */
static char *outfile_name = "a.out";

/* The table of destructors elements must be laid out in the reverse
   order that the table of constructors is laid out.  Thus,
   is easiest to just cons up a list of destructors to call,
   and then write them out when they have all been collected.  */

struct dtor_list_elem
{
  struct dtor_list_elem *next;
  char *name;
} *dtor_chain;

main (argc, argv)
     int argc;
     char *argv[];
{
  int i, nerrs = 0;
  enum error_code code;
  FILE *fp;

  if (argc > 2 && !strcmp (argv[1], "-o"))
    {
      outfile_name = argv[2];
      i = 3;
    }
  else
    i = 1;

  if ((outfile = fopen (outfile_name, "w")) == NULL)
    {
      perror ("collect");
      exit (-1);
    }
  fprintf (outfile, "%s\n", TEXT_SECTION_ASM_OP);
  ASM_GLOBALIZE_LABEL (outfile, CTOR_TABLE_NAME);
  ASM_OUTPUT_LABEL (outfile, CTOR_TABLE_NAME);

  for (; i < argc; i++)
    {
      char buf[80];

      /* This is a library, skip it.  */
      if (argv[i][0] == '-' && argv[i][1] == 'l')
	continue;

      if ((fp = fopen (argv[i], "r")) == NULL)
	{
	  sprintf (buf, "collect `%s'", argv[i]);
	  perror (buf);
	  exit (-1);
	}

      switch (code = process (fp))
	{
	case OK:
	  break;

	case BAD_MAGIC:
	  fprintf (stderr, "file `%s' has a bad magic number for collect\n",
		   argv[i]);
	  exit (-1);

	case NO_NAMELIST:
	  fprintf (stderr, "file `%s' has a no namelist for collect\n",
		   argv[i]);
	  exit (-1);

	case RANDOM_ERROR:
	  fprintf (stderr, "random error while processing file `%s':\n",
		   argv[i]);
	  perror ("collect");
	  exit (-1);

	case FOPEN_ERROR:
	  fprintf (stderr, "fopen(3S) error while processing file `%s' in collect\n",
		   argv[i]);
	  exit (-1);

	case FREAD_ERROR:
	  fprintf (stderr, "fread(3S) error while processing file `%s' in collect\n",
		   argv[i]);
	  exit (-1);

	case FWRITE_ERROR:
	  fprintf (stderr, "fwrite(3S) error while processing file `%s' in collect\n",
		   argv[i]);
	  exit (-1);

	default:
	  abort ();
	}

      fclose (fp);

    }

  switch (code = coalesce ())
    {
    case OK:
      fclose (outfile);
      exit (0);
    case FREAD_ERROR:
      perror ("fread(3S) failed in collect, at end");
      break;
    case FWRITE_ERROR:
      perror ("fwrite(3S) failed in collect, at end");
      break;
    case FOPEN_ERROR:
      perror ("fopen(3S) failed in collect, at end");
      break;
    case RANDOM_ERROR:
      fprintf (stderr, "random error in collect, at end");
      break;
    }
  exit (-1);
}

/* Figure out the type of file we need to process.
   Currently, only .o and .a formats are acceptable.  */
enum error_code
process (fp)
     FILE *fp;
{
  struct stat file_stat;
  union
    {
      char ar_form[SARMAG];
      struct exec a_out_form;
    } header;
  int size;

  if (fstat (fp->_file, &file_stat))
    return RANDOM_ERROR;

  size = file_stat.st_size;

  if (fread (header.ar_form, SARMAG, 1, fp) < 1)
    return RANDOM_ERROR;
  if (strncmp (ARMAG, header.ar_form, SARMAG))
    {
      fseek (fp, 0, 0);
      if (fread (&header.a_out_form, sizeof (struct exec), 1, fp) < 1)
	return RANDOM_ERROR;
      if (N_BADMAG (header.a_out_form))
	return BAD_MAGIC;
      return process_o (fp, &header.a_out_form, size);
    }
  return process_a (fp);
}

enum error_code
process_o (fp, header, size)
     FILE *fp;
     struct exec *header;
     int size;
{
  int symoff, symend;
#ifndef hp9000s300
  struct nlist *nelem, *nelems, *nend;
  char *strtab;
#else
  struct nlist_ *nelem, *nelems, *nend;
#endif /* hp9000s300 */
  int to_find = 2;

  if (N_BADMAG (*header))
    return BAD_MAGIC;

#ifndef hp9000s300
  symoff = N_SYMOFF (*header);
  symend = N_STROFF (*header);
#else
  symoff = LESYM_OFFSET (*header);
  symend = DNTT_OFFSET (*header);
#endif /* hp9000s300 */
  if (symoff == symend)
    return NO_NAMELIST;
  fseek (fp, symoff - sizeof (struct exec), 1);
#ifndef hp9000s300
  nelems = (struct nlist *)alloca (symend - symoff);
#else
  nelems = (struct nlist_ *)alloca (symend - symoff);
#endif /* hp9000s300 */
  if (fread (nelems, sizeof (char), symend - symoff, fp) < symend - symoff)
    return FREAD_ERROR;

#ifndef hp9000s300
  strtab = (char *)alloca ((char *)size - (char *)symend);
  if (fread (strtab, sizeof (char), (char *)size - (char *)symend, fp)
      < ((char *)size - (char *)symend) * sizeof (char))
    return FREAD_ERROR;

  nend = (struct nlist *)((char *)nelems + symend - symoff);
  for (nelem = nelems; nelem < nend; nelem++)
#else
  nend = (struct nlist_ *)((char *)nelems + symend - symoff);
  for (nelem = nelems; nelem < nend; )
#endif /* hp9000s300 */
    {
#ifndef hp9000s300
      int strindex = nelem->n_un.n_strx;
#else
      int symlen = nelem->n_length;
      char p[255];
      memcpy(p, (char *) (++nelem), symlen);
      p[symlen]='\0';
     
      /* printf("'%s'\n",p);   */
#endif /* hp9000s300 */

#ifndef hp9000s300
      if (strindex)
#else
      nelem   = (struct nlist_ *)((char *)nelem + symlen);
      
      if (symlen)
#endif /* hp9000s300 */
	{
#ifndef hp9000s300
	  char *p = strtab+strindex;
#endif /* hp9000s300 */

	  if (! strncmp ("__GLOBAL_$", p, 10))
	    {
	      if (p[10] == 'I')
		{
		  ASM_OUTPUT_LABELREF (outfile, p+1);
		}
	      else
		{
		  struct dtor_list_elem *new = (struct dtor_list_elem *)xmalloc (sizeof (struct dtor_list_elem));
		  new->next = dtor_chain;
		  new->name = (char *)xmalloc (strlen (p));
		  strcpy (new->name, p+1);
		  dtor_chain = new;
		}
	      if (--to_find == 0)
		break;
	    }
	}
    }
  return OK;
}

enum error_code
process_a (fp)
     FILE *fp;
{
  struct ar_hdr header;
  struct exec exec_header;
  int size;
  enum error_code code;

  while (! feof (fp))
    {
      char c;
#ifdef hp9000s300
      int curpos;
#endif /* hp9000s300 */

      if (fread (&header, sizeof (struct ar_hdr), 1, fp) < 1)
	return RANDOM_ERROR;

      size = atoi (header.ar_size);
#ifdef hp9000s300
      curpos = ftell(fp);
#endif /* hp9000s300 */

#ifndef hp9000s300
      if (fread (&exec_header, sizeof (struct exec), 1, fp) < 1)
	return RANDOM_ERROR;
#else
      /* if the name starts with /, it's an index file */
      if (header.ar_name[0] != '/') {
      
        if (fread (&exec_header, sizeof (struct exec), 1, fp) < 1)
	  return RANDOM_ERROR;
#endif /* hp9000s300 */

      code = process_o (fp, &exec_header, size);
      if (code != OK)
	return code;
#ifdef hp9000s300
      } 
      
      if (fseek(fp,(long) curpos + size,0))
	return RANDOM_ERROR;
#endif /* hp9000s300 */
      if ((c = getc (fp)) == '\n')
	;
      else
	ungetc (c, fp);
      c = getc (fp);
      if (c != EOF)
	ungetc (c, fp);
    }
  return OK;
}

enum error_code
coalesce ()
{
  int dtor_offset;

  /* NULL-terminate the list of constructors.  */
  ASM_OUTPUT_INT (outfile, 0);

  /* Now layout the destructors.  */
  fprintf (outfile, "%s\n", DATA_SECTION_ASM_OP);
  ASM_GLOBALIZE_LABEL (outfile, DTOR_TABLE_NAME);
  ASM_OUTPUT_LABEL (outfile, DTOR_TABLE_NAME);
  if (dtor_chain)
    {
      ASM_OUTPUT_PTR_INT_SUM (outfile, DTOR_TABLE_NAME, sizeof (int));
      dtor_offset = 3 * sizeof (int);
      while (dtor_chain)
	{
	  if (dtor_chain->next)
	    ASM_OUTPUT_PTR_INT_SUM (outfile, DTOR_TABLE_NAME, dtor_offset);
	  else
	    ASM_OUTPUT_INT (outfile, 0);
	  dtor_offset += 2 * sizeof (int);

	  ASM_OUTPUT_LABELREF (outfile, dtor_chain->name);
	  dtor_chain = dtor_chain->next;
	}
    }
  ASM_OUTPUT_INT (outfile, 0);

  fclose (outfile);
  return OK;
}

/* Output to FILE a reference to the assembler name of a C-level name NAME.
   If NAME starts with a *, the rest of NAME is output verbatim.
   Otherwise NAME is transformed in an implementation-defined way
   (usually by the addition of an underscore).
   Many macros in the tm file are defined to call this function.

   Swiped from `varasm.c'  */

void
assemble_name (file, name)
     FILE *file;
     char *name;
{
  if (name[0] == '*')
    fputs (&name[1], file);
  else
    fprintf (file, "_%s", name);
}

int
xmalloc (size)
     int size;
{
  int result = malloc (size);
  if (! result)
    {
      fprintf (stderr, "Virtual memory exhausted\n");
      exit (-1);
    }
  return result;
}
