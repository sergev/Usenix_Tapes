
/*
 *      mxreset - reset the local machine.
 */

main()
{
	int fd;
	if((fd=mxfile()) < 0) {
		printf("can't get an mx file to issue stty\n");
		exit();
	}
	mxxs(fd,4,0,0);
}
