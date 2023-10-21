#
#define HSIZE   12
#define HSIZE2  6
#define NC      1
#define NR      3
#define BS      5

int head[HSIZE2];
main(argc,argv)
char *argv[];
{
	register i;

	if(argc != 2) {
		printf("\n\tUSAGE --- header picfile\n\n");
		return;
	}

	if ((i = open(argv[1], 0)) < 0) {
		printf("\n\tCan't open %s\n\n",argv[1]);
		return;
	}

	if(read(i, head, HSIZE) != HSIZE) {
		printf("\n\tCan not read in the header\n\n");
		return;
	}

	printf("\n\t\07%s has %d columns and %d rows of %d-byte pixels.\n\n",argv[1],head[NC],head[NR],head[BS]);
}
