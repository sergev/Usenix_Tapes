#include "stdio.h"

main() {
      FILE *fp = stdout ;

      setbuf(fp, NULL) ;
      close(1) ;

      if (putc('X', fp) == EOF)
            fprintf(stderr, "putc returned EOF\n") ;
      else
            fprintf(stderr, "putc did not return EOF\n") ;
      if (ferror(fp))
            fprintf(stderr, "ferror set\n") ;
}
