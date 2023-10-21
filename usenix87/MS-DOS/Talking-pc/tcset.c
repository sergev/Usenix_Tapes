
/* tcset.c: initialize word replacement table in talking device driver */
/* take word and punctuation pronounciations from talkcon.sys */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
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

char filename[] = "talkcon.sys";
int lineno = 0;
char line[81];
char replace[81];
unsigned wdofs = 0; /*offset into word replacement table */

error(s)
char *s;
{
	fprintf(stderr, "%s: %d, %s\n", filename, lineno, s);
}

/* don't need arguments or environment */
setargv(){
}
setenv(){
}

main()
{
	register unsigned i;
	register char *s;
	FILE *f;
	char c;
	int j;

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
		fprintf(stderr, "tcset: bad ioctl() call on console device driver\n");
		exit(1);
	}

	f = fopen(filename, "r");
	if(!f)
	{
		fprintf(stderr, "tcset: cannot open file %s to initialize talking device driver\n", filename);
		exit(1);
	}

	line[80] = 0;
	devparams.wdreptable[0] = 0;

	while(fgets(line, 80, f))
	{
		++lineno;
		c = line[0];
		i = strlen(line) - 1;
		if(line[i] == '\n') line[i] = 0;

		/* skip empty or indented lines */
		if(isspace(c)) continue;

		if(isdigit(c) || c < ' ')
		{
			error("initial character is invalid");
			continue;
		}

		if(strchr(line + 1, '=')) /* key assignment */
		{
			static numpad[] =
			{
				0x52,0x4f,0x50,0x51,0x4b,0x4c,0x4d,0x47,0x48,0x49			};
			i = 0;
			if(toupper(c) == 'F' && isdigit(line[1]))
			{
				s = line+1;
				i = *s++ - '0';
				if(i == 1 && *s == '0') i = 10, ++s;
				i += 0x3b-1;
			}
			if(c == '^' && isalpha(line[1]))
				s = line+2, i = toupper(line[1]) - 'A' + 1;
			if(c == '#' && isdigit(line[1]))
				s = line+2, i = numpad[line[1] - '0'];
			if(i == 0x4c)
			{
				error("cannot assign a command to #5");
				continue;
			}
			while(isspace(*s)) ++s;
			if(!i || *s++ != '=')
			{
				error("syntax error in key/command assignment");
				continue;
			}
			while(isspace(*s)) ++s;
			c = 127;
			if(isdigit(*s)) c = *s++ - '0';
			if(isdigit(*s)) c = 10*c + *s++ - '0';
			if(c > 24 || isdigit(*s))
			{
				error("key must be assigned a command number from 0 to 24");
				continue;
			}
			if(c == 4)
			{
				error("cannot reassign transparent mode");
				continue;
			}
			devparams.keymap[i] = c;
			continue;
		}

		i = 1;
		if(isalpha(c)) /* word */
		{
			for(i=0; isalpha(line[i]); ++i)
			{
				if(i == devparams.wdlen)
				{
					error("word too long");
					goto nextline;
				}
				replace[i+1] = tolower(line[i]);
			}
		}

		if(!isspace(line[i]))
		{
			error("no space after initial word or punctuation mark");
			continue;
		}

		replace[0] = i;
		for(s=line+i; isspace(*s); ++s)  /* empty */ ;
		i += 2;

		/* get replacement text */
		while(isalnum(*s) || *s == ' ')
		{
			if(i - replace[0] == devparams.wdlen + 2)
			{
				error("replacement text too long");
				goto nextline;
			}
			replace[i++] = tolower(*s);
			++s;
		}
		if(*s)
		{
			error("invalid characters in replacement text");
			continue;
		}
		i -= replace[0] + 2;
		if(!i)
		{
			error("no replacement text");
			continue;
		}
		replace[replace[0] + 1] = i;

		if(isalpha(c))
		{
			i += replace[0] + 2;
			if(wdofs+i+1 >= devparams.wdrepsz)
			{
				error("word replacement table overflow");
				exit(1);
			}
			replace[i]=0;
			j = i;
			while(j >= 0)
			{
				devparams.wdreptable[wdofs + j] = replace[j];
				--j;
			}
			wdofs += i;
			continue;
		}

		/* punctuation mark, compute index */
		if(i > 10)
		{
			error("punctuation replacement cannot exceed 10 characters");
			continue;
		}
		for(++i; i<=10; ++i) replace[i+2] = '~';
		if(c > 'z') c -= 26;
		if(c > 'Z') c -= 26;
		if(c > '9') c -= 10;
		c -= ' '-6;
		for(i=0; i<10; ++i)
			devparams.punctable[10*c + i] = replace[3+i];

nextline:
		;
	}

	exit(0);
}

