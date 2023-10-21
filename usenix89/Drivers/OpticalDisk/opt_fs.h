/* opt_fs.h  4/23/85	*/

/* 	Hitachi Optical Disk Filesystem     */
/*      Author:  Jungbo Yang  7/8/85        */

/* error codes */
#define E_OK		0	/* no error. successful return */
#define E_VOLUME	-1	/* invalid volume */
#define E_OPEN 		-2	/* open error */
#define E_READ		-3	/* read error */
#define E_BADMODE	-4	/* bad open mode */
#define E_NOT_MOUNTED	-5	/* volume not mounted */
#define E_FIXFMS	-6	/* cannot fix FMS */
#define E_FDESC		-7	/* no more free desc block */
#define E_FREE		-8	/* no more free file block */
#define E_WR_DESC	-9	/* write_desc error */
#define E_FREAD		-10	/* fast read error */
#define E_FWRITE	-11	/* fast write error */
#define E_NOVID		-12	/* no more volume block */
#define E_SEEK		-13	/* hseek error */
#define E_SRCHUNWTN	-14	/* srchunwtn error */
#define E_WRITE		-15	/* write error */
#define E_BADFMS	-16	/* bad FMS */
#define E_IOCTL		-17	/* ioctl error return */
#define E_BADPBN	-18	/* bad pbn */
#define E_FULL		-19	/* disk full */
#define E_BADVD		-20	/* bad volume descriptor */
#define E_BADFD		-21	/* bad file descriptor */
#define E_FDMANY	-22	/* too many open files */
#define E_BUSY		-23	/* resource busy */
#define E_EMPTY		-24	/* fs empty */
#define E_BADDRV 	-25	/* bad drive number */
#define E_BADVID	-26	/* bad volume id */
#define E_BADVTYPE	-27	/* bad volume type */
#define E_BADFS		-28	/* bad fs layout */
#define E_BADSIDE	-29	/* bad fs layout */
#define E_BADVLAB	-30	/* bad volume label */
#define E_OUTSWAP	-31	/* out swap error */
#define E_NOT_IN_JUKEBOX	-32	/* volume not found in jukebox */
#define E_DOOR		-33	/* open/close door operation error */
#define E_LOAD		-34	/* platter loading error */
#define E_UNLOAD	-35	/* platter unloading error */
#define E_HK_FILE	-36	/* housekeeping file error */
#define E_FMHK		-37	/* housekeeping file error */
#define E_HK_CELL	-38	/* bad cell entry in hk file */
#define E_DOOR_OPEN	-39	/* jukebox door is open */
#define E_LAST		-39	/* last error code */

/* FMS */
#define VIDLEN 16		/* size of volume id in bytes */
#define FNAMELEN 16
#define VNAMELEN 32
#define FMS_VLEN	(2*62)	/* number of volume block */
#define FMS_NCELL	32	/* number of cells in jukebox */

#define FMS_OK		0	/* fms_dstat, fms_fstat */
#define FMS_DIRTY	1	/* fms_dstat, fms_fstat */
#define FMS_NOT_MOUNTED	0	/* fms_state */
#define FMS_MOUNTED	1	/* fms_state */
#define FMS_WRITE	2	/* fms_state */
#define FMS_EXCLUSIVE	4	/* fms_state */
#define FMS_OPENED	8	/* fms_state */
#define FMS_EMPTY	0x10	/* fms_state */
#define FMS_BADISK	0x20	/* fms_state */
#define FMS_LOCK	0x8000	/* fms_state */
#define FMS_0DRIVE	"/dev/hitaa"	/* drive 0 */
#define FMS_1DRIVE	"/dev/hitab"	/* drive 1 */

#define TYPE_IMAGE	0	/* fs type */
#define TYPE_BACKUP	1	/* fs type */
#define SIDE_A		0	/* platter side a */
#define SIDE_B		1	/* platter side b */

#define FMHK_NAME "fmhousekeeping"
#define NCELL 64	/* number of platters in jukebox */
extern int hk_file(),hk_cell();
#define lockhk_file() hk_file(1)
#define relhk_file() hk_file(0)
#define gethk_cell(a,b) hk_cell(1,a,b)
#define puthk_cell(a,b) hk_cell(0,a,b)

#include <time.h>

typedef struct {         /* FM Structure definition */
    short  fms_state;     /* file system status */
    char fms_vol[VIDLEN];       /* volume ID */
    long fms_dfree;      /* next free PBN in descriptor area */
    short fms_dstat;      /* descriptor area status        */
                         /* status values, such as        */
                         /*        OK    - normal         */
                         /*        DIRTY - d_free is wrong */
    long fms_ffree;      /* next free PBN in file are */
    short fms_fstat;      /* file area status */
                         /* status values, such as        */
                         /*        OK    - normal         */
                         /*        DIRTY - f_free is wrong */
    long fms_vstart;	/* volume area */
    long fms_dstart;	/* desc area */
    long fms_fstart;	/* file area */
    long fms_sstart;	/* spare area */
    short fms_ocnt;	/* open proc count. should be 0 when fms_state
			   is not FMS_OPENED */
    short fms_cell;	/* keeps jukebox cell number here */
} HFMS_t;

/* File descriptor structure */
typedef struct {
	char 	f_name[16];	/* file verification area */
	short	f_version;	/* format version number */
	struct tm f_time;	/* time */
	long	f_type;		/* file type */
	long	f_pbn;		/* starting physical block number */
	long	f_dpbn;		/* desc physical block number */
	long	f_bcnt;		/* length of file in bytes */
	char	f_acc[16];	/* account number (null terminated) */
	char	f_ff[16];	/* file folder number (null terminated) */
	long	f_doc;		/* document number */
	long	f_page;		/* page number */
	long	f_image;	/* image number */
	long	f_doctype;	/* document type */
	long	f_xpos; 	/* x position */
	long	f_ypos; 	/* y position */
	long	f_dispmode;	/* display mode */
	long	f_itype;	/* image type */
} Opt_file_t;

/* Volume block structure */
typedef struct {
	char	v_name[32];	/* volume label verification region */
	short	v_version;	/* label version number */
	char	v_id[VIDLEN];	/* volume ID */
	short	v_vtype;	/* volume type */
	short	v_side;		/* platter side */
	struct tm v_time;	/* time */
	long	v_dfirst;	/* first desc block */
	long	v_ffirst;	/* first data block */
	long	v_sfirst;	/* first spare block */
	char	v_project[32];	/* project name */
	char	v_institution[128];	/* institution description */
} Opt_volume_t;

#define FMS_CLOSE	0
#define FMS_BWRITE	1
#define FMS_BREAD	2
#define FMS_FWRITE	4
#define FMS_FREAD	8
#define FMS_EWRITE	0x10
#define FMS_EREAD	0x20
#define FMS_MAX_FILE	8	/* max # of file can be opened at once */
#define FMS_NDRIVE	2	/* number of optical drives on line */
#define FMS_VD	0x1234		/* magic number for volume desc */
#define FMS_VOPEN 0xf000	/* _vstat volume open bit */

/* globals */

/* optical file struct */
typedef struct {
    int  start_stat;
    int  opt_fd;
    long start_dpbn;
    long start_fpbn;
    long start_cnt;
    Opt_file_t desc;
} _Opt_t;

/* optical volume struct */
typedef struct {
    int vs_stat;
    int vs_fd;
    long vs_vol[VIDLEN];
    long vs_vstart;	/* volume area */
    long vs_dstart;	/* desc area */
    long vs_fstart;	/* file area */
    long vs_sstart;	/* spare area */
    _Opt_t vs_opt[FMS_MAX_FILE];
} _Vol_t;


