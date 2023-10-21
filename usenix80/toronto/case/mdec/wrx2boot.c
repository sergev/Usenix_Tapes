main()
{
	int buf[512];
	seek(0,020,0);	/* skip a.out header */
	seek(1,498,3);	/* seek to end of floppy */
	read(0,buf,1024);
	write(1,buf,1024);
}
