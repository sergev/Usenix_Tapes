#include "tu58.h"
#include <stdio.h>
#ifndef	EMPTY
#include <fcntl.h>
#endif
#include "debug.h"

 extern int r_device, w_device;
 extern int errno;

 char wbuf[512];
 int wcnt = 0;
 char rbuf[512];
 int rcnt = 0;
 char *rptr;

cpeak()
{
#ifndef	EMPTY
	fcntl( r_device, F_SETFL, FNDELAY | fcntl(r_device,F_GETFL,0) );
	rcnt = read(r_device, rbuf, sizeof rbuf);
	fcntl( r_device, F_SETFL, ~FNDELAY & fcntl(r_device,F_GETFL,0) );
#else
	if (empty(r_device))
		return -1;
	rcnt = read(r_device, rbuf, sizeof rbuf);
#endif
	if( rcnt < 0 ) 
		return -1;
	rptr = rbuf;
	return (*rptr&0377);
}

cget()				/* return char from rbuf */
{
	register char *cp;
	register int cnt;

	if (rcnt <= 0) {
		if( (rcnt = read(r_device, rbuf, sizeof rbuf)) < 0) {
			debugd("Read file: err %d\n",errno);
			return -1;
			}
		rptr = rbuf;
	}
	rcnt--;
	return 0377&(*rptr++);
}

cflush()
{
		if (wcnt && write(w_device, wbuf, wcnt) != wcnt)
			fprintf(stderr, "Packet write error\n");
		wcnt = 0;
}

cput(c)			/* put char on wbuf */
register char c;
{
	static char *wpnt;
	static char *wptr;

	if (wcnt >= sizeof wbuf)
		cflush();
	if (wcnt == 0)
		wptr = wbuf;
	wcnt++;
	*wptr++ = c;
}

sendpacket(header,mbuf)
u_char *mbuf;
struct packet *header;
{
	register int cnt;
	register u_char *cp;
	u_short docksum(), sum;

	cput(header->pk_flag);
	cput(header->pk_mcount);
	for( cp = mbuf, cnt = 0xff & header->pk_mcount; cnt; cnt--, cp++)
		cput(*cp);
	sum = docksum(header,mbuf);
	debugx("chsum",sum);
	cput(0377&sum);
	cput(0377&sum>>8);
	cflush();
}

getpacket(header,mbuf)	/* get message part of packet. already got header */
u_char *mbuf;
struct packet *header;
{
	register int cnt;
	register u_char *cp;
	u_short docksum();

	for(cnt = 0xff & header->pk_mcount, cp = mbuf; cnt; cnt--)
		*cp++ = 0377&cget();
	cnt = 0377&cget();		/* checksum */
	cnt += cget()<<8;
	return( (u_short)cnt == docksum(header, mbuf) );
}

clearcbuf()
{
	rcnt = wcnt = 0;
}

bootio(fd)			/* read of boot is not in packet format */
int fd;
{
	int count;

	if( lseek(fd, 0, 0) < 0)
		fprintf(stderr,"Boot seek error\n");
	if( (count = read(fd, wbuf, 512)) != 512 )
		fprintf(stderr,"Boot read error\n");
	if( 512 != write(w_device, wbuf, 512) )
		fprintf(stderr,"Boot write error\n");
}
