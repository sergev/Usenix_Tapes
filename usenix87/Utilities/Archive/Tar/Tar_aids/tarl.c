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

main()
{
	int loop;
	int dir;
	char *block;
	extern char *readblock();

	bad = 0;

	for(;;) {
		block = readblock(0);

		if (block != NULL && istar(block))
			printf("%s\n", block);
	}
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
