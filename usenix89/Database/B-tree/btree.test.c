/***************************************************************************
*
* Module Name:		btree.test.c
* ============
*
* Function:		BTREE TEST PROGRAM
* =========
*
* Description:
* ============
*	-see main routine
*	Libraries
*	----------
*	stdio.h
*	btree.c (--> btree.h)
*	btree.test.h
*	btree.prt.h
*
******************************************************************************/

static char btree_tests[] = "@(#)btree.test.c	1.1 7/16/86";


/*****************************************************************************
* INCLUDES and TEST PROGRAM PARAMETERS
*****************************************************************************/

#include <stdio.h>
#include "btree.c"
#include "btree.test.h"
#include "btree.prt.h"

	/*
	Number of key values to be used
	*/
#define MAXKEYS	1500	

	/*
	Values used by flag table (below)
	*/
#define IN_TREE		1
#define NOT_IN_TREE 	0

	/* 
	Seed for random number generator
	*/
static int	seed;
	/* 
	This array will contain boolean values -
	if flag[i]=NOT_IN_TREE then key i is currently not in the tree
	if flag[i]=IN_TREE     then key i is currently in the tree.
	*/
static int	flag[MAXKEYS];	

static int	keytotal;

BTREE 	tree;	/* Tree to be used */

/******************************************************************************
* MAIN TEST PROGRAM
******************************************************************************/

/*
** MAIN TEST PROGRAM
** =================
**
** Purpose:	Test the B-tree routines
**
** Parameters:	argv[1]		= "" or a seed for the random tests
**
** Returns:	A continuous set of diagnostic messages is sent to stdout
**		This continues until the tests are successfully completed or
**		until the first error occurs
** 
** Description:	There are 3 sets of tests performed
**		(1) Random tests - a large number of random inserts, deletes
**			and searches is performed.
**		(2) All keys are inserted into the tree in ascending order
**			then deleted in ascending order.
**		(3) All keys are inserted into the tree in descending order
**			then deleted in descending order.
**		For the purposes of this test a limited range of keys is used
**		- these are numbered 0 to MAXKEYS-1 (where MAXKEYS is #defined
**		above) and a function getkey() returns the actual key value of
**		a given key number - currently keys are integers so 
**		getkey(i) = i.
*/

main(argc,argv)
int	argc;
char	*argv[];
{
	int statuscode;
	int randomtest(), ascendingtest(), descendingtest();

	/* 
	First thing to do is to check for a seed parameter
	*/
	if (argc != 0)
		sscanf(argv[1],"%d",&seed);
	else
		seed = 0;
	srandom(seed);

	/*
	Random test
	*/
	statuscode = randomtest();
	if (statuscode != SUCCESS)
		error(statuscode);
	
	/*
	Ascending test
	*/
	statuscode = ascendingtest();
	if (statuscode != SUCCESS)
		error(statuscode);

	/*
	Descending test
	*/
	statuscode = descendingtest();
	if (statuscode != SUCCESS)
		error(statuscode);
	
	/*
	Given message to say that it all worked
	*/
	printf("\n\n----> TESTS SUCCEEDED <----\n\n\n");
	exit(1);
}


/****************************************************************************
* RANDOM INSERT/DELETE/SEARCH TEST
****************************************************************************/

	/*
	Number of inserts + deletes + searches for random tests.
	*/
#define PASSES		2000

	/* 
	Number of actions available to random test and the codes associated 
	with them.
	*/
#define ACTIONS		3
#define SEARCH		0
#define INSERT		1
#define DELETE		2



/*
** RANDOMTEST
** ==========
** Purpose:	Perform the random tests
**
** Parameters:	None
**
** Returns:	Diagnostic messages are continuously fed to stdout
**		and upon completion/error a success/error code is returned
**		SUCCESS:	Everything went okay
**		others :	see routines 	testsearch(),
**						testinsert(),
**						testdelete().
**
** Description: For a given number of iterations (PASSES), generate a random
**		key number and randomly select an insert, a delete or a search
**		to do on the key corresponding to the key number. 
*/

int randomtest()
{
	int 	keynumber;
	int 	pass,
		status;
	int	randrange();

	/*
	Set up tree for new test
	*/
	inittree();

	/*
	Send start-of-test-banner
	*/
	printf("**************** RANDOM TESTS *****************\n\n");

	/*
	Now repeat the following process PASSES times:
	1) Generate a random key value
	2) Generate a random number to determine what action to perform
	   on that key
	3) Perform the action and check the resulting tree.
	*/

	for (pass=1; pass<PASSES; pass++)
	{
		keynumber = randrange(MAXKEYS);
		switch (randrange(ACTIONS))
		{
		case SEARCH:
			status = testsearch(keynumber);
			break;
		case INSERT:
			status = testinsert(keynumber);
			break;
		case DELETE:
			status = testdelete(keynumber);
			break;
		}
		if (status != SUCCESS)
			return status;
	}
	return SUCCESS;
}


/*
** RANDRANGE
** =========
**
** Purpose:	Generate a random integer within a given range
**
** Parameters:	number_of_values	= range of random int to be generated
**
** Returns:	a random in in the range 0..(number_of_values-1)
**
** Description: Very simple - most of work done by a system routine.
*/

int randrange(number_of_values)
int number_of_values;
/* 
Routine to generate a random number in a given range
*/
{
	long	random();

	return	(random() % number_of_values);
}


/*****************************************************************************
* ASCENDING KEY TEST
*****************************************************************************/

/*
** ASCENDINGTEST
** =============
**
** Purpose:	Performs ascending key value test
**
** Parameters:	None
**
** Returns:	Sends continuous diagnostic stream to stdout
**		Returns a success/error code at end of tests or if an error
**		occurs.
**
** Description: Clear the tree, fill it with keys by adding them in ascending 
**		order, then delete them all in ascending key order.
*/

int ascendingtest()
{
	int	i;
	int	status;

	/*
	Set up tree for new test
	*/
	inittree();

	/*
	Print start-of-test banner
	*/
	printf("*************** ASCENDING TESTS ******************\n\n");

	/*
	Insert keys in order
	*/
	for (i=0; i<MAXKEYS; i++)
	{
		status = testinsert(i);
		if (status != SUCCESS)
			return(status);
	}

	/*
	Delete keys in order
	*/
	for (i=0; i<MAXKEYS; i++)
	{
		status = testdelete(i);
		if (status != SUCCESS)
			return(status);
	}
	return SUCCESS;
}


/*****************************************************************************
* DESCENDING KEY TEST
*****************************************************************************/

/*
** DESCENDINGTEST
** ==============
**
** Purpose:	Performs descending key value test
**
** Parameters:	None
**
** Returns:	Sends continuous diagnostic stream to stdout
**		Returns a success/error code at end of tests or if an error
**		occurs.
**
** Description: Clear the tree, fill it with keys by adding them in descending 
**		order, then delete them all in descending key order.
*/

descendingtest()
{
	int	i;
	int	status;

	/*
	Set up tree for new test
	*/
	inittree();
	/*
	Print start-of-test banner
	*/
	printf("*************** DESCENDING TESTS ******************\n\n");

	/*
	Insert keys in order
	*/
	for (i=MAXKEYS-1; i>=0; --i)
	{
		status = testinsert(i);
		if (status != SUCCESS)
			return(status);
	}

	/*
	Delete keys in order
	*/
	for (i=MAXKEYS-1; i>=0; --i)
	{
		status = testdelete(i);
		if (status != SUCCESS)
			return(status);
	}
	return SUCCESS;
}


/*****************************************************************************
* TREE INITIALISATION
*****************************************************************************/

/*
** INITTREE
** ========
**
** Purpose:	Set up tree & lookup tables ready for a new test
**
** Parameters:	none.
**
** Returns:	none.
**
** Description:	trivial.
*/

inittree()
{
	int i;

	tree = (BTREE)NULL;
	for (i=0; i<MAXKEYS; i++)
		flag[i] = NOT_IN_TREE;
	keytotal = 0;
}


/***********************************************************************
* TEST ROUTINES FOR SEARCH/INSERT/DELETE
***********************************************************************/

/*
** TESTSEARCH
** ==========
** 
** Purpose:	Test a search for a given key
**
** Parameters:	keynumber	= number of key to be located
**
** Returns:	A diagnostic message is sent to stdout and upon completion
**		an error/success code is returned:
**			SUCCESS	- everything worked
**			FOUND_NON_EXISTANT_KEY
**				- error - located a key when it wasn't in
**				  the tree
**			NOT_FOUND_EXISTING_KEY
**				- error - could locate a key which should be
**				  in the tree.
**
** Description:	Basically, do the search and compare the result with the
**		flag lookup table - also, if found successfully, check that
**		the routine has returned a pointer to the correct keyed DATUM.
*/

int testsearch(keynumber)
int keynumber;
/*
Routine to test the search for a key within a tree
*/
{
	KEY 	getkey();
	KEY 	key;
	DATUM 	dtm;
	int	i;
	int 	status;

	/*
	Translate a key number into a key value
	*/
	key = getkey(keynumber);

	/*
	Send message to log file
	*/
	printf("Search\t");
	printkey(key);

	
	/*
	Now attempt the search itself
	*/
	status = Search(tree,key,&dtm);
	
	/*
	Print out result of search
	*/
	if (status==SUCCESS)
		printf("found       \t");
	else
		printf("not found   \t");

	/*
	We can get 2 sorts of error:
	(1) A non-existant key was found
	(2) An existing key could not be found
	(3) 'dtm' doesn't point to the right key
	*/
	if ((status==SUCCESS) && (flag[i]==NOT_IN_TREE))
		return FOUND_NON_EXISTANT_KEY;
	if ((status==KEY_NOT_FOUND_ERROR) && (flag[i]==IN_TREE))
		return NOT_FOUND_EXISTING_KEY;
	if ((status==SUCCESS) && (dtm.key != key))
		return FOUND_WRONG_KEY;

	/*
	Send final diagnostic message then exit successfully
	*/
	printf("CORRECT\n");
	return SUCCESS;
}


/*
** TESTINSERT
** ==========
** 
** Purpose:	Test the insertion of a given key
**
** Parameters:	keynumber	= number of key to be inserted
**
** Returns:	A diagnostic message is sent to stdout and upon completion
**		an error/success code is returned:
**			SUCCESS	- everything worked
**			INSERTED_EXISTING_KEY
**				- error - successfully inserted a key which was
**				  already in the tree
**			CANNOT_INSERT_KEY
**				- error - could insert the key although it 
**				  shouldn't be in the tree.
**			also can get various errors from check_consistency()
**
** Description:	Basically, do the insert, compare the result with the
**		flag lookup table - if everything seems to be okay then
**		update the lookup table and check the consistency of the
**		new B-tree
*/

int testinsert(keynumber)
int keynumber;
{
	DATUM	dtm;
	int 	status;

	/*
	Translate key index number into a key value
	*/
	dtm.key = getkey(keynumber);
	
	/*
	Send message to log file
	*/
	printf("Insert\t");
	printkey(dtm.key);

	/*
	Now do the insert itself - can get two main errors:
	(1) Successfully inserted a key which was already there
	(2) Couldn't insert a key even though it wasn't there
	*/
	status = Insert(&tree,dtm);

	/*
	Now print out result of insert 
	*/
	if (status==SUCCESS)
		printf("inserted    \t");
	else
		printf("not inserted\t");

	/*
	Check for error
	*/
	if ((status==SUCCESS) && (flag[keynumber]==IN_TREE))
		return INSERTED_EXISTING_KEY;
	if ((status==KEY_EXISTS_ERROR) && (flag[keynumber]==NOT_IN_TREE))
		return CANNOT_INSERT_KEY;

	/*
	Update lookup table
	*/
	if (status==SUCCESS)
	{
		keytotal++;
		flag[keynumber] = IN_TREE;
	}

	/*
	Print 'no errors' message
	*/
	printf("CORRECT\t");

	/*
	Now check consistency of tree 
	*/
	status = check_consistency();
	if (status==SUCCESS)
		printf("tree still okay\n");
	return status;
}


/*
** TESTDELETE
** ==========
** 
** Purpose:	Test the deletion of a given key
**
** Parameters:	keynumber	= number of key to be deleted
**
** Returns:	A diagnostic message is sent to stdout and upon completion
**		an error/success code is returned:
**			SUCCESS	- everything worked
**			TREE_CORRUPTED_ERROR
**				- this is an error generated by Delete()
**				  itself when it thinks the tree has become
**				  corrupted
**			DELETED_NON_EXISTANT_KEY
**				- error - successfully deleted a key which
**				  should have been there in the first place
**			CANNOT_DELETE_KEY
**				- error - could delete a key which should have
**				  been in the tree
**			also can get various errors from check_consistency()
**
** Description:	Basically, do the delete, compare the result with the
**		flag lookup table - if everything seems to be okay then
**		update the lookup table and check the consistency of the
**		new B-tree
*/

int testdelete(keynumber)
int keynumber;
{
	KEY	key;
	int 	status;

	/*
	Translate key index number into a key value
	*/
	key = getkey(keynumber);
	
	/*
	Send message to log file
	*/
	printf("Delete\t");
	printkey(key);

	/*
	Now do the delete itself - can get three main errors:
	(1) Successfully deleted a key which wasn't there
	(2) Couldn't delete a key even though it was there
	(3) Delete detected a corrupted tree
	*/
	status = Delete(&tree,key);

	/*
	Print result of delete 
	*/
	if (status==SUCCESS)
		printf("deleted     \t");
	else
		printf("not deleted \t");

	/*
	Check for errors
	*/
	if (status == TREE_CORRUPTED_ERROR)
		return status;
	if ((status==SUCCESS) && (flag[keynumber]==NOT_IN_TREE))
		return DELETED_NON_EXISTANT_KEY;
	if ((status==KEY_NOT_FOUND_ERROR) && (flag[keynumber]==IN_TREE))
		return CANNOT_DELETE_KEY;

	/*
	Update lookup table
	*/
	if (status==SUCCESS)
	{
		flag[keynumber] = NOT_IN_TREE;
		--keytotal;
	}

	/*
	Print acknowledgement of success
	*/
	printf("CORRECT\t");
	/*
	Now check consistency of tree 
	*/
	status = check_consistency();
	if (status==SUCCESS)
		printf("tree still okay\n");
	return status;
}


/*********************************************************************
* CONSISTENCY CHECK ROUTINES
*********************************************************************/

	/*
	Total of number of keys found so far
	*/
static int 	total;

	/*
	Depth of tree
	*/
static int	treedepth;


/*
** CHECKCONSISTENCY
** ================
**
** Purpose:	Routine to check that a tree is in a consistent state
**
** Parameters:	None
**
** Returns:	One of the following error/success codes
**		SUCCESS - all okay
**		NODE_NOT_HALF_FULL
**			- a node was found which was not half full and it
**			  wasn't the root.
**		WRONG_KEY_IN_NODE
**			- a node was found with a key value outside its
**			  specified range
**		TREE_DEPTH_INCONSISTENT
**			- the depth of the tree is not constant throughout
**		KEY_TOTAL_MISMATCH
**			- an external count of keys in the tree does not
**			  tally with a scan of the tree.
**
** Description:	This routine fires off the 'performcheck' routine which
**		is a recursive affair - upon return, any error from
**		'performcheck' is returned to the calling routine
**		- otherwise the total number of keys in the tree is checked
**		and this determines the final return code.
*/

int check_consistency()
{
	KEY getkey();
	int status;
	
	/*
	Set up initial total number of keys and depth of tree
	*/
	total = 0;
	treedepth = 0;

	/*
	Now do the real consistency check
	*/
	status = performcheck(tree, 0, getkey(-1), getkey(MAXKEYS));
	
	/*
	if this gives any sort of error, return the error code
	*/
	if (status != SUCCESS)
		return status;

	/*
	The only remaining check is the number of keys in the tree
	- compare the test program running total (keytotal) with the
	total from this tree scan (total)
	*/
	if (total != keytotal)
		return KEY_TOTAL_MISMATCH;
	return SUCCESS;
}


/*
** PERFORMCHECK
** ============
**
** Purpose:	The main routine for checking consistency of a tree
**
** Parameters:	tree	= pointer to (sub-)tree to be checked
**		thisdepth 
**			= depth of parent node (for root this is 0)
**		lowkey	= lower bound on range of key values in
**			  current top-level node
**		highkey	= upper bound on range of key values in
**			  current top-level node
**
** Returns:	An error / success code
**			SUCCESS - all okay
**			TREE_DEPTH_INCONSISTENT
**				- tree isn't of constant depth
**			NODE_NOT_HALF_FULL
**				- current top level node is not the root
**				  and is not 1/2 full
**			WRONG_KEY_IN_NODE
**				- a key (or keys) in this node does not
**				  lie in the correct key value range
**			KEYS_NOT_IN_ORDER
**				- at least 1 key of a node is not in order
**
** Description:	This routine is a pretty complex affair and recursive to boot
**		- the basic algorithm is:
**		if we're at a root
**		then check that depth agrees with system total
**		else check size of node, range of keys in it and
**		if these are okay, try all subtrees below here
*/

int performcheck(tree, thisdepth, lowkey, highkey)
BTREE 	tree;
int	thisdepth;
KEY	lowkey,
	highkey;
{
	int 	i;
	int 	keys_in_node;
	KEY	lo,
		hi,
		upper;
	int 	status;

	/*
	Update record of dpeth of current node
	*/
	thisdepth++;

	/*
	If current tree is null we've hit a leaf
	- only check that can be performed is whether the depth is correct
	*/
	if (tree==(BTREE)NULL)
	{
		/*
		Check the depth we've reached
		*/
		if (treedepth == 0)
		{
			/*
			If overall depth is 0 then this is the first
			leaf we've reached so no error
			*/
			treedepth = thisdepth;
			return SUCCESS;
		}
		if (treedepth != thisdepth)
			/* 
			if overall depth differs from current depth then
			generate an error
			*/
			return TREE_DEPTH_INCONSISTENT;
		return SUCCESS;
	}

	keys_in_node = tree->t_active;
	/*
	If not at a leaf, check whether this node is at least 1/2 full
	- which is a prerequisite of all non-root nodes
	*/
	if ((keys_in_node<M) && (thisdepth != 1))
		return NODE_NOT_HALF_FULL;

	/*
	Now update running total of keys found so far
	*/
	total += keys_in_node;

	/*
	Now check whether the keys in this node are between lowkey and
	highkey - this only checks the left- and right- hand keys of the
	node - a check for the order of keys in a node is done later.
	*/
	if ((KeyCmp(tree->t_dat[0].key, lowkey)<=0) || 
	    (KeyCmp(tree->t_dat[keys_in_node-1].key, highkey)>=0))
		return WRONG_KEY_IN_NODE;
	
	/*
	If all's okay so far, go into the guts of the test - a recursive
	consistency check of all nodes below here
	*/
	for (i=0; i<=keys_in_node; i++)
	{
		/*
		Establish 3 quantities:
			lo    = lower bound on t_dat[i] and keys under t_ptr[i]
			hi    = upper bound on values in t_ptr[i]
			upper = upper bound on t_dat[i]
		*/
		if (i==0)
			lo = lowkey;
		else
			lo = tree->t_dat[i-1].key;

		if (i==keys_in_node)
			hi = highkey;
		else
		{
			hi = tree->t_dat[i].key;
			
			/*
			The quantity 'upper' is only needed here 
			(where i<keys_in_node)
			*/
			if (i==(keys_in_node-1))
				upper = highkey;
			else
				upper = tree->t_dat[i+1].key;
			
			/*
			Now check that t_dat[i] lies between 'lo' and 'upper'
			*/
		    	if ((KeyCmp(tree->t_dat[i].key,lo)<=0) ||
			    (KeyCmp(tree->t_dat[i].key,upper)>=0))
				return KEYS_NOT_IN_ORDER;
		}

		/*
		Do consistency check of subtree t_ptr[i]
		*/
		status = performcheck(tree->t_ptr[i],thisdepth,lo,hi);

		/*
		If a check of the subtree gave an error then bail out now
		*/
		if (status != SUCCESS)
			break;
	}
	return status;
}
		

/*************************************************************************
* VARIOUS MISCELLANEOUS ROUTINES
*************************************************************************/

/*
** GETKEY
** ======
**
** Purpose:	Routine to translate a key number into a key value
**
** Parameters:	keynumber	= number of key to be obtained
**
** Returns:	Returns a key value
**
** Description:	This routine is key dependent - it just returns a key value
**		coresponding to a key number.
**		Note that the routine must return keys with key values
**		-1 ... MAXKEYS - where the 2 extremes are used in consistency
**		checks for bounds on the key values in a tree.
*/

KEY getkey(keynumber)
int keynumber;
{
	return keynumber;
}


/*
** PRINTKEY
** ========
**
** Purpose:	Routine to print a key value
**
** Parameters:	key	= key value to be printed
**
** Returns:	None
**
** Description:	Mind-numbingly simple
*/

printkey(key)
KEY key;
{
	printf("%d\t",key);
}



/*
** ERROR
** =====
**
** Purpose:	Routine to convert an error number into an error message
**
** Parameters:	errno	= error number
**
** Returns:	none.
**
** Description:	Nothing particularly taxing about this one
*/

error(errno)
int errno;
{
	printf("\n");
	switch (errno)
	{
	case FOUND_NON_EXISTANT_KEY:
		printf("!!! SEARCH found a key which should be in the tree\n");
		break;
	case NOT_FOUND_EXISTING_KEY:
		printf("!!! SEARCH failed to find a key which should be in the tree\n");
		break;
	case FOUND_WRONG_KEY:
		printf("!!! SEARCH located the wrong key\n");
		break;
	case INSERTED_EXISTING_KEY:
		printf("!!! INSERT inserted a key into the tree when it was already there\n");
		break;
	case CANNOT_INSERT_KEY:
		printf("!!! INSERT claimed that a key was already in the tree when it wasn't\n");
		break;
	case DELETED_NON_EXISTANT_KEY:
		printf("!!! DELETE managed to delete a key which wasn't in the tree");
		break;
	case CANNOT_DELETE_KEY:
		printf("!!! DELETE claimed that a key wasn't in the tree when it was\n");
		break;
	case TREE_CORRUPTED_ERROR:
		printf("!!! TREE CORRUPTED\n");
		break;
	case NODE_NOT_HALF_FULL:
		printf("!!! CONSISTENCY ERROR - a node was found to be less than half full\n");
		break;
	case WRONG_KEY_IN_NODE:
		printf("!!! CONSISTENCY ERROR - a key was found in the wrong node\n"); 
		break;
	case TREE_DEPTH_INCONSISTENT:
		printf("!!! CONSISTENCY ERROR - the tree is not of constant depth\n");
		break;
	case KEYS_NOT_IN_ORDER:
		printf("!!! CONSISTENCY ERROR - the keys within a node are not in ascending order\n");
		break;
	case KEY_TOTAL_MISMATCH:
		printf("!!! CONSISTENCY ERROR - the total number of keys in the tree is wrong\n");
		break;
	}
	printf("\nThe tree is :-\n");
	ShowTree(tree,0);
	printf("\n\n ----> TEST ABORTED <----\n\n\n");
	exit(0);
}
