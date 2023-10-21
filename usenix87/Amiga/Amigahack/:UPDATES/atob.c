/* atob: version 4.0
 * stream filter to change printable ascii from "btoa" back into 8 bit bytes
 * if bad chars, or Csums do not match: exit(1) [and NO output]
 *
 *  Paul Rutter		Joe Orost
 *  philabs!per		petsd!joe
 *  Fred Fish		(change into pure filter, always output)
 */

#include <stdio.h>

#define reg register

#define streq(s0, s1)  strcmp(s0, s1) == 0

#define times85(x)	((((((x<<2)+x)<<2)+x)<<2)+x)

long int Ceor = 0;
long int Csum = 0;
long int Crot = 0;
long int word = 0;
long int bcount = 0;

#define DE(c) ((c) - '!')

fatal() {
  fprintf(stderr, "bad format or Csum to atob\n");
  exit(1);
}

decode(c) 
  reg c;
{
  if (c == 'z') {
    if (bcount != 0) {
      fatal();
    } else {
      byteout(0);
      byteout(0);
      byteout(0);
      byteout(0);
    }
  } else if ((c >= '!') && (c < ('!' + 85))) {
    if (bcount == 0) {
      word = DE(c);
      ++bcount;
    } else if (bcount < 4) {
      word = times85(word);
      word += DE(c);
      ++bcount;
    } else {
      word = times85(word) + DE(c);
      byteout((int)((word >> 24) & 255));
      byteout((int)((word >> 16) & 255));
      byteout((int)((word >> 8) & 255));
      byteout((int)(word & 255));
      word = 0;
      bcount = 0;
    }
  } else {
    fatal();
  }
}

byteout(c) 
  reg c;
{
  Ceor ^= c;
  Csum += c;
  Csum += 1;
  if ((Crot & 0x80000000)) {
    Crot <<= 1;
    Crot += 1;
  } else {
    Crot <<= 1;
  }
  Crot += c;
  putc(c, stdout);
}

main(argc, argv) 
  char **argv;
{
  reg c;
  char buf[100];
  long int n1, n2, oeor, osum, orot;

  if (argc != 1) {
    fprintf(stderr,"bad args to %s\n", argv[0]);
    exit(2);
  }
  /*search for header line*/
  for (;;) {
    if (fgets(buf, sizeof buf, stdin) == NULL) {
      fatal();
    }
    if (streq(buf, "xbtoa Begin\n")) {
      break;
    }
  }

  while ((c = getchar()) != EOF) {
    if (c == '\n') {
      continue;
    } else if (c == 'x') {
      break;
    } else {
      decode(c);
    }
  }
  if(scanf("btoa End N %ld %lx E %lx S %lx R %lx\n",
         &n1, &n2, &oeor, &osum, &orot) != 5) {
    fatal();
  }
  if ((n1 != n2) || (oeor != Ceor) || (osum != Csum) || (orot != Crot)) {
    fatal();
  }
  exit(0);
}
