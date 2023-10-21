int area[16];
main(argc,argv)
char *argv[];
{
	register a,b,c;
	int d,e,f;

	if(gopen(area,0,0,512,512)) return;

	f = 0;
	if(argc > 1) f = 1;

loop:
	a = (rand() >> 4) & 511;
	b = (rand() >> 4) & 511;
	c = (rand() >> 4) & 511;
	d = (rand() >> 4) & 511;
	e = rand() & 4095;

	genter(area,0,e,0,0,0);

	gwvec(area,a,b,c,d,f);
	if(f) for(a=0;a<25;a++)for(b=0;b<1000;b++);
	goto loop;
}
