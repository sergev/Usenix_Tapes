/************************************************************************
*
* B-tree header file
*
* This just defines various error codes etc.
*
***********************************************************************/

static char btree_header[] = "@(#)btree.h	1.3 7/16/86";

#define	NODE_SPLIT		-1	/* Information code - used internally
					   within insertion code */
#define SUCCESS		 	 0	/* Success code */
#define KEY_EXISTS_ERROR 	 1	/* Returned by insertion routines */
#define KEY_NOT_FOUND_ERROR	 2	/* Returned by deletion & search */
#define TREE_CORRUPTED_ERROR	 3	/* Returned by deletion */
