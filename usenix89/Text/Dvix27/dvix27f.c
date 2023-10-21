/*
 *    dvipsf.c - output filter for postscript to printer
 *      uses these other programs:
 *        /usr/lib/postf   (the lpr filter for postscript to laserwriter)
 *        /usr/local/bin/dvi2ps (dvi to postscript translator)
 *        /usr/lib/tex.ps  (the postscript prologue to support TeX)
 *
 *    Copyright 1985 Massachusetts Institute of Technology
 *    Author: CJL@OZ
 *
 */

/* #define DEBUGGING */

#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define PROLOGUE   "/usr/lib/tex.ps"
#ifndef	DVI2PS
#define DVI2PS     "/usr/local/bin/dvi2ps -q"
#endif
#define CAT        "/bin/cat"
#ifndef	POSTF
#define POSTF      "/usr/lib/applef"
#endif
#ifndef	TEXTF
#define TEXTF      "/usr/lib/applef"
#endif

FILE *popen();
char *user,*host;

main(argc, argv) 
    int argc;
    char *argv[];
{
  char tfn[BUFSIZ],cmdbuf[BUFSIZ],efn[BUFSIZ];
  register int ch;
  register FILE *tf;
  int olderrfd,i;
  char *rindex();
  FILE *ef,*of;

  if (rindex(argv[0],'/')) argv[0] = rindex(argv[0],'/')+1;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') switch (argv[i][1]) {
    case 'h':
      host = argv[++i];
      break;

    case 'n':
      user = argv[++i];
      break;

    default:
      break;
    }
  }
  /* Copy the dvi data to a temp file, since we have to give it by name
   * to dvi2ps.
   */
  if ((tf = fopen(mktemp(strcpy(tfn,"/usr/tmp/dvipsf.XXXXXX")),"w"))
      == NULL) {
    perror(tfn);
    exit(2);
  }
  while (!ferror(stdin) && (ch = getchar()) != EOF) putc(ch,tf);
  fclose(tf);
  if (ferror(stdin)) {
    perror("dvipsf stdin");
    unlink(tfn);
    exit(2);
  }

  /* Now divert standard error to a temp file so we can print it later. */
  if ((ef = fopen(mktemp(strcpy(efn,"/usr/tmp/dvipsf.err.XXXXXX")),"w+"))
      == NULL) {
    perror(efn);
    unlink(tfn);
    exit(2);
  }
  unlink(efn);
  fflush(stderr); olderrfd = dup(2); close(2);
  dup(fileno(ef));

  /* Run the filters */
  if (user != NULL && host != NULL) 
    sprintf(cmdbuf,"%s %s | %s -n %s -h %s",DVI2PS,tfn,POSTF,user,host);
  else
    sprintf(cmdbuf,"%s %s | %s", DVI2PS, tfn, POSTF);
  system(cmdbuf);
  unlink(tfn);

  /* Now put back the old stderr */
  fflush(stderr); close(2); dup(olderrfd); close(olderrfd);

  /* And send all the messages to the printer */
  if (user != NULL && host != NULL) 
    sprintf(cmdbuf,"%s -w80 -l66 -n %s -h %s",TEXTF,user,host);
  else
    sprintf(cmdbuf,"%s -w80 -l66", TEXTF);
  if ((of = popen(cmdbuf,"w")) == NULL) {
    perror(TEXTF);
    exit(2);
  }
  rewind(ef);
  while (!ferror(ef) && (ch = getc(ef)) != EOF) if (ch != '\031') {
    putc(ch,of);
#ifdef DEBUGGING
    putc(ch,stderr);
#endif
  }
  fclose(ef); pclose(of);
  if (ferror(ef)) {
    perror(efn);
    exit(2);
  }
  
  exit(0);
}
