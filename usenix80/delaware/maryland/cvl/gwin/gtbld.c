int a[16];
int tab[4096];     /* Contains table values to be written out */
main(argc, argv)
char **argv;
{
	register value, cnt;
	register *rtab;
	int i;

	gopen(a,0,0,512,512);
	rtab = tab;
	if(argc > 1) {
		switch(argv[1][0]){

		case 's':       /* Same as input */
			for (cnt=0;cnt<4096;cnt++)
				*rtab++ = cnt;
			break;
		case 'r':       /* Complement of input */
			value = 4095;
			for (cnt=0;cnt<4096;cnt++)
				*rtab++ = value--;
			break;
		case 't':       /* Threshold at input value */
			value = 0;
			if((cnt = atoi(argv[2])) < 0 || cnt > 4095) {
				printf("tval should be between 0 and 4095\n");
				exit();
			}
			for(i=0;i<cnt;i++) *rtab++ = value;
			value = 07777;
			for(i=cnt;i<4096;i++) *rtab++ = value;
			break;
		default:
			printf("Unknown key\n");
			return;
		}
		gwtab(a,tab,0,4096);  /* Blow away the table */
	}
	else
		printf("\n\tUsage --- gtbld [s r [t tval]]\n\n");
}
