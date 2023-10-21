main()
{
	int buf[256];
	seek(0,020,0);	/* skip a.out header */
	read(0,buf,512);
	write(1,buf,512);
}
