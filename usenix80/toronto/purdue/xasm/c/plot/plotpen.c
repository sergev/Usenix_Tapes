#include "/v/wa1yyn/c/plot/plot.h"
plotpen(pstat,pen)
	int pstat[];	char pen;
{
	if(pstat[CD] == HP){
		if((pstat[CM] & 1) == 0){
			plotp(pstat,ETX);
			pstat[CM] =& 0177774;
			pstat[CM] =+ 1;
			}
		if((pen < '0')||(pen > '4'))return;
		if(pstat[CM] & 2){
			plotp(pstat,'}');
			pstat[CM] =& 0177774;	pstat[CM] =+ 1;
			}
		plotp(pstat,'v');
		plotp(pstat,0100 + (pen - '0'));
		if(pstat[DEBUG]){
			plotp(pstat,'~');
			plotp(pstat,'\'');
			pstat[CM] =& 0177774;
			}
		}
}
