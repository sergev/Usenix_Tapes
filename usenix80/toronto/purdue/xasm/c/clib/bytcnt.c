bytcnt(byte)
	int byte;{

register n;
int savb; n = 1 ; savb = byte ; /*initalize variables*/

byte= (byte>>4)&017;		/* get 4 msb in 4 lsb*/
if((byte & 013)==013)n=3;
	else if(((byte & 010)== 010)||((byte & 016)== 06)
			||(byte==02)){
		if((savb==140)||(savb==142)||(savb==206))n=3;
			else n=2;
					}
	if(byte== 7)n = 3;
	if(savb == 143)return(3);
	if(savb == 207)return(3);
	if(savb == 221)return(1);
	return(n);
}
