#define STAND_ALONE		/* Zap the leading comment opener for testing */
/***

* module name:
	btree.c
* function:
	Provide a library of routines for creating and manipulating
	B-Trees.  The "order" of the B-Tree is controlled by a manifest
	constant.

	This module runs stand-alone with a dummy main program
	if the symbol STAND_ALONE is defined.
* interface routines:
	BTREE
	Insert(dtm, tree)	Insert DATUM dtm into B-tree "tree",
				returning a reference to the updated
				tree.
	BTREE
	Delete(key, tree)	Remove the entry associated with "key"
				from the B-Tree.  Returns a reference
				to the updated tree.
	
	DATUM *
	Search(key, tree)	Look for key "key" in B-tree "tree".
				Return a reference to the matching
				DATUM if found, else NULL.

	Apply(t, func)		Applies function "func" to every cell
				in the B-Tree -- uses an inorder
				traversal.
* libraries used:
	standard
* compile time parameters:
	cc -O -c btree.c
* history:
	Richard Hellier, University of Kent at Canterbury,
	working from "Algorithms+Data Structures = Programs"
	by N.Wirth -- also, "Data Structures and Program Design" by B.Kruse
	Pub. Prentice-Hall.

***/


		/*
		 *	System-wide header files
		 */

#include <stdio.h>

		/*
		 *	Global structures and definitions
		 */


#define TRUE	1
#define FALSE	0

		/*
		 *	Declare the type of the KEY
		 */

typedef int	KEY;


		/*
		 *	... ditto for the INFO field
		 */

typedef int	INFO;

typedef struct {
	KEY	key;
	INFO	inf;
} DATUM;

DATUM	NullDatum = {
	-1,
};

		/*
		 *	This is the definition of
		 *	the nodes of the B-Tree
		 */

#define	M	2
typedef struct btnode {
	int			t_active;		/* # of active keys */
	DATUM			t_dat  [2 * M];		/* Keys  + Data */
	struct btnode		*t_ptr [2 * M + 1];	/* Subtree ptrs */
} NODE, *BTREE;

static BTREE		NewNode;


/*
**  ERROR -- Print an error message
**
**	Write the error text to the
**	standard error stream.
**
**	Parameters:
**		fmt       --  Printf-style format string
**		arg[1-3]  --  Arguments as needed.
**
**	Returns:
**		None.
**
*/

/* ARGSUSED */

Error(fmt, arg1, arg2, arg3)
char	*fmt;{
	fprintf(stderr, fmt, arg1, arg2, arg3);
}

/*
**  KEYCMP -- Compare two key values
**
**	Like "strcmp" but use key
**	values rather than strings.
**
**	Parameters:
**		key1  --  First key,
**		key2  --  Second key.
**
**	Returns:
**		-1 if key1 < key2;
**		0  if key1 = key2;
**		1  if key1 > key2;
**
*/

KeyCmp(key1, key2)
KEY	key1,
	key2;{

	return key1 < key2 ? -1 : key1 == key2 ? 0 : 1;
}

/*
**  SHOWDATUM -- Display a datum
**
**	Atomic routine used to display
**	the contents of a datum.
**
**	Parameters:
**		datum  --  Datum to print.
**
**	Returns:
**		None.
**
*/

ShowDatum(dtm)
DATUM	dtm;{
	printf("%d ", dtm . key);
}

/*
**  MKNODE -- Make a new B-tree node
**
**	Allocate storage for a new B-tree node
**	and copy in some pointers.
**
**	Parameters:
**		k1  --  First key value,
**		p0  --  First ptr,
**		p1  --  Second ptr.
**
**	Returns:
**		Reference to the new node.
**
*/

BTREE
MkNode(dtm, p0, p1)
DATUM	dtm;
BTREE	p0,
	p1;{
	char	*malloc();
	BTREE	t;

	t = (BTREE) malloc(sizeof(NODE));

	t -> t_ptr [0] = p0;
	t -> t_ptr [1] = p1;
	t -> t_dat [0] = dtm;
	t -> t_active  = 1;

	return t;
}
/*
**  DISPOSE -- Dispose of a tree node
**
**	Return the storage occupied by the
**	tree node to the pool.
**
**	Parameters:
**		t  --  Ptr to the tree node.
**
**	Returns:
**		None.
**
*/

Dispose(t)
BTREE	t;{
	free((char *) t);
}

/*
**  INSINNODE -- Add a key to a node
**
**	Add a key value into a
**	B-tree node.  Splitting of
**	nodes is handled elsewhere.
**
**	Parameters:
**		t   --  Ptr to the node,
**		key --  Key value to enter,
**		ptr --  Link to subordinate node.
**
**	Returns:
**		None.
**
*/

InsInNode(t, d, ptr)
BTREE	t,
	ptr;
DATUM	d;{
	int	i;

	for ( i = t -> t_active; i > 0 && KeyCmp(d . key, t -> t_dat [i-1] . key) < 0; i--) {
		t -> t_dat [i]     = t -> t_dat [i - 1];
		t -> t_ptr [i + 1] = t -> t_ptr [i];
	}

	t -> t_active++;
	t -> t_dat [i]   = d;
	t -> t_ptr [i+1] = ptr;
}

/*
**  INTERNALINSERT -- Key insert routine proper
**
**	The business end of the key insertion
**	routine.
**
**	Parameters:
**		key  --  Key to insert,
**		t    --  Tree to use.
**
**	Returns:
**		Value of the key inserted.
**
*/

DATUM
InternalInsert(dtm, t)
DATUM	dtm;
BTREE	t;{
	int	i,
		j;
	DATUM	ins;
	BTREE	tmp;

	if (t == NULL) {
		NewNode = NULL;
		return dtm;
	} else {
		for (i = 0; i < t -> t_active && KeyCmp(t -> t_dat [i] . key, dtm . key) < 0; ++i)
			;		/* NULL BODY */
		if (i < t -> t_active && KeyCmp(dtm . key, t -> t_dat [i] . key) == 0)
			fprintf(stderr, "Already had it!\n");
		else {
			ins = InternalInsert(dtm, t -> t_ptr [i]);

			if (KeyCmp(ins . key, NullDatum . key) != 0)
				if (t -> t_active < 2 * M)
					InsInNode(t, ins, NewNode);
				else {
					if (i <= M) {
						tmp = MkNode(t -> t_dat [2 * M - 1], (BTREE) NULL, t -> t_ptr [2 * M]);
						t -> t_active--;
						InsInNode(t, ins, NewNode);
					} else
						tmp = MkNode(ins, (BTREE) NULL, NewNode);
					/*
					 *	Move keys and pointers ...
					 */

					for (j = M + 2; j <= 2 * M; ++j)
						InsInNode(tmp, t -> t_dat [j-1], t -> t_ptr [j]);

					t -> t_active = M;
					tmp -> t_ptr [0] = t -> t_ptr [M+1];
					NewNode = tmp;

					return t -> t_dat [M];
				}
		}
		return NullDatum;
	}
}
/*
**  INSERT -- Put a key into the B-tree
**
**	Enter the given key into the
**	B-tree.
**
**	Parameters:
**		key  --  Key value to enter.
**
**	Returns:
**		Reference to the new B-tree.
**
*/

BTREE
Insert(dtm, t)
DATUM	dtm;
BTREE	t;{
	DATUM	ins;

	ins = InternalInsert(dtm, t);

	/*
	 *	Did the root grow ?
	 */

	return KeyCmp(ins . key, NullDatum . key) != 0 ? MkNode(ins, t, NewNode) : t;
}
/*
**  DELETE -- Remove a key from a B-tree
**
**	Remove the data associated with a
**	given key from a B-tree.
**
**	Parameters:
**		key  --  Key to use,
**		t    --  B-tree to update.
**
**	Returns:
**		Reference to the updated B-tree.
**
**	Notes:
**		Real work is done by RealDelete() q.v.
**
*/

static int	hadit = FALSE;

BTREE
Delete(key, t)
KEY	key;
BTREE	t;{
	BTREE	savet;

	RealDelete(key, t);

	if (!hadit) {
		Error("No such key\n");
		return NULL;
	} else if (t -> t_active == 0) {	/* Root is now empty */
		savet = t -> t_ptr [0];
		Dispose(t);
		return savet;
	} else
		return t;
}

/*
**  SEARCHNODE -- Find a key in a node
**
**	Look for a key in a single B-tree
**	node.
**
**	Parameters:
**		key  --  Key to look for,
**		t    --  Ptr to B-tree node.
**
**	Returns:
**		Index of matching key.
**
*/

int
SearchNode(key, t)
KEY	key;
BTREE	t;{
	int	i;

	if (KeyCmp(key, t -> t_dat [0] . key) < 0) {
		hadit = FALSE;
		return 0;
	} else {
		for (i = t -> t_active; KeyCmp(key, t -> t_dat [i-1] . key) < 0 && i > 1; --i)
			;		/* NULL BODY */
		hadit = (KeyCmp(key, t -> t_dat [i-1] . key) == 0);

		return i;
	}
}
/*
**  REALDELETE -- Remove a key from a tree
**
**	The business end of the Delete() function.
**
**	Parameters:
**		key  --  Key to use,
**		t    --  Tree to update.
**
**	Returns:
**		None.
**
*/

RealDelete(key, t)
KEY	key;
BTREE	t;{
	int	k;

	if (t == NULL)
		hadit = FALSE;
	else {
		k = SearchNode(key, t);

		if (hadit) {
			if (t -> t_ptr [k-1] == NULL)		/* A leaf */
				Remove(t, k);
			else {
				Succ(t, k);
				RealDelete(t -> t_dat [k-1] . key, t -> t_ptr [k]);
				if (!hadit)
					Error("Hmm ???");
			}
		} else {
			RealDelete(key, t -> t_ptr [k]);

			if (t -> t_ptr [k] != NULL && t -> t_ptr [k] -> t_active < M)
				Restore(t, k);
		}
	}
}

/*
**  REMOVE -- Remove a single datum
**
**	Remove a datum from a B-tree node
**	by shuffling down the pointers and
**	data above it.
**
**	Parameters:
**		t   --  Ptr to the B-tree node,
**		pos --  Index of the key to be removed.
**
**	Returns:
**		None.
**
*/

Remove(t, pos)
BTREE	t;
int	pos;{
	int	i;

	for (i = pos + 1; i <= t -> t_active; ++i) {
		t -> t_dat [i-2] = t -> t_dat [i-1];
		t -> t_ptr [i-1] = t -> t_ptr [i];
	}
	t -> t_active--;
}

/*
**  SUCC -- Replace a key by its successor
**
**	Using the natural ordering, replace
**	a key with its successor.
**
**	Parameters:
**		t   --  Ptr to a B-tree node,
**		pos --  Index of the key to be succ'ed.
**
**	Returns:
**		None.
**
**	Notes:
**		This routine relies on the fact
**		that if the key to be deleted is
**		not in a leaf node, then its
**		immediate successor will be.
*/

Succ(t, pos)
BTREE	t;
int	pos;{
	BTREE	tsucc;

	/*
	 *	Using the branch *above* the key
	 *	chain down to leftmost node below
	 *	it.
	 */

	for (tsucc = t -> t_ptr [pos]; tsucc -> t_ptr [0] != NULL; tsucc = tsucc -> t_ptr [0])
		;		/* NULL BODY */

	t -> t_dat [pos-1] = tsucc -> t_dat [0];
}
/*
**  RESTORE -- Restore balance to a B-tree
**
**	After removing an item from a B-tree
**	we must restore the balance.
**
**	Parameters:
**		t   --  Ptr to the B-tree node,
**		pos --  Index of the out-of-balance point.
**
**	Returns:
**		None.
**
*/

Restore(t, pos)
BTREE	t;
int	pos;{
	if (pos > 0) {
		if (t -> t_ptr [pos - 1] -> t_active > M)
			MoveRight(t, pos);
		else
			Combine(t, pos);
	} else {
		if (t -> t_ptr [1] -> t_active > M)
			MoveLeft(t, 1);
		else
			Combine(t, 1);
	}
}

/*
**  MOVERIGHT -- Shuffle keys up
**
**	Make room for a key in a B-tree
**	node.
**
**	Parameters:
**		t   --  Ptr to the B-tree node,
**		pos --  Index of the first key
**			to be moved.
**
**	Returns:
**		None.
**
*/

MoveRight(t, pos)
BTREE	t;
int	pos;{
	int	i;
	BTREE	tpos;

	tpos = t -> t_ptr [pos];

	for (i = tpos -> t_active; i >= 1; i--) {
		tpos -> t_dat [i]   = tpos -> t_dat [i-1];
		tpos -> t_ptr [i+1] = tpos -> t_ptr [i];
	}

	tpos -> t_ptr [1] = tpos -> t_ptr [0];
	tpos -> t_active++;
	tpos -> t_dat [0] = t -> t_dat [pos-1];

	tpos = t -> t_ptr [pos-1];

	t -> t_dat [pos-1]            = tpos -> t_dat [tpos -> t_active-1];
	t -> t_ptr [pos] -> t_ptr [0] = tpos -> t_ptr [tpos -> t_active];
	tpos -> t_active--;
}
/*
**  MOVELEFT -- Shuffle keys down
**
**	Shuffle keys down after a removal
**	from a B-tree node.
**
**	Parameters:
**		t   --  Ptr to the B-tree node,
**		pos --  Index of the first key
**			to be moved.
**
**	Returns:
**		None.
**
*/

MoveLeft(t, pos)
BTREE	t;
int	pos;{
	int	i;
	BTREE	tpos;

	tpos = t -> t_ptr [pos-1];

	tpos -> t_active++;
	tpos -> t_dat [tpos -> t_active-1] = t -> t_dat [pos-1];
	tpos -> t_ptr [tpos -> t_active]   = t -> t_ptr [pos] -> t_ptr [0];


	tpos = t -> t_ptr [pos];

	t -> t_dat [pos-1]  = tpos -> t_dat [0];
	tpos -> t_ptr [0]   = tpos -> t_ptr [1];
	tpos -> t_active--;

	for (i = 1; i <= tpos -> t_active; ++i) {
		tpos -> t_dat [i-1] = tpos -> t_dat [i];
		tpos -> t_ptr [i]   = tpos -> t_ptr [i+1];
	}
}
/*
**  COMBINE -- Combine two nodes.
**
**	Coalesce two nodes of a
**	B-tree when they have too few nodes.
**
**	Parameters:
**		t   --  Ptr to B-tree node,
**		pos --  Index of the split point.
**
**	Returns:
**		None.
**
*/

Combine(t, k)
BTREE	t;
int	k;{
	int	i;
	BTREE	p,
		q;

	p = t -> t_ptr [k-1];
	q = t -> t_ptr [k];

	p -> t_active++;
	p -> t_dat [p -> t_active-1] = t -> t_dat [k-1];
	p -> t_ptr [p -> t_active]   = q -> t_ptr [0];

	for (i = 1; i <= q -> t_active; ++i) {
		p -> t_active++;
		p -> t_dat [p -> t_active-1] = q -> t_dat [i-1];
		p -> t_ptr [p -> t_active]   = q -> t_ptr [i];
	}

	for (i = k; i <= t -> t_active - 1; ++i) {
		t -> t_dat [i-1] = t -> t_dat [i];
		t -> t_ptr [i]   = t -> t_ptr [i+1];
	}
	t -> t_active--;

	Dispose(q);
}

/*
**  SEARCH -- Fetch a key from a tree
**
**	Look for a key in a tree.
**
**	Parameters:
**		key  --  Key value to look for,
**		t    --  Tree to look in.
**
**	Returns:
**		None.
**
*/

DATUM *
Search(key, t)
KEY	key;
BTREE	t;{
	int	i;

	while (t != NULL) {
		for (i = 0; i < t -> t_active && KeyCmp(t -> t_dat [i] . key, key) < 0; ++i)
			;		/* NULL BODY */

		if (i < t -> t_active && KeyCmp(key, t -> t_dat [i] . key) == 0) {
			/* FOUND IT */
			return &t -> t_dat [i];
		}
		t = t -> t_ptr [i];
	}
	return NULL;
}
/*
**  SHOWTREE -- Display a tree
**
**	Print the contents of
**	a tree, showing the level
**	of each node.
**
**	Parameters:
**		t     --  Tree to print,
**		level --  Depth in tree.
**
**	Returns:
**		None.
**
*/

ShowTree(t, level)
BTREE	t;
int	level;{
	int	i;

	if (t != NULL) {
		for (i = 0; i < level; ++i)
			putchar(' ');
		for (i = 0; i < t -> t_active; ++i)
			ShowDatum(t -> t_dat [i]);
		putchar('\n');
		for (i = 0; i <= t -> t_active; ++i)
			ShowTree(t -> t_ptr [i], 1 + level);
	}
}

/*
**  APPLY -- Apply a function to a b-tree
**
**	Traverse a B-tree, applying the function
**	to each key we find.
**
**	Parameters:
**		t    --  The tree to display,
**		func --  The function to apply.
**
**	Returns:
**		None.
**
*/

Apply(t, func)
BTREE	t;
int	(*func)();{
	int	i;

	if (t != NULL) {
		for (i = 0; i < t -> t_active; ++i) {
			Apply(t -> t_ptr [i], func);
			(*func) (t -> t_dat [i]);
		}
		Apply(t -> t_ptr [t -> t_active], func);
	}
}
#ifdef STAND_ALONE
main(){
	BTREE	t,
		oldt;
	DATUM	d,
		*dp;
	KEY	k;
	char	buf [BUFSIZ];

	t = NULL;

	printf("Command: "); fflush(stdout);
	while (fgets(buf, sizeof buf, stdin) != NULL) {
		buf [strlen(buf) - 1] = '\0';

		/*
		 *	Get the command
		 */

		switch (buf [0]) {
			default:		/* Error case */
				fprintf(stderr, "I, D, L, P or S please!\n");
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
				sscanf(buf, "%d", &d . key);
				t = Insert(d, t);
				break;

			case 'S':		/* Set up default tree */
				t = NULL;
				d . key = 20;
				t = Insert(d, t);
				d . key = 10;
				t = Insert(d, t);
				d . key = 15;
				t = Insert(d, t);
				d . key = 30;
				t = Insert(d, t);
				d . key = 40;
				t = Insert(d, t);
				d . key = 7;
				t = Insert(d, t);
				d . key = 18;
				t = Insert(d, t);
				d . key = 22;
				t = Insert(d, t);
				d . key = 26;
				t = Insert(d, t);
				d . key = 5;
				t = Insert(d, t);
				d . key = 35;
				t = Insert(d, t);
				d . key = 13;
				t = Insert(d, t);
				d . key = 27;
				t = Insert(d, t);
				d . key = 32;
				t = Insert(d, t);
				d . key = 42;
				t = Insert(d, t);
				d . key = 46;
				t = Insert(d, t);
				d . key = 24;
				t = Insert(d, t);
				d . key = 45;
				t = Insert(d, t);
				d . key = 25;
				t = Insert(d, t);
				ShowTree(t, 0);
				break;

			case 'I':		/* Insert a key */
				sscanf(buf+1, "%d", &d . key);
				t = Insert(d, t);
				break;

			case 'D':		/* Delete a key */
				sscanf(buf+1, "%d", &d . key);
				oldt = t;
				t = Delete(d . key, t);
				if (t == NULL)
					t = oldt;
				break;

			case 'L':		/* Lookup a key */
				sscanf(buf+1, "%d", &d . key);
				dp = Search(d . key, t);
				printf("%s\n",
					dp != NULL ? "Found it" : "No such key");
				break;

			case 'P':		/* Show the tree */
				ShowTree(t, 0);
				break;
		}
		printf("Command: "); fflush(stdout);
	}
	Apply(t, ShowDatum);
	exit(0);
}
#endif STAND_ALONE
