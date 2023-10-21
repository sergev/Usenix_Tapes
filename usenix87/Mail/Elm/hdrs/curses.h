/*** 			curses.h			***/

/*** (C) Copyright 1986 Dave Taylor			***/

     /*** Include file for seperate compilation.  ***/

#define OFF		0
#define ON 		1

int  InitScreen(),      /* This must be called before anything else!! */

     ClearScreen(), 	 CleartoEOLN(),

     MoveCursor(),
     CursorUp(),         CursorDown(), 
     CursorLeft(),       CursorRight(), 

     StartBold(),        EndBold(), 
     StartUnderline(),   EndUnderline(), 
     StartHalfbright(),  EndHalfbright(),
     StartInverse(),     EndInverse(),
	
     transmit_functions(),

     Raw(),              ReadCh();

char *return_value_of();
