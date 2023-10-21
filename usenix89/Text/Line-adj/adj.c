/*
 * Trivial line-fill filter
 * white space at col 1 and empty lines
 * are preserved.
 * flags: -n - dont adjust right margin
 *	  -w {n} line is {n} chars wide
 *	  -w {n1,n2,...} line is cyclically n1, n2... chars wide
 *	  -h - break words with a hyphen
 *	  -p - indent only 1st line of a paragraph
 */
#define MAXB 1024
#define MAXW 100	/* words in a line */
#define NLL  50		/* line lengths vector size */
#define EOF (-1)
/* flags */
#define NOADJ 01	/* no adjust */
#define HYPHN 02	/* hyphenate */
#define PARAG 04	/* indent 1 line */
int ll[NLL] = { 65 };	/* line lengths */
int nll;		/* current line length index */
int nw;		/* current no. of words */
int ind;	/* current indentation */
static char buf[MAXB];
main(argc, argv)
	char **argv;
{
	register char *pb, *pw, *pe;
	register c, flags, nl;

	while(--argc > 0)
		if(**++argv == '-')
			switch(argv[0][1]) {
			case 'n':
				flags = NOADJ;
				break;
			case 'h':
				flags = HYPHN;
				break;
			case 'p':
				flags |= PARAG;
				break;
			case 'w':
				--argc;
				pb = *++argv;
				nl=0;
				do {
					for(ll[nl]=0; *pb>='0' && *pb<='9'; pb++)
						ll[nl] = ll[nl]*10+*pb-'0';
					nl++;
				}while(*pb++);
			}
		else {
			close(0);
			open(*argv, 0, 0);
		}
	nl = 2;
	pb = pw = buf-1;
	while((c = getchar()) != EOF) {
		if(c==' ' || c=='\t' || c=='\n') {
			if(pb>=buf && *pb!=' ') {
				*++pb = ' ';
				nw++;
				pw = pb;	/* word marker in buf */
			}
			if(c == '\n') {
				if(++nl > 1) {
					if(nw > 0) {
						putline(pb, NOADJ);
						pb = pw = buf-1;
						nw = 0;
					}
					putchar('\n');
					ind = 0;
				}
			} else if(nl > 1) {
				if(c == '\t')
					ind = (ind/8+1)*8;
				else
					ind++;
			}
		} else if(c>' ' && c<0177) {
			*++pb = c;
			if(nl) {
				nl = 0;
				pe = &buf[ll[nll]-ind];
			}
		}
		if(pb == pe) {	/* line overflow */
			if((flags&HYPHN) && pb-pw>2) {	/* insert hyphen */
				*++pb = pe[-1];
				pe[-1] = '-';
				*++pb = c;
				pw = pe;
				nw++;
			} else if(nw == 0) {	/* insert blank */
				*++pb = c;
				pw = pe;
				nw++;
			}
			putline(pw, flags);
			for(pe=buf, ++pw; pw<=pb; )
				*pe++ = *pw++;
			pb = pe-1;
			pw = buf-1;
			pe = &buf[ll[nll]-ind];
			nw = 0;
		}
	}
	if(nw > 0)
		putline(pb, NOADJ);
	return(0);
}

/*
 * adjust & print line
 */
putline(pw, flags)
	register char *pw;
{
	register char *pb;
	register i, n, b, m;

	for(i=ind; i>=8; i-=8)
		putchar('\t');
	for(; i>0; i--)
		putchar(' ');
	n = nw-1;
	b = buf+ll[nll]-ind-pw;	/* no. of blanks to distribute */
	m = n/2;
	for(pb=buf; pb<pw; pb++) {
		putchar(*pb);
		if(!(flags&NOADJ) && *pb==' ') {
			m += b;
			for(i=m/n; i>0; i--)
				putchar(' ');
			m %= n;
		}
	}
	putchar('\n');
	/* cyclic increment line lengths vector */
	if(ll[++nll] == 0)
		nll = 0;
	if(flags&PARAG)
		ind = 0;
}
