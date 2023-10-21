/*
	<ttypes.h> -- terminal type definitions


		##############################################
		#					     #
		# Copyright (c) 1982  Geotronics Corporation #
		#			   Austin, Texas     #
		#					     #
		# Unauthorized reproduction or distribution  #
		#	   is strictly prohibited.	     #
		#					     #
		##############################################


				REVISION HISTORY

28-May-1981	D A Gwyn	Created for PROMT system.
10-Apr-1982	D A Gwyn	Standardized source code.
19-Apr-1982	D A Gwyn	Added RG512 (X-10 option).
30-May-1982	D A Gwyn	Merged RG-512s (all use X-10 now).


Usage:
	#include <ttypes.h>		// in your C program source

	ttype = ... ;			// set up type somehow

	switch ( ttype ) ...		// to handle features

Note:
	This file is maintained by the Manager of Software Development.
*/

extern int	ttype;		/* global terminal type id */

#define NTTYPES 3		/* number of terminal types */

#define VT100	0		/* DEC VT100 with AVO */
				/* DEI VT640/VT20LPN raster graphics */

#define ADM3A	1		/* LSI ADM3A, ADM3A+, or ADM5 */
				/* DEI RG512/X10/X12 raster graphics */

#define VK170	2		/* DEC VK170 with SO/SI rev video */
				/* Matrox MTX512 bitmap overlay */
