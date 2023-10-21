/* this program erases the screen on file descriptor fd, of type type
where type is 1-tektronics 4023 2-beehive 3-adds 4-adm3 5-ann arbor
*/
eras(fd,type)
int fd,type;
	{
	register int i;
	switch(type)
	{
	  case 1:
		write(fd,"\33\14",2);
		break;
	  case 2:
		write(fd,"\033E",2);
		/* this may not be needed ... */
		for(i = 0; i < 23; i++)
write(fd,"                                                                                    ",80);
		break;
	  case 3:
		write(fd,"\014",1);
		break;
	  case 4:
		write(fd,"\032",1);
		break;
	  case 5:
		write(fd,"\014",1);
		break;
	  default:
		printf("bad arg to erase %d",type);
	  }
	}
