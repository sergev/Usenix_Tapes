/**		quit.c		**/

/** quit: leave the current mailbox and quit the program.
  
    (C) Copyright 1985, Dave Taylor
**/

#include "headers.h"

long bytes();

quit()
{
	/* a wonderfully short routine!! */

	if (leave_mbox(1) == -1)
	  return;			/* new mail!  (damn it)  resync */

	leave();
}

resync()
{
	/** Resync on the current mailbox... This is simple: simply call
	   'newmbox' to read the file in again, set the size (to avoid
	    confusion in the main loop) and refresh the screen!
	**/

	  newmbox(1, TRUE, TRUE);
	  mailfile_size = bytes(infile);	
	  showscreen();
}
