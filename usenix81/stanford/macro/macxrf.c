#include <stdio.h>

struct ref {
	struct ref * rf_nxt;	/* pointer to the next reference */
	int line;	/* line of this reference */
	int page;	/* page of this reference */
	int rflgs;	/* type of this reference */
	};

struct tabent {
	struct tabent * tb_nxt; /* pointer to next name in list */
	char * tb_prnt;		/* charaters of name */
	struct ref * tb_ent;	/* head of list of references to name */
		} *hdsym, *hdmac, *hdpsct, *hdcsct, *newtb();

char * malloc();

#define EBASE 020
#define VT 013
#define NEWPAGE 02
#define NEWLINE 03
#define NTYPES 4
#define PLAIN 0
#define NUM 1
#define AT 2
#define NUMAT 4
#define PLNCHR "  "
#define ATCHR "@ "
#define NUMCHR "# "
#define NMATCHR "#@"

int flgtab[NTYPES] = { PLAIN, NUM, AT, NUMAT };

char *flgstr[8] = {  PLNCHR, NUMCHR, ATCHR, NMATCHR,
		     NMATCHR, NMATCHR, NMATCHR, NMATCHR };

#define LINMAX 80
#define ENTMAX 32
#define FLGMAX  7
#define SYMCH "S"
#define MACCH "M"
#define PCTCH "P"
#define CCTCH "C"
#define MAXLIN 60

#define HDRFMT "\fcross reference table   %s-%d\r\n\r\n"
#define MACFMT "%2d-%2d"
#define LNKFMT "%d-%d"

char *linfmt = MACFMT;


int lincnt;
int pageno;
char *hdchar;
char bufr[LINMAX+1];
char *unlkfl = NULL;


main(argc, argv)
int argc;char **argv;
{
	int before = 0, line = 0, page = 0;
	int c;
	register char *bfrp;

	/* set up files */
	arginit(argc, argv);

	hdsym = NULL;
	hdmac = NULL;
	hdpsct = NULL;
	hdcsct = NULL;
	bfrp = &bufr[0];
	zerout(bfrp, LINMAX);
/*
 * read input file and put things
 * away in linked list, performing
 * insertion sort of names - lines and
 * pages are encounetered in order
 */
	while( (c = getchar()) != EOF)
		{
		c &= 0377;
		switch( c )
			{
			case 0 :
				break;
			case NEWLINE :
				line++;
				break;
			case NEWPAGE :
				page++;
				line = 0;
				break;
			case VT :
				break;
			case EBASE :
			case EBASE+1 :
			case EBASE+2 :
			case EBASE+3 :
				if(before == 0)
					{
					before = c;
					}
					else
					{
					*bfrp++ = 0;
					tin(before, c, &bufr[0], line, page);
					before = 0;
					bfrp = &bufr[0];
					}
				break;
			default:
				*bfrp++ = c;
			}

		}
/*
 * print out cross reference tables
 * now is a simple linear traverse
 * of the structures
 */
	if(hdsym != NULL)
		{
		lincnt = 0;
		pageno = 1;
		hdchar = SYMCH;
		prntrf(hdsym);
		}
	if(hdmac != NULL)
		{
		lincnt = 0;
		pageno = 1;
		hdchar = MACCH;
		prntrf(hdmac);
		}
	if(hdpsct != NULL)
		{
		lincnt = 0;
		pageno = 1;
		hdchar = PCTCH;
		prntrf(hdpsct);
		}
	if(hdcsct != NULL)
		{
		lincnt = 0;
		pageno = 1;
		hdchar = CCTCH;
		prntrf(hdcsct);
		}
	if(unlkfl != NULL)unlink(unlkfl);
}

/*
 * Setup stdin and stdout for correct files.  <file>.xrf contains
 * cross reference information, cross reference tables are to
 * be appended to <file>.lst or <file>.map.  In the latter case,
 * the linker line format is used for writing out the cross
 * reference table.
 */

arginit(argc, argv)
int argc;char **argv;
{
	register char *np;
	register int len;
	char *flnm;

	if(argc != 4)return;
	/* if we are exec'd by MACRO or LINKR we will
	 * have argc of 4
	 *
	 * argv[0] - name by which we were called
 	 *
	 * argv[1] - an int (short?) which is the fildes
	 *	of the listing file
	 *
	 * argv[2] - the name of the listing file
	 *
	 * argv[3] - an int (short?) which is a flag indicating
	 *	whether or not to spool the listing file for
	 * 	lineprinter output (ignored here)
	 *
	 */
	flnm = malloc(len = strlen(argv[2])+1);
	strcpy(flnm, argv[2]);
	np = &flnm[len-1];
	while( (np > flnm) & (*np != '.') )np--;
	np++;
	if(strcmp(np, "lst") != 0)
		{
		if(strcmp(np, "map") != 0)
			{
			fprintf(stderr,"Cref:  bad file extenstion\n");
			exit(10);
			}
		linfmt = LNKFMT;
		}
	strcpy(np, "xrf");
	/* open files, <file>.xrf for input, <file>.map or lst for output */
	if(freopen(argv[2], "a", stdout) == NULL)
			{
			fprintf(stderr,"Cref: can't append to %s\n",
				argv[2]);
			exit(11);
			}
	if(freopen(flnm, "r", stdin) == NULL)
			{
			fprintf(stderr,"Cref: can't open %s for reading\n",
				flnm);
			exit(12);
			}
	unlkfl = flnm;
}

/*
 * insert name and current reference into 
 * list structure 
 */

tin(type, etype, name, line, page)
int type, etype;
char *name;
int line, page;
{
	register struct tabent *cur, *last, **head;
	register int cmp;

	switch(type)
		{
		case EBASE:
			cur = hdsym;
			head = &hdsym;
			break;
		case EBASE+1:
			cur = hdmac;
			head = &hdmac;
			break;
		case EBASE+2:
			cur = hdpsct;
			head = &hdpsct;
			break;
		case EBASE+3:
			cur = hdcsct;
			head = &hdcsct;
			break;
		default:
			fprintf(stderr,"Cref:  Type error: %o\n",type);
			cur = NULL;
		}
	/* first time, initialize list */

	if(cur == NULL)
		{
		*head = newtb(etype, name, line, page);
		return;
		}

	/* look down list for our spot */

	last = NULL;
	while((cur->tb_nxt != NULL)&((cmp = cmpstr(name, cur->tb_prnt)) > 0))
			{
			last = cur;
			cur = cur->tb_nxt;
			}
	if(cmp == 0) /* add a reference to already encountered name */
		{
		addrf(cur, etype, line, page);
		return;
		}
	if(last == NULL)
		{ /* it is the beginning */
		if(cmp < 0)
			{
			*head = newtb(etype, name, line, page);
			(*head)->tb_nxt = cur;
			}
			else
			cur->tb_nxt = newtb(etype, name, line, page);
		return;
		}
	if(cur->tb_nxt == NULL)
		{ /* it is the end */
		if(cmp > 0)/* append new name */
			cur->tb_nxt = newtb(etype, name, line, page);
			else
			{ /* stick in front of end */
			last->tb_nxt = newtb(etype, name, line, page);
			last->tb_nxt->tb_nxt = cur;
			}
		return;
		}
		else
		{ /* link into list here */
		last->tb_nxt = newtb(etype, name, line, page);
		last->tb_nxt->tb_nxt = cur;
		return;
		}
}

struct tabent *
newtb(etype, name, line, page)
int etype;
char *name;
int line, page;
{
	register struct tabent * here;

	here = (struct tabent *)malloc(sizeof(struct tabent));
	if(here == NULL)
		{
		fprintf(stderr,"Cref: out of memory\n");
		exit(2);
		}
	here->tb_nxt = NULL;
	here->tb_prnt = malloc(strlen(name)+1);
	if(here->tb_prnt == NULL)
		{
		fprintf(stderr,"Cref: out of memory\n");
		exit(3);
		}
	strcpy(here->tb_prnt, name);
	here->tb_ent = (struct ref *)malloc(sizeof(struct ref));
	if(here->tb_ent == NULL)
		{
		fprintf(stderr,"Cref: out of memory\n");
		exit(4);
		}
	here->tb_ent->rf_nxt = NULL;
	here->tb_ent->line = line;
	here->tb_ent->page = page;
	here->tb_ent->rflgs = flgtab[etype-EBASE];
	return(here);
}

addrf(here, etype, line, page)
register struct tabent * here;
int etype, line, page;
{
	register struct ref *now;

	now = here->tb_ent;
	if(now == NULL) /* first entry for this name */
		{
		now = (struct ref *)malloc(sizeof(struct ref));
		now->line = line;
		now->page = page;
		now->rf_nxt = NULL;
		return;
		}
	while( now->rf_nxt != NULL )
		now = now->rf_nxt;
	if( (now->page == page) && (now->line == line) )
		now->rflgs |= flgtab[etype-EBASE];
		else
		{
		now->rf_nxt = (struct ref *)malloc(sizeof(struct ref));
		if(now->rf_nxt == NULL)
			{
			fprintf(stderr,"Cref: out of memory\n");
			exit(5);
			}
		now = now->rf_nxt;
		now->rf_nxt = NULL;
		now->page = page;
		now->line = line;
		now->rflgs = flgtab[etype-EBASE];
		}
}

struct tabent *
initab()
{
	register struct tabent *here;

	here = (struct tabent *)malloc(sizeof(struct tabent));
	if(here == NULL)
		{
		fprintf(stderr,"Cref: out of memory\n");
		exit(6);
		}
	here->tb_nxt = NULL;
	here->tb_prnt = NULL;
	here->tb_ent = NULL;
	return(here);
}

prntrf(head)
struct tabent *head;
{
	register struct tabent *here;

	here = head;
	fprintf(stdout,HDRFMT,hdchar,pageno);
	lincnt += 2;
	while(here != NULL)
		{
		outent(here);
		here = here->tb_nxt;
		}
}

outent(here)
struct tabent *here;
{
	char entbuf[ENTMAX];
	char linbuf[LINMAX+1];
	register int col = 0;
	register int entln;
	register struct ref * now;

	zerout(linbuf, LINMAX+1);
	sprintf(linbuf,"%-7s ",here->tb_prnt);
	col = 9;
	now = here->tb_ent;
	while(now != NULL)
		{
		zerout(entbuf, ENTMAX);
		sprintf(entbuf,linfmt,now->page,now->line);
		if(now->rflgs > FLGMAX)
			{
			fprintf(stderr,"Cref: flag error\n");
			now->rflgs = 0;
			}
		strcat(entbuf, flgstr[now->rflgs]);
		strcat(entbuf, " ");
		entln = strlen(entbuf);
		if(entln + col > LINMAX)
			{
			fprintf(stdout,"%s\n",linbuf);
			zerout(linbuf, LINMAX+1);
			strcpy(linbuf, "        ");
			strncat(linbuf, entbuf);
			col = 9 + entln;
			lincnt += 1;
			if(lincnt >= MAXLIN)
				{
				pageno += 1;
				fprintf(stdout,HDRFMT,hdchar,pageno);
				lincnt = 2;
				}
			}
			else
			{
			strncat(linbuf, entbuf);
			col += entln;
			}
		now = now->rf_nxt;
		}
	fprintf(stdout,"%s\n",linbuf);
	lincnt += 1;
	if(lincnt >= MAXLIN)
		{
		pageno += 1;
		fprintf(stdout,HDRFMT,hdchar,pageno);
		lincnt = 2;
		}
}

zerout(buf, siz)
register char * buf;
register int siz;
{
	do { *buf++ = '\0';}
		while(--siz);
}


/* for MACRO's idea of alphabetization */
int alptab[256] = {
	0000, 0401, 0402, 0403, 0404, 0405, 0406, 0407,
	0410, 0411, 0412, 0413, 0414, 0415, 0416, 0417,
	0420, 0421, 0422, 0423, 0424, 0425, 0426, 0427,
	0430, 0431, 0432, 0433, 0434, 0435, 0436, 0437,
	0440, 0441, 0442, 0443, 0444, 0445, 0446, 0447,
	0450, 0451, 0452, 0453, 0454, 0455, 0456, 0457,
	0460, 0461, 0462, 0463, 0464, 0465, 0466, 0467,
	0470, 0471, 0472, 0473, 0474, 0475, 0476, 0477,
	0500, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
	0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
	0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127,
	0130, 0131, 0132, 0533, 0534, 0535, 0536, 0537,
	0540, 0141, 0142, 0143, 0144, 0145, 0146, 0147,
	0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157,
	0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167,
	0170, 0171, 0172, 0573, 0574, 0575, 0576, 0577,
	0600, 0601, 0602, 0603, 0604, 0605, 0606, 0607,
	0610, 0611, 0612, 0613, 0614, 0615, 0616, 0617,
	0620, 0621, 0622, 0623, 0624, 0625, 0626, 0627,
	0630, 0631, 0632, 0633, 0634, 0635, 0636, 0637,
	0640, 0641, 0642, 0643, 0644, 0645, 0646, 0647,
	0650, 0651, 0652, 0653, 0654, 0655, 0656, 0657,
	0660, 0661, 0662, 0663, 0664, 0665, 0666, 0667,
	0670, 0671, 0672, 0673, 0674, 0675, 0676, 0677,
	0700, 0701, 0702, 0703, 0704, 0705, 0706, 0707,
	0710, 0711, 0712, 0713, 0714, 0715, 0716, 0717,
	0720, 0721, 0722, 0723, 0724, 0725, 0726, 0727,
	0730, 0731, 0732, 0733, 0734, 0735, 0736, 0737,
	0740, 0741, 0742, 0743, 0744, 0745, 0746, 0747,
	0750, 0751, 0752, 0753, 0754, 0755, 0756, 0757,
	0760, 0761, 0762, 0763, 0764, 0765, 0766, 0767,
	0770, 0771, 0772, 0773, 0774, 0775, 0776, 0777 
	};

cmpstr(s1, s2)
register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++=='\0')
			return(0);
	s2--;
	return(alptab[*s1 & 0377] - alptab[*s2 & 0377]);
}
