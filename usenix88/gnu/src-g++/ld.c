/* Linker `ld' for GNU
   Copyright (C) 1988 Free Software Foundation, Inc.
   Hacked by Michael Tiemann (tiemann@mcc.com)

		       NO WARRANTY

  BECAUSE THIS PROGRAM IS LICENSED FREE OF CHARGE, WE PROVIDE ABSOLUTELY
NO WARRANTY, TO THE EXTENT PERMITTED BY APPLICABLE STATE LAW.  EXCEPT
WHEN OTHERWISE STATED IN WRITING, FREE SOFTWARE FOUNDATION, INC,
RICHARD M. STALLMAN AND/OR OTHER PARTIES PROVIDE THIS PROGRAM "AS IS"
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY
AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
CORRECTION.

 IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW WILL RICHARD M.
STALLMAN, THE FREE SOFTWARE FOUNDATION, INC., AND/OR ANY OTHER PARTY
WHO MAY MODIFY AND REDISTRIBUTE THIS PROGRAM AS PERMITTED BELOW, BE
LIABLE TO YOU FOR DAMAGES, INCLUDING ANY LOST PROFITS, LOST MONIES, OR
OTHER SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE
USE OR INABILITY TO USE (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR
DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY THIRD PARTIES OR
A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) THIS
PROGRAM, EVEN IF YOU HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES, OR FOR ANY CLAIM BY ANY OTHER PARTY.

		GENERAL PUBLIC LICENSE TO COPY

  1. You may copy and distribute verbatim copies of this source file
as you receive it, in any medium, provided that you conspicuously
and appropriately publish on each copy a valid copyright notice
"Copyright (C) 1988 Free Software Foundation, Inc.", and include
following the copyright notice a verbatim copy of the above disclaimer
of warranty and of this License.

  2. You may modify your copy or copies of this source file or
any portion of it, and copy and distribute such modifications under
the terms of Paragraph 1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating
    that you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish,
    that in whole or in part contains or is a derivative of this
    program or any part thereof, to be licensed at no charge to all
    third parties on terms identical to those contained in this
    License Agreement (except that you may choose to grant more extensive
    warranty protection to some or all third parties, at your option).

    c) You may charge a distribution fee for the physical act of
    transferring a copy, and you may at your option offer warranty
    protection in exchange for a fee.

Mere aggregation of another unrelated program with this program (or its
derivative) on a volume of a storage or distribution medium does not bring
the other program under the scope of these terms.

  3. You may copy and distribute this program (or a portion or derivative
of it, under Paragraph 2) in object code or executable form under the terms
of Paragraphs 1 and 2 above provided that you also do one of the following:

    a) accompany it with the complete corresponding machine-readable
    source code, which must be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    b) accompany it with a written offer, valid for at least three
    years, to give any third party free (except for a nominal
    shipping charge) a complete machine-readable copy of the
    corresponding source code, to be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    c) accompany it with the information you received as to where the
    corresponding source code may be obtained.  (This alternative is
    allowed only for noncommercial distribution and only if you
    received the program in object code or executable form alone.)

For an executable file, complete source code means all the source code for
all modules it contains; but, as a special exception, it need not include
source code for modules which are standard libraries that accompany the
operating system on which the executable file runs.

  4. You may not copy, sublicense, distribute or transfer this program
except as expressly provided under this License Agreement.  Any attempt
otherwise to copy, sublicense, distribute or transfer this program is void and
your rights to use the program under this License agreement shall be
automatically terminated.  However, parties who have received computer
software programs from you with this License Agreement will not have
their licenses terminated so long as such parties remain in full compliance.

  5. If you wish to incorporate parts of this program into other free
programs whose distribution conditions are different, write to the Free
Software Foundation at 675 Mass Ave, Cambridge, MA 02139.  We have not yet
worked out a simple rule that can be stated here, but we will often permit
this.  We will be guided by the two goals of preserving the free status of
all derivatives of our free software and of promoting the sharing and reuse of
software.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

/* Written by Richard Stallman with some help from Eric Albert.  */

/* define this if on a system 
 where the names etext, edata and end should not have underscores.  */
/* #define nounderscore 1 */

#include <a.out.h>
#include <ar.h>
#include <stdio.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/stat.h>
#include "symseg.h"
#include <sys/file.h>
#include "version.c"

#ifndef SEGSIZ
#ifdef vax
#define PAGSIZ 1024
#define SEGSIZ PAGSIZ

#define	N_PAGSIZ(x) PAGSIZ
#define	N_SEGSIZ(x) SEGSIZ
#endif vax
#endif

#ifndef N_TXTADDR
#ifdef vax
#define	TXTRELOC	0
#define N_TXTADDR(x) TXTRELOC
#endif vax

#define N_DATADDR(x) \
	(((x).a_magic==OMAGIC)? (N_TXTADDR(x)+(x).a_text) \
	: (N_SEGSIZ(x)+((N_TXTADDR(x)+(x).a_text-1) & ~(N_SEGSIZ(x)-1))))

#define N_BSSADDR(x)  (N_DATADDR(x)+(x).a_data)
#endif

#define min(a,b) ((a) < (b) ? (a) : (b))

/* Size of a page; obtained from the operating system.  */

int page_size;

/* Symbol table */

/* Global symbol data is recorded in these structures,
   one for each global symbol.
   They are found via hashing in 'symtab', which points to a vector of buckets.
   Each bucket is a chain of these structures through the link field.  */

typedef
  struct glosym
    {
      /* Pointer to next symbol in this symbol's hash bucket.  */
      struct glosym *link;
      /* Name of this symbol.  */
      char *name;
      /* Value of this symbol as a global symbol.  */
      long value;
      /* Chain of external 'nlist's in files for this symbol, both defs and refs.  */
      struct nlist *refs;
      /* Nonzero means definitions of this symbol as common have been seen,
	 and the value here is the largest size specified by any of them.  */
      int max_common_size;
      /* For relocatable_output, records the index of this global sym in the
	 symbol table to be written, with the first global sym given index 0.  */
      int def_count;
      /* Nonzero means a definition of this global symbol is known to exist.
	 Library members should not be loaded on its account.  */
      char defined;
      /* Nonzero means a reference to this global symbol has been seen
	 in a file that is surely being loaded.
	 A value higher than 1 is the n_type code for the symbol's definition.  */
      char referenced;
      /* Nonzero means print a message at all refs or defs of this symbol */
      char trace;
    }
  symbol;

/* Number of buckets in symbol hash table */
#define	TABSIZE	1009

/* The symbol hash table: a vector of TABSIZE pointers to struct glosym. */
symbol *symtab[TABSIZE];

/* Number of symbols in symbol hash table. */
int num_hash_tab_syms = 0;

/* Count the number of nlist entries that are for local symbols.
   This count and the three following counts
   are incremented as as symbols are entered in the symbol table.  */
int local_sym_count;

/* Count number of nlist entries that are for local symbols
   whose names don't start with L. */
int non_L_local_sym_count;

/* Count the number of nlist entries for debugger info.  */
int debugger_sym_count;

/* Count the number of global symbols referenced and not defined.  */
int undefined_global_sym_count;

/* Count the number of defined global symbols.
   Each symbol is counted only once
   regardless of how many different nlist entries refer to it,
   since the output file will need only one nlist entry for it.
   This count is computed by `digest_symbols';
   it is undefined while symbols are being loaded. */
int defined_global_sym_count;

/* Total number of symbols to be written in the output file.
   Computed by digest_symbols from the variables above.  */
int nsyms;


/* Nonzero means ptr to symbol entry for symbol to use as start addr.
   -e sets this.  */
symbol *entry_symbol;

symbol *edata_symbol;   /* the symbol _edata */
symbol *etext_symbol;   /* the symbol _etext */
symbol *end_symbol;	/* the symbol _end */

/* Each input file, and each library member ("subfile") being loaded,
   has a `file_entry' structure for it.

   For files specified by command args, these are contained in the vector
   which `file_table' points to.

   For library members, they are dynamically allocated,
   and chained through the `chain' field.
   The chain is found in the `subfiles' field of the `file_entry'.
   The `file_entry' objects for the members have `superfile' fields pointing
   to the one for the library.  */

struct file_entry {
  /* Name of this file.  */
  char *filename;
  /* Name to use for the symbol giving address of text start */
  /* Usually the same as filename, but for a file spec'd with -l
     this is the -l switch itself rather than the filename.  */
  char *local_sym_name;

  /* Describe the layout of the contents of the file */

  /* The file's a.out header.  */
  struct exec header;
  /* Offset in file of GDB symbol segment, or 0 if there is none.  */
  int symseg_offset;

  /* Describe data from the file loaded into core */

  /* Symbol table of the file.  */
  struct nlist *symbols;
  /* Size in bytes of string table.  */
  int string_size;
  /* Pointer to the string table.
     The string table is not kept in core all the time,
     but when it is in core, its address is here.  */
  char *strings;

  /* Next two used only if `relocatable_output' */

  /* Text reloc info saved by `write_text' for `coptxtrel'.  */
  struct relocation_info *textrel;
  /* Data reloc info saved by `write_data' for `copdatrel'.  */
  struct relocation_info *datarel;

  /* Relation of this file's segments to the output file */

  /* Start of this file's text seg in the output file core image.  */
  int text_start_address;
  /* Start of this file's data seg in the output file core image.  */
  int data_start_address;
  /* Start of this file's bss seg in the output file core image.  */
  int bss_start_address;
  /* Offset in bytes in the output file symbol table
     of the first local symbol for this file.  Set by `write_file_symbols'.  */
  int local_syms_offset;				   

  /* For library members only */

  /* For a library, points to chain of entries for the library members.  */
  struct file_entry *subfiles;
  /* For a library member, offset of the member within the archive.
     Zero for files that are not library members.  */
  int starting_offset;
  /* Size of contents of this file, if library member.  */
  int total_size;
  /* For library member, points to the library's own entry.  */
  struct file_entry *superfile;
  /* For library member, points to next entry for next member.  */
  struct file_entry *chain;

  /* 1 if file is a library. */
  char library_flag;

  /* 1 if file's header has been read into this structure.  */
  char header_read_flag;

  /* 1 means search a set of directories for this file.  */
  char search_dirs_flag;

  /* 1 means this is base file of incremental load.
     Do not load this file's text or data.
     Also default text_start to after this file's bss. */
  char just_syms_flag;
};

/* Vector of entries for input files specified by arguments.
   These are all the input files except for members of specified libraries.  */
struct file_entry *file_table;

/* Length of that vector.  */
int number_of_files;

/* When loading the text and data, we can avoid doing a close
   and another open between members of the same library.

   These two variables remember the file that is currently open.
   Both are zero if no file is open.

   See `each_file' and `file_close'.  */

struct file_entry *input_file;
int input_desc;

/* The name of the file to write; "a.out" by default.  */

char *output_filename;

/* Descriptor for writing that file with `mywrite'.  */

int outdesc;

/* Header for that file (filled in by `write_header').  */

struct exec outheader;

/* The following are computed by `digest_symbols'.  */

int text_size;		/* total size of text of all input files.  */
int data_size;		/* total size of data of all input files.  */
int bss_size;		/* total size of bss of all input files.  */
int text_reloc_size;	/* total size of text relocation of all input files.  */
int data_reloc_size;	/* total size of data relocation of all input files.  */

/* Amount of cleared space to leave between the text and data segments.  */

int text_pad;

/* Amount of bss segment to include as part of the data segment.  */

int data_pad;

/* Format of __.SYMDEF:
   First, a longword containing the size of the 'symdef' data that follows.
   Second, zero or more 'symdef' structures.
   Third, a word containing the length of symbol name strings.
   Fourth, zero or more symbol name strings (each followed by a zero).  */

struct symdef {
  int symbol_name_string_index;
  int library_member_offset;
};

/* Record most of the command options.  */

/* Address we assume the text section will be loaded at.
   We relocate symbols and text and data for this, but we do not
   write any padding in the output file for it.  */
int text_start;

/* Offset of default entry-pc within the text section.  */
int entry_offset;

/* Address we decide the data section will be loaded at.  */
int data_start;

/* `text-start' address is normally this much plus a page boundary.
   This is not a user option; it is fixed for each system.  */
int text_start_alignment;

/* Nonzero if -T was specified in the command line.
   This prevents text_start from being set later to default values.  */
int T_flag_specified;

/* Size to pad data section up to.
   We simply increase the size of the data section, padding with zeros,
   and reduce the size of the bss section to match.  */
int specified_data_size;

/* Magic number to use for the output file, set by switch.  */
int magic;

/* Nonzero means print names of input files as processed.  */
int trace_files;

/* Which symbols should be stripped (omitted from the output):
   none, all, or debugger symbols.  */
enum { STRIP_NONE, STRIP_ALL, STRIP_DEBUGGER } strip_symbols;

/* Which local symbols should be omitted:
   none, all, or those starting with L.
   This is irrelevant if STRIP_NONE.  */
enum { DISCARD_NONE, DISCARD_ALL, DISCARD_L } discard_locals;

/* 1 => write load map.  */
int write_map;

/* 1 => write relocation into output file so can re-input it later.  */
int relocatable_output;

/* 1 => assign space to common symbols even if `relocatable_output'.  */
int force_common_definition;

/* Standard directories to search for files specified by -l.  */
char *standard_search_dirs[] = {"/lib", "/usr/lib", "/usr/local/lib"};

/* Actual vector of directories to search;
   this contains those specified with -L plus the standard ones.  */
char **search_dirs;

/* Length of the vector `search_dirs'.  */
int n_search_dirs;

/* Nonzero means should make the output file executable when done.  */
/* Cleared by nonfatal errors.  */
int make_executable;

/* C++: Take care of global constructors and destructors.  */
void symtab_init_for_cplusplus ();

/* Nonzero if we are doing fancies for C++.  */
int cplusplus = 0;

/* Name for file containing (temporary) constructor and destructor
   information.  This file is in a.out.h format, alas.  */
#ifndef L_tmpnam
#define L_tmpnam 32
#endif
char ctor_and_dtor_filename[L_tmpnam];

/* Data for list of global constructors and destructors respectively.  */
symbol **ctor_vec, **dtor_vec;

/* Sizes of the respective vectors.  */
int ctor_size_max, dtor_size_max;

/* Current sizes of the respective vectors.  */
int ctor_size = 0, dtor_size = 0;

/* nonzero if we are looking for global constructors and
   destructors.  This starts out nonzero, and is cleared
   after the files have all been seen once.

   In the case of incremental loading, it is reset
   when loading symbol information from the executing
   program (the ctors and dtors have already been put in
   place), and set for each additional file which
   is added.  */
int looking_for_ctors_and_dtors = 0;

/* The symbols that crt0+.c expects to find in order to
   call the global constructors and destructors for the
   program.  */
symbol *ctor_list_symbol;
symbol *dtor_list_symbol;

/* 1 if the -A flag has been specified, 0 otherwise.  */
int A_flag_specified;

/* For incremental loading, this will be patched to point to
   the initial function of the code being incrementally loaded.  */
symbol *init_symbol;

/* For incremental loading, this is the head and tail of the
   new list of destructors to call.  */
symbol *head_dtor_symbol, *tail_dtor_symbol;

void layout_ctors_and_dtors ();

void digest_symbols ();
void print_symbols ();
void load_symbols ();
void decode_command ();
void list_undefined_symbols ();
void write_output ();
void write_header ();
void write_text ();
void write_data ();
void write_rel ();
void write_syms ();
void padfile ();
char *concat ();
symbol *getsym (), *getsym_soft ();
void symtab_init ();

main (argc, argv)
     char **argv;
     int argc;
{
  page_size = getpagesize ();

  /* Clear the cumulative info on the output file.  */

  text_size = 0;
  data_size = 0;
  bss_size = 0;
  text_reloc_size = 0;
  data_reloc_size = 0;

  data_pad = 0;
  text_pad = 0;

#ifdef sun
  outheader.a_machtype = M_68020;
#endif

  /* Initialize the data about options.  */

  specified_data_size = 0;
  strip_symbols = STRIP_NONE;
  trace_files = 0;
  discard_locals = DISCARD_NONE;
  entry_symbol = 0;
  write_map = 0;
  relocatable_output = 0;
  force_common_definition = 0;
  T_flag_specified = 0;
  magic = ZMAGIC;
  make_executable = 1;

  /* Initialize the cumulative counts of symbols.  */

  local_sym_count = 0;
  non_L_local_sym_count = 0;
  debugger_sym_count = 0;
  undefined_global_sym_count = 0;

  /* Completely decode ARGV.  */

  decode_command (argc, argv);

  /* Create the symbols `etext', `edata' and `end'.  */

  if (!relocatable_output)
    symtab_init ();

  if (cplusplus)
    symtab_init_for_cplusplus ();

  /* Determine whether to count the header as part of
     the text size, and initialize the text size accordingly.
     This depends on the kind of system and on the output format selected.  */

  outheader.a_magic = magic;
  text_size = sizeof (struct exec) - N_TXTOFF (outheader);
  if (text_size < 0)
    text_size = 0;
  entry_offset = text_size;

  if (!T_flag_specified && !relocatable_output)
    text_start = N_TXTADDR (outheader);

  /* The text-start address is normally this far past a page boundary.  */
  text_start_alignment = text_start % page_size;

  /* Load symbols of all input files.
     Also search all libraries and decide which library members to load.

     C++: this does not include the fake ctor and dtor file.  That file
     is created on entry to `digest_symbols'.  */

  load_symbols ();

  /* Compute where each file's sections go, and relocate symbols.  */

  digest_symbols ();

  /* Print error messages for any missing symbols.  */

  if (!relocatable_output)
    list_undefined_symbols (stderr);

  /* Print a map, if requested.  */

  if (write_map) print_symbols (stdout);

  /* Write the output file.  */

  write_output ();

  if (cplusplus && unlink (ctor_and_dtor_filename))
    perror_file (&file_table[number_of_files-1]);

  return 0;
}

void decode_option ();

/* Analyze a command line argument.
   Return 0 if the argument is a filename.
   Return 1 if the argument is a option complete in itself.
   Return 2 if the argument is a option which uses an argument.

   Thus, the value is the number of consecutive arguments
   that are part of options.  */

int
classify_arg (arg)
     register char *arg;
{
  if (*arg != '-') return 0;
  switch (arg[1])
    {
    case 'A':
    case 'D':
    case 'e':
    case 'L':
    case 'l':
    case 'o':
    case 'T':
    case 'u':
    case 'y':
      if (arg[2])
	return 1;
      return 2;
    }

  return 1;
}

/* Process the command arguments,
   setting up file_table with an entry for each input file,
   and setting variables according to the options.  */

void
decode_command (argc, argv)
     char **argv;
     int argc;
{
  register int i;
  register struct file_entry *p;

  number_of_files = 1;		/* C++: one for ctors and dtors.  */
  output_filename = "a.out";

  n_search_dirs = 0;
  search_dirs = (char **) xmalloc (sizeof (char *));

  /* First compute number_of_files so we know how long to make file_table.  */
  /* Also process most options completely.  */

  for (i = 1; i < argc; i++)
    {
      register int code = classify_arg (argv[i]);
      if (code)
	{
	  if (i + code > argc)
	    fatal ("no argument following %s\n", argv[i]);

	  decode_option (argv[i], argv[i+1]);

	  if (argv[i][1] == 'l' || argv[i][1] == 'A')
	    number_of_files++;

	  i += code - 1;
	}
      else
	number_of_files++;
    }

  if (!number_of_files)
    fatal ("no input files", 0);

  p = file_table
    = (struct file_entry *) xmalloc (number_of_files * sizeof (struct file_entry));
  bzero (p, number_of_files * sizeof (struct file_entry));
  number_of_files -= 1;		/* C++: now forget until later.  */

  /* Now scan again and fill in file_table.  */
  /* All options except -A and -l are ignored here.  */

  for (i = 1; i < argc; i++)
    {
      register int code = classify_arg (argv[i]);

      if (code)
	{
	  char *string;
	  if (code == 2)
	    string = argv[i+1];
	  else
	    string = &argv[i][2];

	  if (argv[i][1] == 'A')
	    {
	      if (p != file_table)
		fatal ("-A specified before an input file other than the first");

	      p->filename = string;
	      p->local_sym_name = string;
	      p->just_syms_flag = 1;
	      A_flag_specified = 1;
	      p++;
	    }
	  if (argv[i][1] == 'l')
	    {
	      p->filename = concat ("lib", string, ".a");
	      p->local_sym_name = concat ("-l", string, "");
	      p->search_dirs_flag = 1;
	      p++;
	    }
	  i += code - 1;
	}
      else
	{
	  p->filename = argv[i];
	  p->local_sym_name = argv[i];
	  p++;
	}
    }

  /* Now check some option settings for consistency.  */

  if ((magic == ZMAGIC || magic == NMAGIC)
      && (text_start - text_start_alignment) & (page_size - 1))
    fatal ("-T argument not multiple of page size, with sharable output", 0);

  /* Append the standard search directories to the user-specified ones.  */
  {
    int n = sizeof standard_search_dirs / sizeof standard_search_dirs[0];
    n_search_dirs += n;
    search_dirs
      = (char **) xrealloc (search_dirs, n_search_dirs * sizeof (char *));
    bcopy (standard_search_dirs, &search_dirs[n_search_dirs - n],
	   n * sizeof (char *));
  }
}

/* Record an option and arrange to act on it later.
   ARG should be the following command argument,
   which may or may not be used by this option.

   The `l' and `A' options are ignored here since they actually
   specify input files.  */

void
decode_option (swt, arg)
     register char *swt, *arg;
{
  if (swt[2] != 0)
    arg = &swt[2];

  switch (swt[1])
    {
    case 'A':
      return;

    case 'D':
      specified_data_size = parse (arg, "%x", "invalid argument to -D");
      return;

    case 'd':
      force_common_definition = 1;
      return;

    case 'e':
      entry_symbol = getsym (arg);
      if (!entry_symbol->defined && !entry_symbol->referenced)
	undefined_global_sym_count++;
      entry_symbol->referenced = 1;
      return;

    case 'l':
      return;

    case 'L':
      n_search_dirs++;
      search_dirs
	= (char **) xrealloc (search_dirs, n_search_dirs * sizeof (char *));
      search_dirs[n_search_dirs - 1] = arg;
      return;
      
    case 'M':
      write_map = 1;
      return;

    case 'N':
      magic = OMAGIC;
      return;

    case 'n':
      magic = NMAGIC;
      return;

    case 'o':
      output_filename = arg;
      return;

    case 'r':
      relocatable_output = 1;
      magic = OMAGIC;
      text_start = 0;
      return;

    case 'S':
      strip_symbols = STRIP_DEBUGGER;
      return;

    case 's':
      strip_symbols = STRIP_ALL;
      return;

    case 'T':
      text_start = parse (arg, "%x", "invalid argument to -T");
      T_flag_specified = 1;
      return;

    case 't':
      trace_files = 1;
      return;

    case 'u':
      {
	register symbol *sp = getsym (arg);
	if (!sp->defined && !sp->referenced) undefined_global_sym_count++;
	sp->referenced = 1;
      }
      return;

    case 'X':
      discard_locals = DISCARD_L;
      return;

    case 'x':
      discard_locals = DISCARD_ALL;
      return;

    case 'y':
      {
	register symbol *sp = getsym (&swt[2]);
	sp->trace = 1;
      }
      return;

    case 'z':
      magic = ZMAGIC;
      return;

      /* C++ extension.  */
    case 'C':
      cplusplus = 1;
      looking_for_ctors_and_dtors = 1;
      return;

    default:
      fatal ("invalid command option `%s'", swt);
    }
}

/** Convenient functions for operating on one or all files being loaded.  */

/* Call FUNCTION on each input file entry.
   Do not call for entries for libraries;
   instead, call once for each library member that is being loaded.

   FUNCTION receives two arguments: the entry, and ARG.  */

void
each_file (function, arg)
     register void (*function)();
     register int arg;
{
  register int i;

  for (i = 0; i < number_of_files; i++)
    {
      register struct file_entry *entry = &file_table[i];
      if (entry->library_flag)
        {
	  register struct file_entry *subentry = entry->subfiles;
	  for (; subentry; subentry = subentry->chain)
	    (*function) (subentry, arg);
	}
      else
	(*function) (entry, arg);
    }
}

/* Like `each_file' but ignore files that were just for symbol definitions.  */

void
each_full_file (function, arg)
     register void (*function)();
     register int arg;
{
  register int i;

  for (i = 0; i < number_of_files; i++)
    {
      register struct file_entry *entry = &file_table[i];
      if (entry->just_syms_flag)
	continue;
      if (entry->library_flag)
        {
	  register struct file_entry *subentry = entry->subfiles;
	  for (; subentry; subentry = subentry->chain)
	    (*function) (subentry, arg);
	}
      else
	(*function) (entry, arg);
    }
}

/* Close the input file that is now open.  */

void
file_close ()
{
  close (input_desc);
  input_desc = 0;
  input_file = 0;
}

/* Open the input file specified by 'entry', and return a descriptor.
   The open file is remembered; if the same file is opened twice in a row,
   a new open is not actually done.  */

int
file_open (entry)
     register struct file_entry *entry;
{
  register int desc;

  if (entry->superfile)
    return file_open (entry->superfile);

  if (entry == input_file)
    return input_desc;

  if (input_file) file_close ();

  if (entry->search_dirs_flag)
    {
      register char **p = search_dirs;
      int i;

      for (i = 0; i < n_search_dirs; i++)
	{
	  register char *string
	    = concat (search_dirs[i], "/", entry->filename);
	  desc = open (string, O_RDONLY, 0);
	  if (desc > 0)
	    {
	      entry->filename = string;
	      entry->search_dirs_flag = 0;
	      break;
	    }
	  free (string);
	}
    }
  else
    desc = open (entry->filename, O_RDONLY, 0);

  if (desc > 0)
    {
      input_file = entry;
      input_desc = desc;
      return desc;
    }

  perror_file (entry);
}

/* Print the filename of ENTRY on OUTFILE (a stdio stream),
   and then a newline.  */

prline_file_name (entry, outfile)
     struct file_entry *entry;
     FILE *outfile;
{
  print_file_name (entry, outfile);
  fprintf (outfile, "\n");
}

/* Print the filename of ENTRY on OUTFILE (a stdio stream).  */

print_file_name (entry, outfile)
     struct file_entry *entry;
     FILE *outfile;
{
  if (entry->superfile)
    {
      print_file_name (entry->superfile, outfile);
      fprintf (outfile, "(%s)", entry->filename);
    }
  else
    fprintf (outfile, "%s", entry->filename);
}

/* Medium-level input routines for rel files.  */

/* Read a file's header into the proper place in the file_entry.
   DESC is the descriptor on which the file is open.
   ENTRY is the file's entry.  */

void
read_header (desc, entry)
     int desc;
     register struct file_entry *entry;
{
  register int len;
  struct exec *loc = &entry->header;

  lseek (desc, entry->starting_offset, 0);
  len = read (desc, loc, sizeof (struct exec));
  if (len != sizeof (struct exec))
    fatal_with_file ("failure reading header of ", entry);
  if (N_BADMAG (*loc))
    fatal_with_file ("bad magic number in ", entry);

  entry->header_read_flag = 1;
}

/* Read the symbols of file ENTRY into core.
   Assume it is already open, on descriptor DESC.
   Also read the length of the string table, which follows the symbol table,
   but don't read the contents of the string table.  */

void
read_entry_symbols (desc, entry)
     struct file_entry *entry;
     int desc;
{
  int str_size;

  if (!entry->header_read_flag)
    read_header (desc, entry);

  entry->symbols = (struct nlist *) xmalloc (entry->header.a_syms);

  lseek (desc, N_SYMOFF (entry->header) + entry->starting_offset, 0);
  if (entry->header.a_syms != read (desc, entry->symbols, entry->header.a_syms))
    fatal_with_file ("premature end of file in symbols of ", entry);

  lseek (desc, N_STROFF (entry->header) + entry->starting_offset, 0);
  if (sizeof str_size != read (desc, &str_size, sizeof str_size))
    fatal_with_file ("bad string table size in ", entry);

  entry->string_size = str_size;
}

/* Read the string table of file ENTRY into core.
   Assume it is already open, on descriptor DESC.
   Also record whether a GDB symbol segment follows the string table.  */

void
read_entry_strings (desc, entry)
     struct file_entry *entry;
     int desc;
{
  int buffer;

  if (!entry->header_read_flag)
    read_header (desc, entry);

  lseek (desc, N_STROFF (entry->header) + entry->starting_offset, 0);
  if (entry->string_size != read (desc, entry->strings, entry->string_size))
    fatal_with_file ("premature end of file in strings of ", entry);

  /* While we are here, see if the file has a symbol segment at the end.
     For a separate file, just try reading some more.
     For a library member, compare current pos against total size.  */
  if (entry->superfile)
    {
      if (entry->total_size == N_STROFF (entry->header) + entry->string_size)
	return;
    }
  else
    {
      buffer = read (desc, &buffer, sizeof buffer);
      if (buffer == 0)
	return;
      if (buffer != sizeof buffer)
	fatal_with_file ("premature end of file in GDB symbol segment of ", entry);
    }

  entry->symseg_offset = N_STROFF (entry->header) + entry->string_size;
}

/* Read in the symbols of all input files.  */

void read_file_symbols (), read_entry_symbols (), read_entry_strings ();
void enter_file_symbols (), enter_global_ref (), search_library ();

void
load_symbols ()
{
  register int i;

  if (trace_files) fprintf (stderr, "Loading symbols:\n\n");

  for (i = 0; i < number_of_files; i++)
    {
      register struct file_entry *entry = &file_table[i];
      if (cplusplus && A_flag_specified)
	looking_for_ctors_and_dtors = i != 0;
      read_file_symbols (entry);
    }

  if (trace_files) fprintf (stderr, "\n");
}

/* If ENTRY is a rel file, read its symbol and string sections into core.
   If it is a library, search it and load the appropriate members
   (which means calling this function recursively on those members).  */

void
read_file_symbols (entry)
     register struct file_entry *entry;
{
  register int desc;
  register int len;
  int magicnum;

  desc = file_open (entry);

  len = read (desc, &magicnum, sizeof magicnum);
  if (len != sizeof magicnum)
    fatal_with_file ("failure reading header of ", entry);

  if (!N_BADMAG (*((struct exec *)&magicnum)))
    {
      read_entry_symbols (desc, entry);
      entry->strings = (char *) alloca (entry->string_size);
      read_entry_strings (desc, entry);
      enter_file_symbols (entry);
      entry->strings = 0;
    }
  else
    {
      char armag[SARMAG];

      lseek (desc, 0, 0);
      if (SARMAG != read (desc, armag, SARMAG) || strncmp (armag, ARMAG, SARMAG))
	fatal_with_file ("malformed input file (not rel or archive) ", entry);
      entry->library_flag = 1;
      search_library (desc, entry);
    }

  file_close ();
}

/* Enter the external symbol defs and refs of ENTRY in the hash table.  */

void
enter_file_symbols (entry)
     struct file_entry *entry;
{
   register struct nlist *p, *end = entry->symbols + entry->header.a_syms / sizeof (struct nlist);

  if (trace_files) prline_file_name (entry, stderr);

  for (p = entry->symbols; p < end; p++)
    if (p->n_type & N_EXT)
      enter_global_ref (p, p->n_un.n_strx + entry->strings, entry);
    else if (p->n_un.n_strx && !(p->n_type & (N_STAB | N_EXT)))
      {
	if ((p->n_un.n_strx + entry->strings)[0] != 'L')
	  non_L_local_sym_count++;
	local_sym_count++;
      }
    else debugger_sym_count++;

   /* Count one for the local symbol that we generate,
      whose name is the file's name (usually) and whose address
      is the start of the file's text.  */

  local_sym_count++;
  non_L_local_sym_count++;
}

/* Enter one global symbol in the hash table.
   NLIST_P points to the `struct nlist' read from the file
   that describes the global symbol.  NAME is the symbol's name.
   ENTRY is the file entry for the file the symbol comes from.

   The `struct nlist' is modified by placing it on a chain of
   all such structs that refer to the same global symbol.
   This chain starts in the `refs' field of the symbol table entry
   and is chained through the `n_name'.

   C++: if this symbol is a global constructor or destructor,
   make a note of it.  */

void
enter_global_ref (nlist_p, name, entry)
     register struct nlist *nlist_p;
     char *name;
     struct file_entry *entry;
{
  register symbol *sp = getsym (name);
  register int type = nlist_p->n_type;
  int oldref = sp->referenced;
  int olddef = sp->defined;

  nlist_p->n_un.n_name = (char *) sp->refs;
  sp->refs = nlist_p;

  sp->referenced = 1;

  if (type != (N_UNDF | N_EXT) || nlist_p->n_value)
    {
#ifdef nounderscore
#define CTOR_OR_DTOR_PREFIX "_GLOBAL_$"
#else
#define CTOR_OR_DTOR_PREFIX "__GLOBAL_$"
#endif

      sp->defined = 1;
      if (oldref && !olddef) undefined_global_sym_count--;
      /* If this is a common definition, keep track of largest
	 common definition seen for this symbol.  */
      if (type == (N_UNDF | N_EXT)
	  && sp->max_common_size < nlist_p->n_value)
	sp->max_common_size = nlist_p->n_value;

      /* C++: global constructors and destructors are only
	 ever defined.  They are never multiply defined,
	 unless a file is duplicated on the command line.
	 In that case, other errors will be reported.  */
      if (looking_for_ctors_and_dtors
	  && ! strncmp (sp->name, CTOR_OR_DTOR_PREFIX,
			sizeof (CTOR_OR_DTOR_PREFIX)-1))
	{
	  if (sp->name[sizeof (CTOR_OR_DTOR_PREFIX)-1] == 'I')
	    {
	      if (ctor_size >= ctor_size_max)
		{
		  ctor_size_max <<= 1;
		  ctor_vec = (symbol **)xrealloc (ctor_vec, ctor_size_max);
		}
	      ctor_vec[ctor_size++] = sp;
	    }
	  else
	    {
	      if (dtor_size >= dtor_size_max)
		{
		  dtor_size_max <<= 1;
		  dtor_vec = (symbol **)xrealloc (dtor_vec, dtor_size_max);
		}
	      dtor_vec[dtor_size++] = sp;
	    }
	}
    }
  else
    if (!oldref) undefined_global_sym_count++;

  if (sp == end_symbol && entry->just_syms_flag && !T_flag_specified)
    text_start = nlist_p->n_value;

  if (sp->trace)
    {
      register char *reftype;
      switch (type & N_TYPE)
	{
	case N_UNDF:
	  if (nlist_p->n_value)
	    reftype = "defined as common";
	  else reftype = "referenced";
	  break;

	case N_ABS:
	  reftype = "defined as absolute";
	  break;

	case N_TEXT:
	  reftype = "defined in text section";
	  break;

	case N_DATA:
	  reftype = "defined in data section";
	  break;

	case N_BSS:
	  reftype = "defined in BSS section";
	  break;
	}

      fprintf (stderr, "symbol %s %s in ", sp->name, reftype);
      print_file_name (entry, stderr);
      fprintf (stderr, "\n");
    }
}

/* Searching libraries */

struct file_entry *decode_library_subfile ();
void linear_library (), symdef_library ();

/* Search the library ENTRY, already open on descriptor DESC.
   This means deciding which library members to load,
   making a chain of `struct file_entry' for those members,
   and entering their global symbols in the hash table.  */

void
search_library (desc, entry)
     int desc;
     struct file_entry *entry;
{
  int member_length;
  register char *name;
  register struct file_entry *subentry;

  if (!undefined_global_sym_count) return;

  /* Examine its first member, which starts SARMAG bytes in.  */
  subentry = decode_library_subfile (desc, entry, SARMAG, &member_length);
  if (!subentry) return;

  name = subentry->filename;
  free (subentry);

  /* Search via __.SYMDEF if that exists, else linearly.  */

  if (!strcmp (name, "__.SYMDEF"))
    symdef_library (desc, entry, member_length);
  else
    linear_library (desc, entry);
}

/* Construct and return a file_entry for a library member.
   The library's file_entry is library_entry, and the library is open on DESC.
   SUBFILE_OFFSET is the byte index in the library of this member's header.
   We store the length of the member into *LENGTH_LOC.  */

struct file_entry *
decode_library_subfile (desc, library_entry, subfile_offset, length_loc)
     int desc;
     struct file_entry *library_entry;
     int subfile_offset;
     int *length_loc;
{
  int bytes_read;
  register int namelen;
  int member_length;
  register char *name;
  struct ar_hdr hdr1;
  register struct file_entry *subentry;

  lseek (desc, subfile_offset, 0);

  bytes_read = read (desc, &hdr1, sizeof hdr1);
  if (!bytes_read)
    return 0;		/* end of archive */

  if (sizeof hdr1 != bytes_read)
    fatal_with_file ("malformed library archive ", library_entry);

  if (sscanf (hdr1.ar_size, "%d", &member_length) != 1)
    fatal_with_file ("malformatted header of archive member in ", library_entry);

  subentry = (struct file_entry *) xmalloc (sizeof (struct file_entry));
  bzero (subentry, sizeof (struct file_entry));

  for (namelen = 0;
       namelen < sizeof hdr1.ar_name
       && hdr1.ar_name[namelen] != 0 && hdr1.ar_name[namelen] != ' ';
       namelen++);

  name = (char *) xmalloc (namelen+1);
  strncpy (name, hdr1.ar_name, namelen);
  name[namelen] = 0;

  subentry->filename = name;
  subentry->local_sym_name = name;
  subentry->symbols = 0;
  subentry->strings = 0;
  subentry->subfiles = 0;
  subentry->starting_offset = subfile_offset + sizeof hdr1;
  subentry->superfile = library_entry;
  subentry->library_flag = 0;
  subentry->header_read_flag = 0;
  subentry->just_syms_flag = 0;
  subentry->chain = 0;
  subentry->total_size = member_length;

  (*length_loc) = member_length;

  return subentry;
}

/* Search a library that has a __.SYMDEF member.
   DESC is a descriptor on which the library is open.
     The file pointer is assumed to point at the __.SYMDEF data.
   ENTRY is the library's file_entry.
   MEMBER_LENGTH is the length of the __.SYMDEF data.  */

void
symdef_library (desc, entry, member_length)
     int desc;
     struct file_entry *entry;
     int member_length;
{
  int *symdef_data = (int *) xmalloc (member_length);
  register struct symdef *symdef_base;
  char *sym_name_base;
  int number_of_symdefs;
  int length_of_strings;
  int not_finished;
  int bytes_read;
  register int i;
  struct file_entry *prev = 0;
  int prev_offset = 0;

  bytes_read = read (desc, symdef_data, member_length);
  if (bytes_read != member_length)
    fatal_with_file ("malformatted __.SYMDEF in ", entry);

  number_of_symdefs = *symdef_data / sizeof (struct symdef);
  if (number_of_symdefs < 0 ||
       number_of_symdefs * sizeof (struct symdef) + 2 * sizeof (int) >= member_length)
    fatal_with_file ("malformatted __.SYMDEF in ", entry);

  symdef_base = (struct symdef *) (symdef_data + 1);
  length_of_strings = *(int *) (symdef_base + number_of_symdefs);

  if (length_of_strings < 0
      || number_of_symdefs * sizeof (struct symdef) + length_of_strings
	  + 2 * sizeof (int) != member_length)
    fatal_with_file ("malformatted __.SYMDEF in ", entry);

  sym_name_base = sizeof (int) + (char *) (symdef_base + number_of_symdefs);

  /* Check all the string indexes for validity.  */

  for (i = 0; i < number_of_symdefs; i++)
    {
      register int index = symdef_base[i].symbol_name_string_index;
      if (index < 0 || index >= length_of_strings
	  || (index && *(sym_name_base + index - 1)))
	fatal_with_file ("malformatted __.SYMDEF in ", entry);
    }

  /* Search the symdef data for members to load.
     Do this until one whole pass finds nothing to load.  */

  not_finished = 1;
  while (not_finished)
    {
      not_finished = 0;

      /* Scan all the symbols mentioned in the symdef for ones that we need.
	 Load the library members that contain such symbols.  */

      for (i = 0; i < number_of_symdefs && undefined_global_sym_count; i++)
	if (symdef_base[i].symbol_name_string_index >= 0)
	  {
	    register symbol *sp;

	    sp = getsym_soft (sym_name_base
			      + symdef_base[i].symbol_name_string_index);

	    /* If we find a symbol that appears to be needed, think carefully
	       about the archive member that the symbol is in.  */

	    if (sp && sp->referenced && !sp->defined)
	      {
		int junk;
		register int j;
		register int offset = symdef_base[i].library_member_offset;
		struct file_entry *subentry;

		/* Don't think carefully about any archive member
		   more than once in a given pass.  */

		if (prev_offset == offset)
		  continue;
		prev_offset = offset;

		/* Read the symbol table of the archive member.  */

		subentry = decode_library_subfile (desc, entry, offset, &junk);
		read_entry_symbols (desc, subentry);
		subentry->strings = (char *) malloc (subentry->string_size);
		read_entry_strings (desc, subentry);

		/* Now scan the symbol table and decide whether to load.  */

		if (!subfile_wanted_p (subentry))
		  {
		    free (subentry->symbols);
		    free (subentry);
		  }
		else
		  {
		    /* This member is needed; load it.
		       Since we are loading something on this pass,
		       we must make another pass through the symdef data.  */

		    not_finished = 1;

		    enter_file_symbols (subentry);

		    if (prev)
		      prev->chain = subentry;
		    else entry->subfiles = subentry;
		    prev = subentry;

		    /* Clear out this member's symbols from the symdef data
		       so that following passes won't waste time on them.  */

		    for (j = 0; j < number_of_symdefs; j++)
		      {
			if (symdef_base[j].library_member_offset == offset)
			  symdef_base[j].symbol_name_string_index = -1;
		      }
		  }

		/* We'll read the strings again if we need them again.  */
		free (subentry->strings);
		subentry->strings = 0;
	      }
	  }
    }

  free (symdef_data);
}

/* Search a library that has no __.SYMDEF.
   ENTRY is the library's file_entry.
   DESC is the descriptor it is open on.  */

void
linear_library (desc, entry)
     int desc;
     struct file_entry *entry;
{
  register struct file_entry *prev = 0;
  register int this_subfile_offset = SARMAG;

  while (undefined_global_sym_count)
    {
      int member_length;
      register struct file_entry *subentry;

      subentry = decode_library_subfile (desc, entry, this_subfile_offset, &member_length);

      if (!subentry) return;

      read_entry_symbols (desc, subentry);
      subentry->strings = (char *) alloca (subentry->string_size);
      read_entry_strings (desc, subentry);

      if (!subfile_wanted_p (subentry))
	{
	  free (subentry->symbols);
	  free (subentry);
	}
      else
	{
	  enter_file_symbols (subentry);

	  if (prev)
	    prev->chain = subentry;
	  else entry->subfiles = subentry;
	  prev = subentry;
	}

      this_subfile_offset += member_length + sizeof (struct ar_hdr);
      if (this_subfile_offset & 1) this_subfile_offset++;
      subentry->strings = 0;
    }
}

/* ENTRY is an entry for a library member.
   Its symbols have been read into core, but not entered.
   Return nonzero if we ought to load this member.  */

int
subfile_wanted_p (entry)
     struct file_entry *entry;
{
  register struct nlist *p;
  register struct nlist *end
    = entry->symbols + entry->header.a_syms / sizeof (struct nlist);

  for (p = entry->symbols; p < end; p++)
    {
      register int type = p->n_type;

      if (type & N_EXT && (type != (N_UNDF | N_EXT) || p->n_value))
	{
	  register char *name = p->n_un.n_strx + entry->strings;
	  register symbol *sp = getsym_soft (name);

	  /* If this symbol has not been hashed, we can't be looking for it. */

	  if (!sp) continue;

	  if (sp->referenced && !sp->defined)
	    {
	      /* This is a symbol we are looking for.  */
	      if (type == (N_UNDF | N_EXT))
		{
		  /* Symbol being defined as common.
		     Remember this, but don't load subfile just for this.  */

		  if (sp->max_common_size < p->n_value)
		    sp->max_common_size = p->n_value;
		  if (!sp->defined)
		    undefined_global_sym_count--;
		  sp->defined = 1;
		  continue;
		}

	      if (write_map)
		{
		  print_file_name (entry, stdout);
		  fprintf (stdout, " needed due to %s\n", sp->name);
		}
	      return 1;
	    }
	}
    }

  return 0;
}

/* Do what must be done *after* decoding argv.  */
void
symtab_init_for_cplusplus ()
{
  /* C++: these are the names that crt0+.c looks for.  */
#ifndef nounderscore
  if (A_flag_specified)
    {
      ctor_list_symbol = getsym ("__xCTOR_LIST__");
      dtor_list_symbol = getsym ("__xDTOR_LIST__");

      head_dtor_symbol = getsym("__head");
      tail_dtor_symbol = getsym("__tail");
      init_symbol = getsym("__initfn");
    }
  else
    {
      ctor_list_symbol = getsym ("___CTOR_LIST__");
      dtor_list_symbol = getsym ("___DTOR_LIST__");
    }
#else
  if (A_flag_specified)
    {
      ctor_list_symbol = getsym ("_xCTOR_LIST__");
      dtor_list_symbol = getsym ("_xDTOR_LIST__");

      head_dtor_symbol = getsym("_head");
      tail_dtor_symbol = getsym("_tail");
      init_symbol = getsym("_initfn");
    }
  else
    {
      ctor_list_symbol = getsym ("__CTOR_LIST__");
      dtor_list_symbol = getsym ("__DTOR_LIST__");
    }
#endif

  /* C++: initialize ctor and dtor vectors.  */
  ctor_size = 0;
  ctor_size_max = 120;
  dtor_size = 0;
  dtor_size_max = 120;
  ctor_vec = (symbol **) xmalloc (ctor_size_max * sizeof (symbol *));
  dtor_vec = (symbol **) xmalloc (dtor_size_max * sizeof (symbol *));

  if (A_flag_specified)
    {
      ctor_list_symbol->defined = N_DATA;
      dtor_list_symbol->defined = N_DATA;
      init_symbol->defined = N_DATA;
      head_dtor_symbol->defined = N_DATA;
      tail_dtor_symbol->defined = N_DATA;

      init_symbol->referenced = 1;
      head_dtor_symbol->referenced = 1;
      tail_dtor_symbol->referenced = 1;

      head_dtor_symbol->trace = 1;
      tail_dtor_symbol->trace = 1;
      init_symbol->trace = 1;
    }
  else
    {
      ctor_list_symbol->defined = N_TEXT | N_EXT;
      dtor_list_symbol->defined = N_DATA | N_EXT;
    }
  ctor_list_symbol->referenced = 1;
  dtor_list_symbol->referenced = 1;
}

/* C++: Having found all of the file modules we will need, and found,
   for each module, the global constructor and/or destructor it
   will need, lay out the table of constructors and destructors.

   The format of the constructor table is an integer length,
   followed by a list of function pointers.  The list is 0
   terminated, for good luck.  The length includes the size
   of the length field, but does not include the 0 terminating
   entry.

   The format of the destructor table is a linked list of
   elements.  Each element is a pair consisting of a next pointer
   and a function pointer.  The list is built in this function.

   Layout of symbol numbers:

   0 for ctor_list_symbol
   1 for dtor_list_symbol
   2 .. ctor_size-1 for ctors
   2 + ctor_size .. 2 + ctor_size + dtor_size - 1 for dtors
   2 + ctor_size + dtor_size for _head
   2 + ctor_size + dtor_size + 1 for _tail
   2 + ctor_size + dtor_size + 2 for _initfn.

   */
void
layout_ctors_and_dtors ()
{
  struct file_entry *entry = &file_table[number_of_files];
  struct exec *header = &entry->header;
  int this_text_size = sizeof (int) * (ctor_size + 1);
  int this_data_size = sizeof (int) * (1 + 2 * dtor_size);
  char *strp;			/* end of entry->strings, if any */
  FILE *fp;
  int i, strindex = sizeof (int);
  struct nlist nl;
  struct relocation_info r1, r2;
#ifdef __GNUC__
  /* workaround "incomplete in scope ending here" error message.  */
  struct dtor_list { int x, y; };
#endif

  struct
    {
      union
	{
	  struct dtor_list *next;
	  int offset;
	} u;
      void (*pf)();
    } dtor_list_elem;

  if (input_file) file_close ();

  number_of_files += 1;		/* now add this file.  */

  strcpy (ctor_and_dtor_filename, "C++.hacksXXXXXX");
  if ((fp = fopen (mktemp (ctor_and_dtor_filename), "w")) == NULL)
    fatal ("could not open file for global constructors and destructors");
  entry->filename = ctor_and_dtor_filename;
  entry->local_sym_name = ctor_and_dtor_filename;

  /* Now make header correspond to new reality.  */
#ifdef sun
  header->a_machtype = M_68020;
#endif
  header->a_magic = OMAGIC;
  header->a_text = this_text_size;
  header->a_data = this_data_size;
  header->a_bss = 0;
  header->a_entry = 0;
  header->a_trsize = sizeof (struct relocation_info) * ctor_size;
  header->a_drsize = sizeof (struct relocation_info) * (2 * dtor_size);
  header->a_syms = sizeof (struct nlist) * (2 + ctor_size + dtor_size);

  if (fwrite (header, sizeof (struct exec), 1, fp) != 1)
    fatal ("error writing header");

  /* Automagically fill text segment with 0 up to this address.  */
  fseek (fp, this_text_size, 1);

  /* Now write out data information.  */
  /* First, point __DTOR_LIST__ at its table.  */
  if (dtor_size)
    dtor_list_elem.u.offset = sizeof (int);
  else
    dtor_list_elem.u.offset = 0;
  if (fwrite (&dtor_list_elem.u.offset, sizeof (int), 1, fp) != 1)
    fatal ("error writing destructor list head");

  /* Now, write out the offsets for the table.  */
  dtor_list_elem.pf = (void (*)())0;
  for (i = 0; i < dtor_size-1; i++)
    {
      dtor_list_elem.u.offset = (2*(i+1)+1) * sizeof (int);
      if (fwrite (&dtor_list_elem, sizeof (dtor_list_elem), 1, fp) != 1)
	fatal ("error writing destructor list elements");
    }
  if (dtor_size)
    {
      /* Space for last element.  */
      fseek (fp, sizeof (dtor_list_elem), 1);
    }

  /* Do relocation.  Initialize invariant data.  */
  r1.r_pcrel = 0;
  r1.r_length = 2;
  r1.r_extern = 1;
  r2.r_pcrel = 0;
  r2.r_length = 2;
  r2.r_extern = 1;

  /* Now do text relocation.  */
  for (i = 0; i < ctor_size; i++)
    {
      /* relocation for the pointer to constructor function.  */
      r1.r_address = i * sizeof (int);
      r1.r_symbolnum = i + 2;
      if (fwrite (&r1, sizeof (struct relocation_info), 1, fp) != 1)
	fatal ("error writing relocation info to global constructor file");
    }

  /* Now do data relocation.  */
  r1.r_symbolnum = 1;

  if (dtor_size)
    {
      /* Point __DTOR_LIST__ at its table.  */
      r1.r_address = 0;
      if (fwrite (&r1, sizeof (struct relocation_info), 1, fp) != 1)
	fatal ("error writing relocation info to global destructor file");
    }

  /* Now, build the table.  */
  for (i = 0; i < dtor_size-1; i++)
    {
      /* relocation for the `next' pointer.  */
      r1.r_address = (2*i + 1) * sizeof (int);

      /* relocation for the pointer to destructor function.  */
      r2.r_address = (2*i + 1 + 1) * sizeof (int);
      /* Call destructors in reverse order.  */
      r2.r_symbolnum = 2 + ctor_size + ((dtor_size-1) - i);

      if (fwrite (&r1, sizeof (struct relocation_info), 1, fp) != 1
	  || fwrite (&r2, sizeof (struct relocation_info), 1, fp) != 1)
	fatal ("error writing relocation info to global destructor file");
    }

  if (dtor_size)
    {
      /* relocation for the last element's function pointer.  */
      r2.r_address = (2 * dtor_size) * sizeof (int);
      r2.r_symbolnum = 2 + ctor_size;
      if (fwrite (&r2, sizeof (struct relocation_info), 1, fp) != 1)
	fatal ("error writing relocation info to global destructor file");
    }

  /* Now write out the symbol information.  */

  entry->symbols = (struct nlist *)xmalloc (sizeof (struct nlist) * (2 + ctor_size + dtor_size));

  nl.n_other = 0;
  nl.n_desc = 0;

  nl.n_type = N_TEXT | N_EXT;
  nl.n_un.n_strx = strindex;
  nl.n_value = 0;
  strindex += strlen (ctor_list_symbol->name) + 1;
  entry->symbols[0] = nl;

  nl.n_type = N_DATA | N_EXT;
  nl.n_un.n_strx = strindex;
  nl.n_value = this_text_size;
  strindex += strlen (dtor_list_symbol->name) + 1;
  entry->symbols[1] = nl;

  nl.n_type = N_UNDF | N_EXT;
  nl.n_value = 0;

  for (i = 0; i < ctor_size; i++)
    {
      symbol *sp = ctor_vec[i];
      nl.n_un.n_strx = strindex;
      strindex += strlen (sp->name) + 1;
      entry->symbols[i+2] = nl;
    }

  for (i = 0; i < dtor_size; i++)
    {
      symbol *sp = dtor_vec[i];
      nl.n_un.n_strx = strindex;
      strindex += strlen (sp->name) + 1;
      entry->symbols[ctor_size + i + 2] = nl;
    }

  if (fwrite (entry->symbols, sizeof (struct nlist),
	      2 + ctor_size + dtor_size, fp)
      != 2 + ctor_size + dtor_size)
    fatal ("error writing symbol entries");

  entry->string_size = strindex;

  /* If entry thinks it has strings, it is wrong.  But
     we will now give it correct strings.  */
  if (entry->strings)
    fatal ("strings non-null in layout_ctors_and_dtors");

  entry->strings = (char *)alloca (strindex + sizeof (int));
  strp = entry->strings;
  *((int *)strp) = strindex;
  strp += sizeof (int);

  strcpy (strp, ctor_list_symbol->name);
  strp += strlen (strp) + 1;
  strcpy (strp, dtor_list_symbol->name);
  strp += strlen (strp) + 1;

  for (i = 0; i < ctor_size; i++)
    {
      symbol *sp = ctor_vec[i];
      strcpy (strp, sp->name);
      strp += strlen (strp) + 1;
    }

  for (i = 0; i < dtor_size; i++)
    {
      symbol *sp = dtor_vec[i];
      strcpy (strp, sp->name);
      strp += strlen (strp) + 1;
    }

  if (fwrite (entry->strings, strp - entry->strings, 1, fp) != 1)
    fatal ("error writing strings");

  looking_for_ctors_and_dtors = 0;
  enter_file_symbols (entry);
  entry->strings = 0;
  fclose (fp);
}

/* When we know what we need to know for crt1+.o to be
   happy, patch the data to do so.  */
void
patch_data (bytes)
     char *bytes;
{
  /* Fixup the _head, _tail, and _initfn pointers.  */
  struct nlist *p;
  struct file_entry *entry = &file_table[1]; /* Assume crt1+.o is 1th.  */
  struct file_entry *last_entry = &file_table[number_of_files-1];
  int to_do = 4;
  int *ints = (int *)bytes;
  int *doffset = (int *)entry->data_start_address;

  entry->strings = (char *) alloca (entry->string_size);
  read_entry_strings (file_open (entry), entry);

  for (p = entry->symbols; to_do; p++)
    if (!(p->n_type & (N_STAB | N_EXT)))
      {
	char *symname = entry->strings + p->n_un.n_strx;

	if (symname[0] != '_')
	  continue;

	if (! strcmp (ctor_list_symbol->name, symname))
	  {
	    ints[(int *)p->n_value - doffset]
	      = last_entry->text_start_address;
	    to_do--;
	  }
	if (! strcmp (head_dtor_symbol->name, symname))
	  {
	    ints[(int *)p->n_value - doffset]
	      = dtor_size ? last_entry->data_start_address + sizeof (int) : 0;
	    to_do--;
	  }
	else if (! strcmp (tail_dtor_symbol->name, symname))
	  {
	    ints[(int *)p->n_value - doffset] = dtor_size
	      ? (last_entry->data_start_address
		 + last_entry->header.a_data - 2 * sizeof (int))
	      : 0;
	    to_do--;
	  }
	else if (! strcmp (init_symbol->name, symname))
	  {
	    ints[(int *)p->n_value - doffset]
	      = file_table[2].text_start_address;
	    to_do--;
	  }
      }
  entry->strings = 0;
}

void consider_file_section_lengths (), relocate_file_addresses ();

/* Having entered all the global symbols and found the sizes of sections
   of all files to be linked, make all appropriate deductions from this data.

   We propagate global symbol values from definitions to references.
   We compute the layout of the output file and where each input file's
   contents fit into it.  */

void
digest_symbols ()
{
  register int i;

  if (trace_files)
    fprintf (stderr, "Digesting symbol information:\n\n");

  /* C++: Now that we know what modules are required, build
     the tables of calls for the global constructors and
     destructors.  This function will act if we are doing a
     normal executable.  */
  
  if (cplusplus)
    layout_ctors_and_dtors ();

  /* Compute total size of sections */

  each_full_file (consider_file_section_lengths, 0);

  /* If necessary, pad text section to full page in the file.
     Include the padding in the text segment size.  */

  if (magic == NMAGIC || magic == ZMAGIC)
    {
      int text_end = text_size + N_TXTOFF (outheader);
      text_pad = ((text_end + page_size - 1) & (- page_size)) - text_end;
      text_size += text_pad;
    }

  outheader.a_text = text_size;

  /* Make the data segment address start in memory on a suitable boundary.  */

  data_start = N_DATADDR (outheader) + text_start - N_TXTADDR (outheader);

  /* Compute start addresses of each file's sections and symbols.  */

  each_full_file (relocate_file_addresses, 0);

  /* Now, for each symbol, verify that it is defined globally at most once.
     Put the global value into the symbol entry.
     Common symbols are allocated here, in the BSS section.
     Each defined symbol is given a '->defined' field
      which is the correct N_ code for its definition,
      except in the case of common symbols with -r.
     Then make all the references point at the symbol entry
     instead of being chained together. */

  defined_global_sym_count = 0;

  for (i = 0; i < TABSIZE; i++)
    {
      register symbol *sp;
      for (sp = symtab[i]; sp; sp = sp->link)
	{
	  register struct nlist *p, *next;
	  int defs = 0, com = sp->max_common_size, erred = 0;
	  for (p = sp->refs; p; p = next)
	    {
	      register int type = p->n_type;
	      if ((type & N_EXT) && type != (N_UNDF | N_EXT))
		{
		  /* non-common definition */
		  if (defs++ && sp->value != p->n_value)
		    if (!erred++)
		      {
		        make_executable = 0;
			error ("multiple definitions of symbol %s", sp->name);
		      }
		  sp->value = p->n_value;
		  sp->defined = type;
		}
	      next = (struct nlist *) p->n_un.n_name;
	      p->n_un.n_name = (char *) sp;
	    }
	  /* Allocate as common if defined as common and not defined for real */
	  if (com && !defs)
	    {
	      if (!relocatable_output || force_common_definition)
		{
		  com = (com + sizeof (int) - 1) & (- sizeof (int));
    
		  sp->value = data_start + data_size + bss_size;
		  sp->defined = N_BSS | N_EXT;
		  bss_size += com;
		  if (write_map)
		    printf ("Allocating common %s: %x at %x\n",
			    sp->name, com, sp->value);
		}
	    }
	  if (sp->defined)
	    defined_global_sym_count++;
	}
    }

  if (! relocatable_output)
    {
      etext_symbol->value = text_size + text_start;
      edata_symbol->value = data_start + data_size;
      end_symbol->value = data_start + data_size + bss_size;
    }

  /* Figure the data_pad now, so that it overlaps with the bss addresses.  */

  if (specified_data_size && specified_data_size > data_size)
    data_pad = specified_data_size - data_size;

  if (magic == ZMAGIC)
    data_pad = ((data_pad + data_size + page_size - 1) & (- page_size))
               - data_size;

  bss_size -= data_pad;
  if (bss_size < 0) bss_size = 0;

  data_size += data_pad;
}

/* Accumulate the section sizes of input file ENTRY
   into the section sizes of the output file.  */

void
consider_file_section_lengths (entry)
     register struct file_entry *entry;
{
  entry->text_start_address = text_size;
  text_size += entry->header.a_text;
  entry->data_start_address = data_size;
  data_size += entry->header.a_data;
  entry->bss_start_address = bss_size;
  bss_size += entry->header.a_bss;

  text_reloc_size += entry->header.a_trsize;
  data_reloc_size += entry->header.a_drsize;
}

/* Determine where the sections of ENTRY go into the output file,
   whose total section sizes are already known.
   Also relocate the addresses of the file's local and debugger symbols.  */

void
relocate_file_addresses (entry)
     register struct file_entry *entry;
{
  entry->text_start_address += text_start;
  /* Note that `data_start' and `data_size' have not yet been
     adjusted for `data_pad'.  If they had been, we would get the wrong
     results here.  */
  entry->data_start_address += data_start;
  entry->bss_start_address += data_start + data_size;

  {
    register struct nlist *p;
    register struct nlist *end
      = entry->symbols + entry->header.a_syms / sizeof (struct nlist);

    for (p = entry->symbols; p < end; p++)
      {
	/* If this belongs to a section, update it by the section's start address.  */
	register int type = p->n_type & N_TYPE;

	if (type == N_TEXT)
	  p->n_value += entry->text_start_address;
	else if (type == N_DATA)
	  /* A symbol whose value is in the data section
	     is present in the input file as if the data section
	     started at an address equal to the length of the file's text.  */
	  p->n_value += entry->data_start_address - entry->header.a_text;
	else if (type == N_BSS)
	  /* likewise for symbols with value in BSS.  */
	  p->n_value += entry->bss_start_address
	    - entry->header.a_text - entry->header.a_data;
      }
  }
}

void describe_file_sections (), list_file_locals ();

/* Print a complete or partial map of the output file.  */

void
print_symbols (outfile)
     FILE *outfile;
{
  register int i;

  fprintf (outfile, "\nFiles:\n\n");

  each_file (describe_file_sections, outfile);

  fprintf (outfile, "\nGlobal symbols:\n\n");

  for (i = 0; i < TABSIZE; i++)
    {
      register symbol *sp;
      for (sp = symtab[i]; sp; sp = sp->link)
	{
	  if (sp->defined == 1)
	    fprintf (outfile, "  %s: common, length 0x%x\n", sp->name, sp->max_common_size);
	  if (sp->defined)
	    fprintf (outfile, "  %s: 0x%x\n", sp->name, sp->value);
	  else if (sp->referenced)
	    fprintf (outfile, "  %s: undefined\n", sp->name);
	}
    }

  each_file (list_file_locals, outfile);
}

void
describe_file_sections (entry, outfile)
     struct file_entry *entry;
     FILE *outfile;
{
  fprintf (outfile, "  ");
  print_file_name (entry, outfile);
  if (entry->just_syms_flag)
    fprintf (outfile, " symbols only\n", 0);
  else
    fprintf (outfile, " text %x(%x), data %x(%x), bss %x(%x) hex\n",
	     entry->text_start_address, entry->header.a_text,
	     entry->data_start_address, entry->header.a_data,
	     entry->bss_start_address, entry->header.a_bss);
}

void
list_file_locals (entry, outfile)
     struct file_entry *entry;
     FILE *outfile;
{
  register struct nlist *p, *end = entry->symbols + entry->header.a_syms / sizeof (struct nlist);

  entry->strings = (char *) alloca (entry->string_size);
  read_entry_strings (file_open (entry), entry);

  fprintf (outfile, "\nLocal symbols of ");
  print_file_name (entry, outfile);
  fprintf (outfile, ":\n\n");

  for (p = entry->symbols; p < end; p++)
    /* If this is a definition, 
       update it if necessary by this file's start address.  */
    if (!(p->n_type & (N_STAB | N_EXT)))
      fprintf (outfile, "  %s: 0x%x\n",
	       entry->strings + p->n_un.n_strx, p->n_value);
  entry->strings = 0;
}

/* Print on OUTFILE a list of all the undefined global symbols.
   If there are any such, it is an error, so clear `make_executable'.  */

void
list_undefined_symbols (outfile)
     FILE *outfile;
{
  register int i;
  register int count = 0;

  for (i = 0; i < TABSIZE; i++)
    {
      register symbol *sp;
      for (sp = symtab[i]; sp; sp = sp->link)
	{
	  if (sp->referenced && !sp->defined)
	    {
	      if (!count++)
		fprintf (outfile, "Undefined symbols:\n");
	      fprintf (outfile, " %s\n", sp->name);
	    }
	}
    }

  if (count)
    fprintf (outfile, "\n");
  if (count)
    make_executable = 0;
}

/* Write the output file */ 

void
write_output ()
{
  struct stat statbuf;
  int filemode;

  outdesc = open (output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (outdesc < 0) perror_name (output_filename);

  if (fstat (outdesc, &statbuf) < 0)
    perror_name (output_filename);

  filemode = statbuf.st_mode;

  chmod (output_filename, filemode & ~0111);

  /* Output the a.out header.  */
  write_header ();

  /* Output the text and data segments, relocating as we go.  */
  write_text ();
  write_data ();

  /* Output the merged relocation info, if requested with `-r'.  */
  if (relocatable_output)
    write_rel ();

  /* Output the symbol table (both globals and locals).  */
  write_syms ();

  /* Copy any GDB symbol segments from input files.  */
  write_symsegs ();

  close (outdesc);

  if (make_executable)
    chmod (output_filename, filemode | 0111);
}

void modify_location (), perform_relocation (), copy_text (), copy_data ();

void
write_header ()
{
  outheader.a_magic = magic;
#ifdef sun
  outheader.a_machtype = M_68020;
#endif
  outheader.a_text = text_size;
  outheader.a_data = data_size;
  outheader.a_bss = bss_size;
  outheader.a_entry = (entry_symbol ? entry_symbol->value
		       : text_start + entry_offset);

  if (strip_symbols == STRIP_ALL)
    nsyms = 0;
  else
    {
      nsyms = defined_global_sym_count + undefined_global_sym_count;
      if (discard_locals == DISCARD_L)
	nsyms += non_L_local_sym_count;
      else if (discard_locals == DISCARD_NONE)
	nsyms += local_sym_count;
    }

  if (strip_symbols == STRIP_NONE)
    nsyms += debugger_sym_count;

  outheader.a_syms = nsyms * sizeof (struct nlist);

  if (relocatable_output)
    {
      outheader.a_trsize = text_reloc_size;
      outheader.a_drsize = data_reloc_size;
    }
  else
    {
      outheader.a_trsize = 0;
      outheader.a_drsize = 0;
    }

  mywrite (&outheader, sizeof (struct exec), 1, outdesc);

  /* Output whatever padding is required in the executable file
     between the header and the start of the text.  */

  padfile (N_TXTOFF (outheader) - sizeof outheader, outdesc);
}

/* Relocate the text segment of each input file
   and write to the output file.  */

void
write_text ()
{
  if (trace_files)
    fprintf (stderr, "Copying and relocating text:\n\n");

  each_full_file (copy_text);
  file_close ();

  if (trace_files)
    fprintf (stderr, "\n");

  padfile (text_pad, outdesc);
}

int
text_offset (entry)
     struct file_entry *entry;
{
  return entry->starting_offset + N_TXTOFF (entry->header);
}

/* Read the text segment contents of ENTRY, relocate them,
   and write the result to the output file.
   If `-r', save the text relocation for later reuse.  */

void
copy_text (entry)
     struct file_entry *entry;
{
  register char *bytes;
  register int desc;
  register struct relocation_info *reloc;

  if (trace_files)
    prline_file_name (entry, stderr);

  desc = file_open (entry);

  /* Allocate space for the file's text section and text-relocation.  */

  bytes = (char *) alloca (entry->header.a_text);

  if (relocatable_output)
    reloc = (struct relocation_info *) xmalloc (entry->header.a_trsize);
  else
    reloc = (struct relocation_info *) alloca (entry->header.a_trsize);

  /* Read those two sections into core.  */

  lseek (desc, text_offset (entry), 0);
  if (entry->header.a_text != read (desc, bytes, entry->header.a_text))
    fatal_with_file ("premature eof in text section of ", entry);

  lseek (desc, text_offset (entry) + entry->header.a_text + entry->header.a_data, 0);
  if (entry->header.a_trsize != read (desc, reloc, entry->header.a_trsize))
    fatal_with_file ("premature eof in text relocation of ", entry);

  /* Relocate the text according to the text relocation.  */

  perform_relocation (bytes, entry->text_start_address, entry->header.a_text,
		      reloc, entry->header.a_trsize, entry);

  /* Write the relocated text to the output file.  */

  mywrite (bytes, 1, entry->header.a_text, outdesc);

  /* If `-r', record the text relocation permanently
     so the combined relocation can be written later.  */

  if (relocatable_output)
    entry->textrel = reloc;
}

/* Relocate the data segment of each input file
   and write to the output file.  */

void
write_data ()
{
  if (trace_files)
    fprintf (stderr, "Copying and relocating data:\n\n");

  each_full_file (copy_data);
  file_close ();

  if (trace_files)
    fprintf (stderr, "\n");

  padfile (data_pad, outdesc);
}

/* Read the data segment contents of ENTRY, relocate them,
   and write the result to the output file.
   If `-r', save the data relocation for later reuse.
   See comments in `copy_text'.  */

void
copy_data (entry)
     struct file_entry *entry;
{
  register struct relocation_info *reloc;
  register char *bytes;
  register int desc;

  if (trace_files)
    prline_file_name (entry, stderr);

  desc = file_open (entry);

  bytes = (char *) alloca (entry->header.a_data);

  if (relocatable_output)
    reloc = (struct relocation_info *) xmalloc (entry->header.a_drsize);
  else
    reloc = (struct relocation_info *) alloca (entry->header.a_drsize);

  lseek (desc, text_offset (entry) + entry->header.a_text, 0);
  if (entry->header.a_data != read (desc, bytes, entry->header.a_data))
    fatal_with_file ("premature eof in data section of ", entry);

  lseek (desc, text_offset (entry) + entry->header.a_text
		 + entry->header.a_data + entry->header.a_trsize,
	       0);
  if (entry->header.a_drsize != read (desc, reloc, entry->header.a_drsize))
    fatal_with_file ("premature eof in data relocation of ", entry);

  perform_relocation (bytes, entry->data_start_address - entry->header.a_text,
		      entry->header.a_data, reloc, entry->header.a_drsize, entry);

  if (cplusplus && A_flag_specified && entry == &file_table[1])
    patch_data (bytes);

  mywrite (bytes, 1, entry->header.a_data, outdesc);

  if (relocatable_output)
    entry->datarel = reloc;
}

/* Relocate ENTRY's text or data section contents.
   DATA is the address of the contents, in core.
   DATA_SIZE is the length of the contents.
   PC_RELOCATION is the difference between the address of the contents
     in the output file and its address in the input file.
   RELOC_INFO is the address of the relocation info, in core.
   RELOC_SIZE is its length in bytes.  */

void
perform_relocation (data, pc_relocation, data_size, reloc_info, reloc_size, entry)
     char *data;
     struct relocation_info *reloc_info;
     struct file_entry *entry;
     int pc_relocation;
     int data_size;
     int reloc_size;
{
  register struct relocation_info *p = reloc_info;
  struct relocation_info *end
    = reloc_info + reloc_size / sizeof (struct relocation_info);
  int text_relocation = entry->text_start_address;
  int data_relocation = entry->data_start_address - entry->header.a_text;
  int bss_relocation
    = entry->bss_start_address - entry->header.a_text - entry->header.a_data;

  for (; p < end; p++)
    {
      register int relocation = 0;
      register int addr = p->r_address;
      register int symbolnum = p->r_symbolnum;
      register int length = p->r_length;

      if (addr >= data_size)
	fatal_with_file ("relocation address out of range in ", entry);
      if (p->r_extern)
	{
	  int symindex = symbolnum * sizeof (struct nlist);
	  symbol *sp = ((symbol *)
			(((struct nlist *)
			  (((char *)entry->symbols) + symindex))
			 ->n_un.n_name));

	  if (symindex >= entry->header.a_syms)
	    fatal_with_file ("relocation symbolnum out of range in ", entry);

	  /* If the symbol is undefined, leave it at zero.  */
	  if (! sp->defined)
	    relocation = 0;
	  else
	    relocation = sp->value;
	}

      else switch (symbolnum)
	{
	case N_TEXT:
	case N_TEXT | N_EXT:
	  relocation = text_relocation;
	  break;

	case N_DATA:
	case N_DATA | N_EXT:
	  /* A word that points to beginning of the the data section
	     initially contains not 0 but rather the "address" of that section
	     in the input file, which is the length of the file's text.  */
	  relocation = data_relocation;
	  break;

	case N_BSS:
	case N_BSS | N_EXT:
	  /* Similarly, an input word pointing to the beginning of the bss
	     initially contains the length of text plus data of the file.  */
	  relocation = bss_relocation;
	  break;

	default:
	  fatal_with_file ("nonexternal relocation code invalid in ", entry);
	}

      if (p->r_pcrel)
	relocation -= pc_relocation;

      switch (length)
	{
	case 0:
	  *(char *) (data + addr) += relocation;
	  break;

	case 1:
	  *(short *) (data + addr) += relocation;
	  break;

	case 2:
	  *(int *) (data + addr) += relocation;
	  break;

	default:
	  fatal_with_file ("invalid relocation field length in ", entry);
	}
    }
}

/* For relocatable_output only: write out the relocation,
   relocating the addresses-to-be-relocated.  */

void coptxtrel (), copdatrel ();

void
write_rel ()
{
  register int i;
  register int count = 0;

  if (trace_files)
    fprintf (stderr, "Writing text relocation:\n\n");

  /* Assign each global symbol a sequence number, giving the order
     in which `write_syms' will write it.
     This is so we can store the proper symbolnum fields
     in relocation entries we write.  */

  for (i = 0; i < TABSIZE; i++)
    {
      symbol *sp;
      for (sp = symtab[i]; sp; sp = sp->link)
	if (sp->referenced || sp->defined)
	  sp->def_count = count++;
    }
  if (count != defined_global_sym_count + undefined_global_sym_count)
    fatal ("internal error");

  /* Write out the relocations of all files, remembered from copy_text.  */

  each_full_file (coptxtrel);

  if (trace_files)
    fprintf (stderr, "\nWriting data relocation:\n\n");

  each_full_file (copdatrel);

  if (trace_files)
    fprintf (stderr, "\n");
}

void
coptxtrel (entry)
     struct file_entry *entry;
{
  register struct relocation_info *p, *end;
  register int reloc = entry->text_start_address;

  p = entry->textrel;
  end = (struct relocation_info *) (entry->header.a_trsize + (char *) p);
  while (p < end)
    {
      p->r_address += reloc;
      if (p->r_extern)
	{
	  register int symindex = p->r_symbolnum * sizeof (struct nlist);
	  symbol *symptr = ((symbol *)
			    (((struct nlist *)
			      (((char *)entry->symbols) + symindex))
			     ->n_un.n_name));

	  if (symindex >= entry->header.a_syms)
	    fatal_with_file ("relocation symbolnum out of range in ", entry);
	  /* If the symbol is now defined, change the external relocation
	     to an internal one.  */
	  if (symptr->defined)
	    {
	      p->r_extern = 0;
	      p->r_symbolnum = (symptr->defined & N_TYPE);
	    }
	  else
	    p->r_symbolnum = (symptr->def_count + nsyms
			      - defined_global_sym_count
			      - undefined_global_sym_count);
	}
      p++;
    }
  mywrite (entry->textrel, 1, entry->header.a_trsize, outdesc);
}

void
copdatrel (entry)
     struct file_entry *entry;
{
  register struct relocation_info *p, *end;
  /* Relocate the address of the relocation.
     Old address is relative to start of the input file's data section.
     New address is relative to start of the output file's data section.  */
  register int reloc = entry->data_start_address - text_size;

  p = entry->datarel;
  end = (struct relocation_info *) (entry->header.a_drsize + (char *) p);
  while (p < end)
    {
      p->r_address += reloc;
      if (p->r_extern)
	{
	  register int symindex = p->r_symbolnum * sizeof (struct nlist);
	  if (symindex >= entry->header.a_syms)
	    fatal_with_file ("relocation symbolnum out of range in ", entry);
	  p->r_symbolnum = ((symbol *)
			    (((struct nlist *)
			      (((char *)entry->symbols) + symindex))
			     ->n_un.n_name))
			   ->def_count
			 + nsyms - defined_global_sym_count - undefined_global_sym_count;
	}
      p++;
    }
  mywrite (entry->datarel, 1, entry->header.a_drsize, outdesc);
}

void write_file_syms ();
void write_string_table ();

/* Offsets and current lengths of symbol and string tables in output file. */

int symbol_table_offset;
int symbol_table_len;

/* Address in output file where string table starts.  */
int string_table_offset;

/* Offset within string table
   where the strings in `strtab_vector' should be written.  */
int string_table_len;

/* Total size of string table strings allocated so far,
   including strings in `strtab_vector'.  */
int strtab_size;

/* Vector whose elements are strings to be added to the string table.  */
char **strtab_vector;

/* Vector whose elements are the lengths of those strings.  */
int *strtab_lens;

/* Index in `strtab_vector' at which the next string will be stored.  */
int strtab_index;

/* Add the string NAME to the output file string table.
   Record it in `strtab_vector' to be output later.
   Return the index within the string table that this string will have.  */

int
assign_string_table_index (name)
     char *name;
{
  register int index = strtab_size;
  register int len = strlen (name) + 1;

  strtab_size += len;
  strtab_vector[strtab_index] = name;
  strtab_lens[strtab_index++] = len;

  return index;
}

FILE *outstream = (FILE *) 0;

/* Write the contents of `strtab_vector' into the string table.
   This is done once for each file's local&debugger symbols
   and once for the global symbols.  */

void
write_string_table ()
{
  register int i;

  lseek (outdesc, string_table_offset + string_table_len, 0);

  if (!outstream)
    outstream = fdopen (outdesc, "w");

  for (i = 0; i < strtab_index; i++)
    {
      fwrite (strtab_vector[i], 1, strtab_lens[i], outstream);
      string_table_len += strtab_lens[i];
    }

  fflush (outstream);

  /* Report I/O error such as disk full.  */
  if (ferror (outstream))
    perror_name (output_filename);
}

/* Write the symbol table and string table of the output file.  */

void
write_syms ()
{
  /* Number of symbols written so far.  */
  int syms_written = 0;
  register int i;
  register symbol *sp;

  /* Buffer big enough for all the global symbols.  */
  struct nlist *buf
    = (struct nlist *) alloca ((defined_global_sym_count + undefined_global_sym_count)
			       * sizeof (struct nlist));
  /* Pointer for storing into BUF.  */
  register struct nlist *bufp = buf;

  /* Size of string table includes the bytes that store the size.  */
  strtab_size = sizeof strtab_size;

  symbol_table_offset = N_SYMOFF (outheader);
  symbol_table_len = 0;
  string_table_offset = N_STROFF (outheader);
  string_table_len = strtab_size;

  if (strip_symbols == STRIP_ALL)
    return;

  /* Write the local symbols defined by the various files.  */

  each_file (write_file_syms, &syms_written);
  file_close ();

  /* Now write out the global symbols.  */

  /* Allocate two vectors that record the data to generate the string
     table from the global symbols written so far.  */

  strtab_vector = (char **) alloca (num_hash_tab_syms * sizeof (char *));
  strtab_lens = (int *) alloca (num_hash_tab_syms * sizeof (int));
  strtab_index = 0;

  /* Scan the symbol hash table, bucket by bucket.  */

  for (i = 0; i < TABSIZE; i++)
    for (sp = symtab[i]; sp; sp = sp->link)
      {
	struct nlist nl;

	nl.n_other = 0;
	nl.n_desc = 0;

	/* Compute a `struct nlist' for the symbol.  */

	if (sp->defined || sp->referenced)
	  {
	    if (!sp->defined)	      /* undefined -- legit only if -r */
	      {
		nl.n_type = N_UNDF | N_EXT;
		nl.n_value = 0;
	      }
	    else if (sp->defined > 1) /* defined with known type */
	      {
		nl.n_type = sp->defined;
		nl.n_value = sp->value;
	      }
	    else if (sp->max_common_size) /* defined as common but not allocated. */
	      {
		/* happens only with -r and not -d */
		/* write out a common definition */
		nl.n_type = N_UNDF | N_EXT;
		nl.n_value = sp->max_common_size;
	      }
	    else
	      fatal ("internal error: %s defined in mysterious way", sp->name);

	    /* Allocate string table space for the symbol name.  */

	    nl.n_un.n_strx = assign_string_table_index (sp->name);

	    /* Output to the buffer and count it.  */

	    *bufp++ = nl;
	    syms_written++;
	  }
      }

  /* Output the buffer full of `struct nlist's.  */

  lseek (outdesc, symbol_table_offset + symbol_table_len, 0);
  mywrite (buf, sizeof (struct nlist), bufp - buf, outdesc);
  symbol_table_len += sizeof (struct nlist) * (bufp - buf);

  if (syms_written != nsyms)
    fatal ("internal error: wrong number of symbols written into output file", 0);

  if (symbol_table_offset + symbol_table_len != string_table_offset)
    fatal ("internal error: inconsistent symbol table length", 0);

  /* Now the total string table size is known, so write it.
     We are already positioned at the right place in the file.  */

  mywrite (&strtab_size, sizeof (int), 1, outdesc);  /* we're at right place */

  /* Write the strings for the global symbols.  */

  write_string_table ();
}

/* Write the local and debugger symbols of file ENTRY.
   Increment *SYMS_WRITTEN_ADDR for each symbol that is written.  */

/* Note that we do not combine identical names of local symbols.
   dbx or gdb would be confused if we did that.  */

void
write_file_syms (entry, syms_written_addr)
     struct file_entry *entry;
     int *syms_written_addr;
{
  register struct nlist *p = entry->symbols;
  register struct nlist *end = p + entry->header.a_syms / sizeof (struct nlist);

  /* Buffer to accumulate all the syms before writing them.
     It has one extra slot for the local symbol we generate here.  */
  struct nlist *buf
    = (struct nlist *) alloca (entry->header.a_syms + sizeof (struct nlist));
  register struct nlist *bufp = buf;

  /* Upper bound on number of syms to be written here.  */
  int max_syms = (entry->header.a_syms / sizeof (struct nlist)) + 1;

  /* Make tables that record, for each symbol, its name and its name's length.
     The elements are filled in by `assign_string_table_index'.  */

  strtab_vector = (char **) alloca (max_syms * sizeof (char *));
  strtab_lens = (int *) alloca (max_syms * sizeof (int));
  strtab_index = 0;

  /* Generate a local symbol for the start of this file's text.  */

  if (discard_locals != DISCARD_ALL)
    {
      struct nlist nl;

      nl.n_type = N_TEXT;
      nl.n_un.n_strx = assign_string_table_index (entry->local_sym_name);
      nl.n_value = entry->text_start_address;
      nl.n_desc = 0;
      nl.n_other = 0;
      *bufp++ = nl;
      (*syms_written_addr)++;
      entry->local_syms_offset = *syms_written_addr * sizeof (struct nlist);
    }

  /* Read the file's string table.  */

  entry->strings = (char *) alloca (entry->string_size);
  read_entry_strings (file_open (entry), entry);

  for (; p < end; p++)
    {
      register int type = p->n_type;
      register int write = 0;

      /* WRITE gets 1 for a non-global symbol that should be written.  */

      if (!(type & (N_STAB | N_EXT)))
        /* ordinary local symbol */
	write = (discard_locals != DISCARD_ALL)
		&& !(discard_locals == DISCARD_L &&
		     (p->n_un.n_strx + entry->strings)[0] == 'L');
      else if (!(type & N_EXT))
	/* debugger symbol */
        write = (strip_symbols == STRIP_NONE);

      if (write)
	{
	  /* If this symbol has a name,
	     allocate space for it in the output string table.  */

	  if (p->n_un.n_strx)
	    p->n_un.n_strx = assign_string_table_index (p->n_un.n_strx +
							entry->strings);

	  /* Output this symbol to the buffer and count it.  */

	  *bufp++ = *p;
	  (*syms_written_addr)++;
	}
    }

  /* All the symbols are now in BUF; write them.  */

  lseek (outdesc, symbol_table_offset + symbol_table_len, 0); 
  mywrite (buf, sizeof (struct nlist), bufp - buf, outdesc);
  symbol_table_len += sizeof (struct nlist) * (bufp - buf);

  /* Write the string-table data for the symbols just written,
     using the data in vectors `strtab_vector' and `strtab_lens'.  */

  write_string_table ();
  entry->strings = 0;
}

/* Copy any GDB symbol segments from the input files to the output file.
   The contents of the symbol segment is copied without change
   except that we store some information into the beginning of it.  */

void write_file_symseg ();

write_symsegs ()
{
  each_file (write_file_symseg, 0);
}

void
write_file_symseg (entry)
     struct file_entry *entry;
{
  char buffer[4096];
  struct symbol_root root;
  int indesc;
  int len;

  if (entry->symseg_offset == 0)
    return;

  /* This entry has a symbol segment.  Read the root of the segment.  */

  indesc = file_open (entry);
  lseek (indesc, entry->symseg_offset + entry->starting_offset, 0);
  if (sizeof root != read (indesc, &root, sizeof root))
    fatal_with_file ("premature end of file in symbol segment of ", entry);

  /* Store some relocation info into the root.  */

  root.ldsymoff = entry->local_syms_offset;
  root.textrel = entry->text_start_address;
  root.datarel = entry->data_start_address - entry->header.a_text;
  root.bssrel = entry->bss_start_address
    - entry->header.a_text - entry->header.a_data;

  /* Write the modified root into the output file.  */

  mywrite (&root, sizeof root, 1, outdesc);

  /* Copy the rest of the symbol segment unchanged.  */

  if (entry->superfile)
    {
      /* Library member: number of bytes to copy is determined
	 from the member's total size.  */

      int total = entry->total_size - entry->symseg_offset - sizeof root;

      while (total > 0)
	{
	  len = read (indesc, buffer, min (sizeof buffer, total));

	  if (len != min (sizeof buffer, total))
	    fatal_with_file ("premature end of file in symbol segment of ", entry);
	  total -= len;
	  mywrite (buffer, len, 1, outdesc);
	}
    }
  else
    {
      /* A separate file: copy until end of file.  */

      while (len = read (indesc, buffer, sizeof buffer))
	{
	  mywrite (buffer, len, 1, outdesc);
	  if (len < sizeof buffer)
	    break;
	}
    }

  file_close ();
}

/* Create the symbol table entries for `etext', `edata' and `end'.  */

void
symtab_init ()
{
#ifndef nounderscore
  edata_symbol = getsym ("_edata");
  etext_symbol = getsym ("_etext");
  end_symbol = getsym ("_end");
#else
  edata_symbol = getsym ("edata");
  etext_symbol = getsym ("etext");
  end_symbol = getsym ("end");
#endif

  edata_symbol->defined = N_DATA | N_EXT;
  etext_symbol->defined = N_TEXT | N_EXT;
  end_symbol->defined = N_BSS | N_EXT;

  edata_symbol->referenced = 1;
  etext_symbol->referenced = 1;
  end_symbol->referenced = 1;
}

/* Compute the hash code for symbol name KEY.  */

int
hash_string (key)
     char *key;
{
  register char *cp;
  register int k;

  cp = key;
  k = 0;
  while (*cp)
    k = (((k << 1) + (k >> 14)) ^ (*cp++)) & 0x3fff;

  return k;
}

/* Get the symbol table entry for the global symbol named KEY.
   Create one if there is none.  */

symbol *
getsym (key)
     char *key;
{
  register int hashval;
  register symbol *bp;

  /* Determine the proper bucket.  */

  hashval = hash_string (key) % TABSIZE;

  /* Search the bucket.  */

  for (bp = symtab[hashval]; bp; bp = bp->link)
    if (! strcmp (key, bp->name))
      return bp;

  /* Nothing was found; create a new symbol table entry.  */

  bp = (symbol *) xmalloc (sizeof (symbol));
  bp->refs = 0;
  bp->name = (char *) xmalloc (strlen (key) + 1);
  strcpy (bp->name, key);
  bp->defined = 0;
  bp->referenced = 0;
  bp->trace = 0;
  bp->value = 0;
  bp->max_common_size = 0;

  /* Add the entry to the bucket.  */

  bp->link = symtab[hashval];
  symtab[hashval] = bp;

  ++num_hash_tab_syms;

  return bp;
}

/* Like `getsym' but return 0 if the symbol is not already known.  */

symbol *
getsym_soft (key)
     char *key;
{
  register int hashval;
  register symbol *bp;

  /* Determine which bucket.  */

  hashval = hash_string (key) % TABSIZE;

  /* Search the bucket.  */

  for (bp = symtab[hashval]; bp; bp = bp->link)
    if (! strcmp (key, bp->name))
      return bp;

  return 0;
}

/* Report a fatal error.
   STRING is a printf format string and ARG is one arg for it.  */

fatal (string, arg)
     char *string, *arg;
{
  fprintf (stderr, "ld: ");
  fprintf (stderr, string, arg);
  fprintf (stderr, "\n");
  exit (1);
}

/* Report a fatal error.  The error message is STRING
   followed by the filename of ENTRY.  */

fatal_with_file (string, entry)
     char *string;
     struct file_entry *entry;
{
  fprintf (stderr, "ld: ");
  fprintf (stderr, string);
  print_file_name (entry, stderr);
  fprintf (stderr, "\n");
  exit (1);
}

/* Report a fatal error using the message for the last failed system call,
   followed by the string NAME.  */

perror_name (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for %s");
  else
    s = "cannot open %s";
  fatal (s, name);
}

/* Report a fatal error using the message for the last failed system call,
   followed by the name of file ENTRY.  */

perror_file (entry)
     struct file_entry *entry;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for ");
  else
    s = "cannot open ";
  fatal_with_file (s, entry);
}

/* Report a nonfatal error.
   STRING is a format for printf, and ARG1 ... ARG3 are args for it.  */

error (string, arg1, arg2, arg3)
     char *string, *arg1, *arg2, *arg3;
{
  fprintf (stderr, string, arg1, arg2, arg3);
  fprintf (stderr, "\n");
}

/* Output COUNT*ELTSIZE bytes of data at BUF
   to the descriptor DESC.  */

mywrite (buf, count, eltsize, desc)
     char *buf;
     int count;
     int eltsize;
     int desc;
{
  register int val;
  register int bytes = count * eltsize;

  while (bytes > 0)
    {
      val = write (desc, buf, bytes);
      if (val <= 0)
	perror_name (output_filename);
      buf += val;
      bytes -= val;
    }
}

/* Output PADDING zero-bytes to descriptor OUTDESC.
   PADDING may be negative; in that case, do nothing.  */

void
padfile (padding, outdesc)
     int padding;
     int outdesc;
{
  register char *buf;
  if (padding <= 0)
    return;

  buf = (char *) alloca (padding);
  bzero (buf, padding);
  mywrite (buf, padding, 1, outdesc);
}

/* Return a newly-allocated string
   whose contents concatenate the strings S1, S2, S3.  */

char *
concat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  register int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  register char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  result[len1 + len2 + len3] = 0;

  return result;
}

/* Parse the string ARG using scanf format FORMAT, and return the result.
   If it does not parse, report fatal error
   generating the error message using format string ERROR and ARG as arg.  */

int
parse (arg, format, error)
     char *arg, *format;
{
  int x;
  if (1 != sscanf (arg, format, &x))
    fatal (error, arg);
  return x;
}

/* Like malloc but get fatal error if memory is exhausted.  */

int
xmalloc (size)
     int size;
{
  register int result = malloc (size);
  if (!result)
    fatal ("virtual memory exhausted", 0);
  return result;
}

/* Like realloc but get fatal error if memory is exhausted.  */

int
xrealloc (ptr, size)
     char *ptr;
     int size;
{
  register int result = realloc (ptr, size);
  if (!result)
    fatal ("virtual memory exhausted", 0);
  return result;
}
