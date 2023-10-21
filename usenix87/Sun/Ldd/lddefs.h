/* ldreg.h, lzh, 01/31/86 */

/*
 * Structures and definitions for the loadable device driver
 */

#define LD_MAGIC 	96401815	/* Magic number */
#define LD_SIZE 	32768		/* Size of text and data of driver */
#define LD_DEV_SIZE	16		/* Size of device */
#define LD_PROBE_OFFSET 0xe		/* Offset from start of dev to probe */
#define DEMO				/* Device is always present */

struct ld_dev
{
	int	(*ld_open)();
	int	(*ld_close)();
	int	(*ld_read)();
	int	(*ld_write)();
	int	(*ld_ioctl)();
	int	(*ld_mmap)();
	int	(*ld_intr)();
	int	(*ld_strategy)();
	int	(*ld_size)();
	int	(*ld_dump)();
};
