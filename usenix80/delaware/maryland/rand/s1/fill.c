#
/* fill/just - source code for preprocessors used to run roff
	from the Rand Editor.
   Walt Bilofsky - Bolt Beranek and Newman Inc. - 13 September 1976

   Unless the "x" argument is given, preprocessor replaces all multiple
	blanks/tabs by a single blank, and handles each paragraph by
	leaving the indentation alone on the first line, and indenting
	all subsequent lines according to the indentation on the second
	line.
*/

#define JUST 0                  /* 0 for fill, 1 for justify */
#define LBUF 512

char file[200];
int i;

int nch 0;
char *pch,chbuf[512];

main(argc,argv)
char **argv;
{
 int j,nind,noutd;
 char buf[LBUF],*args[50],rflag,bflag,**d,*firstl,e,nflag;
 register char **c, *q, *p;

 p = file;
 for (q = "/tmp/rejust."; *p++ = *q++; );

 if (getpw(0377 & getuid(), p = &file[12])) exit(-2);
 while (*p != ':') p++;
 *p = 0;

 /* Copy standard input over to first argument, adding right commands */
 if ((i = creat(file,0666)) == -1) exit(-2);

 if (JUST == 0) write(i,".na\n",4);
 write(i,".hy 0\n.pl 32767\n",16);                  /* hyphenate off */

 /* Copy argument list either to roff or to roff file */
 c = args;
 d = argv;

 rflag = 0;
 d++;
 *c++ = "/usr/bin/nroff";
 while (*d != -1) {
	if (**d == '.') {
		p = *d;
		while (*p) write(i,p++,1);
		write (i,'\n',1);
		}
	  else if (**d == 'x') rflag = 1;
	  else *c++ = *d;
	d++;
	}
 *c++ = file;
 *c++ = 0;

 nch = 0;

 write(i,".c2 ~\n.cc ~\n~ec ~\n",18);
 nflag = bflag = noutd = nind = 0;
 p = buf;
 for (;;) {
	while ((*p++ = e = ch()) != '\n') if (e == 0) {
		if (bflag == 1) break; goto endit; }
	if (buf - p > LBUF) exit(-2);
	if (rflag) { contin:
		     write(i,buf,p - buf);
		     p = buf;
		     continue; }
	/* bflag - 0 for last line blank
		   1 for last line first nonblank
		   2 for in middle of text. */
	if (p - buf == 1) {                   /* blank line */
		if (bflag && nind) {          /* terminates indented text */
			printf("~~in -%d\n",nind);
			nind = 0;
			}
		bflag = 0;
		/* This horrible fudge copies leading blank lines
		   directly to output file 'cause nroff wont! */
		if (nflag == 0) write(1,"\n",1);
		goto contin;
		}
	nflag = 1;
	if (bflag == 0) {
			q = buf;        /* first nonblank line */
			noutd = 0;
			for (;;) {
				if (q >= p) break;
				if (*q == ' ') noutd++;
				 else if (*q == '\t') noutd = 0177770 &
						       (noutd + 8);
				 else break;
				q++; }
			bflag = 1;
			firstl = p;
			continue;
		 }
	if (bflag == 1) {
		if (p - firstl == 1) bflag = 0;
		else {  bflag = 2;      /* second nonblank line */
			q = firstl;
			nind = 0;
			for (;;) {
				if (q >= p) break;
				if (*q == ' ') nind++;
				 else if (*q == '\t') nind = 0177770 &
						       (nind + 8);
				 else break;
				q++; }
			if (nind) printf("~~in +%d\n",nind);
			if (noutd > nind)
				printf("~~ti +%d\n",noutd - nind);
			if (nind > noutd)
				printf("~~ti -%d\n",nind - noutd);
		     }
	 }
	*p = 0;         /* copy, stripping extra blanks */
	p = q = buf;
	if (bflag == 0) while (*q == ' ' || *q == '\t') *p++ = *q++;
	while (*q == ' ' || *q == '\t') q++;
	for (;;) {
		*p++ = *q;
		if (*q == 0) goto contin;
		if (*q == ' ' || *q == '\t' || *q++ == '\n')
		    while (*q == ' ' || *q == '\t') q++;
			}
	}

endit:
 /* suppress terminal blank lines */
 write (i,"~~pl 0\n",7);
 close(i);

 if ((j = fork()) == 0) {
		execv(args[0],args);
		exit(0);
		}
 while (wait(&i) != j);
 unlink(file);
 }


ch()
{
	if (nch == 0) { nch = read(0,chbuf,512);
			if (nch == 0) return 0;
			pch = chbuf; }
	nch--;
	return *pch++;
}
putchar(c)
{ write(i,&c,1); }
