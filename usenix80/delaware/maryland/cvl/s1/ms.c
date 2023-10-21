#define SIZE    512
#define MASK    511
#include "/usr/lib/grdefs.c"

int buf[2000], gFD, n;

main(argc, argv)
	int argc;
	char *argv[];

	{
	int pixel, refPixel, i, j, inc;
	register x, y, *ptr;

	inc = -1;
	refPixel = 0330;
	inc = 1;

	if( argc == 1 ) {
		printf("usage: ms <increment> [r][b][g]\n");
		exit();
		}

	else {
		if( argc >= 2 ) {
			inc = atoi(argv[1]);
			if( argc >= 3 ) {
				if( argv[2][0] == 'r' )
					refPixel = 00017;
				else if( argv[2][0] == 'b' )
					refPixel = 00360;
				else if( argv[2][0] == 'g' )
					refPixel = 07400;
				else printf("illegal color parameter\n");
				}
			}
		}

	if( (gFD = open("/dev/gr", 1)) < 0 ) {
		printf("can't open display\n");
		exit();
		}

	clear();
	buf[0] = LDC | 3;
	buf[1] = LSM | 07777;
	buf[2] = LWM | ZEROW;
	buf[3] = LUM | ECA;
	buf[4] = LEA | 0;
	buf[5] = LEC | 1;
	pixel = refPixel;

	for(;;) {
		for( i = 1 ; i <= 2 ; i++ ) {
			for( j = 0 ; j < SIZE ; j++) {
				n =+ inc;
				ptr = &buf[6];

				for( x = 0 ; x < SIZE ; x++ ) {
					y = (n ^ x) & MASK;
					*ptr++ = LLA | y;
					*ptr++ = WID | pixel;
					}

				write( gFD, buf, SIZE*4 + 12 );
				}
			pixel = (pixel == 0) ? refPixel : 0;
			}
		}
	}

clear()
	{
	buf[0] = LWM | LIGHT;
	buf[1] = LEA;
	buf[2] = LLA;
	buf[3] = LEB | MASK;
	buf[4] = LLB | MASK;
	buf[5] = LDC | 3;
	buf[6] = LSM | 07777;
	buf[7] = EGW;
	if( write( gFD, buf, 16 ) != 16 )
		printf("write failed\n");
	}
