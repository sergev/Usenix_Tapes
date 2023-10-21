#include <stdio.h>
#include <sgtty.h>

hpopen_(mdevin,mdevot)
long *mdevin, *mdevot;

{
	int fin, fot;
	struct sgttyb st;

	fin = open("/dev/hp",2);
	if (fin == -1) {
		printf("Device busy.\n");
		exit(-1);
	}
	gtty(fin,&st);
	st.sg_ospeed = st.sg_ispeed = B2400;
	stty(fin,&st);


	*mdevin = fin;
	*mdevot = fin;
	return(0);
}
