

program uudecode;

  CONST defaultSuffix = '.uue';
        offset = 32;

  TYPE string80 = string[80];

  VAR infile: text;
      outfile: file of byte;
      lineNum: integer;
      line: string80;

  procedure Abort(message: string80);

    begin {abort}
      writeln;
      if lineNum > 0 then write('Line ', lineNum, ': ');
      writeln(message);
      halt
    end; {Abort}

  procedure NextLine(var s: string80);

    begin {NextLine}
      LineNum := succ(LineNum);
      write('.');
      readln(infile, s)
    end; {NextLine}

  procedure Init;

    procedure GetInFile;

      VAR infilename: string80;

      begin {GetInFile}
        if ParamCount = 0 then abort ('Usage: uudecode <filename>');
        infilename := ParamStr(1);
        if pos('.', infilename) = 0
          then infilename := concat(infilename, defaultSuffix);
        assign(infile, infilename);
        {$i-}
        reset(infile);
        {$i+}
        if IOresult > 0 then abort (concat('Can''t open ', infilename));
        writeln ('Decoding ', infilename)
      end; {GetInFile}

    procedure GetOutFile;

      var header, mode, outfilename: string80;
          ch: char;

      procedure ParseHeader;

        VAR index: integer;

        Procedure NextWord(var word:string80; var index: integer);

          begin {nextword}
            word := '';
            while header[index] = ' ' do
              begin
                index := succ(index);
                if index > length(header) then abort ('Incomplete header')
              end;
            while header[index] <> ' ' do
              begin
                word := concat(word, header[index]);
                index := succ(index)
              end
          end; {NextWord}

        begin {ParseHeader}
          header := concat(header, ' ');
          index := 7;
          NextWord(mode, index);
          NextWord(outfilename, index)
        end; {ParseHeader}

      begin {GetOutFile}
        if eof(infile) then abort('Nothing to decode.');
        NextLine (header);
        while not ((copy(header, 1, 6) = 'begin ') or eof(infile)) do
          NextLine(header);
        writeln;
        if eof(infile) then abort('Nothing to decode.');
        ParseHeader;
        assign(outfile, outfilename);
        writeln ('Destination is ', outfilename);
        {$i-}
        reset(outfile);
        {$i+}
        if IOresult = 0 then
          begin
            write ('Overwrite current ', outfilename, '? [Y/N] ');
            repeat
              read (kbd, ch);
              ch := UpCase(ch)
            until ch in ['Y', 'N'];
            writeln(ch);
            if ch = 'N' then abort ('Overwrite cancelled.')
          end;
        rewrite (outfile);
      end; {GetOutFile}

    begin {init}
      lineNum := 0;
      GetInFile;
      GetOutFile;
    end; { init}

  Function CheckLine: boolean;

    begin {CheckLine}
      if line = '' then abort ('Blank line in file');
      CheckLine := not (line[1] in [' ', '`'])
    end; {CheckLine}


  procedure DecodeLine;

    VAR lineIndex, byteNum, count, i: integer;
        chars: array [0..3] of byte;
        hunk: array [0..2] of byte;

{    procedure debug;

      var i: integer;

      procedure writebin(x: byte);

        var i: integer;

        begin
          for i := 1 to 8 do
            begin
              write ((x and $80) shr 7);
              x := x shl 1
            end;
          write (' ')
        end;

      begin
        writeln;
        for i := 0 to 3 do writebin(chars[i]);
        writeln;
        for i := 0 to 2 do writebin(hunk[i]);
        writeln
      end;      }

    function nextch: char;

      begin {nextch}
        lineIndex := succ(lineIndex);
        if lineIndex > length(line) then abort('Line too short.');
        if not (line[lineindex] in [' '..'`'])
          then abort('Illegal character in line.');
{        write(line[lineindex]:2);}
        if line[lineindex] = '`' then nextch := ' '
                                  else nextch := line[lineIndex]
      end; {nextch}

    procedure DecodeByte;

      procedure GetNextHunk;

        VAR i: integer;

        begin {GetNextHunk}
          for i := 0 to 3 do chars[i] := ord(nextch) - offset;
          hunk[0] := (chars[0] shl 2) + (chars[1] shr 4);
          hunk[1] := (chars[1] shl 4) + (chars[2] shr 2);
          hunk[2] := (chars[2] shl 6) + chars[3];
          byteNum := 0  {;
          debug          }
        end; {GetNextHunk}

      begin {DecodeByte}
        if byteNum = 3 then GetNextHunk;
        write (outfile, hunk[byteNum]);
        {writeln(bytenum, ' ', hunk[byteNum]);}
        byteNum := succ(byteNum)
      end; {DecodeByte}

    begin {DecodeLine}
      lineIndex := 0;
      byteNum := 3;
      count := (ord(nextch) - offset);
      for i := 1 to count do DecodeByte
    end; {DecodeLine}

  procedure terminate;

    var trailer: string80;

    begin {terminate}
      if eof(infile) then abort ('Abnormal end.');
      NextLine (trailer);
      if length (trailer) < 3 then abort ('Abnormal end.');
      if copy (trailer, 1, 3) <> 'end' then abort ('Abnormal end.');
      close (infile);
      close (outfile)
    end;

  begin {uudecode}
    init;
    NextLine(line);
    while CheckLine do
      begin
        DecodeLine;
        NextLine(line)
      end;
    terminate
  end.

**************************************************************************


	...and now the C version:


*************************************************************************

     
uudecode and uuencode are easily implemented under MSDOS as well.  Here
are the sources for Microsoft C v3.0, but if you have another kind of C
compiler, there should be perhaps only 1 change -- the output file of
uudecode and the input file of uuencode must be in binary format.
(ie.  binary files, like .EXE files may have byte patterns that are the
same as ^Z, which signals end-of-file in non-binary (text) mode).

If you need more info, write back.  Note that the included files are
*not* in "shar" format -- you will have to use an editor to cut the
files out.

	Don Kneller
UUCP:	...ucbvax!ucsfcgl!kneller
ARPA:	kneller@ucsf-cgl.ARPA
BITNET:	kneller@ucsfcgl.BITNET

====================================================
#ifndef lint
static char sccsid[] = "@(#)uuencode.c	5.1 (Berkeley) 7/2/83";
#endif

/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) (((c) & 077) + ' ')

main(argc, argv)
char **argv;
{
	FILE *in;
	struct stat sbuf;
	int mode;

	/* optional 1st argument */
	if (argc > 2) {
#ifdef MSDOS
		/* Use binary mode */
		if ((in = fopen(argv[1], "rb")) == NULL) {
#else
		if ((in = fopen(argv[1], "r")) == NULL) {
#endif
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

	if (argc != 2) {
		printf("Usage: uuencode [infile] remotefile\n");
		exit(2);
	}

	/* figure out the input file mode */
	fstat(fileno(in), &sbuf);
	mode = sbuf.st_mode & 0777;
	printf("begin %o %s\n", mode, argv[1]);

	encode(in, stdout);

	printf("end\n");
	exit(0);
}

/*
 * copy from in to out, encoding as you go along.
 */
encode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	int i, n;

	for (;;) {
		/* 1 (up to) 45 character line */
		n = fr(in, buf, 45);
		putc(ENC(n), out);

		for (i=0; i<n; i += 3)
			outdec(&buf[i], out);

		putc('\n', out);
		if (n <= 0)
			break;
	}
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
outdec(p, f)
char *p;
FILE *f;
{
	int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	putc(ENC(c1), f);
	putc(ENC(c2), f);
	putc(ENC(c3), f);
	putc(ENC(c4), f);
}

/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
	int c, i;

	for (i=0; i<cnt; i++) {
		c = getc(fd);
		if (c == EOF)
			return(i);
		buf[i] = c;
	}
	return (cnt);
}
====================================================
#ifndef lint
static char sccsid[] = "@(#)uudecode.c	5.1 (Berkeley) 7/2/83";
#endif

/*
 * uudecode [input]
 *
 * create the specified file, decoding as you go.
 * used with uuencode.
 */
#include <stdio.h>
#ifndef MSDOS
#include <pwd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

/* single character decode */
#define DEC(c)	(((c) - ' ') & 077)

main(argc, argv)
char **argv;
{
	FILE *in, *out;
	struct stat sbuf;
	int mode;
	char dest[128];
	char buf[80];

	/* optional input arg */
	if (argc > 1) {
		if ((in = fopen(argv[1], "r")) == NULL) {
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

	if (argc != 1) {
		printf("Usage: uudecode [infile]\n");
		exit(2);
	}

	/* search for header line */
	for (;;) {
		if (fgets(buf, sizeof buf, in) == NULL) {
			fprintf(stderr, "No begin line\n");
			exit(3);
		}
		if (strncmp(buf, "begin ", 6) == 0)
			break;
	}
	sscanf(buf, "begin %o %s", &mode, dest);

	/* handle ~user/file format */
#ifndef MSDOS
	if (dest[0] == '~') {
		char *sl;
		struct passwd *getpwnam();
		char *index();
		struct passwd *user;
		char dnbuf[100];

		sl = index(dest, '/');
		if (sl == NULL) {
			fprintf(stderr, "Illegal ~user\n");
			exit(3);
		}
		*sl++ = 0;
		user = getpwnam(dest+1);
		if (user == NULL) {
			fprintf(stderr, "No such user as %s\n", dest);
			exit(4);
		}
		strcpy(dnbuf, user->pw_dir);
		strcat(dnbuf, "/");
		strcat(dnbuf, sl);
		strcpy(dest, dnbuf);
	}
#endif

	/* create output file */
#ifdef MSDOS
	/* binary output file */
	out = fopen(dest, "wb");
#else
	out = fopen(dest, "w");
#endif
	if (out == NULL) {
		perror(dest);
		exit(4);
	}
	chmod(dest, mode);

	decode(in, out);

	if (fgets(buf, sizeof buf, in) == NULL || strcmp(buf, "end\n")) {
		fprintf(stderr, "No end line\n");
		exit(5);
	}
	exit(0);
}

/*
 * copy from in to out, decoding as you go along.
 */
decode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	char *bp;
	int n;

	for (;;) {
		/* for each input line */
		if (fgets(buf, sizeof buf, in) == NULL) {
			printf("Short file\n");
			exit(10);
		}
		n = DEC(buf[0]);
		if (n <= 0)
			break;

		bp = &buf[1];
		while (n > 0) {
			outdec(bp, out, n);
			bp += 4;
			n -= 3;
		}
	}
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 */
outdec(p, f, n)
char *p;
FILE *f;
{
	int c1, c2, c3;

	c1 = DEC(*p) << 2 | DEC(p[1]) >> 4;
	c2 = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
	c3 = DEC(p[2]) << 6 | DEC(p[3]);
	if (n >= 1)
		putc(c1, f);
	if (n >= 2)
		putc(c2, f);
	if (n >= 3)
		putc(c3, f);
}


/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
	int c, i;

	for (i=0; i<cnt; i++) {
		c = getc(fd);
		if (c == EOF)
			return(i);
		buf[i] = c;
	}
	return (cnt);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

#define	NULL	0

char *
index(sp, c)
register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return(NULL);
}
