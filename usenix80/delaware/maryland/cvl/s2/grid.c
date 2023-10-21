int a[16];
int b[512];
main()
{
	register i;

	for(i=0;i<512;i++)
		b[i] = 07777;

	gopen(a,0,0,512,512);

	genter(a,4,0,0,0,0);

	for(i=0;i<512;i =+ 32)
		gwrow(a,b,i,0,512,1);
	gwrow(a,b,--i,0,512,1);

	for(i=0;i<512;i =+ 32)
		gwcol(a,b,i,0,512,1);
	gwcol(a,b,--i,0,512,1);
}
