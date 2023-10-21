#include "/v/wa1yyn/c/plot/plot.h"
movec(pstat,c)
	int pstat[];
	char c;
/*
 *	Ajust X and Y to reflect charactor 'c' being printed.
 */
{
	register vert,horz;

	switch(pstat[CCS]){
		case '0':
			vert = 80;	horz = 50;
			break;
		case '1':
			vert = 122;	horz = 77;
			break;
		case '2':
			vert = 135;	horz = 85;
			break;
		case '3':
			vert = 206;	horz = 126;
			break;
		case '4':
			vert = 223;	horz = 138;
			break;
		default:
			printf("Bad charactor mode- '%c'.\n",pstat[CCS]);
		}
    switch(c){
	case CR:
		pstat[X] = 0;
		break;
	case LF:
		pstat[Y] =- vert;
		break;
	case VT:
		pstat[Y] =+ vert;
		break;
	case BS:
		pstat[X] =- horz;
		break;
	default:
		pstat[X] =+ horz;
	}

}
