#include <stdio.h>

#define NAMSIZ 100
struct matches {
	int offset;
	char value;
} matches[] = {
	NAMSIZ+6,	' ',
	NAMSIZ+7,	'\0',
	NAMSIZ+8+6,	' ',
	NAMSIZ+8+7,	'\0',
	NAMSIZ+16+6,	' ',
	NAMSIZ+16+7,	'\0',
	NAMSIZ+24+11,	' ',
	NAMSIZ+36+11,	' ',
	NAMSIZ+48+6,	'\0',
	0,		0,
};

int
istar(block)
char *block;
{
	int loop;

	for (loop = 0; matches[loop].offset != 0; loop++)
		if (block[matches[loop].offset] != matches[loop].value)
			return(0);
	return(1);
}

char buf[10240];
int bad;
int nleft = 0;
int whichnow;

int opened = 0;
int f;
long nwrite;

main(argc, argv)
int argc;
char **argv;
{
	int loop;
	char *block;
	extern char *readblock();

	bad = 0;

	for(;;) {
		block = readblock(0);

		if (block != NULL)
			doblock(block, argc, argv);
	}
}

doblock(block, argc, argv)
char *block;
int argc;
char **argv;
{
	int count;

	if (istar(block)) {
		if (opened) {
			printf("--- premature end\n");
			close(f);
			opened = 0;
		}
		if (match(block, argc, argv)) {
			f = creat(block, 0666);
			if (f < 0)
				printf("--- unable to create %s\n", block);
			else {
				opened = 1;
				sscanf(block+NAMSIZ+24, "%lo", &nwrite);
				printf("--- reading %s %ld\n", block, nwrite);
				if (nwrite <= 0) {
					close(f);
					opened = 0;
					printf("--- done\n");
				}
			}
		}
	} else {
		if (opened) {
			count = (nwrite > 512) ? 512 : (int)nwrite;
			write(f, block, count);
			nwrite -= count;
			if (nwrite <= 0) {
				opened = 0;
				close(f);
				printf("--- done\n");
			}
		}
	}
}

int
match(s, argc, argv)
char *s;
int argc;
char **argv;
{
	int i;
	int c;

	for (i = 1; i < argc; i++) {
		if (strncmp(s, argv[i], strlen(argv[i])) == 0) {
			c = s[strlen(argv[i])];
			if (c == '\0' || c == '/')
				return(1);
		}
	}
	return(0);
}

char *
readblock(desc)
int desc;
{
	int count;

	if (nleft > 0) {
		whichnow++;
		nleft--;
		return(buf+whichnow*512);
	}

	count = read(desc, buf, (int)sizeof(buf));
	if (count <= 0 || count%512 != 0) {
		if (count == 0)
			printf("---apparent EOF\n");
		else
			printf("---error\n");
		if (bad > 20)
			exit(1);
		bad++;
		return(NULL);
	}
	bad = 0;
	whichnow = 0;
	nleft = count/512 - 1;
	return(buf);
}
