#include "/v/wa1yyn/c/plot/plot.h"
setchr(pstat,csize)
	int pstat[];
	char csize;
/*
 *	Sets charactor size to correct size.  Valid sizes are
 *	0 - 4.  Note that the TEK device does not support a size
 *	0 charactor.  If using 'plotc', the routine takes care of
 *	this.  Otherwise, the charactor size is 1.
 */
{
	register i,j;
	register char c;

	if((csize < '0') || (csize > '4')){
		printf("Bad charactor size - '%c'.\n",csize);
		return;
		}

    if(pstat[CD] == TEK){
		pstat[CCS] = csize;
		pstat[CM] =& 0177775;
		if(csize == '0')csize = '1';
		csize =- '1';
		csize = ';' - csize;
		plotp(pstat,ESC);
		plotp(pstat,csize);
		}

    if(pstat[CD] == HP){
	pstat[CCS] = csize;
	if((pstat[CM] & 1)== 0){
		plotp(pstat,ETX);
		pstat[CM] =& 0177774;
		pstat[CM] =+ 1;
		}
	if((pstat[CM] & 2)){
		plotp(pstat,'}');
		pstat[CM] =& 0177774;	pstat[CM] =+ 1;
		}
	switch(csize){
		case '0':
			i = 50;	j = 80;
			break;
		case '1':
			i = 77;	j = 122;
			break;
		case '2':
			i = 85;	j = 135;
			break;
		case '3':
			i = 126;	j = 206;
			break;
		case '4':
			i = 138;	j = 223;
			break;
		}
	plotp(pstat,'~');	plotp(pstat,'%');
	pmbpo(pstat,i,j);
	if(pstat[DEBUG]){
		plotp(pstat,'~');
		plotp(pstat,'\'');
		pstat[CM] =& 0177774;
		}
	}
}
