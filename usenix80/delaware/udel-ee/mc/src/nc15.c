#include "nc1h.c"



/*
	This file contains all of the operator level code generation routines.
	Each operator in the intermediate language has a routine that will
	generate code for it.  The individual routines generate the code based
	on the types and storage classes of the operands.  The "pick" routine
	does much of the type analysis.
	All entry to these routines is through the "gen" routine.  All code
	is actually produced through the "printf" routine.  The "cat" routine
	catenates character strings from the code table into one long string
	that can be processed by printf.
 */

AMPER1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op AMPER\n");
 abort();
}



AND1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 switch(pick(op,type,leaf,leaftype,actype))
    {
     case STKCHR:
	 printf(cat(29,51,0));
	 xvalid = 0;
	 return;

     case INDXCHR:
	 printf(cat(154,51,0));
	 xvalid = 0;
	 return;

     case AUTOCHR0:
	 printf(cat(1,51,0),((leaf->nloc+autosize) & 0377),
			(((leaf->nloc+autosize)>>8)&0377));
	 xvalid = 0;
	 return;

     case AUTOCHR1:
	 loadx();
	 printf(cat(52,0),leaf->nloc + autosize);
	 return;

     case EXTCHR:
	 printf(cat(53,0),&leaf->nloc,leaf->offset);
	 return;

     case STATCHR:
	 printf(cat(54,0),leaf->nloc);
	 return;

     case UNKCHR:
	 error("unknown class op=AND,type=CHAR,class=%d",leaf->class);
	 abort();
	 exit(1);

     case CONCHR:
     case CONINT:
	 printf(cat(109,110,0),leaf->value&0377,(leaf->value >> 8) & 0377);
	 break;
     case STKINT:
	 printf(cat(29,55,56,0));
	 xvalid = 0;
	 break;

     case INDXINT:
	 printf(cat(154,55,56,0));
	 xvalid = 0;
	 break;

     case AUTOINT0:
	 printf(cat(1,55,56,0),(leaf->nloc+autosize)&0377,
			((leaf->nloc+autosize)>>8)&0377);
	 xvalid = 0;
	 break;

     case AUTOINT1:
	 loadx();
	 printf(cat(52,57,0),leaf->nloc+autosize+1,
			leaf->nloc+autosize);
	 break;

     case EXTINT:
	 printf(cat(58,59,0),&leaf->nloc,leaf->offset+1,&leaf->nloc,leaf->offset);
	 break;

     case STATINT:
	 printf(cat(60,61,0),leaf->nloc,leaf->nloc);
	 break;

     case UNKINT:
	 error("unknown class op=AND,type=%d,class=%d",leaftype,
			leaf->class);
	 abort();
	 exit(1);

     default:
	 error("unknown pick, op=AND pick=%d",pick(op,type,leaf,leaftype,actype));
	 abort();
    }

 if ((type & TYPEBITS) == CHAR)
      printf(cat(4,0));

}



ARROW1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op ARROW");
 abort();
}



ASDIV1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 DIVIDE1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASLSH1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 LSHIFT1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASMINUS1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 MINUS1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASMOD1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 MOD1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASOR1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 OR1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASPLUS1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 PLUS1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASRSH1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 RSHIFT1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASSAND1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 AND1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASSIGN1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 if (leaf->type & INDEX)
    {
     printf(cat(154,0));
     if ((leaftype & 07) == CHAR)
	 printf(cat(18,4,0),0);
     else
	 printf(cat(18,23,0),1,0);
     xvalid = 0;
     return;
    }

 switch(pick(op,type,leaf,leaftype,actype))
    {
     case STKCHR:
	 printf(cat(29,17,0));
	 xvalid = 0;
	 return;

     case INDXCHR:
	 printf(cat(154,17,0));
	 xvalid = 0;
	 return;

     case AUTOCHR0:
	 printf(cat(1,17,4,0),((leaf->nloc+autosize) & 0377),
			(((leaf->nloc+autosize)>>8)&0377));
	 xvalid = 0;
	 return;

     case AUTOCHR1:
	 loadx();
	 printf(cat(18,4,0),leaf->nloc + autosize);
	 return;

     case EXTCHR:
	 printf(cat(19,4,0),&leaf->nloc,leaf->offset);
	 return;

     case STATCHR:
	 printf(cat(20,4,0),leaf->nloc);
	 return;

     case UNKCHR:
	 error("unknown class op=ASSIGN,type=CHAR,class=%d",leaf->class);
	 abort();
	 exit(1);

     case STKINT:
	 printf(cat(29,21,22,0));
	 xvalid = 0;
	 break;

     case INDXINT:
	 printf(cat(154,21,22,0));
	 xvalid = 0;
	 break;

     case AUTOINT0:
	 printf(cat(1,21,22,0),(leaf->nloc+autosize)&0377,
			((leaf->nloc+autosize)>>8)&0377);
	 xvalid = 0;
	 break;

     case AUTOINT1:
	 loadx();
	 printf(cat(18,23,0),leaf->nloc+autosize+1,
			leaf->nloc+autosize);
	 break;

     case EXTINT:
	 printf(cat(24,25,0),&leaf->nloc,leaf->offset+1,&leaf->nloc,leaf->offset);
	 break;

     case STATINT:
	 printf(cat(26,27,0),leaf->nloc,leaf->nloc);
	 break;

     case UNKINT:
	 error("unknown class op=ASSIGN,type=%d,class=%d",leaftype,
			leaf->class);
	 abort();
	 exit(1);

     default:
	 error("unknown pick op=ASSIGN pick=%d",pick(op,type,leaf,leaftype,actype));
	 abort;
    }

 if ((type & TYPEBITS) == CHAR)
      printf(cat(4,0));
}



ASSNAND1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 NAND1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASTIMES1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 TIMES1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



ASXOR1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 EXOR1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



AUTOD1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op AUTOD");
 abort();
}



AUTOI1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op AUTOI");
 abort();
}



CAL1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 register int i;

 printf(cat(139,140,0));
 depth =+ 2;
 if (leaftype == PTR)
	{
	 error("function pointers not yet implemented\n");
	 abort();
	 exit(1);
	}
 
 if (leaftype != FUNC)
	{
	 error("bad type op=CALL\n");
	 abort();
	 exit(1);
	}
 
 printf(cat(141,0),&leaf->nloc);
 while(depth-- > 0) printf(cat(144,0));
 depth = 0;
}



CALL11(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op CALL1");
 abort();
}



CALL21(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op CALL21");
 abort();
}



CBRANCH1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 printf(cat(33,0));
 if (leaf->cond)
     printf(cat(86,0),tmplabel);
 else
     printf(cat(85,0),tmplabel);

 printf(cat(93,87,0),leaf->lbl,tmplabel++);
}



COLON1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 diag("warning: colon not implemented yet\n");
}



COMMA1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 printf(cat(139,140,0));
 depth =+ 2;
}



COMPL1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

	printf(cat(31,0));
	if ((actype & TYPEBITS) != CHAR)
	    printf(cat(32,0));
}



DECAFT1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 printf(cat(90,91,0));
 MINUS1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
 printf(cat(133,134,0));
}



DECBEF1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 MINUS1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



DIVIDE1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 divused = 1;
 PUSH1(0,0,0,0,INT);
 if (leaftype & STACK)
    {
     printf(cat(37,151,144,144,0),"div");
     return;
    }

 LOADAC1(op,type,leaf,leaftype,actype);
 PUSH1(0,0,0,0,INT);
 printf(cat(15,151,144,144,144,144,0),1,"div");
}



EQUAL1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,85,122,85,85);
}



EXCLA1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
	printf(cat(33,0));
	if ((actype & TYPEBITS) != CHAR)
	    {
	    printf(cat(85,35,85,31,32,88,87,37,4,87,0),tmplabel,tmplabel,
			tmplabel+1,tmplabel,tmplabel+1);
	    tmplabel =+ 2;
	    return;
	    }

	printf(cat(85,31,88,87,37,87,0),tmplabel,tmplabel+1,tmplabel,tmplabel+1);
	tmplabel =+ 2;
}



EXOR1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 switch(pick(op,type,leaf,leaftype,actype))
    {
     case STKCHR:
	 printf(cat(29,73,0));
	 xvalid = 0;
	 return;

     case INDXCHR:
	 printf(cat(154,73,0));
	 xvalid = 0;
	 return;

     case AUTOCHR0:
	 printf(cat(1,73,4,0),((leaf->nloc+autosize) & 0377),
			(((leaf->nloc+autosize)>>8)&0377));
	 xvalid = 0;
	 return;

     case AUTOCHR1:
	 loadx();
	 printf(cat(74,4,0),leaf->nloc + autosize);
	 return;

     case EXTCHR:
	 printf(cat(75,4,0),&leaf->nloc,leaf->offset);
	 return;

     case STATCHR:
	 printf(cat(76,4,0),leaf->nloc);
	 return;

     case UNKCHR:
	 error("unknown class op=EXOR,type=CHAR,class=%d",leaf->class);
	 abort();
	 exit(1);

     case CONCHR:
     case CONINT:
	 printf(cat(107,108,0),leaf->value&0377,(leaf->value >> 8)&0377);
	 break;

     case STKINT:
	 printf(cat(29,77,78,0));
	 xvalid = 0;
	 break;

     case INDXINT:
	 printf(cat(154,77,78,0));
	 xvalid = 0;
	 break;

     case AUTOINT0:
	 printf(cat(1,77,78,0),(leaf->nloc+autosize)&0377,
			((leaf->nloc+autosize)>>8)&0377);
	 xvalid = 0;
	 break;

     case AUTOINT1:
	 loadx();
	 printf(cat(74,79,0),leaf->nloc+autosize+1,
			leaf->nloc+autosize);
	 break;

    case EXTINT:
	 printf(cat(80,81,0),&leaf->nloc,leaf->offset+1,&leaf->nloc,leaf->offset);
	 break;

     case STATINT:
	 printf(cat(82,83,0),leaf->nloc,leaf->nloc);
	 break;

     case UNKINT:
	 error("unknown class op=EXOR,type=%d,class=%d",leaftype,
			leaf->class);
	 abort();
	 exit(1);

     default:
	 error("unknown pick op=EXOR pick=%d",pick(op,type,leaf,leaftype,actype));
	 abort();
    }

 if ((type & TYPEBITS) == CHAR)
      printf(cat(4,0));
}



FSEL1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op FSEL");
 abort();
}



GREAT1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,124,123,129,130);
}



GREATEQ1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,124,123,126,124);
}



GREATP1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,126,125,129,130);
}



GREATQP1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,126,125,126,124);
}



INCAFT1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 printf(cat(90,91,0));
 PLUS1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
 printf(cat(131,132,0));
}



INCBEF1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 PLUS1(op,type,leaf,leaftype,actype);
 ASSIGN1(op,type,leaf,leaftype,actype);
}



INIT1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 if (leaf->op == NAME)
    {
     switch(leaf->class)
	{
	 case STATIC:
		printf(cat(159,0),leaf->nloc);
		return;

	 case EXTERN:
		printf(cat(160,0),&leaf->nloc,leaf->offset);
		return;

	 default:
		error("Illegal initialization, class=%d",leaf->class);
	}
    }

 if (leaf->op == CON)
    {
     printf(cat(161,0),leaf->value);
     return;
    }

 error("Illegal initialization");
}



JUMP1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op JUMP");
 abort();
}



LESS1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,123,124,128,127);
}



LESSEQ1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,123,124,125,123);
}



LESSEQP1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,125,126,125,123);
}



LESSP1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,125,126,128,127);
}



LOADAC1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 if (leaf->op != CON)
    {
    if (leaftype & STACK)
        {
         if ((leaftype & TYPEBITS) == CHAR)
	    {
	     printf(cat(94,4,149,0));  /* sign extend 0's */
	     return;
	    }

         printf(cat(95,94,149,149,0));
         return;
        }

    if (leaftype & INDEX)
	{
	 printf(cat(154,0));
	 xvalid = 0;

	 if ((leaftype & TYPEBITS) == CHAR)
	    {
	     printf(cat(157,0));
	     return;
	    }

	 printf(cat(155,156,0));
	 return;
	}

    if ((leaftype & TYPEBITS) == CHAR)
       {
        switch(leaf->class & 0377)
        {
         case OFFS:
		if ((leaf->type & FIXED) == 0)
		     leaf->nloc = leaf->offset;
         case ARG:
         case AUTO:
	 if (((leaf->type & FIXED) == 0) && (leaf->nloc > 0))
		{
		 leaf->type =| FIXED;
		 leaf->nloc =+ 3;
		}
    	 if ((leaf->nloc + autosize) > 255)
    	    {
    	     printf(cat(1,2,4,0),((leaf->nloc+autosize) & 0377),
    			(((leaf->nloc+autosize)>>8)&0377));
	     xvalid = 0;
    	     break;
    	    }
    	 else
    	    {
	     loadx();
    	     printf(cat(5,4,0),leaf->nloc + autosize);
    	     break;
    	    }

         case EXTERN:
    	 printf(cat(6,4,0),&leaf->nloc,leaf->offset);
    	 break;

         case STATIC:
    	 printf(cat(7,4,0),leaf->nloc);
    	 break;

         default:
	 error("unknown class op=LOADAC,type=CHAR,class=%d",leaf->class);
	 abort();
	 exit(1);
        }
    printf(cat(84,0));  /* sign extend 0's */
    }


    else
         switch(leaf->class & 0377)
         {
         case OFFS:
		if ((leaf->type & FIXED) == 0)
		     leaf->nloc = leaf->offset;
         case ARG:
         case AUTO:
	 if (((leaf->type & FIXED) == 0) && (leaf->nloc > 0))
		{
		 leaf->type =| FIXED;
		 leaf->nloc =+ 2;
		}
    	 if (leaf->nloc+autosize > 255)
	    {
    	     printf(cat(1,9,10,0),(leaf->nloc+autosize)&0377,
    		((leaf->nloc+autosize)>>8)&0377);
	     xvalid = 0;
	    }
    	 else
	    {
	     loadx();
    	     printf(cat(5,8,0),leaf->nloc+autosize+1,
    			leaf->nloc+autosize);
	    }
    	return;

         case EXTERN:
    	 printf(cat(11,12,0),&leaf->nloc,leaf->offset+1,&leaf->nloc,leaf->offset);
    	 return;

         case STATIC:
    	 printf(cat(13,14,0),leaf->nloc,leaf->nloc);
    	 return;

         default:
    	error("unknown class op=LOADAC,type=%d,class=%d",leaftype,
    						leaf->class);
    	abort();
    	exit(1);
         }

    }

 if (leaf->op == CON)
    {
     if ((leaftype & TYPEBITS) == CHAR)
         printf(cat(15,16,0),leaf->value,
    		((leaf->value >> 7) & 01) * 0377);
     else
         printf(cat(15,16,0),leaf->value & 0377,
    		(leaf->value >>8)&0377);
    }
    return;

}



LOADADD1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 switch(pick(op,type,leaf,leaftype,actype))
    {
     case AUTOCHR0:
     case AUTOCHR1:
     case AUTOINT0:
     case AUTOINT1:
	 printf(cat(15,16,147,148,0),((leaf->nloc+autosize)&0377),
			(((leaf->nloc+autosize)>>8)&0377));
	 return;

     case EXTCHR:
     case EXTINT:
	 printf(cat(13,14,88,137,87,0),tmplabel,tmplabel,tmplabel+1,tmplabel,
			&leaf->nloc,leaf->offset,tmplabel+1);
	 tmplabel =+ 2;
	 return;

     case STATCHR:
     case STATINT:
	 printf(cat(13,14,88,138,87,0),tmplabel,tmplabel,tmplabel+1,tmplabel,
			leaf->nloc,tmplabel+1);
	 tmplabel =+ 2;
	 return;

     case INDXCHR:
     case INDXINT:
	 printf(cat(155,156,0));
	 return;

     case UNKCHR:
	 error("unknown class op=LOADADD,type=CHAR,class=%d",leaf->class);
	 abort();
	 exit(1);

     case UNKINT:
	 error("unknown class op=LOADADD type=%d class=%d",leaftype,leaf->class);
	 abort();
	 exit(1);

     case CONCHR:
     case STKCHR:
     case CONINT:
     case STKINT:
	 error("code gen: lvalue required\n");
	 abort();
	 exit(1);
    }
}



LOGAND1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 printf(cat(33,85,35,86,87,0),tmplabel,tmplabel+1,tmplabel);

 LOADAC1(op,type,leaf,leaftype,actype);

 printf(cat(33,85,35,85,87,37,4,88,87,15,89,87,0),tmplabel+2,tmplabel+2,
		tmplabel+1,tmplabel+3,tmplabel+2,0377,tmplabel+3);

 tmplabel =+ 4;
}



LOGOR1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 printf(cat(33,85,35,85,0),tmplabel,tmplabel);

 LOADAC1(op,type,leaf,leaftype,actype);

 printf(cat(33,85,35,85,37,4,88,87,89,87,0),tmplabel,tmplabel,
		tmplabel+1,tmplabel,tmplabel+1);
 tmplabel =+ 2;
}



LSHIFT1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 if ((leaftype & TYPEBITS) == CHAR)
     printf(cat(96,0),7);
 else
     printf(cat(96,0),15);

 printf(cat(142,146,0));

 LOADAC1(op,type,leaf,leaftype,actype);

 printf(cat(3,158,87,0),tmplabel);
 xvalid = 0;

 printf(cat(103,86,0),tmplabel+1);
 printf(cat(98,0));
 if ((leaftype & TYPEBITS) != CHAR)
     printf(cat(99,0));

 printf(cat(88,87,0),tmplabel,tmplabel+1);
 tmplabel =+ 2;
}



LSTAR1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 printf(cat(152,153,0));
}



MCALL1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op MCALL");
 abort();
}



MINUS1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

	if ((actype & TYPEBITS) == CHAR)
	    printf(cat(38,0));
	else
	    printf(cat(32,31,104,39,0),1,0);

	PLUS1(op,type,leaf,leaftype,actype);
}



MOD1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 modused = 1;
 PUSH1(0,0,0,0,INT);
 if (leaftype & STACK)
    {
     printf(cat(37,151,144,144,0),"mod");
     return;
    }

 LOADAC1(op,type,leaf,leaftype,actype);
 PUSH1(0,0,0,0,INT);
 printf(cat(15,151,144,144,144,144,0),1,"mod");
}



NAND1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 COMPL1(op,type,leaf,leaftype,actype);
 AND1(op,type,leaf,leaftype,actype);

}



NEG1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

	if ((actype & TYPEBITS) == CHAR)
	    {
	    printf(cat(38,0));
	    return;
	    }

	printf(cat(32,31,104,39,0),1,0);
}



NEQUAL1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 test(op,type,leaf,leaftype,actype,122,85,86,86);
}



OR1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 switch(pick(op,type,leaf,leaftype,actype))
    {
     case STKCHR:
	 printf(cat(29,62,0));
	 xvalid = 0;
	 return;

     case INDXCHR:
	 printf(cat(154,62,0));
	 xvalid = 0;
	 return;

     case AUTOCHR0:
	 printf(cat(1,62,4,0),((leaf->nloc+autosize) & 0377),
			(((leaf->nloc+autosize)>>8)&0377));
	 xvalid = 0;
	 return;

     case AUTOCHR1:
	 loadx();
	 printf(cat(63,4,0),leaf->nloc + autosize);
	 return;

     case EXTCHR:
	 printf(cat(64,4,0),&leaf->nloc,leaf->offset);
	 return;

     case STATCHR:
	 printf(cat(65,4,0),leaf->nloc);
	 return;

     case UNKCHR:
	 error("unknown class op=OR,type=CHAR,class=%d",leaf->class);
	 abort();
	 exit(1);

     case CONCHR:
     case CONINT:
	 printf(cat(105,106,0),leaf->value&0377,(leaf->value >> 8)&0377);
	 break;

     case STKINT:
	 printf(cat(29,66,67,0));
	 xvalid = 0;
	 break;

     case INDXINT:
	 printf(cat(154,66,67,0));
	 xvalid = 0;
	 break;

     case AUTOINT0:
	 printf(cat(1,66,67,0),(leaf->nloc+autosize)&0377,
			((leaf->nloc+autosize)>>8)&0377);
	 xvalid = 0;
	 break;

     case AUTOINT1:
	 loadx();
	 printf(cat(63,68,0),leaf->nloc+autosize+1,
			leaf->nloc+autosize);
	 break;

     case EXTINT:
	 printf(cat(69,70,0),&leaf->nloc,leaf->offset+1,&leaf->nloc,leaf->offset);
	 break;

     case STATINT:
	 printf(cat(71,72,0),leaf->nloc,leaf->nloc);
	 break;

     case UNKINT:
	 error("unknown class op=OR,type=%d,class=%d",leaftype,
			leaf->class);
	 abort();
	 exit(1);

     default:
	 error("unknown pick op=OR pick=%d",pick(op,type,leaf,leaftype,actype));
    }

 if ((type & TYPEBITS) == CHAR)
      printf(cat(4,0));
}



PLUS1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

 switch(pick(op,type,leaf,leaftype,actype))
    {
    case STKCHR:
	printf(cat(29,40,0));
	xvalid = 0;
	return;

    case INDXCHR:
	printf(cat(154,40,0));
	xvalid = 0;
	return;

    case AUTOCHR0:
	printf(cat(1,40,39,0),((leaf->nloc+autosize) & 0377),
	        (((leaf->nloc+autosize)>>8)&0377),0);
	xvalid = 0;
	return;

    case AUTOCHR1:
	loadx();
	printf(cat(41,39,0),leaf->nloc + autosize,0);
	return;

    case EXTCHR:
	printf(cat(42,39,0),&leaf->nloc,leaf->offset,0);
	return;

    case STATCHR:
	printf(cat(43,39,0),leaf->nloc,0);
	return;

    case UNKCHR:
	error("unknown class op=PLUS,type=CHAR,class=%d",leaf->class);
	abort();
	exit(1);

     case CONCHR:
     case CONINT:
	 printf(cat(104,39,0),leaf->value&0377,(leaf->value >> 8) & 0377);
	 break;

    case STKINT:
	printf(cat(29,44,45,0));
	xvalid = 0;
	break;

    case INDXINT:
	printf(cat(154,44,45,0));
	xvalid = 0;
	break;

    case AUTOINT0:
	printf(cat(1,44,45,0),(leaf->nloc+autosize)&0377,
	        ((leaf->nloc+autosize)>>8)&0377);
	xvalid = 0;
	break;

    case AUTOINT1:
	loadx();
	printf(cat(41,46,0),leaf->nloc+autosize+1,
	        leaf->nloc+autosize);
	break;

    case EXTINT:
	printf(cat(47,48,0),&leaf->nloc,leaf->offset+1,&leaf->nloc,leaf->offset);
	break;

    case STATINT:
	printf(cat(49,50,0),leaf->nloc,leaf->nloc);
	break;

    case UNKINT:
	error("unknown class op=PLUS,type=%d,class=%d",leaftype,
	        leaf->class);
	abort();

    default:
	error("unknown pick, op=PLUS pick=%d",pick(op,type,leaf,leaftype,actype));
	abort();
    }

 if ((type & TYPEBITS) == CHAR)
     printf(cat(4,0));
 
}



POP1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

	if ((actype & TYPEBITS) != CHAR)
	    printf(cat(144,0));
	printf(cat(144,0));

}



PUSH1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

	printf("	psha\n");
	if ((actype & TYPEBITS) != CHAR)
	    printf("	pshb\n");

}



QUEST1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 diag("warning: QUEST not implemented yet\n");
}



RFORCE1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 /* due to the accumulator based code generation, this operator
    is legal, but performs no operation  */
}



RSHIFT1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 if ((leaftype & TYPEBITS) == CHAR)
     printf(cat(96,0),7);
 else
     printf(cat(96,0),15);

 printf(cat(142,146,0));

 LOADAC1(op,type,leaf,leaftype,actype);

 printf(cat(3,158,87,0),tmplabel);
 xvalid = 0;

 printf(cat(103,86,0),tmplabel+1);
 if ((leaftype & TYPEBITS) == CHAR)
     printf(cat(100,0));
 else
     printf(cat(101,102,0));

 printf(cat(88,87,0),tmplabel,tmplabel+1);
 tmplabel =+ 2;
}



STAR1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{

	printf(cat(90,91,92,0));
	if ((type & TYPEBITS) == CHAR)
	    {
	    printf(cat(2,84,0));
	    }
	else
	    printf(cat(9,10,0));
	xvalid = 0;
}



TAND1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 error("no code for op TAND");
 abort();
}



TIMES1(op,type,leaf,leaftype,actype)
struct tnode *leaf;
{
 multused = 1;
 PUSH1(0,0,0,0,INT);
 if (leaftype & STACK)
    {
     printf(cat(151,144,144,0),"mult");
     return;
    }

 LOADAC1(op,type,leaf,leaftype,actype);
 PUSH1(0,0,0,0,INT);
 printf(cat(151,144,144,144,144,0),"mult");
}


gen(op,type,leaf,leaftype,actype)
{
 extern int (*generate[])();
 extern int (*higen[])();

 if (op <= LSTAR)
    {
     (*generate[op])(op,type,leaf,leaftype,actype);
     return;
    }
 if ((op < PUSH) || (op > LOADADD))
    {
     noop(op);
     return;
    }
 (*higen[op-PUSH])(op,type,leaf,leaftype,actype);
}

noop(op)
{
 error("no code for op=%d\n",op);
}


int (*generate[])() {
noop,		noop,		noop,		noop,		noop,
noop,		noop,		noop,		COLON1,		COMMA1,
noop,		noop,		noop,		noop,		noop,
noop,		noop,		noop,		noop,		noop,
noop,		noop,		noop,		noop,		noop,
noop,		noop,		AUTOI1,		AUTOD1,		noop,
INCBEF1,	DECBEF1,	INCAFT1,	DECAFT1,	EXCLA1,	
AMPER1,		STAR1,		NEG1,		COMPL1,		noop,
PLUS1,		MINUS1,		TIMES1,		DIVIDE1,	MOD1,
RSHIFT1,	LSHIFT1,	AND1,		OR1,		EXOR1,
noop,		noop,		noop,		LOGAND1,	LOGOR1,
NAND1,		noop,		noop,		noop,		noop,
EQUAL1,		NEQUAL1,	LESSEQ1,	LESS1,		GREATEQ1,
GREAT1,		LESSEQP1,	LESSP1,		GREATQP1,	GREATP1,
ASPLUS1,	ASMINUS1,	ASTIMES1,	ASDIV1,		ASMOD1,	
ASRSH1,		ASLSH1,		ASSAND1,	ASOR1,		ASXOR1,
ASSIGN1,	TAND1,		noop,		noop,		noop,
ASSNAND1,	noop,		noop,		noop,		noop,
QUEST1,		noop,		noop,		noop,		noop,
noop,		noop,		noop,		CALL11,		CALL21,
CAL1,		noop,		JUMP1,		CBRANCH1,	INIT1,
noop,		noop,		noop,		noop,		noop,
RFORCE1,	noop,		noop,		noop,		noop,
LSTAR1};

int (*higen[])() {
PUSH1,		POP1,		LOADAC1,	noop,	LOADADD1};
