#include "/v/wa1yyn/c/plot/plot.h"
setvec(pstat,vsize)
	int pstat[];
	char vsize;
/*
 *	Sets vector type.  Valid vector types are:
 *
 *	' ` '	Straight line
 *	' a '	dotted line ........
 *	' b '	dot-dashed  ._._._._
 *	' c '	short-dash  _ _ _ _
 *	' d '	long-dash   __ __ __
 */
{
	register i,j;
	register char c;

	if((vsize < '`') || (vsize > 'd')){
		printf("Bad vector size - '%c'.\n",vsize);
		return;
		}

    if(pstat[CD] == TEK){
	pstat[CVT] = vsize;
	plotp(pstat,ESC);	plotp(pstat,vsize);
	pstat[CM] =& 177775;
	return;
	}

    if(pstat[CD] == HP){
	if((pstat[CM] & 1)== 0){
		plotp(pstat,ETX);
		pstat[CM] =& 0177774;
		pstat[CM] =+ 1;
		}
	if((pstat[CM] & 2)){
		plotp(pstat,'}');
		pstat[CM] =& 0177774;	pstat[CM] =+ 1;
		}
	pstat[CVT] = vsize;
	plotp(pstat,'~');
	plotp(pstat,'R');
	switch(vsize){
		case '`':
			plotp(pstat,'}');
			break;
		case 'a':
			plotp(pstat,040 + 1);
			plotp(pstat,0100 + 9);
			pmbno(pstat,80);
			break;
		case 'b':
			plotp(pstat,040 + 5);
			plotp(pstat,0100 + 15);
			plotp(pstat,040 + 25);
			plotp(pstat,0100 + 15);
			pmbno(pstat,240);
			break;
		case 'c':
			plotp(pstat,040 + 15);
			plotp(pstat,0100 + 10);
			pmbno(pstat,150);
			break;
		case 'd':
			plotp(pstat,040 + 30);
			plotp(pstat,0100 + 10);
			pmbno(pstat,240);
			break;
		}
	if(pstat[DEBUG]){
		plotp(pstat,'~');	plotp(pstat,'\'');
		pstat[CM] =& 0177774;
		}
	}

}
