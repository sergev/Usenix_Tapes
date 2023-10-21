// This file tests the installation of the GNU C++ compiler system.
// In order for this file to be compiled successfully, the compiler
// driver (g++) must have execute paths to the preprocessor (cpp+)
// and the compiler (c++), as well as the loader (ld++) and the
// special run-time library (crt0+.o).  In addition, the library
// gnulib+ is needed to provide functions such as "_builtin_new"
// and "_builtin_delete".

#include "test0.h"
#include <a.out.h>
#include <sys/file.h>


void test0_message();

class tfile
{
  char tname[L_tmpnam];
  static char *orig_name;
  char *this_name;
  exec header;

public:
  tfile (char*);
  ~tfile ();
  void* load ();
};

tfile::tfile (char *p)
{
  int fd;

  strcpy (tname, "hack.XXXXXX");
  mktemp (tname);
  this_name = new char[strlen (p) + 1];
  strcpy (this_name, p);

  if ((fd = open (this_name, 2, 0)) < 0)
    {
      printf ("Unable to open file %s\n", p);
      exit (1);
    }
  if (read (fd, (void*) &header, sizeof (header)) <= 0)
    {
      printf ("Error in reading file %s\n", p);
    }
  close (fd);
}

tfile::~tfile ()
{
  unlink (tname);
}

void *tfile::load ()
{
  int size = header.a_text + header.a_data;
#ifndef PAGSIZ
  int pagsiz = getpagesize();
#else
  int pagsiz = PAGSIZ;
#endif

  if (size < (pagsiz))
    size = (pagsiz);

  int init_fn = (int) new short[size];

  init_fn += pagsiz-1;
  init_fn &= ~(pagsiz-1);

  fprintf(stderr, "\n ... timing incremental load...\n");

  char command[512], *cmd = command;
  sprintf (cmd, "time /usr/local/lib/gcc-ld++ -C -N -A %s -T %x /usr/local/lib/gcc-crt1+.o %s -o %s",
	   orig_name, init_fn, this_name, tname);

  if (system (cmd))
    {
      fprintf (stderr, "Error in linking file bye\n");
      delete this;
      exit (1);
    }

  int fd = open (tname, 2, 0);
  if (lseek (fd, sizeof (header), L_SET) < 0)
    {
      perror ("Error in temp file seek\n");
      delete this;
      exit (1);
    }

  read (fd, (char*) init_fn, size);
  close (fd);

  return (void *)init_fn;
}

ifile in ("/dev/tty");
ofile out ("/dev/tty");

main (int, char *argv[])
{
  char buf[4096];

  out << "Enter file to link: (test.bye or test.bye2 or test.shell)\n";
  in >> buf;
  out << "Hello! linking `" << buf << "'...\n";
  
  tfile::orig_name = argv[0];

  tfile temp (buf);
  register void (*init_fn)() = temp.load ();
  fprintf(stderr, "\n if execution now aborts, your crt1+.o is bad\n");
  (*init_fn) ();

  out << "Enter another file to link: ";
  in >> buf;
  out << "Hello! linking `" << buf << "'...\n";
  
  tfile temp2 (buf);
  init_fn = temp2.load ();
  (*init_fn)();

  test0_message();
}
