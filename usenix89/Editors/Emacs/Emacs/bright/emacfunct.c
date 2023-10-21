
/*********************************
 * Search for the next occurence of the character at dot.
 * If character is a (){}[]<>, search for matching bracket, taking
 * proper account of nesting.
 * Written by Walter Bright.
 */

int searchparen(f, n)
    {
    register LINE *clp;
    register int cbo;
    register int len;
    register int i;
    char chinc,chdec,ch;
    int count;
    int forward;
    static char bracket[][2] = {{'(',')'},{'<','>'},{'[',']'},{'{','}'}};

    clp = curwp->w_dotp;		/* get pointer to current line	*/
    cbo = curwp->w_doto;		/* and offset into that line	*/
    count = 0;

    len = llength(clp);
    if (cbo >= len)
	chinc = '\n';
    else
	chinc = lgetc(clp,cbo);

    forward = TRUE;			/* assume search forward	*/
    chdec = chinc;
    for (i = 0; i < 4; i++)
	if (bracket[i][0] == chinc)
	{	chdec = bracket[i][1];
		break;
	}
    for (i = 0; i < 4; i++)
	if (bracket[i][1] == chinc)
	{	chdec = bracket[i][0];
		forward = FALSE;	/* search backwards		*/
		break;
	}

    while (1)
    {
	if (forward)
	{
	    if (cbo >= len)
	    {	/* proceed to next line */
		clp = lforw(clp);
		if (clp == curbp->b_linep)	/* if end of buffer	*/
		    break;
		len = llength(clp);
		cbo = 0;
	    }
	    else
		cbo++;
	}
	else /* backward */
	{
	    if (cbo == 0)
            {
		clp = lback(clp);
		if (clp == curbp->b_linep)
		    break;
		len = llength(clp);
		cbo = len;
            }
	    else
		--cbo;
	}

	ch = (cbo < len) ? lgetc(clp,cbo) : '\n';
	if (eq(ch,chdec))
	{   if (count-- == 0)
	    {
		/* We've found it	*/
	        curwp->w_dotp  = clp;
	        curwp->w_doto  = cbo;
	        curwp->w_flag |= WFMOVE;
	        return (TRUE);
	    }
	}
	else if (eq(ch,chinc))
	    count++;
    }
    mlwrite("Not found");
    return (FALSE);
}
