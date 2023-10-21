int a[16];
int tab[4096];
int row[512];
main(argc,argv)
char *argv[];
{
	register i, j, plane;

	if(argc != 2) { printf("Usage --- gt [0-11]\n");return;}

	gopen(a,0,0,512,512);
	for(i=0;i<4096;i++) tab[i] = i;
	gwtab(a,tab,0,4096);
	plane = atoi(argv[1]);         /* The bit plane to test */
	j = 1 << plane;
	genter(a,4,0,0,0,0);
	gwvec(a,0,0,511,511,1);
	for(i=0;i<4096;i++) tab[i] = 4095;
	tab[j] = 0;
	gwtab(a,tab,0,4096);
}
