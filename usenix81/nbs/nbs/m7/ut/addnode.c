/*  addnode.c
*
*  	This routine enters the number of the macro into the list
*	scanlist, which contains the numbers of all macros with the
*	"<" feature. This feature insures the macro is used only once. 
*	   Recall that the macros are added in ascending order; thus we do not
*	need to search the linked list for the correct postition for the macro.
*
*	Parameters: scanlist - the list of macro #'s with '<'.
*	   macnum - the macro's # which is being added to 'scanlist'.
*	   rescanval - the flag's value to show it hasn't been used.
*	   availhead - the next available node in 'scanlist'.
*
*			Programmmer G. Skillman
*/
#include "globdefin.h" /* This defines the size of the list. */
addnode(scanlist, macnum, rescanval, availhead)

int	scanlist [2] [MAXSCANOFFS], macnum, rescanval, *availhead;
{
	scanlist [0] [*availhead] = macnum;
        scanlist [1] [*availhead] = rescanval;
	*availhead = (*availhead) + 1;
}
