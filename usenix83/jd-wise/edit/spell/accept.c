
accept(word)
	char *word;
	{
	register i,j;
	register long *lp;
	register char *wp;
	long h;
	long *p=P[0];

	for (i=0; i<NP; i++) {
		for (wp = word, h = 0, lp = POW2[0][i];
			 (j = *wp) != '\0'; ++wp, ++lp)
			h += j * *lp;
		h += '\n' * *lp;
		h %= p[i];
		set(dicttab,h);
		}
	}
