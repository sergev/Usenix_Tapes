**************************************************************************

Module Name:		btree.test.h
============

Function:		Defines various error codes used by btree.test.c
=========

Description:
============
		Just defines a set of error codes
***************************************************************************
**************************************************************************/

static char btree_test_prog_hdr[]	= "@(#)btree.test.h	1.1 7/15/86";

#define	FOUND_NON_EXISTANT_KEY		100
#define	NOT_FOUND_EXISTING_KEY		101
#define	FOUND_WRONG_KEY			102
#define	INSERTED_EXISTING_KEY		103
#define	CANNOT_INSERT_KEY		104
#define	DELETED_NON_EXISTANT_KEY	105
#define	CANNOT_DELETE_KEY		106
#define	NODE_NOT_HALF_FULL		107
#define	WRONG_KEY_IN_NODE		108
#define	TREE_DEPTH_INCONSISTENT		109
#define KEYS_NOT_IN_ORDER		110
#define	KEY_TOTAL_MISMATCH		111
