/* This is a structure that can be used to overlay the data
 * returned from stat() and fstat() system calls.  It is not
 * used anywhere in the kernel.
 */
struct statbuf
{
	char s_minor;
	char s_major;
	int s_inumber;
	int s_flags;
	char s_nlinks;
	char s_uid;
	char s_gid;
	char s_size0;
	int s_size1;
	int s_addr[8];
	long s_actime;
	long s_modtime;
};
