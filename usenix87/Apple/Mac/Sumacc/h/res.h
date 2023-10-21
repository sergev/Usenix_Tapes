/*	res.h	1.0	04/11/84	*/

/*
 * Resource and segment manager definitions.
 */

/* 
 * Copyright (C) 1984, Stanford Univ. SUMEX project.  
 * May be used but not sold without permission.
 */

/*
 * history
 * 04/11/84	Croft	Created.
 */

/*
 * Resource file format.
 */

struct resfile {		/* resource file header */
	long	rf_offdata;	/* offset to data */
	long	rf_offmap;	/* offset to map */
	long	rf_lendata;	/* length of data */
	long	rf_lenmap;	/* length of map */
};

#define	OFFDATA	256		/* normal offset to data */
#define	ROUNDMAP 256		/* offmap rounded to multiple */
#define	lendata_t long		/* length field preceding each data */


struct resmap {			/* resource map header */
	struct resfile rm_file;	/* (0) copy of file header */
	long	rm_handle;	/* (0) incore handle */
	short	rm_refnum;	/* (0) incore file reference number */
	short	rm_fileatt;	/* file attributes */
	short	rm_offtype;	/* offset from rf_offmap to type list */
	short	rm_offname;	/* offset from rf_offmap to name list */
};

#define	numtypes_t short	/* number of types (-1) preceding type list */


struct restype {		/* resource type list */
	char	rt_type[4];	/* 4 byte upper-case type name */
	short	rt_numids;	/* number of id's of this type */
	short	rt_offids;	/* offset from type list to id list */
};


struct resid {			/* resource id list */
	short	ri_id;		/* resource id */
	short	ri_offname;	/* offset from rm_offname to our name */
	char	ri_att;		/* resource attributes */
	char	ri_offdatahi;	/* hi byte of offset to data */
	short	ri_offdata;	/* offset from rf_offdata to our data */
	short	ri_sysid;	/* id, if system reference */
	short	ri_sysoffname;	/* offset to name, if sys reference */
};

/* resource attributes */

#define	ATT_SYSREF	0200	/* system reference */
#define ATT_SYSHEAP	0100	/* if read into system heap */
#define ATT_PURGEABLE	040	/* purgeable */
#define ATT_LOCKED	020	/* locked */
#define ATT_PROTECTED	010	/* protected */
#define	ATT_PRELOAD	4	/* if to be preloaded */
#define ATT_CHANGED	2	/* (incore) set to rewrite changes */
#define ATT_USER	1	/* (not used) avail for user program */


/*
 * Segment loader formats.
 */

struct jumphead {		/* jump table header */
	long	jh_above;	/* (0x28) size above A5 (sizeof jumphead
				+ size of entire jump table */
	long	jh_below;	/* (0x200) size below A5 (pascal globals)*/
	long	jh_length;	/* (0x8) size of jump table proper */
	long	jh_offset;	/* (0x20) offset of jump table from A5 */
};


struct jumptab {		/* jump table entry */
	short	jt_offset;	/* (0x0) offset from segment beginning */
	short	jt_move;	/* (0x3f3c) op code to move seg id to stack */
	short	jt_id;		/* (0x1) segment id of this entry */
	short	jt_trap;	/* (0xA9F0) trap that executes LoadSeg */
};


struct codehead {		/* header of code segments */
	short	ch_offset;	/* (0x0) offset of our jump table entries */
	short	ch_entries;	/* (0x1) number of our entry points */
};
