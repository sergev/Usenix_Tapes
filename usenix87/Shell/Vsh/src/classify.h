/* Vsh will classify a file as one of the following:  */

#define	CL_NULL		0	/* No such file or protected path */
#define	CL_DIR		1	/* Directory, May NOT be accessable */
#define CL_SPCL		2	/* Special device file */
#define CL_PROTPLN	3	/* Plain file protected unknown fmt */
#define CL_UNKPLN	4	/* Plain file unknown format */
#define CL_AOUT		5	/* A.out format */
#define CL_AR		6	/* Ar format */
#define CL_CPIO		7	/* Cpio format */
#define CL_CORE		8	/* Core dump */
#define CL_TEXT		9	/* Ascii text */
#define	CL_PACK		10	/* Pack/pcat Huffman encoded */
#define	CL_COMPACT	11	/* compact Huffman encoded */
#define	CL_COMPRESS	12	/* compress encoded */
