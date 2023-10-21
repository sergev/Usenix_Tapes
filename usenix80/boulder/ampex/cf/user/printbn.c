/* Print the contents of a bnode.  Args are:
					1. file name
					2. logical block # in file
 */
#include "/cf.h"
main(argc, argv)
char **argv;
{
	struct bnode *bp;
	struct bnbuf bn;
	struct key *k;
	int i, j, nf, bno;
	char *name;

	name = argv[1];
	if(argc < 2 || (i = open(name, 0)) < 0)
		error("arg");
	bno = 0;
	if(argc > 2)
		bno = atoi(argv[2]);
	if(bno < 0)
		error("arg 2");
	if(seek(i, bno, 3) < 0 || read(i, &bn, 512) < 0)
		error("read");
	bp = &bn;
	k = bp->keys;
	nf = bp->nfarea;
	printf("Bnode in file %s residing on block %d:\n", argv[1], bno);
	printf("no. free areas at this level = %d.\n", nf);
	for(i=0; i<nf; i++) {
		printf("lson = %d, max = %d, base = %d, last = %d\n",
			k->lson, k->max, k->base, k->last);
		k++;
	}
	printf("rson = %d, max = %d.\n", k->lson, k->max);
}

error(str){printf("error:  %s.\n", str); exit();}
