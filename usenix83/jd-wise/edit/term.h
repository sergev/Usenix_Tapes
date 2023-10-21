/*
 * Terminal characteristics
 */
#define curev 1

#define INFOTON 1
#define HP2621 2
#define HEATH19 5
#define VISUAL 6
#define TEST 7
#define V50 8

struct termchar{
	/*
	 * integer parameters
	 */
	int	a0;	/* start dummy */
	int	co;	/* number of columns */
	int	li;	/* number of lines */
	int	ty;	/* terminal type code */
	/*
	 * cursor addressing information
	 */
	struct{
		int of;	/* offset */
		int cf;	/* flags */
		} cu;
	/*
	 * character strings
	 */
	char*	aa;	/* start dummy */
	char*	ca;	/* cursor addressing */
	char*	cl;	/* clear screen */
	char*	cT;	/* clear all tabs */
	char*	ct;	/* clear tab */
	char*	dc;	/* delete char */
	char*	dl;	/* delete line */
	char*	el;	/* erase to end of line */
	char*	ep;	/* erase to end of page */
	char*	ho;	/* home */
	char*	ic;	/* insert char */
	char*	il;	/* insert line */
	char*	ma;	/* map */
	char*	st;	/* set tab */
	char*	zz;	/* end dummy */
	}t;
