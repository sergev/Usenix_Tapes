int a[16];
main(argc,argv)
{
	gopen(a,0,0,512,512);
	if(argc == 1) genter(a,7,0,0,0,0);
	if(argc == 2) genter(a,4,0,0,0,0);
	if(argc == 3) genter(a,3,0,0,0,0);
	gclear(a,0,0,512,512,0);
}
