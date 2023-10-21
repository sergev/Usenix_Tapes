#
/*
 * Search: news printer (search rap - srap.c)
 * Greg Ordy, Case Western Reserve University, 1979
 * Copyright (c) Greg Ordy 1979
*/

#define		NEWSFILE	"/nmpc/usr/ordy/bin/.snews"

main()
{
	register int	fd,count;
	char		buf[128];

	if((fd = open(NEWSFILE,0)) < 0)
	{
		printf("No search rap\n");
		exit(0);
	}
	while((count = read(fd,buf,128)) > 0)
		write(1,buf,count);
}
