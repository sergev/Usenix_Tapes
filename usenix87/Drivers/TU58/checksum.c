#include "tu58.h"

u_short docksum(header,mbuf)
struct packet *header;
u_char *mbuf;
{
	register int cnt;
	register u_char *cp;
	register u_short psum;
	unsigned long sum;

	cp = mbuf;
	cnt = 0xff & header->pk_mcount;
	sum = 0xffff&(header->pk_mcount<<8) + (0xff&header->pk_flag);

	while( --cnt >= 0 ) {			/* have at least one */
		psum = 0xff&(*cp++);
		if( --cnt >= 0 ) 		/* have at least two */
			psum += 0xffff&((*cp++)<<8);
		sum += psum + ( (long)psum + (0xffff&sum) > 0xffff ? 1 : 0 );
	}

	return (u_short)sum;
}
