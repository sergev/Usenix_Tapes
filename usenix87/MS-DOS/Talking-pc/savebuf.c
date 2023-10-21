
/* savebuf.c: save the accumulated text in an output file */

#include <stdio.h>
#include <dos.h>

struct
{
	char far *punctable; /* location of punctuation definition table */
	char far *wdreptable; /* location of word replacement table */
	char far *buffer; /* location of internal text buffer */
	char far *keymap; /* location of key/command map */
	unsigned wdrepsz;  /* size of word replacement table */
	short wdlen; /* longest possible word */
	unsigned bufsz;  /* size of internal text buffer */
	unsigned bufstart; /* index into text buffer where valid text begins */
} devparams;

struct WORDREGS regs;

char *filename = "session.log";

setenv(){
} /* no environment */

main(argc, argv)
int argc;
char **argv;
{
	register unsigned i;
	FILE *f;
	register char c;

	if(argc > 1) filename = argv[1];
	if(argc > 2)
	{
		fprintf(stderr, "usage: savebuf [file]\n");
		exit(1);
	}

	/* get device driver parameters from ioctl call */
	/* We assume file handle 0 is asociated with the talking console */
	/* device driver; don't redirect standard input. */
	regs.bx = 0;
	regs.ax = 0x4402; /* read ioctl string */
	regs.cx = 24; /* get 24 bytes */
	regs.dx = (unsigned) &devparams;
	intdos(&regs, &regs);
	if(regs.cflag & 1 || regs.ax != 24)
	{
		fprintf(stderr, "savebuf: bad ioctl() call on console device driver\n");
		exit(1);
	}

	f = fopen(filename, "w");
	if(!f)
	{
		fprintf(stderr, "savebuf: cannot create file %s\n", filename);
		exit(1);
	}

	i = devparams.bufstart;
	while((c = devparams.buffer[i]) >= 0)
	{
		if(c == '\r') c = '\n';
		putc(c,f);
		if(++i == devparams.bufsz) i = 0;
	}

	fclose(f);
	exit(0);
}

