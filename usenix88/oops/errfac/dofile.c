#include "dofile.h"
#include <string.h>
#ifdef BSD
#include <sys/file.h>
#else
#include <fcntl.h>
#endif
#include <osfcn.h>

extern int errno;

/* base_filename -- extract base file name (drop extension) */
char* base_filename(char* p) {
  char* p0 = p;

  while ( (*p0 != '\0')&&(*p0 != '.') ) ++p0;

  char* q = new char[p0-p+1];
  char* q0 = q;
  while ( p != p0 )
    *(q++) = *(p++);
  *q = '\0';
  return q0;
}/* base_filename */

/* ext_filename -- extract extension from filename */
char* ext_filename(char* p) {
  while ( (*p != '\0')&&(*p != '.') ) ++p;
  char* q = new char[strlen(p)+1];
  strcpy(q,p);
  return q;
} /* end ext_filename*/


ostream* open_ostream(char* fname) {
  errno = 0;

  int fd;
  ostream* strm = new ostream((fd=creat(fname,0664),fd));
  if ( fd == -1 ) {
   switch ( errno ) {
     case 2:
       cerr << "errgen> file does not exist -- " << fname << "\n";
       break;
     default:
       cerr << "errgen> file not open -- " << fname << "\n";
       break;
     };
   exit(1);
   };
  cerr << "open_ostream> " << fname << "\n";
  return strm;
}
ostream* append_ostream(char* fname) {
cerr << "append_ostream> " << fname << "\n";
  int fd;
  ostream* strm = new ostream((fd=open(fname,O_APPEND),fd));
  if ( fd == -1 ) {
   cerr << "errgen> file not open -- " << fname << "\n";
   exit(1);
   };
  return strm;
}

istream* open_istream(char* fname) {
  errno = 0;
  int fd;
  istream* strm = new istream((fd=open(fname,O_RDONLY),fd));
  if ( fd == -1 ) {
    switch ( errno ) {
      case 2:
        cerr << "errgen> file does not exist -- " << fname << "\n";
        break;
      default:
        cerr << "errgen> file not open -- " << fname << "\n";
        break;
      };

   exit(1);
   };
  cerr << "open_istream> " << fname << "\n";
  return strm;
}
