#include "uutty.h"
/*
** Close and re-open the port.
*/
opendev()
{
	D6("opendev()");
	if (dev >= 0) close(dev);
	if (device) {
		if ((dev = open(device,2)) < 0) {
			E("FATAL ERROR\n%s Can't open \"%s\"\t[errno=%d]",getime(),device,errno);
			die(1);
		}
		if (debug) P("%s Opened dev=%d=\"%s\".",getime(),dev,device);
	}
	return dev;
}
