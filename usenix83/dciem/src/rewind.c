/*
 * rewind - rewind magtape
 *
 * Written by Hughes O'Neil, 19/May/77
 *
 * Modified for V7 by Alexis Kwan, 16/Feb/81
 */

#include <stdio.h>

main (argc, argv)
	int argc;
	char *argv[];
{
	int mt;
	int sflag;
	char *dev;

	sflag = 0;
	argc--;
	dev = "/dev/rmt$";
	while (argc--) {
		while (*argv[1] == '-')
			*argv[1]++;
		switch (*argv[1]) {
		case 's':
			if (!sflag)
				sflag = 1;
			else
				fprintf(stderr,"s option repeated, ignored\n");
			break;
		case '0':
		case '1':
			if (dev[8] == '$')
				dev[8] = *argv[1]++;
			else {
				fprintf(stderr,"Only one tape unit per command please\n");
				exit(1);
			}
			break;
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			fprintf(stderr,"Unknown tape unit specified, %c\n",*argv[1]);
			exit(1);
		default:
			fprintf(stderr,"Unknown option\n");
			exit(1);
		}
		argv++;
	}

	if (dev[8] == '$') 
		dev[8] = '0';
	if ((mt = open(dev,0)) == -1) {
		fprintf(stderr,"rewind: can't open %s\n",dev);
		exit(1);
	}
	close(mt);
	if (sflag) {
		while( (mt = open(dev,0)) == -1 )
			sleep(1);
		close(mt);
	}
	exit(0);
}
