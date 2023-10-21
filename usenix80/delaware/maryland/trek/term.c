#
/*
 *      Star Trek
 *              by    Robert Haar, Computer Vision Lab., U. of Md.
 *
 *
 *                      I/O routines
 */


/*------------------------------------------------------------

		terminal specific code
			    currently for Datamedia  terminals

---------------------------------------------------------------*/


blinkc(c)                       /* print the char C in blink mode */
char c;
{   printf("\016%c\030",c);
    }

poscur(col,row)                 /* position the cursor at <col,row>
				   zero indexed from upper left corner   */
int col,row;
{
	char pp[3];

	pp[0]=014;
	pp[1]=0140 - (0140 & col) + (037 & col);
	pp[2]=row + 0140;
	write(1,pp,3);
	}


home()                                  /* move cursor to HOME position */
{    printf("\002");   }


homers(){                               /* move to HOME (upper left corner),
					   cancel roll mode,
					   and erase the screen  */
	printf("\002\030\036");
}

erasel(){                               /* erase rest of line from current position */
	printf("\027");
}

roll() {                                /* turn on roll mode */
	printf("\035");
}

rolloff() {                             /* turn off rol mode */
	printf("\030");
}
