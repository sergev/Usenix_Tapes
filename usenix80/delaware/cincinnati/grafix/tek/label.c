extern int ox, oy;
/*	arrange that an initial
 *	character such as +, X, *, etc will fall
 *	right on the point,
 *	but leave it pointing at the end of the label!!
 */
label(s)
char *s;
{
	register i,c;

	putch(035);	/* switch to plot mode */
	pt(ox-4,oy-7);	/* pt() to avoid scaling...this is a dark vector! */
	putch(037);	/* switch out of plot mode */
	for (i=0; c = s[i]; i++)
		putch(c);

	ox =+ (i+1)*12;	/* adjust to point to end of label */
	oy =+ 7;
	putch(035);	/* switch back to plot mode */
	pt(ox,oy);
}
