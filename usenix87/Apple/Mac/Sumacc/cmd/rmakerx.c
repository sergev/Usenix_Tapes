#define DEBUG 1
/*
 * Resource decompiler.
 */

/*
 * Copyright (C) 1984, Ben Hyde.  [intmet@bbna]
 * All rights reserved by Ben Hyde, but full data rights
 * transfered to SUMEX project 
 * May be used but not sold without permission.
 */

/*
 * history
 * 08/20/84	Hyde	Created, derived, loosely, from rmaker.c
 *	The main goal of this beasty is to allow dialog templates
 *	created on the Mac to be brought under configuration control
 *	on the mainframe.  It also works on icons, it shouldn't be hard
 *	to add any other things you have the documentation for.  Read about
 *	the format of resouce files in Inside Mac, it will make your brain hurt.
 *	Think about what byteswapper is doing down below.  Useful stuff is
 *	left in globals by the main iteration over the resources.
 *
 */

#include <stdio.h>
#include <mac/res.h>
#include <mac/b.out.h>
#include <mac/quickdraw.h>
#include <mac/toolintf.h>

#define bh      filhdr

struct  bhdr    bh;             /* b.out header */
char    *malloc();
char    *index();
char    *rindex();
unsigned short  htons();        /* host to "net" byte order, short. */
unsigned long   htonl();

struct  resfile rf;             /* resource file header */
#define MAPMAX 4000
char	resmap_data[MAPMAX];	/* we bring the resource map into core */
struct  resmap  *rm_base;	/* resource map header in resmap_data */
struct  restype *rt_base;	/* resource type list in resmap_data */
numtypes_t rt_size;		/* the number of entries following rt_base */
struct  resid  *ri_base;	/* resource type list in resmap_data */
short	ri_size;		/* the number of entries following ri_base */
char 	*rn_base;		/* name list in resmap_data */
#define RESDATAMAX 2000		/* maximum size of a resource be page in */
char	res_data[RESDATAMAX];	/* we bring some resources into this core */
char	*rd_base= &res_data[0];	/* Pointer to the base of the resource's data */
long	rd_size;

/* Some global pointers that the handlers use to get context */
struct restype *rtp;
struct resid   *rip;
char		*rdp;

char    debugtype[8];           /* debug type switch */
FILE    *fout;                  /* file for output */
FILE    *fin;                   /* file for commands */
char    *fin_name;		/* name of the input file */

# define NoAlias ""

/* type format handlers */
/*****
extern  handstr(), handhexa(), handcode(), handdrvr();
extern  handdlog(), handalrt(), handditl();
extern  handwind(), handmenu(), handcntl();
*****/
extern handditl(),handicnN();

struct  typehand {              /* type string to handler table */
        char    th_type[8];     /* e.g. "CODE" */
        int     (*th_handler)(); /* format handler function */
} typehand[] = {
        "DITL", handditl,
        "ICN#", handicnN,
/*****
        "STR ", handstr,
        "HEXA", handhexa,
        "DRVR", handdrvr,
        "ALRT", handalrt,
        "DLOG", handdlog,
        "WIND", handwind,
        "MENU", handmenu,
        "CNTL", handcntl,
        "ICON", handhexa,
        "CURS", handhexa,
        "PAT ", handhexa,
*****/
        0,      0
};

main(argc, argv)
        char **argv;
{

        if (argc != 3)
                abort("usage: rmakerx input.rsrc output.rc");
	fin_name = argv[1];
        if ((fin = fopen(argv[1], "r")) == NULL)
                abort("can't open resource file");
        if((fout = fopen(argv[2],"w")) == NULL)
	    abort("can't open out output file, i.e. the rmaker input file.");
	razerf();	/* raze the resource file given */
        rmakerx();	/* print the result from in core data structures */
        exit(0);
}

/*
 * Bring the maps in the resource file into core.
 */
razerf()
{
			/* Read in the file header */
    fread(&rf, sizeof rf, 1, fin);		/* read in the file header */
    byteswapper("llll",&rf);
# ifdef DEBUG
    printf(
    "file header: offsets data %lu, map %lu; lenght data  %lu, map len%lu\n",
					rf.rf_offdata, rf.rf_offmap,
					rf.rf_lendata, rf.rf_lenmap);
# endif

			/* Read in the Resource Map */
    if(rf.rf_lenmap>MAPMAX) abort("Resource map too large for rmakerx.");
    fseek(fin,rf.rf_offmap,0);
    fread(&resmap_data[0], sizeof(char), rf.rf_lenmap, fin);

			/* Set up our base pointers into that data */
				/* The map header is easy, it appears first */
    rm_base = (struct resmap *) &resmap_data[0];
    byteswapper("lllllssss",rm_base);
# ifdef DEBUG
    printf("map header: types:%lu, name:%lu\n",
		rm_base->rm_offtype, rm_base->rm_offname);
# endif

				/* Type list is an offset at an the header */
    rt_base = (struct restype *)    ((char *)rm_base 	/* offset is in bytes */
				  + (rm_base->rm_offtype)
				  + (sizeof(short))	/* skip # of entries */
				 );
    rt_size = *( (short *) (				/* get the size */
			      ((char *)rt_base)
			    - (sizeof(short))));	/* what is this lisp? */
    rt_size = htons(rt_size)+1;				/* its off by one. */
    {    					/* clean up byte orderings */
         struct restype *p = rt_base;
	 ri_size = 0;				/* also count # of resources */
	 for(;p<rt_base+rt_size;p++){
	     byteswapper("bbbbss",p);
# ifdef DEBUG
	     printf("type '%4.4s' %d, offids:%lu\n"
		,p->rt_type,1+p->rt_numids,p->rt_offids);
	     ri_size += 1 + p->rt_numids;	
# endif
	 }
    }

				/* The reference list */
    ri_base = (struct resid *) (rt_base + rt_size);
    {						/* clean up byte order */
	struct resid * p = ri_base;
	for(; p < (ri_base + ri_size); p++){
		byteswapper("ssbbsss",p);
# ifdef DEBUG
		printf("ri id:%d, nameoff: %d, att:0%2.2x\n",
			p->ri_id, p->ri_offname, p->ri_att);
# endif
	}
    }

				/* The name list */
    rn_base = (char *) rm_base + (rm_base->rm_offname) ; 
    {				/* Convert Pascal names to C names */
	struct resid * p = ri_base;
	for(; p < (ri_base + ri_size); p++){
	    if(p->ri_offname >= 0)
		my_p2cstr(&rn_base[p->ri_offname]);
	}
    }

# ifdef DEBUG
    {   struct resid * p = ri_base;
	for(; p < (ri_base + ri_size);p++)
	    if(p->ri_offname >= 0)
		printf("'%s'\n",&rn_base[p->ri_offname]);
	    else
		printf("%s\n","NoName");
    }
# endif

}

rmakerx()
{
    register struct rescomp *rcpp;
    register struct typehand *thp;

    /* write some comments to tell who did this evil thing. */
    fprintf(fout,"* Generated from %s by rmakerx.\n",fin_name);
    /* write the usual first line */
    fprintf(fout,"* Output file name:\n%s\n",fin_name);

    /*
     * For each resource, use type id to select handler, and use it.
     */
    for(rtp=rt_base; rtp<(rt_base+rt_size); rtp++){ /* sweep over types */
	for(thp= &typehand[0]; ;thp++ ){
	    if (thp->th_handler == 0){
		fprintf(fout,"*\n");
		fprintf(fout,
		    "* rmakerx can not handle the type %4.4s\n", rtp->rt_type);
		fprintf(fout,"*\n");
		break;
	    }
	    if (b4cmp(thp->th_type, rtp->rt_type)){
		{   register struct resid *ids =	/* list pntr */
			(struct resid *)
			   (((char *)rt_base)		/* from rt_base */
			    -(sizeof(short))		/* from rt count */
			    +(rtp->rt_offids));		/* offset to ids */
		    register int i;			/* #-1 of this type */
		    for(i=rtp->rt_numids; i>=0; i--){
			rip = ids + i;
			(*thp->th_handler)();
		    }
		}
		break;
	    }
	}
    }
    fprintf(fout,"* rmakerx finished\n");
}

/*
 * Abort with message.
 */
abort(s,a)
        char *s;
{
        fprintf(stderr, "rmakerx: ");
        fprintf(stderr, s, 0);
        fprintf(stderr, "\n");
        exit(1);
}


/*
 * Convert a Pascal string into a C string, inplace.
 */
my_p2cstr(ps)
 char *ps;
{    register char *cs = ps;
     char *debug = ps;
     register int cnt = *ps++;
     while(cnt-->0) *cs++ = *ps++;
     *cs = '\00';
}

/*
 * Compare four bytes, see if they are equal
 */
b4cmp(b1,b2)
   char *b1,*b2;
{
	return
		*b1++ == *b2++ &&
		*b1++ == *b2++ &&
		*b1++ == *b2++ &&
		*b1++ == *b2++ ;
}
/*
 *  Byte swappers.
 */
unsigned short htons(a)
        unsigned short a;
{
        unsigned short result;
        register char *sp = (char *)&a;
        register char *dp = (char *)&result;

        dp[1] = *sp++;
        dp[0] = *sp;
        return (result);
}

unsigned long htonl(a)
        unsigned long a;
{
        unsigned long result;
        register char *sp = (char *)&a;
        register char *dp = (char *)&result;

        dp[3] = *sp++;
        dp[2] = *sp++;
        dp[1] = *sp++;
        dp[0] = *sp;
        return (result);
}

/*
 * Walk thru a struct doing the byte swapping on each field.
 * l   is a string it gives the field layout of
 *     the data structure.  For example "bbsl" describes a four field
 *     data structure consisting of a two byte fields, followed by one
 *     short (2byte) field, and one long (4 byte) field.
 * a   is the the data structure, it is swapped in place.
 */
byteswapper(l,a)
  char *l;
  char *a;
{
	register char *lp = l;	/* for sweeping over layout string */
	register char *ap = a;  /* for sweeping over the data structure */

	for(;*lp!='\00';lp++){	/* sweep over layouts */
	    switch(*lp){
	    case 'b':
		ap += 1;
		break;
	    case 's':
		*((unsigned short *)ap) = htons(*(unsigned short *)ap);
		ap += 2;
		break;
	    case 'l':
		*((unsigned long  *)ap) = htonl(*(unsigned long  *)ap);
		ap += 4;
		break;
	    otherwise: abort("evil layout passed to byteswapper.");
	    }
	}
}

/*
 * Misc utilities use by the type handlers.
 */
 doheader(alias)
     char *alias;
 {
    fprintf(fout,"\nType %4.4s%s\n",rtp->rt_type,alias);
     if(    rip->ri_offname>=0
        && '\00'!=*(rn_base+rip->ri_offname))	/* Desk ornaments are funny */
	fprintf(fout,"     |%s,%d(%d)\n"
			,rn_base+rip->ri_offname
			,rip->ri_id
			,rip->ri_att);
     else
	fprintf(fout,"     ,%d(%d)\n",rip->ri_id,rip->ri_att);
}

resread()
{
	register long offset = (rip->ri_offdatahi<<16) + rip->ri_offdata;
	fseek(fin,rf.rf_offdata+offset,0);
	fread(&rd_size,sizeof(long),1,fin);
	rd_size=htonl(rd_size);
	if(rd_size>RESDATAMAX) abort("Resource's data is too large.");
	fread(rd_base,1,rd_size,fin);
}

/*
 * Emit the stuff for various fields found in a data part of a resource
 */

emitstr()
{
	my_p2cstr(rdp);
	fprintf(fout,"%s\n",rdp);
}

emitresid() /* resource id */
{
    register int len;
    len = rdp[0];
    if(len!=2)
	abort(" expected resource id number, but length was wrong");
    /* len = htons(*(short *)&rdp[1]);
    fprintf(fout,"  %d\n",len);
    */
}


/*
 * Type Handler
 */

/*
 * Dialog item list
 */
 struct aDitlHandler {
	char val;	/* of this value */
	char *name;	/* emit this name */
	ProcPtr handler;	/* call this handler if nonzero */
	} ditlhandlers[] = {
	{ctrlItem+btnCtrl,	"BtnItem",	emitstr },
	{ctrlItem+chkCtrl,	"ChkItem",	emitstr },
	{ctrlItem+radCtrl,	"RadioItem",	emitstr },
	{ctrlItem+resCtrl,	"ResItem",	emitresid },
	{	 statText,	"StatText",	emitstr },
	{	 editText,	"EditText",	emitstr },
	{	 iconItem,	"IconItem",	emitresid },
	{	  picItem,	"PicItem",	emitresid },
	{	 userItem,	"UserItem",	0 },
	{ 0,0,0 }};

handditl()
{
    short items;
    Rect * rect;
    char type;
    int itemlen;
    struct aDitlHandler *dhp;

    doheader(NoAlias);
    resread();
    rdp = rd_base;
    items = htons(*((short *)rdp)); rdp += 2;
    fprintf(fout,"  * number of items in dialog item list\n  %d\n",items+1);
    for(;items>=0;items--){
	dhp = 0;
	rdp += 4;		/* skip over place holder for handle */
	rect = (Rect *)rdp;	/* save bound rectangle for later */
	rdp += 8;		/* skip over it */
	type = *rdp++;
	itemlen = *rdp;		/* level pointer on size */

	{	/* print the name of the control */
	    register struct aDitlHandler *i;
	    fprintf(fout,"   ");
	    for(i= &ditlhandlers[0]; i->name!=0; i++){
		if( ( 0177 & type) == i->val){	
		    fprintf(fout, " %s", i->name);
		    if(i->handler!=0)
			dhp = i;
		}
	    }
	    if((itemDisable&type) == itemDisable)
		fprintf(fout," Disabled\n");
	    else
		fprintf(fout," Enabled\n");
	}

	/* Now do the bounding rectangle */
	byteswapper("ssss",rect);
	fprintf(fout,"    * Bounding Rectangle i.e. tlbr\n");
	fprintf(fout,"    %d %d %d %d\n",
	    rect->top,rect->left,rect->bottom,rect->right);

	/* Call the handler for the item list member */
	if(dhp==0)
	    fprintf(fout," No handler\n");
        else
	    (* dhp->handler)();

	rdp += itemlen+1;
	fprintf(fout,"\n");	/* leave blank line at base of item or items */
    }


}

/*
 * ICN# item handler
 */
handicnN()
{
    register char * rdp;
    register int size;
    register long row;
    register int i,beat;
    char line[33];
    doheader(" = HEXA");
    resread();
    /*  emit the icon and mask */
    for( size = rd_size, rdp = rd_base, beat = 7;
	 size>0;
	 size -=4, rdp +=4, beat--){
	row = htonl(*(long *)rdp);
        fprintf(fout," %8.8x ",row);
	if(beat <= 0){
	    beat = 8;
	    fprintf(fout,"\n");
	}
    }

    /* emit a picture of the icon */
    for( size = rd_size, rdp = rd_base;
	 size>0;
	 size -=4, rdp +=4){
	row = htonl(*(long *)rdp);
	/* shift bits out of row, and build line of text backward :-) */
	for(i=31;i >= 0;i--,row>>=1)
	    line[i] = ( ( (01 & row)!=0) ? '@' : '_');
	line[32]='\00';
	fprintf(fout,"* %s\n",&line[0]);
    }
}
