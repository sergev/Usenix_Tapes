/*
**      This routine will load  the Grinnell lookup table entries
**      with the values 0->4080 by 16's (sixteens) or the values
**      0->4095 by 1's (one's), depending on argc.
*/
int a[16];
int tab[4096] { 0};     /* Contains table values to be written out */
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
		case 'u':       /* 12 -> 8 bit mapping */
			value = 0;
			for(i=0;i<256;i++) {
				for(cnt=0;cnt<16;cnt++)
					*rtab++ = value;
				value =+ 16;
			}
			break;
		case 't':       /* Threshold at input value */
			value = 0;
			cnt = atoi(argv[2]);
			for(i=0;i<cnt;i++) *rtab++ = value;
			value = 07777;
			for(i=cnt;i<4096;i++) *rtab++ = value;
			break;
		default:
			printf("Unknown value\n");
			return;
		}
		gwtab(a,tab,0,4096);  /* Blow away the table */
	}
	else
		printf("\n\tUsage --- gtbld [u][s][t val]\n\n");
}
