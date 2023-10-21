#include "/usr/sys/sbuf.h"

struct inode inode;

main()
{
	fstat(0,&inode);
	printf("dh %d, line %d\n",(inode.i_addr[0]&0377)/16,
	(inode.i_addr[0]&0377)%16);
}
