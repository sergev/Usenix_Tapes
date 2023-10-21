#

/*
 *	syms.c
 */

#define	STABENT	12
#define	NTYPES	12

/*	syms:  Lists symbols in an object file or archive, sorted by type
 *		e.g. local/global/common and text/data bss.  This program
 *		is similar to "nm", but can accept archives as well as
 *		single object files.  It also has two extra flags (See
 *		"d" and "g" below.
 *
 *	usage:	syms [-cgudnr] filename ...
 *
 *	flags:	c:  list c-type globals only (those that begin with underscore
 *		g:  list global symbols only
 *		u:  list undefined symbols only
 *		d:  list defined symbols only
 *		n:  sort symbols alphabetically by name, rather than by value
 *		r:  sort in descending, rather than ascending order
 */

/*
 *	Modified by Bill Shannon   07/20/78
 *
 *	Fixed for new archive file format.
 *	Bugfix: close each file before processing next.
 */

char t_code[] {00,01,02,03,04,040,041,042,043,044,037};
char *t_tag[] {	"Undefined Local",
		"Absolute Local",
		"Text Local",
		"Data Local",
		"Bss Local",
		"Undefined Global",
		"Absolute Global",
		"Text Global",
		"Data Global",
		"Bss Global",
		"Ld File",
		"Special",
		"Common"
		};
char *errmsg[] {"Bad flag ignored",
		"Unable to open",
		"Read error or bad length",
		"Not an object module or archive",
		"Symbol segment length incorrect"
		};
int fildes, buffer[8];
int c_only	0;
int by_name	0;
int reverse	1;
int u_only	0;
int g_only	0;
int d_only	0;
struct {char a_name[14];};
struct node {
	char name[7];
	struct node *next;
	char *value;
	};

main(argc,argv)
int argc;
char *argv[];
{	register int i,j;
	char *ctime(), c;
	for (i=1; i<argc; i++)
	{	if (argv[i][0] == '-')
		{	for (j=1; (c = argv[i][j++]) != '\0' ; )
			{	switch(c)
				{	case 'c': c_only++;
					case 'g': g_only++; break;
					case 'u': u_only++; break;
					case 'n': by_name++; break;
					case 'r': reverse = -1; break;
					case 'd': d_only = 1; break;
					default:  printf(errmsg[0]);
				}
			}
		} 
		else if ((fildes=open(argv[i],0)) < 0) error(1);
		else if (read(fildes,buffer,2) != 2) error(2);
		else if (buffer[0] == 0177545)
		{	printf("\n\n\nArchive file %s",argv[i]);
			while (c = read(fildes,buffer,26))
			{	if (c != 26) {error(2); break;}
				printf("\n  Member: %-14.14s\n",
					buffer->a_name);
				if (read(fildes,buffer,2) != 2) error(2);
				else proces1();
			}
			close(fildes);
		}
		else
		{	printf ("\n\n\nFile %s\n",argv[i]);
			proces1();
			close(fildes);
		}
	}
}

proces1()
{	struct {int integ;};
	int j, type, itype;
	struct node stype[NTYPES+1], proto;
	register int i;
	struct node *alloc();
	register struct node *p, *q;
	if (buffer[0] == 0407) printf("Non-sharable\n");
		else if (buffer[0] == 0410) printf ("Sharable\n");
		else if (buffer[0] == 0411) printf ("Split I/D\n");
		else {error(3); return;}
	if (read(fildes,buffer,14) != 14) {error(2); return;}
	printf ("  Text: %-8lData: %-8lBss: %-8l (decimal bytes)\n",
		buffer[0], buffer[1], buffer[2]);
	printf ("  Text: %-8oData: %-8oBss: %-8o (octal bytes)\n",
		buffer[0], buffer[1], buffer[2]);
	for (i = (buffer[6] ? 1 : 2); i--; )
		if (bigseek(fildes, buffer[0]) == -1 ||
		    bigseek(fildes, buffer[1]) == -1)
		{error(2); return;}
	if (buffer[3]%STABENT) {error(4); return;}
	for (i=0; i<=NTYPES; i++)
		stype[i].next = 0;
	for (i = buffer[3]; (i =- STABENT) >= 0; )
	{	p = alloc(sizeof proto);
		if (read(fildes,p,STABENT) != STABENT)
			{error(2); return;}
		type = (p->next).integ;
		for (itype=0; type != t_code[itype] &&
			++itype < 11; );
		if (itype==5 && (p->value != 0)) itype = NTYPES; /* Common */
		if (	(c_only && (p->name[0] != '_'))
		      || (g_only && !(type&040))
		      || (d_only && ((itype == 0) || (itype == 5)
					|| (itype == NTYPES)))
		      || (u_only && (itype != 0) && (itype != 5)) ) continue;
		else sortins(p,&(stype[itype]));
	}
	/*  Print output and free memory */
	for (i=0; i <= NTYPES; i++)
	{	if (p = stype[i].next)
		{	printf("\n%s Symbols...",t_tag[i]);
			do
			{	printf("\n  ");
				for (j=4; j--; )
				{	printf(" %8.8s %-6o",p->name,p->value);
					q = p->next;
					free(p);
					if ((p = q) == 0) break;
				}
			} while (p);
		}
	}
	putchar('\n');
}

sortins(p,q)
struct node *p, *q;
{	struct node *r;
	while ( (r = q->next) &&
		(reverse * (by_name? strcmp(r->name, p->name)
				   : (r->value) >= (p->value) )) <= 0) q = r;
	p->next = r;
	q->next = p;
}

error(i)
int i;
{	printf("%s\n\n",errmsg[i]);
	seek(fildes,2,0); /* Skip rest of file */
	return;
}
strcmp(s1,t1)
char *s1, *t1;
{	register int i;
	register char *s, *t;
	s = s1; t = t1;
	for (i=0; i++<8; )
		if (*s < *t) return(-1);
		else if (*s++ > *t++) return (1);
	return (0);
}
bigseek(f, j)
int f, j;
{       register int i;
	while (j)
	{       i =  (j&0100000 ? 077777 : j);
		if (seek(f, i, 1) == -1) return(-1);
		j =- i;
	}
	return(0);
}
