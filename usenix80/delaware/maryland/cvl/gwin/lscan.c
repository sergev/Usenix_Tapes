#

		/* Written by Russ Smith --- C.V.L. */

#define DFR     50

int image[16];
int overlay[16];

int cursors[4];

int buf[512];

int minv;

int lc;
int lr;

int length 256;

int flag 0;

main(argc,argv)
char *argv[];
{
	register *c, fc, i;

	if(argc > 1) {
		if((length = atoi(argv[1])) <= 0) {
			printf("\nUsage --- lscan [length [c]]\n\n");
			exit();
		}
		if(argc > 2)
			flag++;
	}

	gopen(image,0,0,512,512);
	gopen(overlay,0,0,512,512);
	genter(overlay,4,0,0,0,0);

	c = cursors;

	for(;;) {
		grcur(image, c, 1);

		gclear(overlay,0,0,512,512,0);

		if(flag) {
			gwvec(overlay,c[0],c[1],c[0],min(511,c[1]+length),0);
			grcol(image, buf, c[0], c[1],minv = min(length,512-c[1]),1);
		}
		else    {
			gwvec(overlay,c[0],c[1],min(511,c[0]+length),c[1],0);
			grrow(image, buf, c[1], c[0],minv = min(length,512-c[0]),1);
		}

		if(c[0] == lc && c[1] == lr) {
			fc = (512 - length) / 2;

			for(i=0;i<minv;i++) {
				gwvec(overlay,fc,DFR,fc,DFR+((buf[i]&07700)>>4),0);
				fc++;
			}
			sleep(3);
		}
		else {
			lc = c[0];
			lr = c[1];
		}
	}
}
