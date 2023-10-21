/********************************************************************
*********************************************************************
 
Module Name:		btree.c
============
 
Function:		Provide a library of routines for creating 
=========		and manipulating B-Trees.  
			The "order" of the B-Tree is controlled by a 
			manifest constant.

Description:
============
	Main btree code Insert, Delete, Search etc.

	Interface Routines:
	-------------------
	int Insert(*tree, dtm)	
		Insert DATUM dtm into B-tree "tree",
		returns an error/success code.
	int Delete(*tree, key)	
		Remove the entry associated with "key" from the B-Tree
		returns an error/success code.
	int Search(tree, key, dtmptr)	
		Look for key "key" in B-tree "tree".
		Set "dtmptr" to point to matching DATUM or to NULL.
		Return an error/success code.

	Libraries Used:
	---------------
	stdio.h
	btree.h 

****************************************************************************
****************************************************************************/

static char btreeprog[] = "@(#)btree.c	1.7 7/17/86";

#include <stdio.h>
#include "btree.h"

#define TRUE	1	/* A couple of useful constants */
#define FALSE	0

#define	M	2	/* This constant defines the order of the B-tree */



/************** Global types and data structures **************/ 

typedef int	KEY;	/* Define the key field type */
typedef int	INFO;	/* and define the type of the information field
			   associated with this key */

typedef struct 		/* Define a structure (DATUM) consisting of a key */
{			/* and an information field */
	KEY	key;
	INFO	inf;
} DATUM;


			/* The following is the structure used for defining 
			   B-trees and their nodes */
typedef struct btnode 
{
	int	t_active;		/* # of active keys */
	DATUM	t_dat[2*M];		/* Keys  + Data */
	struct btnode *t_ptr[2*M+1];	/* Subtree ptrs */
} NODE, *BTREE;


/**********************************************************************
* INSERTION ROUTINES
**********************************************************************/

static BTREE splittree;	/* 
			A global variable used by "InternalInsert"
			to return the right hand portion of a tree
			which has been bisected .
			*/

/*
** INSERT 
** ======
**
** Purpose:	Enter the given key into the B-tree.
**
** Parameters:	dtm	= DATUM to be inserted into tree
**		tree	= B-tree to insert into
**
** Returns:	An error/success code which will be one of:
**			SUCCESS
**			KEY_EXISTS_ERROR
**		In addition the tree "tree" is updated.
**
** Description: This is the "front end" of the insertion routine
**		- most of the work is done by "InternalInsert".
**
*/

int Insert(treeptr, dtm)
BTREE	*treeptr;
DATUM	dtm;
{
	int status;
	int InternalInsert();
	BTREE MkNode();

	/* 
	Invoke the main insertion routine, let it do the guts of the
	insertions, then let it return a final status code.
	*/
	status = InternalInsert(*treeptr, &dtm);

	/* 
	Now examine the returned status code - there are 3 cases:
	(1) NODE_SPLIT
	The root node was split - then stitch the tree together by
	creating a new root node containing dtm only and link 'tree'
	to the left-hand link and 'splittree' to the right-hand link
	(2) SUCCESS
	The tree has already been reassembled into a consistent state
	so just return a success code
	(3) KEY_EXISTS_ERROR
	The given key already exists - the tree should already be unchanged
	- just exit with this error code
	*/
	if (status==NODE_SPLIT)
	{
		*treeptr = MkNode(dtm, *treeptr, splittree);
		return SUCCESS;
	}
	else
		return status;
}


/*
** INTERNALINSERT
** ==============
**
** Purpose:	This routine performs the major part of key insertion
**
** Parameters: 	tree	= B-tree to use.
**		dtmptr	= Pointer to the datum to be inserted into the tree
**			  on exit this should contain any datum to be passed
**			  back up the tree as a result of splitting.
**
** Returns: 	An error/success/information code which will be one of:
**		SUCCESS: Tree is okay, no split occurred dtmptr will not
**			 point at anything meaningful and splittree will be
**			 NULL.
**		NODE_SPLIT:
**			 Top level node has been split - dtmptr and splittree
**			 are now meaningful.
**		KEY_EXISTS_ERROR:
**			 The key to be inserted already exists - the tree is
**			 unchanged.
**
** Description: This routine works in a recursive manner under the following 
**		algorithm:
**		1) Scan down the tree for a place to insert the new datum
**		   - this should be a leaf unless the datum itself is found
**		     (in which case return an error code)
**		2) Try to insert the new datum in the located node
**		   - if it will fit, then slot it into the node and exit with
**		     a success code.
**		   - if the node is already full, split it into 2 separate 
**		     nodes (one for the lowest M keys, one for the highest M
**		     keys) and return the NODE_SPLIT code and everything
**		     associated with it
**
*/

int InternalInsert(tree, dtmptr)
DATUM	*dtmptr;
BTREE	tree;
{
	int	index,
		j;
	int	SearchNode();
	int 	status;
	BTREE	tmp;
	BTREE	MkNode();

	if (tree == (BTREE) NULL) 
	{
	/* 
	If current tree is NULL then we're at a leaf, so we can't insert the
	datum here - the procedure is to act as if we've just split a node
	and are returning a datum to be slotted in further up the tree - the 
	only difference is that splittree is set to NULL.
	*/
		splittree = (BTREE) NULL;
		return NODE_SPLIT;
	} 
	else 
	{
		/* 
		Tree is not null - now search for occurence of datum's key 
		in the current top-level node - return its position (or index 
		of pointer to subtree) as a by-product.
		*/
		if (SearchNode(tree, (*dtmptr).key, &index))
			/*
			If key has been found then return an error code 
			*/
			return KEY_EXISTS_ERROR;
		/*
		If not found the try to insert datum in next level of tree -
		this action will then return a status code and possibly a
		splittree.
		*/
		status = InternalInsert(tree->t_ptr[index],dtmptr);
		
		/*
		If we have had a successful insertion (with tree in good order)
		or we have had a key insertion error, then return with this 
		status code without further ado.
		*/
		if (status != NODE_SPLIT)
			return status;
		/*
		We've now had a split node at the level below with dtmptr now
		pointing to a datum throw back up from it and splittree 
		pointing to the right-hand part of the split node
		Next stage is to try to insert things here and tidy up the tree
		once and for all - so check to see whether current top level
		node is full or not (ie. can it support an insertion ?).
		*/
		if (tree->t_active < 2*M)
		{
			/* 
			If there is room then everything's hunky dory
			*/
			InsInNode(tree, *dtmptr, splittree);
			return SUCCESS;
		}

		/*
		Current top-level node is full - so split it into two 
		- set left-hand half to contain lower M keys and leave it
		  attached to main tree.
		- set splittree to point to right-hand half (which contains
		  the highest M keys).
		- set dtmptr to point to central key value
		*/
		if (index <= M) 
		{
			/* 
			If datum should go in a left-hand or central position
			of current top-level node then shift right-element out
			Note that we need a temporary B-tree here just to do
			the swap of datums
			*/
			tmp = MkNode(tree->t_dat[2*M-1], (BTREE)NULL, tree->t_ptr[2*M]);
			tree->t_active--;
			InsInNode(tree, *dtmptr, splittree);
		} 
		else
			/* 
			If datum is definitely part of right-hand half of
			the 2M+1 keys then transfer it to (what will be) 
			splittree immediately.
			*/
			tmp = MkNode(*dtmptr, (BTREE)NULL, splittree);

		/*
		Final part is to move right hand half of current top level node
		out into (whta is about to become) the new splittree
		and then mode central (Mth) key of current top-level node into
		*dtmptr
		*/
		for (j = M+1; j < 2*M; j++)
			InsInNode(tmp, tree->t_dat[j], tree->t_ptr[j+1]);

		*dtmptr		= tree->t_dat[M];
		tree->t_active 	= M;
		tmp->t_ptr[0] 	= tree->t_ptr[M+1];
		splittree	= tmp;

		/* 
		Finally return NODE_SPLIT code
		*/
		return NODE_SPLIT;
	}
}


/*
** INSINNODE
** =========
** 
** Purpose:	Add a datum into a B-tree node working on the assumption 
**		that there is room for it.
**
** Parameters: 	nodeptr	= Ptr to the node,
**		dtm	= Datum to enter,
**		ptr	= "Greater than" link to subordinate node.
**
** Returns: 	None.
**
** Description:	The workings of this routine are pretty straightforward - just
**		sort out where the key should go, shift all greater keys
**		right one position and then insert the key and pointer.
*/

InsInNode(nodeptr, dtm, ptr)
BTREE	nodeptr,
	ptr;
DATUM	dtm;
{
	int	index;
	int	KeyCmp();

	for (index = nodeptr->t_active; index>0 && KeyCmp(dtm.key, nodeptr->t_dat[index-1].key)<0; index--) 
	{
		nodeptr->t_dat[index]   = nodeptr->t_dat[index-1];
		nodeptr->t_ptr[index+1] = nodeptr->t_ptr[index];
	}

	nodeptr->t_active++;
	nodeptr->t_dat[index]   = dtm;
	nodeptr->t_ptr[index+1] = ptr;
}


/*************************************************************************
* DELETION ROUTINES
*************************************************************************/

/*
** DELETE
** ======
**
** Purpose:	Remove the data associated with a given key from a B-tree.
**
** Parameters:	tree	= B-tree to update
**		key  	= Key to use,
**
** Returns:	An error/success code
**		SUCCESS / KEY_NOT_FOUND_ERROR / TREE_CORRUPTED_ERROR
**
** Description: The real work to do with deletion is performed by RealDelete()
**		this routine only checks that RealDelete has actually done
**		a deletion and tidies things up afterwards.
**
*/

int Delete(treeptr, key)
BTREE	*treeptr;
KEY	key;
{
	int	status;
	int	RealDelete();
	BTREE	savetree;

	/*
	Do the main deletion then check whether it managed to find anything
	to delete. If it didn't - return an error code.
	*/
	status = RealDelete(*treeptr, key);
	if (status != SUCCESS)
		return status;

	/* 
	Now check to see whether the previous root node has now
	been superceded.
	*/
	else if ((*treeptr)->t_active == 0) 
	{
		/* 
		If so then deallocate the space that was set aside for the
		root node and make its leftmost child the new root node.
		*/
		savetree = (*treeptr)->t_ptr[0];
		Dispose(*treeptr);
		*treeptr = savetree;
	} 
	return SUCCESS;
}


/*
** REALDELETE 
** ==========
**
** Purpose:	Remove a key from a tree
**
** Parameters:	tree	= B-tree to be updated
**		key	= Key to use,
**
** Returns:	Returns an error/success code (SUCCESS/KEY_NOT_FOUND_ERROR)
**
** Description:	This routine operates recursively in the following manner:
**		1) Locate the required key within the tree.
**		2) If it's in a leaf then remove it and rebalance the tree
**		   otherwise replace it by the next key in the tree (in 
**		   ascending order), then delete that key from its node 
**		   (which must be a leaf).
*/

int RealDelete(tree, key)
BTREE	tree;
KEY	key;
{
	int index;
	int status;
	int SearchNode();
	KEY nextkey;

	if (tree == (BTREE)NULL)
		/* 
		If now trying to scan down a non-existant link,
		mark required key as not found and exit 
		*/
		return KEY_NOT_FOUND_ERROR;

	/* 
	First stage is to search for key within current top-level node
	If it's not there then we must continue down the tree looking
	for it.
	*/
	if (! SearchNode(tree, key, &index))
	{
		/* 
		If required key not found in current top-level node
		then try deleting it from the next node down 
		*/
		status = RealDelete(tree->t_ptr[index], key);
		if (status==SUCCESS)
			Balance(tree,index);
		return status;
	}
	/* 
	If the key was found, then check whether this is
	a leaf 
	*/
	if (tree->t_ptr[index] == (BTREE)NULL)
	{
		/* 
		If this is a leaf, then just remove the required key for now 
		- the tree will be rebalanced on the way back up 
		*/
		Remove(tree,index);
		return SUCCESS;
	}
	else 
	{
		/* 
		If this isn't a leaf, the rule is to replace the key by 
		the next highest in the tree, deleting that key from its 
		node and rebalancing the tree 
		*/
		nextkey = Succ(tree, index);
		status  = RealDelete(tree->t_ptr[index+1],nextkey);
		if (status==SUCCESS)
		{
			Balance(tree,index+1);
			return status;
		}
		else
			return TREE_CORRUPTED_ERROR;
	}
}


/*
** REMOVE
** ======
**
** Purpose:	Remove a single datum
**
** Parameters: 	tree	= Ptr to the B-tree node,
**		pos	= Index of the key to be removed.
**
** Returns: 	None.
**
** Description:	Remove a datum from a B-tree node and fill the gap left
**		by the deletion by shuffling across the other DATUMs and 
**		pointers.
**
*/

Remove(tree, pos)
BTREE	tree;
int	pos;
{
	int	i;

	/* 
	This is all pretty trivial - just shuffle everything to the
	right of the deleted key, left by one.
	*/
	for (i=pos; i<((tree->t_active)-1); ++i) 
	{
		tree->t_dat[i]   = tree->t_dat[i+1];
		tree->t_ptr[i+1] = tree->t_ptr[i+2];
	}

	/* 
	Finally decrement the number of valid keys in the node 
	*/
	tree->t_active--;
}


/*
** SUCC 
** ====
**
** Purpose:	Replace a key by its successor
**
** Parameters: 	tree	= Ptr to a B-tree node,
**		pos	= Index of the key to be succ'ed.
**
** Returns: 	Returns the successor key
**
** Description: Using the natural ordering, replace a key with its successor 
**		(ie the next key in the tree in ascending order).
**		This routine relies on the fact	that the successor of a key 
**		will be the leftmost key in the leftmost leaf of the 
**		"greater than" subtree of the key to be deleted.
**
*/

KEY Succ(tree, pos)
BTREE	tree;
int	pos;
{
	BTREE	ltree;
	DATUM 	successor;

	/*
	Using the "greater than" branch the key chain down to leftmost node 
	below it.
	*/
	for (ltree = tree->t_ptr[pos+1]; ltree->t_ptr[0] != (BTREE)NULL; ltree = ltree->t_ptr[0])
		;		/* NULL BODY */

	successor = ltree->t_dat[0]; 
	tree->t_dat[pos] = successor;

	return successor.key;
}


/*
** BALANCE 
** =======
**
** Purpose:	Restore balance to a potentially unbalanced B-tree
**
** Parameters: 	parent	= Ptr to the parent node of a B-tree structure,
**		pos	= Index of the out-of-balance point within the
**			  parent node.
**
** Returns: 	None.
**
** Description: After removing an item from a B-tree we must restore 
**		the balance to the tree.
** 		In this routine we consider the DATUM in t->t_dat[pos-1]
**		and the DATUMs in its immediate children (and there should 
**		be children for this routine to be called).
**		The rules are:
**		(1) 	If total number of DATUMs <= 2M then we can combine 
**			them into a single node 
**		(2)	Otherwise if the difference in numbers between the 2
**			children is greater than 1, then we must shuffle 
**			DATUM(s) out of the fullest node, through the parent 
**			and into the emptiest node 
**
*/

Balance(parent, pos)
BTREE	parent;
int	pos;
{
	int 	lchildindex, rchildindex;
	int 	lchildlen, rchildlen;

	/*  
	Firstly decide where the children are - this pretty obvious
	- t_ptr[pos] and t_ptr[pos+1] - unless pos=tree->t_active
	when we have t_ptr[t_active-1] & t_ptr[t_active]
	*/
	if (pos==parent->t_active)
		rchildindex = parent->t_active;
	else
		rchildindex = pos+1;
	lchildindex = rchildindex - 1;

	/* 
	Now find out how many valid DATUMs reside in each of the children 
	*/
	lchildlen = (parent->t_ptr[lchildindex])->t_active;
	rchildlen = (parent->t_ptr[rchildindex])->t_active;

	/* 
	Check total number of DATUMs involved 
	*/
	if ((lchildlen + rchildlen + 1) <= 2*M)
		/* 
		If <=2M then combine into 1 node 
		*/
		Combine(parent,rchildindex);

	/* 
	Otherwise if difference in sizes of children is >1 then shift 
	DATUMs one way or the other 
	*/
	else if ((lchildlen-rchildlen)>1)
		MoveRight(parent,lchildindex);
	else if ((rchildlen-lchildlen)>1)
		MoveLeft(parent,rchildindex);
}


/*
** COMBINE 
** =======
**
** Purpose: 	Coalesce two nodes of a B-tree when they have too many DATUMs.
**
** Parameters: 	parent	= Ptr to parent B-tree node,
**		rindex	= Index of the right-hand of the 2 nodes to be combined.
**
** Returns: 	None.
**
** Description: This all pretty simple - just move parent DATUM, followed by
**		DATUMs from right-hand child into left-hand child. Then throw
**		away right-hand child, delete parent DATUM and close the gap in
**		the parent node.
*/

Combine(parent, rindex)
BTREE	parent;
int	rindex;
{
	int	i;
	BTREE	ltree,
		rtree;

	/* 
	Set up pointers to the 2 child trees 
	*/
	ltree = parent->t_ptr[rindex-1];
	rtree = parent->t_ptr[rindex];

	/* 
	We are going to combine the 2 trees into the left-hand tree
	so first thing is to tag parent datum onto end of this left-hand
	child node 
	*/
	ltree->t_active++;
	ltree->t_dat[(ltree->t_active)-1] = parent->t_dat[rindex-1];

	/* 
	Now set right-hand link from this left-hand child to be the left-hand 
	link right-hand child 
	*/
	ltree->t_ptr[ltree->t_active] = rtree->t_ptr[0];

	/* 
	Now tag all of right-hand child onto end of left-hand child 
	*/
	for (i = 1; i <= rtree->t_active; ++i) 
	{
		ltree->t_active++;
		ltree->t_dat[ltree->t_active-1] = rtree->t_dat[i-1];
		ltree->t_ptr[ltree->t_active]   = rtree->t_ptr[i];
	}

	/* 
	Finally, remove parent DATUM from parent node by shifting
	contents of parent node left as necessary 
	*/
	for (i = rindex; i <= parent->t_active-1; ++i)
	{
		parent->t_dat[i-1] = parent->t_dat[i];
		parent->t_ptr[i]   = parent->t_ptr[i+1];
	}
	parent->t_active--;

	Dispose(rtree);
}


/*
** MOVERIGHT
** =========
**
** Purpose: 	Move DATUMs right out of one child into its right-hand 
**		brother.
**
** Parameters: 	parent	= Ptr to the parent B-tree node,
**		lindex	= Index of the link to the left-hand child 
**			  from the parent node.
**
** Returns: 	None.
**
** Description: Main thing to note is that we only need to shift 1 DATUM
**		because the current imbalance was caused by a deletion from 
**		the right-hand child and, prior to that, things were okay (the 
**		tree was balanced last time around).
*/

MoveRight(parent, lindex)
BTREE	parent;
int	lindex;
{
	int	i;
	BTREE	ltree,
		rtree;

	/*
	Set up pointers to 2 trees concerned
	*/
	ltree = parent->t_ptr[lindex];
	rtree = parent->t_ptr[lindex+1];

	/* 
	First stage is to shift contents of right-hand child right by one
	in order to accommodate the incoming DATUM from the parent node 
	*/
	rtree->t_active++;
	for (i = rtree->t_active; i >= 1; i--) 
	{
		rtree->t_dat[i]   = rtree->t_dat[i-1];
		rtree->t_ptr[i+1] = rtree->t_ptr[i];
	}
	rtree->t_ptr[1] = rtree->t_ptr[0];

	/* 
	Now copy parent DATUM into r-hand child 
	*/
	rtree->t_dat[0] = parent->t_dat[lindex];

	/* 
	Finally move rhand DATUM of lhand child into parent node 
	*/
	rtree->t_ptr[0]  	= ltree->t_ptr[ltree->t_active];
	parent->t_dat[lindex] 	= ltree->t_dat[(ltree->t_active)-1];
	ltree->t_active--;
}


/*
** MOVELEFT
** ========
**
** Purpose: 	Move DATUM left from a right-hand child, through its parent
**		DATUM and into a left-hand child.
**
** Parameters: 	parent	= Ptr to the parent B-tree node,
**		rindex	= Index of the right-hand child within the
**			  parent node. 
**
** Returns: 	None.
**
** Description: As with MoveRight (above) the main thing to note is that we
**		only have to shift 1 DATUM.
*/

MoveLeft(parent, rindex)
BTREE	parent;
int	rindex;
{
	int	i;
	BTREE	ltree,
		rtree;

	/*
	Set up pointers to 2 children
	*/
	ltree = parent->t_ptr[rindex-1];
	rtree = parent->t_ptr[rindex];

	/* 
	First stage is to shift DATUM from parent node onto end of
	lhand child 
	*/
	ltree->t_active++;
	ltree->t_dat[(ltree->t_active)-1] = parent->t_dat[rindex-1];

	/* 
	Now move lhand link from rhand child to be rhand link from
	lhand child 
	*/
	ltree->t_ptr[ltree->t_active] = rtree->t_ptr[0];

	/* 
	Next, shift lhand DATUM of rhand node into parent node 
	*/
	parent->t_dat[rindex-1] = rtree->t_dat[0];

	/* 
	Finally shift contents of rhand child left by one 
	*/

	rtree->t_ptr[0] = rtree->t_ptr[1];
	for (i = 1; i <= rtree->t_active; ++i) 
	{
		rtree->t_dat[i-1] = rtree->t_dat[i];
		rtree->t_ptr[i]   = rtree->t_ptr[i+1];
	}
	rtree->t_active--;
}


/*****************************************************************************
* SEARCH ROUTINE
*****************************************************************************/

/*
** SEARCH
** ======
**
** Purpose:	Look for a key in a tree.
**
** Parameters:  tree	= Tree to look in
**		key	= key to look for
**		dtmptr  = pointer to found datum
**
** Returns: 	A success/error code (SUCCESS/KEY_NOT_FOUND_ERROR)
**
** Description:
**
*/

int Search(tree, key, dtmptr)
BTREE	tree;
KEY	key;
DATUM 	*dtmptr;
{
	DATUM	dtm;
	int	index;
	int 	SearchNode();

	while (tree != (BTREE)NULL) 
	{
		/* 
		For each node just check to see whether the requd key value
		is there - if not, go down 1 more level ...
		*/
		if (SearchNode(tree, key, &index))
		{
			dtm = (tree->t_dat[index]);
			*dtmptr = dtm;
			return SUCCESS;
		}
		tree = tree->t_ptr[index];
	}
	dtmptr = (DATUM *)NULL;
	return KEY_NOT_FOUND_ERROR;
}


/**************************************************************************
* MISCELLANEOUS ROUTINES
**************************************************************************/

/*
** SEARCHNODE 
** ==========
**
** Purpose: 	Look for a key in a single B-tree node.
**
** Parameters: 	nodeptr	= Ptr to B-tree node,
**		key	= Key to look for,
**		pindex  = Pointer to integer vbl in which to place
**			  index position of key within node
**
** Returns: 	TRUE/FALSE result indicating whether key was in the node
**
** Description:	The workings of this routine are pretty trivial -see code.
*/

int SearchNode(nodeptr, key, pindex)
BTREE	nodeptr;
KEY	key;
int	*pindex;
{
	int	i;
	int	cmp;
	int	KeyCmp();

	/* 
	Scan down the node from highest to lowest key value - stop when 
	a key <= 'key' is found
	*/
	for (i=(nodeptr->t_active)-1; i>=0; i--)
	{
		cmp = KeyCmp(key,(nodeptr->t_dat[i]).key);
		if (cmp>=0)
		{
			*pindex = (cmp==0)? i: i+1;
			return (cmp==0);
		}
	}

	/* 
	If passed through loop unscathed then 'key' must be less than 
	everything in the node - so act accordingly
	*/
	*pindex = 0;
	return FALSE;
}


/*
** KEYCMP 
** ======
**
** Purpose:	To compare two key values
**
** Parameters:	key1	= First key,
**		key2	= Second key.
**
** Returns: 	-1 	if key1 < key2;
**		 0	if key1 = key2;
**		 1	if key1 > key2;
**
** Description:	The contents of this routine are dependent upon the type of
**		key being used - as long as it accepts the same parameters
**		and returns the same results, any key type can be used.
*/

int KeyCmp(key1, key2)
KEY	key1,
	key2;
{
	return key1<key2 ? -1 : key1==key2 ? 0 : 1;
}


/*
** MKNODE
** ======
**
** Purpose:	Allocate storage for a new B-tree node and copy in an
**		initial datum and two pointers from it.
**
** Parameters:	dtm	= Initial datum for new node
**		ptr0	= "less than" pointer from dtm
**		ptr1	= "greater than" pointer from dtm
**
** Returns: 	Reference to the new node.
**
** Description:	All pretty obvious...
*/

BTREE MkNode(dtm, ptr0, ptr1)
DATUM	dtm;
BTREE	ptr0,
	ptr1;
{
	char	*malloc();
	BTREE	tree;

	tree = (BTREE) malloc(sizeof(NODE));

	tree->t_ptr[0] = ptr0;
	tree->t_ptr[1] = ptr1;
	tree->t_dat[0] = dtm;
	tree->t_active = 1;

	return tree;
}


/*
** DISPOSE 
** =======
**
** Purpose:	Return the storage occupied by a tree node to the pool.
**
** Parameters: 	nodeptr	= Ptr to the tree node.
**
** Returns: 	None.
**
** Description: Another simple one ...
*/

Dispose(nodeptr)
BTREE	nodeptr;
{
	free((char *) nodeptr);
}

