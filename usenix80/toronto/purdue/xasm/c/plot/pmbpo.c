pmbpo(pstat,x,y)
	int pstat[],x,y;
{
/*	Puts out x and y in HP MBP format.	*/
	register max;
	register char c;

	x =& 037777;	y =& 037777;
	max = x;
	if(y > x)max = y;
	if(max < 4){
		plotp(pstat,0140 + (x << 2) + y);
		return;
		}
	if(max < 32){
		plotp(pstat,0140 + (x >> 1));
		c = ((x <<5) & 040) + y;
		if(!(c & 040))c =+ 0100;
		plotp(pstat,c);
		return;
		}
	if(max < 256){
		plotp(pstat,0140 + (x >> 4));
		c = ((x << 2) & 074) + ((y >> 6) & 3);
		if(!(c & 040))c =+ 0100;
		plotp(pstat,c);
		c = y & 077;
		if(!(c & 040))c =+ 0100;
		plotp(pstat,c);
		return;
		}
	if(max < 2048){
		c = 0140 + ((x >> 7) & 017);
		plotp(pstat,c);
		c = (x >> 1) & 077;
		if(!(c & 040))c =+ 0100;
		plotp(pstat,c);
		c = ((x << 5) & 040) + ((y >> 6)& 037);
		if(!(c & 040))c =+ 0100;
		plotp(pstat,c);
		c = y & 077;
		if(!(c & 040))c =+ 0100;
		plotp(pstat,c);
		return;
		}
	c = (x >> 10) & 017;
	plotp(pstat,c + 0140);
	c = (x >> 4) & 077;
	if(!(c & 040))c =+ 0100;
	plotp(pstat,c);
	c = ((x << 2) & 074) + ((y >> 12) & 3);
	if(!(c & 040))c =+ 0100;
	plotp(pstat,c);
	c = (y >> 6) & 077;
	if(!(c & 040))c =+ 0100;
	plotp(pstat,c);
	c = y & 077;
	if(!(c & 040))c =+ 0100;
	plotp(pstat,c);
}
