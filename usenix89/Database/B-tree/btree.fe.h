/********************************************************************
*********************************************************************
 
Module Name:		btree.fe.h
============
 
Function:		Front end for btree code ...
=========		

Description:
============
	Implements a front-end program for the btree code

****************************************************************************
****************************************************************************/

static char btreefrontend[] = "@(#)btree.fe.h	1.2 8/18/86";



/*
** FRONTEND
** ========
**
** Purpose:	Front panel type thing for btree code - allows interactive
**		manipulation of tree
**
** Parameters:	none
**
** Returns:	none
**
** Description:	The following 'commands' are implemented
**		<n>	Insert key 'n'
**		i<n>	ditto
**		d<n>	Delete key 'n'
**		l<n>	Locate key 'n'
**		p<n>	Print tree
**		s	Set up default tree
**		x	Exit from front end
*/

frontend()
{
	int 	status;
	BTREE	tree;
	DATUM	dtm,
		dtm2;
	KEY	key;
	char	buf[BUFSIZ];

	tree = (BTREE)NULL;

	printf("Command: "); fflush(stdout);
	while (fgets(buf, sizeof buf, stdin) != NULL) 
	{
		buf[strlen(buf) - 1] = '\0';
		switch (buf[0]) 
		{
		default:		/* Error case */
			fprintf(stderr, "i, d, l, p, s or x please!\n");
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			sscanf(buf, "%d", &dtm.key);
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			break;

		case 's':	/* Set up default tree */
			tree = (BTREE)NULL;
			dtm.key = 20;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 10;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 15;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 30;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 40;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 7;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 18;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 22;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 26;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 5;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 35;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 13;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 27;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 32;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 42;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 46;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 24;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 45;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			dtm.key = 25;
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			ShowTree(tree, 0);
			break;

		case 'i':		/* Insert a key */
			sscanf(buf+1, "%d", &dtm.key);
			status = Insert(&tree, dtm);
			if (status != SUCCESS)
				btree_err(status);
			break;

		case 'd':		/* Delete a key */
			sscanf(buf+1, "%d", &dtm.key);
			status = Delete(&tree, dtm.key);
			if (status != SUCCESS)
				btree_err(status);
			break;

		case 'l':		/* Lookup a key */
			sscanf(buf+1, "%d", &dtm.key);
			status = Search(tree, dtm.key, &dtm2);
			if (status != SUCCESS)
				btree_err(status);
			printf("Found %d\n",dtm2.key);
			break;

		case 'p':		/* Show the tree */
			ShowTree(tree, 0);
			break;
		
		case 'x':
			break;
		}
		if (buf[0]=='x')
			break;
		else
		{
			printf("Command: "); 
			fflush(stdout);
		}
	}
}


/*
** ERROR ROUTINE
** =============
**
** Purpose:	Error message routine for btree front-end system
**
** Parameters:	errcode	= errcode
**
** Returns:	none
**
** Description:	Pretty simple...
*/

btree_err(errcode)
int	errcode;
{
	switch (errcode)
	{
	case KEY_EXISTS_ERROR:
		fprintf(stderr,"Key already exists !\n");
		break;
	case KEY_NOT_FOUND_ERROR:
		fprintf(stderr,"Key not found !\n");
		break;
	case TREE_CORRUPTED_ERROR:
		fprintf(stderr,"Tree corrupted !\n");
		break;
	}
}
