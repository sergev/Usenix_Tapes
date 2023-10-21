#include <std.h>
#include <stdio.h>
#include <stddef.h>

class ifile
{
  FILE *fp;
  char *name;

 public:
  ifile (char *name)
    {
      this->name = new char[strlen(name) + 1];
      strcpy (this->name, name);
      if ((fp = fopen (name, "r")) == NULL)
	{
	  fprintf (stderr, "could not open input file `%s'\n", name);
	  exit (1);
	}
    }

  ~ifile ()
    {
      fclose (fp);
      if (fp) fprintf (stderr, "closing input file `%s'\n", name);
    }

  ifile& operator>> (int &i)
    { fscanf (fp, "%d", &i); return *this; }
  ifile& operator>> (char *p)
    { fscanf (fp, "%s", p); return *this; }
};

class ofile
{
  FILE *fp;
  char *name;
 public:
  ofile (char *name)
    {
      this->name = new char[strlen(name) + 1];
      strcpy (this->name, name);
      if ((fp = fopen (name, "w")) == NULL)
	{
	  fprintf (stderr, "could not open output file `%s'\n", name);
	  exit (1);
	}
    }

  ~ofile ()
    {
      fclose (fp);
      if (fp) fprintf (stderr, "closing output file `%s'\n", name);
    }

  ofile& operator<< (int i)
    {
      fprintf (fp, "%d", i);
      fflush (fp);
      return *this;
    }
  ofile& operator<< (char *p)
    {
      fprintf (fp, "%s", p);
      fflush (fp);
      return *this;
    }
};
