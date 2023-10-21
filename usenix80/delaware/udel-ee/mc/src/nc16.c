#include "nc1h.c"
cat(p1)
{
 int *argp;
 register int i,j,k;
 static char outline[512];
 char *outp;
 char *stringp;
 extern char *code[];

 argp = &p1;
 outp = outline;

 while(*argp)
    {
    stringp = code[*argp++ - 1];

    while( *outp++ = *stringp++);
    outp--;
    }

 return(outline);
}



/*	This routine will generate code to load the index register
	if it needs loading, otherwise, it will do nothing.
 */

loadx()
{
 if (xvalid != 0) return;
 printf(cat(145,0));
 xvalid = 1;
}
/*
	This routine will check the type of the leaf to be operated
	on, and return a code which describes it.  The codes returned are:

		CONCHR:		constant char
		STKCHR:		stack char
		AUTOCHR0:	auto char (> 255 past index reg)
		AUTOCHR1:	auto char (< 255 past index reg)
		EXTCHR:		extern char
		STATCHR:	static char
		UNKCHR:		unknown char
		CONINT:		constant int
		STKINT:		stack int
		AUTOINT0:	auto int (> 255 past index reg)
		AUTOINT1:	auto int (< 255 past index reg)
		EXTINT:		extern int
		STATINT:	static int
		UNKINT:		unknown int
 */


pick(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 if (leaf->op != CON)
    {
    if (leaftype & STACK)
        {
         if ((leaftype & TYPEBITS) == CHAR)
	    {
	     return(STKCHR);
	    }

         return(STKINT);
        }

    if (leaftype & INDEX)
        {
         if ((leaftype & TYPEBITS) == CHAR)
	    {
	     return(INDXCHR);
	    }

         return(INDXINT);
        }
    if ((leaftype & TYPEBITS) == CHAR)
       {
        switch(leaf->class & 0377)
        {
         case OFFS:
		if ((leaf->type & FIXED) == 0)
		     leaf->nloc = leaf->offset; /* optimizer puts real address
						in offset  */
         case AUTO:
         case ARG:
	 /*  the following code adjusts argument references (once only) */
	 if (((leaf->type & FIXED) == 0) && (leaf->nloc > 0))
	    {
	     leaf->nloc =+ 3;
	     leaf->type =| FIXED;
	    }
    	 if ((leaf->nloc + autosize) > 255)
    	    {
	     return(AUTOCHR0);
    	    }
    	 else
    	    {
	     return(AUTOCHR1);
    	    }

         case EXTERN:
    	 return(EXTCHR);

         case STATIC:
    	 return(STATCHR);

         default:
	 return(UNKCHR);
        }
    }


    else
         switch(leaf->class & 0377)
         {
         case OFFS:
		if ((leaf->type & FIXED) == 0)
		     leaf->nloc = leaf->offset;
         case AUTO:
         case ARG:
	 if (((leaf->type & FIXED) == 0) && (leaf->nloc > 0))
	    {
	     leaf->nloc =+ 2;
	     leaf->type =| FIXED;
	    }
    	 if (leaf->nloc+autosize > 255)
	     return(AUTOINT0);
    	 else
	     return(AUTOINT1);

         case EXTERN:
    	 return(EXTINT);

         case STATIC:
    	 return(STATINT);

         default:
	 return(UNKINT);
         }

    }

 if (leaf->op == CON)
    {
     if ((leaftype & TYPEBITS) == CHAR)
	 return(CONCHR);
	 return(CONINT);
    }
}



test(op,type,leaf,leaftype,actype,test1,test2,test3,chartest)
struct tnode *leaf;
{
 switch(pick(op,type,leaf,leaftype,actype))
    {

     case CONCHR:
	 printf(cat(120,chartest,0),(leaf->value)&0377,tmplabel+1);
	 break;

     case STKCHR:
	 printf(cat(29,111,chartest,0),0,tmplabel+1);
	 xvalid = 0;
	 break;

     case INDXCHR:
	 printf(cat(154,111,chartest,0),0,tmplabel+1);
	 xvalid = 0;
	 break;

     case AUTOCHR0:
	 printf(cat(1,111,chartest,0),(leaf->nloc+autosize)&0377,
		((leaf->nloc+autosize)>>8)&0377,0,tmplabel+1);
	 xvalid = 0;
	 break;

     case AUTOCHR1:
	 loadx();
	 printf(cat(111,chartest,0),leaf->nloc+autosize,tmplabel+1);
	 break;

     case EXTCHR:
	 printf(cat(112,chartest,0),&leaf->nloc,leaf->offset,tmplabel+1);
	 break;

     case STATCHR:
	 printf(cat(113,chartest,0),leaf->nloc);
	 break;

     case UNKCHR:
	 error("unknown class op=%s, type=CHAR, class=%d",opntab[op],
			leaf->class);
	 abort();

     case CONINT:
	 printf(cat(121,test1,test2,120,test3,0),(leaf->value)&0377,tmplabel+1,
		tmplabel,(leaf->value>>8)&0377,tmplabel+1);
	 break;

     case STKINT:
	 printf(cat(29,114,test1,test2,115,test3,0),0,tmplabel+1,
		tmplabel,tmplabel+1);
	 xvalid = 0;
	 break;

     case INDXINT:
	 printf(cat(154,114,test1,test2,115,test3,0),0,tmplabel+1,
		tmplabel,tmplabel+1);
	 xvalid = 0;
	 break;

     case AUTOINT0:
	 printf(cat(1,114,test1,test2,115,test3,0),(leaf->nloc+autosize)&0377,
		((leaf->nloc+autosize)>>8)&0377,0,tmplabel+1,tmplabel,tmplabel+1);
	 xvalid = 0;
	 break;

     case AUTOINT1:
	 loadx();
	 printf(cat(114,test1,test2,111,test3,0),leaf->nloc+autosize,
		tmplabel+1,tmplabel,leaf->nloc+autosize+1,tmplabel+1);
	 break;

     case EXTINT:
	 printf(cat(117,test1,test2,116,test3,0),&leaf->nloc,leaf->offset,tmplabel+1,
		tmplabel,&leaf->nloc,leaf->offset+1,tmplabel+1);
	 break;

     case STATINT:
	 printf(cat(119,test1,test2,118,test3,0),leaf->nloc,tmplabel+1,
		tmplabel,leaf->nloc,tmplabel+1);
	 break;

     case UNKINT:
	 error("unknown class op=%s, type=%d, class=%d",opntab[op],
			leaftype,leaf->class);
	 abort();

     default:
	 error("unknown pick, op=%s, pick=%d",opntab[op],
			pick(op,type,leaf,leaftype,actype));
	 abort();
    }

 printf(cat(87,15,89,88,87,37,4,87,0),tmplabel,0377,tmplabel+2,tmplabel+1,
			tmplabel+2);
 tmplabel =+ 3;
}
