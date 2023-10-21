/*
 *	this program erases the screen on file descriptor 1, of type type
 *	where type is 1-tektronics 4023 2-beehive 3-adds 4-adm3 5-ann arbor
 */

eras(type)
int type;
{
	switch(type)
	{
	  case 1:
		write(1,"\33\14",2);
		break;
	  case 2:
		write(1,"\033E",2);
		break;
	  case 3:
		write(1,"\014",1);
		break;
	  case 4:
		write(1,"\032",1);
		break;
	  case 5:
		write(1,"\014",1);
		break;
	  default:
		write(2,"eras\n",5);
	  }
}
