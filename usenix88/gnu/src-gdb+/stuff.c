/* Program to stuff files into a specially prepared space in kdb.
   Copyright (C) 1986 Free Software Foundation, Inc.

GDB is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GDB General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GDB,
but only under the conditions described in the GDB General Public
License.  A copy of this license is supposed to have been given to you
along with GDB so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GDB, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

/* Written 13-Mar-86 by David Bridgham. */

#include <stdio.h>
#include <a.out.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

extern char *sys_errlist[];
extern int errno;

main (argc, argv)
     int argc;
     char *argv[];
{
  register char *cp;
  char *outfile;
  register int i;
  int offset;
  int out_fd, in_fd;
  struct stat stat_buf;
  int size, pad;
  char buf[1024];
  static char zeros[4] = {0};

  if (argc < 4)
    err("Not enough arguments\nUsage: %s -o kdb file1 file2 ...\n",
	argv[0]);

  outfile = 0;
  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-o") == 0)
	outfile = argv[++i];
    }
  if (outfile == 0)
    err("Output file not specified\n");

  offset = get_offset (outfile, "_heap");

  out_fd = open (outfile, O_WRONLY);
  if (out_fd < 0)
    err ("Error opening %s for write: %s\n", outfile, sys_errlist[errno]);
  if (lseek (out_fd, offset, 0) < 0)
    err ("Error seeking to heap in %s: %s\n", outfile, sys_errlist[errno]);

  /* For each file listed on the command line, write it into the
   * 'heap' of the output file.  Make sure to skip the arguments
   * that name the output file. */
  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-o") == 0)
	continue;
      if ((in_fd = open (argv[i], O_RDONLY)) < 0)
	err ("Error opening %s for read: %s\n", argv[i], sys_errlist[errno]);
      if (fstat (in_fd, &stat_buf) < 0)
	err ("Error stat'ing %s: %s\n", argv[i], sys_errlist[errno]);
      size = strlen (argv[i]);
      pad = 4 - (size & 3);
      size += pad + stat_buf.st_size + sizeof (int);
      write (out_fd, &size, sizeof (int));
      write (out_fd, argv[i], strlen (argv[i]));
      write (out_fd, zeros, pad);
      while ((size = read (in_fd, buf, sizeof (buf))) > 0)
	write (out_fd, buf, size);
      close (in_fd);
    }
  size = 0;
  write (out_fd, &size, sizeof (int));
  close (out_fd);
  return (0);
}

/* Read symbol table from file and returns the offset into the file
 * where symbol sym_name is located.  If error, print message and
 * exit. */
get_offset (file, sym_name)
     char *file;
     char *sym_name;
{
  int f;
  struct exec file_hdr;
  struct nlist *symbol_table;
  int size;
  char *strings;

  f = open (file, O_RDONLY);
  if (f < 0)
    err ("Error opening %s: %s\n", file, sys_errlist[errno]);
  if (read (f, &file_hdr, sizeof (file_hdr)) < 0)
    err ("Error reading exec structure: %s\n", sys_errlist[errno]);
  if (N_BADMAG (file_hdr))
    err ("File %s not an a.out file\n", file);

  /* read in symbol table */
  if ((symbol_table = (struct nlist *)malloc (file_hdr.a_syms)) == 0)
    err ("Couldn't allocate space for symbol table\n");
  if (lseek (f, N_SYMOFF (file_hdr), 0) == -1)
    err ("lseek error: %s\n", sys_errlist[errno]);
  if (read (f, symbol_table, file_hdr.a_syms) == -1)
    err ("Error reading symbol table from %s: %s\n", file, sys_errlist[errno]);

  /* read in string table */
  if (read (f, &size, 4) == -1)
    err ("reading string table size: %s\n", sys_errlist[errno]);
  if ((strings = (char *)malloc (size)) == 0)
    err ("Couldn't allocate memory for string table\n");
  if (read (f, strings, size - 4) == -1)
    err ("reading string table: %s\n", sys_errlist[errno]);

  /* Find the core address at which the first byte of kdb text segment
     should be loaded into core when kdb is run.  */
  origin = find_symbol ("_etext", symbol_table, file_hdr.a_syms, strings)
    - file_hdr.a_text;
  /* Find the core address at which the heap will appear.  */
  coreaddr = find_symbol (sym_name, symbol_table, file_hdr.a_syms, strings);
  /* Return address in file of the heap data space.  */
  return (N_TXTOFF (file_hdr) + core_addr - origin);
}

find_symbol (sym_name, symbol_table, length, strings)
     char *sym_name;
     struct nlist *symbol_table;
     int length;
     char *strings;
{
  register struct nlist *sym;

  /* Find symbol in question */
  for (sym = symbol_table;
       sym != (struct nlist *)((char *)symbol_table + length);
       sym++)
      {
	if ((sym->n_type & N_TYPE) != N_DATA) continue;
	if (sym->n_un.n_strx == 0) continue;
	if (strcmp (sym_name, strings + sym->n_un.n_strx - 4) == 0)
	  return sym->n_value;
      }
    err ("Data symbol %s not found in %s\n", sym_name, file);
}

err (msg, a1, a2, a3)
     char *msg;
     int a1, a2, a3;
{
  fprintf (stderr, msg, a1, a2, a3);
  exit (-1);
}
