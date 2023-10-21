/****************************************************************************
*****************************************************************************
**
** Module Name:		btree.prt.h
** ============
**
** Function:		routine for printing a tree
** =========
**
** Modification History
** --------------------
** Vers		Date		Mdfr	Reason for Change
** ------------------------------------------------------
** 1.1		17/07/86	SMJ	Adapted from btree.c v1.4
** -------------------------------------------------------
**
** Description:
** ============
** 	See for yourself
*/

static char btreeprt[] = "@(#)btree.prt.h	1.1 7/17/86";

/****************************************************************************
* TREE PRINTING ROUTINES 
****************************************************************************/

/*
** SHOWTREE
** ========
**
** Purpose:	Print the contents of a tree, showing the level of each node.
**
** Parameters: 	tree	= Tree to print,
**		level	= Depth in tree.
**
** Returns: 	None.
**
** Description:	Recursively scan down the tree, printing out the
**		contents of each node in turn, indented in accordance with
**		their depth in the tree.
**		The 'level' parameter allows a limit to be set on the depth
**		printout.
*/

ShowTree(tree, level)
BTREE	tree;
int	level;
{
	int	i;

	if (tree != (BTREE)NULL) 
	{
		for (i=0; i<level; ++i)
			putchar('	');
		for (i=0; i<tree->t_active; ++i)
			ShowDatum(tree->t_dat[i]);
		putchar('\n');
		for (i=0; i<=tree->t_active; ++i)
			ShowTree(tree->t_ptr[i], 1+level);
	}
}


/*
** SHOWDATUM
** =========
**
** Purpose:	Routine to print the contents of a given datum
**
** Parameters: 	dtm	= Datum to print.
**
** Returns: 	None.
**
*/

ShowDatum(dtm)
DATUM	dtm;
{
	printf("%d ", dtm.key);
}
