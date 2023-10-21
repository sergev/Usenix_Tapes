#
/*
 *	CA
 *
 *	Dave Sincoskie
 *	Department of Electrical Engineering
 *	University of Delaware
 *	Newark, Delaware  19711
 *
 *	version 2.5:	9/18/78 Put B...G format binary output in
 *	version 2.4:	9/10/78 Hash symbol table, and speed up
 *	version 2.3:	5/28/78	Check for duplicate labels
 *	version 2.2:	4/7/78  Allow 9 character labels
 *	version 2.1:	3/6/78  Fixed error messages
 *	version 2:	5/18/77
 *	version 1:	12/20/76
 *
 *
 *
 *
 *	*********************************************************
 *	*							*
 *	*	MOTOROLA M6800 CROSS ASSEMBLER			*
 *	*							*
 *	*********************************************************
 *
 *
 *
 *	This program is a cross assembler for the Motorola M6800
 * microprocessor.  It is compatable with Motorola cross assembler
 * syntax, except for the exceptions noted below.
 *
 *	1)  All opcodes must be in lower case.  Upper case letters
 *		are allowed in most other circumstances.
 *
 *	3)  Some pseudo-ops have no effect.  All are legal, but
 *		mon, nam, and opt have no effect.
 *
 *
 *		OPERATING INSTRUCTIONS
 *
 *	The input is specified as an argument to the cross assembler
 *	   call.  To cross assemble a file, type:
 *			ca [-l] [-b] file
 *	The output will appear in two files:
 *		ca6800.lst  --  listing
 *		ca6800.obj  --  mikbug load format of object code.
 *		   or
 *		ca6800.bin  --  binary format of object code.
 *
 *		ca6800.tmp  --  a temporary file created by pass 1.
 *
 *	The '-l' flag toggles the list file on or off, and the '-b'
 *	flag toggles the output format between S9 and B...G format.
 *
 *		FUNCTIONAL DESCRIPTION
 *
 *	   This is a two pass assembler.  The first pass takes each input
 *	line, checks syntax, looks up the opcode, places labels in
 *	a symbol table, and puts all information needed by pass2
 *	in a file called ca6800.tmp.
 *	
 *	   The format of the temp file is as follows:
 *   linenumber action bytes address opcode
 *   operand 1(optional)
 *   operand 2(optional)
 *
 *   linenumber is the number of the line in the source file
 *  	that is being processed.  Most lines will produce only one
 *	line in the temp file, but some pseudo ops will produce
 *	more.
 *   action is an integer representing the type of line being processed
 *	values are:
 *	<0  :  error in pass one
 *       0  :  comment
 *       1  :  inherent mode, no pass 2
 *       2  :  indexed
 *       3  :  immediate
 *       4  :  direct
 *       5  :  extended
 *       6  :  pseudo op, no pass 2 needed  (this type can have multiple
 *			temp file lines.)
 *	 7  :  pseudo op, pass 2 req'd.  (same)
 *
 *   bytes is the number of bytes of machine code produced by the instruction
 *		(0,1,2, or 3)
 *   address is the address of the start of the instruction.
 *   opcode is the opcode produced by pass 1. (except for action=6 or 7
 *	in which case opcode will contain the type of pseudo op.
 *   operand1 is the ascii field containing operand1.
 *   operand 2    same.
 *
 * 	pass 2 opens the temp and source files as input.  It creates
 *	any needed machine code and merges the two files to create
 *	the listing file.  At the same time, it formats the machine code
 *	and puts it in a file named ca6800.obj or ca6800.bin.
 *
 *****************************************************************/



#define CPX	43
#define LDS	61
#define LDX	62
#define OPCTABLEN	107
#define	TABLEN	1024
#define	LABELFOUND	01
#define	OPCODEFOUND	02
#define	OPR1FOUND	04
#define OPR2FOUND	010

#define HASHMOD		64
#define MEMINC		2048
struct symtab
	{
	 struct symtab *nextp;
	 unsigned int symval;
	 char symname[10];
	};
#define ENTRYSIZE	14
struct symtab *hashtab[HASHMOD];
struct symtab *endhash[HASHMOD];
struct symtab *lastsym;  /* last inserted symbol */

int freemem;
char *memp;
struct parsetree
	{
	 char label[10];
	 char opcode[5];
	 char opr1[100];
	 char opr2[100];
	} line;
struct filebuf
	{
	 int fildes;
	 int nleft;
	 char *np;
	 char buff[512];
	} buf1, *iobuf1;
struct filebuf buf2, *iobuf2;
struct filebuf buf3, *iobuf3;
struct filebuf buf4, *iobuf4;
struct opcodes
	{
	 char opc[5];
	 char inherent;
	 char immediate;
	 char direct;
	 char extended;
	 char indexed;
	};

struct opcodes tabopc[OPCTABLEN] {
	"aba",033,00,00,00,00,
	"adca",00,0211,0231,0271,0251,
	"adcb",00,0311,0331,0371,0351,
	"adda",00,0213,0233,0273,0253,
	"addb",00,0313,0333,0373,0353,
	"anda",00,0204,0224,0264,0244,
	"andb",00,0304,0324,0364,0344,
	"asl",00,00,00,0170,0150,
	"asla",0110,00,00,00,00,
	"aslb",0130,00,00,00,00,
	"asr",00,00,00,0167,0147,
	"asra",0107,00,00,00,00,
	"asrb",0127,00,00,00,00,
	"bcc",044,00,00,00,00,
	"bcs",045,00,00,00,00,
	"beq",047,00,00,00,00,
	"bge",054,00,00,00,00,
	"bgt",056,00,00,00,00,
	"bhi",042,00,00,00,00,
	"bita",00,0205,0225,0265,0245,
	"bitb",00,0305,0325,0365,0345,
	"ble",057,00,00,00,00,
	"bls",043,00,00,00,00,
	"blt",055,00,00,00,00,
	"bmi",053,00,00,00,00,
	"bne",046,00,00,00,00,
	"bpl",052,00,00,00,00,
	"bra",040,00,00,00,00,
	"bsr",0215,00,00,00,00,
	"bvc",050,00,00,00,00,
	"bvs",051,00,00,00,00,
	"cba",021,00,00,00,00,
	"clc",014,00,00,00,00,
	"cli",016,00,00,00,00,
	"clr",00,00,00,0177,0157,
	"clra",0117,00,00,00,00,
	"clrb",0137,00,00,00,00,
	"clv",012,00,00,00,00,
	"cmpa",00,0201,0221,0261,0241,
	"cmpb",00,0301,0321,0361,0341,
	"com",00,00,00,0163,0143,
	"coma",0103,00,00,00,00,
	"comb",0123,00,00,00,00,
	"cpx",00,0214,0234,0274,0254,
	"daa",031,00,00,00,00,
	"dec",00,00,00,0172,0152,
	"deca",0112,00,00,00,00,
	"decb",0132,00,00,00,00,
	"des",064,00,00,00,00,
	"dex",011,00,00,00,00,
	"eora",00,0210,0230,0270,0250,
	"eorb",00,0310,0330,0370,0350,
	"inc",00,00,00,0174,0154,
	"inca",0114,00,00,00,00,
	"incb",0134,00,00,00,00,
	"ins",061,00,00,00,00,
	"inx",010,00,00,00,00,
	"jmp",00,00,00,0176,0156,
	"jsr",00,00,00,0275,0255,
	"ldaa",00,0206,0226,0266,0246,
	"ldab",00,0306,0326,0366,0346,
	"lds",00,0216,0236,0276,0256,
	"ldx",00,0316,0336,0376,0356,
	"lsr",00,00,00,0164,0144,
	"lsra",0104,00,00,00,00,
	"lsrb",0124,00,00,00,00,
	"neg",00,00,00,0160,0140,
	"nega",0100,00,00,00,00,
	"negb",0120,00,00,00,00,
	"nop",01,00,00,00,00,
	"oraa",00,0212,0232,0272,0252,
	"orab",00,0312,0332,0372,0352,
	"psha",066,00,00,00,00,
	"pshb",067,00,00,00,00,
	"pula",062,00,00,00,00,
	"pulb",063,00,00,00,00,
	"rol",00,00,00,0171,0151,
	"rola",0111,00,00,00,00,
	"rolb",0131,00,00,00,00,
	"ror",00,00,00,0166,0146,
	"rora",0106,00,00,00,00,
	"rorb",0126,00,00,00,00,
	"rti",073,00,00,00,00,
	"rts",071,00,00,00,00,
	"sba",020,00,00,00,00,
	"sbca",00,0202,0222,0262,0242,
	"sbcb",00,0302,0322,0362,0342,
	"sec",015,00,00,00,00,
	"sei",017,00,00,00,00,
	"sev",013,00,00,00,00,
	"staa",00,00,0227,0267,0247,
	"stab",00,00,0327,0367,0347,
	"sts",00,00,0237,0277,0257,
	"stx",00,00,0337,0377,0357,
	"suba",00,0200,0220,0260,0240,
	"subb",00,0300,0320,0360,0340,
	"swi",077,00,00,00,00,
	"tab",026,00,00,00,00,
	"tap",06,00,00,00,00,
	"tba",027,00,00,00,00,
	"tpa",07,00,00,00,00,
	"tst",00,00,00,0175,0155,
	"tsta",0115,00,00,00,00,
	"tstb",0135,00,00,00,00,
	"tsx",060,00,00,00,00,
	"txs",065,00,00,00,00,
	"wai",076,00,00,00,00};

char *tmpfil	"ca6800.tmp";
char *listfil	"ca6800.lst";
char *objfil	"ca6800.obj";
char *binfil	"ca6800.bin";
char *errmsg[]
	{
	 "Label too long",
	 "Opcode too long",
	 "Operand 1 too long",
	 "Operand 2 too long",
	 "Illegal operand",
	 "Illegal addressing mode",
	 "Symbol table overflow",
	 "Operand required",
	 "Label required with equate",
	 "Operand truncated to 8 bits",
	 "Label not allowed with -org-",
	 "Illegal opcode",
	 "temp file error",
	 "undefined operand",
	 "destination out of range of branch",
	 "operand must be less then 255",
	 "illegal character constant",
	 "duplicate label"
	};
char *pops[]
	{
	 "end",
	 "equ",
	 "fcb",
	 "fcc",
	 "fdb",
	 "mon",
	 "nam",
	 "opt",
	 "org",
	 "page",
	 "rmb",
	 "spc"
	};
char linebuf[100];
char outbuf[300];
char *null	"";
int ptabop[26];
int flag, location, linenum, linerd, action, opcd, bytes, addr;
int prline, tbyte, cbyte, csum, lastloc, fildes4;

/*  set this variable to 0 if you want list by default, else 1 */
int nolist	1;

/*  set this variable to 0 if you want S9 format output, 1 for B...G  */
int bin		1;

char *inputf;
char usage[]	"usage -- %s [-l] [-b] file\n";
char signon[]	"M6800 cross assembler v2.5 9/18/78\n";


/*
 *	main program
 */
main(argc,argv)
int argc;
char **argv;
{
 register int i;

 if (argc < 2)
    {
     printf(signon);
     printf(usage,argv[0]);
     exit();
    }

 for (i=1; i<argc; i++)
    {
     if (argv[i][0] == '-')
	{
	 switch(argv[i][1])
	    {
	     case 'l':
		if (nolist == 0)
		     nolist = 1;
		else
		     nolist = 0;
		break;

	     case 'b':
		if (bin == 0)
		     bin = 1;
		else
		     bin = 0;
		break;

	     default:
		printf("%s: unknown arg %s\n",argv[0],argv[i]);
		printf(usage,argv[0]);
		exit();
	    }
	 continue;
	}

     if (inputf == 0)
	 inputf = argv[i];
     else
	{
	 printf("%s: only 1 source file allowed\n",argv[0]);
	 exit();
	}
    }
 if (pass1(argv,inputf) < 0) exit();
 pass2(argv,inputf);
 exit();
}
/*
 *	PASS 1
 */
pass1(argv,fname)
char **argv;
char *fname;
{
 extern char linebuf[];
 extern struct filebuf buf1, *iobuf1;
 extern struct filebuf buf2, *iobuf2;
 extern struct parsetree line;
 struct symtab *psyms;
 extern int location, linenum, flag;
 extern char *null;
 register int i;
 int addmode, mark, byte, val[2];
 iobuf1 = &buf1;
 iobuf2 = &buf2;
 if (fopen(fname,iobuf1) < 0)
	{
	 printf ("%s: No file %s\n",argv[0],fname);
	 return(-1);
	}
 if (initopc() < 0)
	 return(-1);
 if (opentmp() < 0)
	return(-1);
 location = 0;

 for (i=0; i<HASHMOD; i++)
     hashtab[i] = endhash[i] = 0;
 freemem = 0;

 linenum = 0;
 while ((linebuf[0] = getc(iobuf1)) != -1)
	{
	 i=0;
	 if (linebuf[0] != '\n')
	     for (i=1; (linebuf[i] = getc(iobuf1)) != '\n'; i++);
	 linebuf[i] = '\0';
	 linenum++;
	 flag = parse(linebuf);
	 if (flag < 0)
		{
		 errmes(flag);
		 wrtemp(flag,0,location);
		 continue;
		}
	 if (flag == 0)
		{
		 wrtemp(flag,0,0);
		 continue;
		}
	 if (!(flag & OPR2FOUND))
		 line.opr2[0] = '\0';
	 if (flag & LABELFOUND)
		{
		 if (line.label[0] != '~')  /*  concession to compiler labels */
			{
			 if (lookup(line.label) != -1)
				{
				 errmes(-18);
				 wrtemp(-18,0,0);
				 continue;
				}
			}
		 if (install(line.label,location) < 0)
			{
			 errmes(-7);
			 return(-1);
			}
		}
	 if (flag & OPCODEFOUND)
		{
		 mark = findopc(line.opcode);
		 if (mark == -1)
			{
			 if (pseudoop() == -1)
				{
				 errmes(-12);
				 wrtemp(-12,0,location);
				}
			 continue;
			}
		}
	 /*   check for inherent mode   */
	 if (tabopc[mark].inherent != 0)
		{
		 if (((tabopc[mark].inherent > 31) && (tabopc[mark].inherent < 48))
			|| (tabopc[mark].inherent == 0177615))
			{
			 if (!(flag & OPR1FOUND))
				{
				 errmes(-8);
				 wrtemp(-8,0,0);
				 continue;
				}
			 wrtemp(1,2,location,tabopc[mark].inherent,line.opr1,null);
			 location =+ 2;
			 continue;
			}
		 wrtemp(1,1,location,tabopc[mark].inherent,null,null);
		 location++;
		 continue;
		}
	 if ((((line.opr1[0] == 'x') || (line.opr1[0] == 'X')) &&
		(line.opr1[1] == '\0')) || (flag & OPR2FOUND))
		{
		 if (tabopc[mark].indexed == 0)
			{
			 errmes(-6);
			 wrtemp(-6,0,location);
			 continue;
			}
		 wrtemp (2,2,location,tabopc[mark].indexed,line.opr1,
							line.opr2);
		 location=+ 2;
		 continue;
		}
	 if (!(flag & OPR1FOUND))
		{
		 wrtemp(-8,0,location);
		 errmes(-8);
		 continue;
		}
	 if (line.opr1[0] == '#')
		{
		 if (tabopc[mark].immediate == 0)
			{
			 errmes(-6);
			 wrtemp(-6,0,location);
			 continue;
			}
		 byte = 2;
		 if ((mark == CPX) || (mark == LDX) || (mark == LDS))
			 byte++;
		 wrtemp(3,byte,location,tabopc[mark].immediate,
					&line.opr1[1],line.opr2);
		 location=+ byte;
		 continue;
		}
	/*
	 *	take care of implicit direct addressing
	 */
	 eval(line.opr1,val);
	 if ((val[0] == 0) && ((val[1] < 256) && (val[1] >= 0)))
		{
		 if (tabopc[mark].direct != 0)
			{
			 wrtemp(4,2,location,tabopc[mark].direct,
					line.opr1,line.opr2);
			 location=+ 2;
			 continue;
			}
		}
	/*   default : extended */
	 if (tabopc[mark].extended == 0)
		{
		 errmes(-6);
		 wrtemp(-6,0,location);
		 continue;
		}
	 wrtemp(5,3,location,tabopc[mark].extended,line.opr1,
						line.opr2);
	 location =+ 3;
	}
 fflush(iobuf2);
 close(buf2.fildes);
 close(buf1.fildes);
}
/*
 *	parsing routine
 *		the returned value flag=parse(buf)
 *		=-4	(operand 2 >100 chrs.)
 *		=-3	(operand 1 >100 chrs.)
 *		=-2	(opcode >4 chrs.)
 *		=-1	(label >8 chrs.)
 *		=0	(comment)
 *		=1	(labelfound)
 *		=2	(opcode found)
 *		=4	(operand 1 found)
 *		=8	(operand 2 found)
 */
parse(buf)
char buf[];
{
 extern int flag;
 extern struct parsetree line;
 register int i, j, k;
 flag = 0;
 i = 0;
/*	is line a comment?				*/
/*	no, process label if present			*/
 if ((buf[0] == '*') || (buf[0] == '\0'))
	 return(flag);
 if ((buf[0] != ' ') && (buf[0] != '\011'))
	{
	 flag =|LABELFOUND;
	 for (i=0; i < 9; i++)
		{
		 switch (buf[i])
			{
			 case '\0':
				    line.label[i] = '\0';
				    return(flag);
			 case ' ': case '\011':
				    line.label[i] = '\0';
				    break;
			default:
				 line.label[i] = buf[i];
				 continue;
			} break;
		 line.label[i] = '\0';
		}
	 if ((i == 9) && ((buf[i] != ' ')&&(buf[i] != '\011')))
		{
		 return(-1);
		}
	}
  while ((buf[i] == ' ')||(buf[i] == '\011'))
 	 i++;
 k=i;
/*	process opcode					*/
 line.opcode[3] = '\0';
 for (j=0; j < 4; j++)
 	{
	 switch(buf[j+k])
 		{
 		 case '\0':
 			    line.opcode[j] = '\0';
			    if (j == 0) return(flag);
			  return(flag | OPCODEFOUND);
 		 case ' ': case '\011':
 			  line.opcode[j]= '\0';
 			  break;
 		 default:
 			line.opcode[j] = buf[j+k];
 			continue;
 		} break;
 	}
 i = j+k;
 if ((j == 4)&&((buf[i] != ' ')&&(buf[i] != '\t')&&(buf[i] != '\0')))
 	{
	 return(-2);
 	}
/*	condense four letter opcode if present		*/
 if (j == 3)
 	{
 	 i++;
 	 switch (buf[i])
 		{
 		 case 'a': case 'A': case 'b': case 'B':
 		 switch (buf[i+1])
 			{
			 case ' ': case '\t': case '\0':
 				line.opcode[j] = buf[i];
 				line.opcode[j+1] = '\0';
				i++;
 			}
 		}
 	}
 flag =|OPCODEFOUND;
while ((buf[i] == ' ')||(buf[i] == '\011'))
 i++;
if (buf[i] != '\0') flag =|OPR1FOUND;
else return(flag);
 k=i;
/*	process operands				*/
 for (j=0; j < 100; j++)
 {
  switch (buf[j+k])
	{
 	 case '\0':
 		line.opr1[j]= '\0';
 		return(flag);
 	 case ' ': case '\011':
 		line.opr1[j] = '\0';
 		return(flag);
 	 case ',':
		 i= j+k+1;
		 k= i;
 		 flag =|OPR2FOUND;
 		 line.opr1[j] = '\0';
		 for (j=0; j < 100; j++)
 			{
 			 switch (buf[j+k])
 				{
 				 case '\0':
 					line.opr2[j]= '\0';
 					return(flag);
 				 case ' ': case '\011':
 					line.opr2[j]= '\0';
 					return(flag);
 				 default:
 					line.opr2[j]=buf[j+k];
 					continue;
 				}
			 return(-4);
 			}
	 case '\'':		/* character constant  */
		 if (buf[j+k+1] == '\0')  return(-17);
		 line.opr1[j] = buf[j+k];
		 j++;
		 line.opr1[j] = buf[j+k];
		 break;
 	default:
 		line.opr1[j]=buf[j+k];
 	}
  }
 return(-3);
}
/*
 *	function to initialize opcode table
 */
initopc()
{
 char c;
 register int i, j;

 j = 0;
 c = 'b';
 ptabop[0] = 0;
 for (i = 0; i < 107; i++)
	{
	 if (tabopc[i].opc[0] < c)  continue;
	 if (tabopc[i].opc[0] == c)
	{
	 ptabop[++j] = i;
	 c++;
	 continue;
	}
	 if (tabopc[i].opc[0] > c)
		{
		 while (c++ < tabopc[i].opc[0])
			 ptabop[++j] = -1;
		}
	i--;
	 c--;
	}
 while (++j < 26)
	 ptabop[j] = -1;
 ptabop[22] = 106;
}
/*
 *	convert ascii to hex (8 bits at a time)
 */
char asciitohex(s1,s2)
char s1,s2;
{
 register int a;
 s1=| 040;
 s2=| 040;
 if (s1 < 072)
	s1=- 060;
 else
	s1=- 0127;
 if (s2 < 072)
	s2=- 060;
 else
	s2=- 0127;
	 a = (s1 << 4) + s2;
	 a =& 0377;
	 return(a);
}
/*
 *	open temporary file
 */
opentmp()
{
 extern struct filebuf buf2, *iobuf2;
 extern char *tmpfil;
 iobuf2 = &buf2;
 if (fcreat(tmpfil,iobuf2) < 0)
	{
	 printf("ca: Unable to create temp file.\n");
	 return(-1);
	}
 return(1);
}
/*
 *	error message printer
 */
errmes(errno)
int errno;
{
 extern int linenum;
 extern char *errmsg[];
 int nooferrs;
 nooferrs = 18;
 printf("ca:%d: ",linenum);
 errno = (-errno)-1;
 if ((errno < 0) || (errno > nooferrs))
	{
	 printf("ca: temp file format error\n");
	 return;
	}
 pri (errmsg[errno]);
 return;
}
/*
 *	procedure to write onto temp file
 */
wrtemp(action1,bytes1,addr1,opc1,opr11,opr21)
char *opr11, *opr21;
{
 register int i;

 putw(linenum,iobuf2);
 putw(action1,iobuf2);

 if (action1 <= 0)
	{
	 putw(addr1,iobuf2);
	 return;
	}

 putw(bytes1,iobuf2);
 putw(addr1,iobuf2);
 putw(opc1,iobuf2);

 for (i=0; opr11[i] != '\0'; i++)
	 putc(opr11[i],iobuf2);
 putc('\n',iobuf2);
 for (i=0; opr21[i] != '\0'; i++)
	 putc(opr21[i],iobuf2);
 putc('\n',iobuf2);
}
/*
 *	procedure to convert hexadecimal to ascii
 */
itoh(num,string)
char string[];
int num;
{
 register int i, j;
 string[0] = (num & 0170000) >> 12;
 string[0] = (string[0] & 017) + 060;
 string[1] = 060 + ((num &   07400) >> 8);
 string[2] = 060 + ((num &    0360) >> 4);
 string[3] = 060 + (num & 017);
 for (i=0; i < 4; i++)
	if (string[i] > 071)
		string[i] =+ 047;
 i = -1;
 while ((i < 3) && (string[++i] == '0'));
 j = 0;
 while (i < 4)
	 string[j++] = string[i++];
 string[j] = '\0';
 return;
}
/*
 *	write 4 characters
 */
/*
 *	pseudo operator evaluation
 */
pseudoop()
{
 extern struct parsetree line;
 extern char linebuf[];
 extern int flag, linenum, location;
 extern char *pops[];
 struct symtab *psyms;
 extern char *null;
 register int i,j,k;
 char delim;
 int val[2], comma;
 for (i=0; i < 12; i++)
	{
	 if (compar(pops[i],line.opcode) > 0)
	 break;
	}
 if (i >= 12)
	return(-1);
 switch (i)
	{
	 case 0: case 5: case 6: case 7:
	 /*  end    mon     nam    opt
		all ignored		*/
		 wrtemp(6,0,location,i,null,null);
		 return;
	 case 1:  /*  equ  */
		if (!(flag & LABELFOUND))
			{
			 errmes(-9);
			 wrtemp(-9,0,0);
			 return;
			}
		eval(line.opr1,val);
		if (val[0] < 0)
			{
			 wrtemp(-5,0,0);
			 return;
			}
		if (val[0] > 0)
			{
			 wrtemp(7,0,location,i,line.opr1,line.label);
			 return;
			}
		 lastsym->symval = val[1];
		 wrtemp(6,0,location,i,null,null);
		 return;
	case 2:  /*  fcb  */
		if (!(flag & OPR1FOUND))
			{
			 errmes(-8);
			 wrtemp(-8,0,0);
			 return;
			}
		eval(line.opr1,val);
		if (val[0] < 0) return;
		if (val[0] > 0)
			wrtemp(7,1,location,i,line.opr1,null);
		else
			{
			if ((val[1] > 255) || (val[1] < 0))
				{
				 errmes(-10);
				 val[1]=& 0377;
				}
			wrtemp(6,1,location,val[1],null,null);
			}
		location++;
		if (!(flag & OPR2FOUND)) return;
		comma = 0;
		for (i=0; line.opr2[i] != '\0'; i++)
			if (line.opr2[i] == ',')
				comma++;
		k=0;
		for (i=0; i<=comma; i++)
			{
			 j=0;
			 while ((line.opr2[k] != '\0') &&
			        (line.opr2[k] != ',') &&
			        (line.opr2[k] != ' ') &&
			        (line.opr2[k] != '\t'))
				{
				 line.opr1[j] = line.opr2[k];
				 j++;
				 k++;
				}
			 line.opr1[j] = '\0';
			 eval(line.opr1,val);
			 if (val[0] < 0) return;
			 if (val[0] > 0)
			 wrtemp(7,1,location,i,line.opr1,null);
			 else
				{
				if ((val[1] > 255) || (val[1] < 0))
					{
					 errmes(-10);
					 val[1]=& 0377;
					}
				wrtemp(6,1,location,val[1],null,null);
				}
			 location++;
			 if (line.opr2[k] != ',')
				 break;
			 k++;
			}
		return;
	case 3:  /*  fcc  */
		if (!(flag & OPR2FOUND))
			{
			 delim = line.opr1[0];
			 i=0;
			 while (linebuf[i] != delim) i++;
			 i++;
			 while ((linebuf[i] != delim) && (linebuf[i] != '\0'))
				{
				 j = linebuf[i];
				 wrtemp(6,1,location,j,null,null);
				 location++;
				 i++;
				}
			}
		else
			{
			 j = atoi(line.opr1);
			 i = 0;
			 while (linebuf[i] != ',') i++;
			 i++;
			 k=0;
			 while ((k<j) && (linebuf[i] != '\0'))
				{
				 wrtemp(6,1,location,linebuf[i],null,null);
				 k++;
				 i++;
				 location++;
				}
			if (linebuf[i] == '\0')
				{
				 while (k<j)
					{
					 wrtemp(6,1,location,32,null,null);
					 k++;
					 location++;
					}
				}
			}
		return;
	case 4:  /*  fdb  */
		if (!(flag & OPR1FOUND))
			{
			 errmes(-8);
			 wrtemp(-8,0,0);
			 return;
			}
		eval(line.opr1,val);
		if (val[0] < 0) return;
		 if (val[0] > 0)
			wrtemp(7,2,location,i,line.opr1,null);
		 else
			wrtemp(6,2,location,val[1],null,null);
		 location=+ 2;
		 if (!(flag & OPR2FOUND)) return;
		 comma=0;
		 for (i=0;line.opr2[i] != '\0'; i++)
			 if (line.opr2[i] == ',') comma++;
		 k=0;
		 for (i=0; i<=comma; i++)
			{
			 j=0;
			 while ((line.opr2[k] != '\0') &&
			        (line.opr2[k] != ',') &&
			        (line.opr2[k] != ' ') &&
			        (line.opr2[k] != '\t'))
				{
				 line.opr1[j] = line.opr2[k];
				 j++;
				 k++;
				}
			line.opr1[j] = '\0';
			 eval(line.opr1,val);
			 if (val[0] < 0) return;
			 if (val[0] > 0)
				wrtemp(7,2,location,i,line.opr1,null);
			 else
				wrtemp(6,2,location,val[1],null,null);
			 location=+ 2;
			 if (line.opr2[k] != ',') break;
			 k++;
			}
		return;
	case 8:  /*  org  */
		if (flag & LABELFOUND)
			{
			 errmes(-11);
			 wrtemp(-11,0,0);
			 return;
			}
		if (!(flag & OPR1FOUND))
			{
			 errmes(-8);
			 wrtemp(-8,0,0);
			 return;
			}
		eval(line.opr1,val);
		if (val[0] < 0) return;
		if (val[0] > 0)
			{
			 errmes(-5);
			 wrtemp(-5,0,0);
			 return;
			}
		wrtemp(6,0,val[1],i,null,null);
		location = val[1];
		return;
	case 9:  /*  page  */
		wrtemp(7,0,location,i,null,null);
		return;
	case 10:  /*  rmb  */
		if (!(flag & OPR1FOUND))
			{
			 errmes(-8);
			 wrtemp(-8,0,0);
			 return;
			}
		eval(line.opr1,val);
		if (val[0] < 0) return;
		if (val[0] > 0)
			{
			 errmes(-5);
			 wrtemp(-5,0,0);
			 return;
			}
		wrtemp(6,val[1],location,i,line.opr1,null);
		location =+ val[1];
		return;
	case 11:  /*  spc  */
		wrtemp(7,0,location,i,line.opr1,null);
		return;
	}
}
/*
 *	operand evaluation procedure
 *
 *	returns a 2 integer answer
 *		if ans[0] == 1, operand could not be evaluated.
 *		if ans[0] == 0, operand was evaluated, and
 *		   the value is in ans[1].
 */
eval(string,ans)
char string[];
int ans[];
{
 register int i,j,k;
 char term1[100], op;
 int v1, v2;
 j = 0;
 i = 0;
 if (string[0] == '\'')
	{
	 termval(string,ans);
	 return;
	}
 while ((string[i] != '+') && (string[i] != '-') && (string[i] != '*') &&
		(string[i] != '/') && (string[i] != '\0'))
	 term1[j++] = string[i++];
 if ((j == 0) && (string[0] == '*'))
	{
	 if (string[1] == '\0')
		{
		 ans[0] = 0;
		 ans[1] = location;
		 return;
		}
	 term1[j++] = string[i++];
	}
 term1[j] = '\0';
 termval(term1,ans);
 if (ans[0] != 0) return;
 v1 = ans[1];
 if (string[i] == '\0') return;
 op = string[i];
 i++;
 j = 0;
 while ((string[i] != '+') && (string[i] != '-') && (string[i] != '*') &&
			(string[i] != '/') && (string[i] != '\0'))
	term1[j++] = string[i++];
 term1[j] = '\0';
 termval(term1,ans);
 if (ans[0] != 0) return;
 v2 = ans[1];
 switch (op)
	{
	 case'+':
		v1=+ v2;
		break;
	 case '-':
		v1=- v2;
		break;
	 case '*':
		v1=* v2;
		break;
	 case '/':
		v1=/ v2;
	}
 if (string[i] == '\0')
	{
	ans[1] = v1;
	ans[0] = 0;
	return;
	}
}
/*
 *	single term evaluation procedure
 */
termval(string,ans)
char string[];
int ans[];
{
 struct symtab *psyms;
 register int i;
 i=0;
 switch (string[0])
	{
	 case '*':
		ans[0] = 0;
		ans[1] = location;
		return;
	 case '&': i = 1;
	 case '0': case '1': case '2': case '3': case '4': case '5':
	 case '6': case '7': case '8': case '9':
		ans[0] = 0;
		ans[1] = atoi(&string[i]);
		return;
	 case'$':
		ans[0] = 0;
		ans[1] = atoh(&string[1]);
		return;
	 case '@':
		ans[0] = 0;
		ans[1] = atoo(&string[1]);
		return;
	 case '%':
		ans[0] = 0;
		ans[1] = atob(&string[1]);
		return;
	 case '\'':
		ans[0] = 0;
		ans[1] = string[1];
		return;
	}
 if (((string[0] == 'x') || (string[0] == 'X')) && (string[1] == '\0'))
	{
	 ans[0] = 0;
	 ans[1] = 0;
	 return;
	}
 psyms = lookup(string);
 if (psyms == -1)
	{
	ans[0] = 1;
	return;
	}
 ans[0] = 0;
 ans[1] = psyms->symval;
 return;
}
/*
 *	ascii to hexadecimal conversion procedure
 */
atoh(string)
char string[];
{
 register int i, ans, h;
 ans = 0;
 for (i=0;;i++)
	{
	 h=string[i];
	 if (h < 060)
		 return(ans);
	 h =| 040;
	 if ((h > 071) && (h < 0141))
		 return(ans);
	 if (h > 0146)
		 return(ans);
	 ans=<< 4;
	 h =- 060;
	 if (h > 9)
	 h =- 047;
	 ans =+ h;
	}
 return(ans);
}
/*
 *	ascii to octal conversion
 */
atoo(string)
char string[];
{
 register int i, ans, h;
 ans = 0;
 for (i=0;;i++)
	{
	 h = string[i];
	 if ((h < 060) || (h > 067)) return(ans);
	 h =- 060;
	 ans =<< 3;
	 ans =+ h;
	}
 return(ans);
}
/*
 *	ascii to binary conversion
 */
atob(string)
char string[];
{
 register int i, ans, h;
 ans = 0;
 for (i=0;;i++)
	{
	 h = string[i];
	 if ((h < 060) || (h > 061)) return(ans);
	 h =- 060;
	 ans =<< 1;
	 ans =+ h;
	}
 return(ans);
}
/*
 *	opcode table lookup
 */
findopc(name)
char name[];
{
 register int i, j;
 extern int ptabop[];
 extern struct opcodes tabopc[];
 j = name[0] - 'a';
 if ((i = ptabop[j]) < 0)   return(-1);
 for (; i < OPCTABLEN; i++)
	{
	 if (compar(name,tabopc[i].opc) > 0)
		 return(i);
	}
 return(-1);
}
/*
 *	function to install symbols in symbol table
 */
install(name,value)
register char *name;
unsigned int value;
{
 char *q;
 int hash;
 struct symtab *p, *new;

 hash = hashindx(name);

 p = endhash[hash];
 new = getmem(ENTRYSIZE);

 if (p == 0)   /*  empty list  */
     hashtab[hash] = endhash[hash] = new;
 else
    {
     p->nextp = new;
     endhash[hash] = new;
    }

 new->nextp = 0;
 q = new->symname;
 while(*name != '\0') *q++ = *name++;
 new->symval = value;
 lastsym = new;
 return(0);
}

/*
 *	routine to manage and allocate free memory
 *	returns pointer to contiguous space of size amount
 *	does sbrk's as needed
 */
getmem(amount)
{
 register char *p;

 if (freemem < amount)
    {
     memp = sbrk(MEMINC);
     if (memp == -1)
	{
	 errmes(-7);
	 exit();
	}

     freemem = MEMINC;
    }

 p = memp;
 memp =+ amount;
 freemem =- amount;
 return(p);
}
/*
 *	function to look up symbol in table and return pointer to entry
 */
lookup(name)
register char *name;
{
 register int hash;
 register struct symtab *p;

 hash = hashindx(name);
 p = hashtab[hash];
 while (p != 0)
    {
     if (compar(name,p->symname) > 0)  return(p);
     p = p->nextp;
    }

 return(-1);
}


/*
 *	function to compute hash index by summing characters
 */
hashindx(name)
register char *name;
{
 register int hash;

 hash = 0;
 while(*name != '\0')
     hash =+ *name++;

 return(hash%HASHMOD);
}

/*
 *	function to compare two strings
 *	returns 1 if equal, 0 if not
 */
compar(s1,s2)
register char *s1, *s2;
{
 while (*s1++ == *s2)
	if (*s2++ == '\0')
	return(1);
 return(0);
}
/*
 *	print character string
 */
pri(s)
char *s;
{
 while (*s != '\0')
	putchar(*s++);
 putchar('\n');
}

/*
 *		PASS2
 *
 *	Read the temp file created by pass1, evaluate all
 *	operands, create the listing and opcode files.
 *
 */

pass2(argv,fname)
char **argv;
char *fname;
{
 extern char linebuf[];
 extern struct filebuf buf1, *iobuf1;
 extern struct filebuf buf2, *iobuf2;
 extern struct filebuf buf3, *iobuf3;
 extern struct filebuf buf4, *iobuf4;
 struct syms *psyms;
 extern char *tmpfil, *listfil, *objfil;
 extern int linenum, location, linerd, action, opcd, bytes;
 extern int prline;
 int val[2], disp, highbyte, lobyte;
 int comp();
 register int i, j, k;
 char string[6];
 iobuf1 = &buf1;
 iobuf2 = &buf2;
 iobuf3 = &buf3;
 if (fopen(fname,iobuf1) < 0)
	{
	 printf("%s: No source file %s\n",argv[0],fname);
	 return;
	}
 if (fopen(tmpfil,iobuf2) < 0)
	{
	 printf("%s: No temp file\n",argv[0]);
	 return;
	}
 if (fcreat(listfil,iobuf3) < 0)
	{
	 printf("%s: Can't create list file\n",argv[0]);
	 return;
	}

 if ((fildes4 = creat(binfil,0644)) < 0)
    {
     printf("%s: Can't create bin file\n",argv[0]);
     return;
    }
 if (fcreat(objfil,iobuf4) < 0)
    {
     printf("%s: Can't create object file\n",argv[0]);
     return;
    }

 tbyte = 0;
 linenum = 0;
 while (rdtmp() >= 0)  /*  fetch a line from the temp file  */
	{
	 location = addr;
	 prline = 1;
	 linenum++;
	 if ((linerd != linenum) && (action < 6))
		{
		 errmes(-13);
		 continue;
		}
	 if(action < 0)  /*  pass1 error?  */
		{
		 errput();
		 continue;
		}
	 if (action == 0)  /*  no code generated?  */
		{
		 lout0();
		 continue;
		}
	 switch(action)
		{
		 case 1:  /*  inherent  */
			{
			 if (bytes == 1)
				{
				 lout1(opcd);
				 objout(1,opcd);
				 continue;
				}
			 eval(line.opr1,val);
			 if (echk(val) < 0)  continue;
			 disp = val[1] - addr - 2;
			 if ((disp > 127) || (disp < -128))
				{
				 errmes(-15);
				 errput();
				 continue;
				}
			 lout2(opcd,disp);
			 objout(2,opcd,disp);
			 continue;
			}
		case 2:  /*  indexed  */
		case 3:  /*  immediate  */
		case 4:  /*  direct  */
			{
			 eval(line.opr1,val);
			 if (echk(val) < 0)  continue;
			 if (bytes == 3)
				{
				 lout3(opcd,val[1]);
				 highbyte = (val[1] >> 8) & 0377;
				 lobyte = val[1] & 0377;
				 objout(3,opcd,highbyte,lobyte);
				 continue;
				}
			 if ((val[1] > 255) || (val[1] < 0))
				{
				 errmes(-16);
				 errput();
				 continue;
				}
			 lout2(opcd,val[1]);
			 objout(2,opcd,val[1]);
			 continue;
			}
		case 5:  /*  extended  */
			{
			 eval(line.opr1,val);
			 if (echk(val) < 0)  continue;
			 lout3(opcd,val[1]);
			 highbyte = (val[1] >> 8) & 0377;
			 lobyte = val[1] & 0377;
			 objout(3,opcd,highbyte,lobyte);
			 continue;
			}
		case 6:  /*  pseudo-op, no pass2 req'd */
			{
			 if (linerd != linenum)
				{
				 linenum--;
				 prline = 0;
				}
			 if ((opcd == 10)&&(line.opr1[0] != '\0'))
				{
				 highbyte = (bytes >> 8) & 0377;
				 lobyte = bytes & 0377;
				 lout2(highbyte,lobyte);
				 continue;
				}
			 if (bytes == 1)
				{
				 lout1(opcd);
				 objout(1,opcd);
				 continue;
				}
			 if (bytes == 2)
				{
				 highbyte = (opcd >> 8) & 0377;
				 lobyte = opcd & 0377;
				 lout2(highbyte,lobyte);
				 objout(2,highbyte,lobyte);
				 continue;
				}
			 lout0();
			 continue;
			}
		case 7:  /*  pseudo-op, pass2 required  */
			{
			 if (linerd != linenum)
				{
				 linenum--;
				 prline = 0;
				}
			 if (opcd == 1)  /*  equ  */
				{
				 eval(line.opr1,val);
				 if (echk(val) < 0)  continue;
				 psyms = lookup(line.opr2);
				 psyms->symval = val[1];
				 lout0();
				 continue;
				}
			 if (opcd == 2)  /*  fcb  */
				{
				 eval(line.opr1,val);
				 if (echk(val) < 0)  continue;
				 if ((val[1] > 255) || (val[1] < 0))
					{
					 errmes(-16);
					 errput();
					 continue;
					}
				 lout1(val[1]);
				 objout(1,val[1]);
				 continue;
				}
			 if (opcd == 4)  /*  fdb  */
				{
				 eval(line.opr1,val);
				 if (echk(val) < 0)  continue;
				 highbyte = (val[1] >> 8) & 0377;
				 lobyte = val[1] & 0377;
				 lout2(highbyte,lobyte);
				 objout(2,highbyte,lobyte);
				 continue;
				}
			 if (opcd == 9)  /*  page  */
				{
				 putc('\014',iobuf3);
				 while (getc(iobuf1) != '\n');
				 continue;
				}
			 if (opcd == 11)  /*  spc  */
				{
				 eval(line.opr1,val);
				 if (echk(val) < 0)
					val[1] = 0;
				 for (i=0; i<val[1]; i++)
					 putc('\n',iobuf3);
				 while (getc(iobuf1) != '\n');
				 continue;
				}
			}
		}
	}
if (nolist == 0)
     outsym(iobuf3);
 if (cbyte != 0)
	flushobj();
 if (bin)
    {
     write(fildes4,"G",1);
     close(fildes4);
    }
 else
    {
     putc('S',iobuf4);
     putc('9',iobuf4);
     fflush(iobuf4);
     close(buf4.fildes);
    }
 close(buf1.fildes);
 close(buf2.fildes);
 close (buf3.fildes);
 unlink(tmpfil);
 exit();
}
/*
 *	read in from the temp file, and recreate the format
 *	that was used by parse(), i.e. convert the hex ascii used
 *	in temp file format back to integers.
 */
rdtmp()
{
 register int i;
 i = getw(iobuf2);
 if (i == -1) return(-1);

 linerd = i;
 action = getw(iobuf2);
 if (action <= 0)
	{
	 addr = getw(iobuf2);
	 return;
	}

 bytes = getw(iobuf2);
 addr = getw(iobuf2);
 opcd = getw(iobuf2);

 i = 0;
 while ((line.opr1[i++] = getc(iobuf2)) != '\n');
 line.opr1[--i] = '\0';

 i = 0;
 while ((line.opr2[i++] = getc(iobuf2)) != '\n');
 line.opr2[--i] = '\0';
}
/*
 *
 *	The following routines are used for creating
 *	the list file:
 *		errput()  --  lines with errors
 *		lout0()  --  lines without any code
 *		lout1()  --  lines with 1 byte of code
 *		lout2()  --  lines with 2 bytes of code
 *		lout3()  --  lines with 3 bytes of code
 *
 */
/*
 *	create listing for line that has an error in it.
 *	the listing will have 13 '*' in the code field.
 */
errput()
{
 extern struct filebuf buf3, *iobuf3;
 extern struct filebuf buf1, *iobuf1;
 register int i;
 char c;
 if (nolist) return;
 for (i = 0; i < 12; i++)
	 putc('*',iobuf3);
 putc('\t',iobuf3);
 while((c = getc(iobuf1)) != '\n')
	 putc(c,iobuf3);
 putc(c,iobuf3);
 return;
}
/*
 *	create listing for 0 byte line (eg. comments, pseudo-ops
 */
lout0()
{
 extern struct filebuf buf3, *iobuf3;
 extern struct filebuf buf1, *iobuf1;
 extern int prline;
 register int i;
 char c;
 if (nolist) return;
 for (i = 0; i < 12; i++)
	 putc(' ',iobuf3);
 if (prline == 0)
	{
	 putc('\n',iobuf3);
	 return;
	}
 putc('\t',iobuf3);
 while ((c = getc(iobuf1)) != '\n')
	 putc(c,iobuf3);
 putc(c,iobuf3);
 return;
}
/*
 *	create listing for 1 byte line
 */
lout1(arg1)
int arg1;
{
 extern char linebuf[];
 extern struct filebuf buf3, *iobuf3;
 extern struct filebuf buf1, *iobuf1;
 extern int addr, prline;
 register int i;
 char c;
 if (nolist) return;
 itohf(arg1,&linebuf[3]);
 itohf(addr,linebuf);
 linebuf[4] = ' ';
 for (i=7; i<12; i++)
	 linebuf[i] = ' ';
 for (i = 0; i < 12; i++)
	 putc(linebuf[i],iobuf3);
 if (prline == 0)
	{
	 putc('\n',iobuf3);
	 return;
	}
 putc('\t',iobuf3);
 while((c = getc(iobuf1)) != '\n')
	 putc(c,iobuf3);
 putc(c,iobuf3);
}
/*
 *	create listing for 2 byte line
 */
lout2(arg1,arg2)
int arg1,arg2;
{
 extern char linebuf[];
 extern struct filebuf buf3, *iobuf3;
 extern struct filebuf buf1, *iobuf1;
 extern int addr, prline;
 register int i;
 char c;
 if (nolist) return;
 itohf(arg2,&linebuf[6]);
 linebuf[7] = ' ';
 itohf(arg1,&linebuf[3]);
 itohf(addr,linebuf);
 linebuf[4] = ' ';
 for (i=10; i<12; i++)
	 linebuf[i] = ' ';
 for (i=0; i<12; i++)
	 putc(linebuf[i],iobuf3);
 if (prline == 0)
	{
	 putc('\n',iobuf3);
	 return;
	}
 putc('\t',iobuf3);
 while ((c = getc(iobuf1)) != '\n')
	 putc(c,iobuf3);
 putc(c,iobuf3);
}
/*
 *	create listing for 3 bytes of code generated by 1 instruction
 */
lout3(arg1,arg2)
int arg1,arg2;
{
 extern char linebuf[];
 extern struct filebuf buf3, *iobuf3;
 extern struct filebuf buf1, *iobuf1;
 extern int addr;
 register int i;
 char c;
if (nolist) return;
 itohf(arg2,&linebuf[8]);
 linebuf[7] = ' ';
 itohf(arg1,&linebuf[3]);
 itohf(addr,linebuf);
 linebuf[4] = ' ';
 linebuf[12] = ' ';
 for (i=0; i<12; i++)
	 putc(linebuf[i], iobuf3);
 putc('\t',iobuf3);
 while ((c = getc(iobuf1)) != '\n')
	 putc(c,iobuf3);
 putc(c,iobuf3);
}
/*
 *	convert integer to hexadecimal ascii.
 */
itohf(num,string)
char string[];
int num;
{
 register int i;
 string[0] = (num & 0170000) >> 12;
 string[0] = (string[0] & 017) + 060;  /*  sign extension probs  */
 string[1] = 060 + ((num & 07400) >> 8);
 string[2] = 060 + ((num & 0360) >> 4);
 string[3] = 060 + (num & 017);
 for (i=0; i<4; i++)
	if (string[i] > 071)
		string[i] =+ 07;
 return;
}

/*	These three dummy routines simply decide which format the code
 *	output is to be in based on the value of the global variable
 *	'bin'.  This is easier than tracking down the places they are
 *	called from.  Objs9, puts9, and flushs9 are the routines that
 *	create the S1...S9 format code, and Objbg, putbg, and flushbg
 *	create B...G type code.
 */

objout(n,b1,b2,b3)
{
 if (bin)
     objbg(n,b1,b2,b3);
 else
     objs9(n,b1,b2,b3);
}


putin(arg1)
{
 if (bin)
     putbg(arg1);
 else
     puts9(arg1);
}

flushobj()
{
 if (bin)
     flushbg();
 else
     flushs9();
}
/*
 *	objs9(), puts9(), and flushs9() are used to create the
 *	object code in minibug format.  objs9() is called  from
 *	pass2(), as it creates the object code.
 *	in objs9(n,b1,b2,b3), n is the number of bytes of object
 *	code to be inserted, and b1, b2, and b3 are the codes.
 */
objs9(n,b1,b2,b3)
int n,b1,b2,b3;
{
 extern int tbyte, cbyte, csum, lastloc, addr;
 extern struct filebuf buf4, *iobuf4;
 extern char outbuf[];
 char con[4];
 if (tbyte == 0)
	{
	 cbyte = 0;
	 outbuf[0] = 'S';
	 outbuf[1] = '1';
	}
 if (cbyte == 0)
	{
	 if (tbyte != 0)
		 outbuf[1] = '1';
	 itohf(addr,&outbuf[4]);
	 cbyte = 3;
	 csum = (addr >> 8) + addr;
	 lastloc = addr - 1;
	}
 if (++lastloc != addr)
	{
	 flushs9();
	 lastloc = addr;
	 outbuf[1] = '1';
	 itohf(addr,&outbuf[4]);
	 cbyte = 3;
	 csum = (addr >> 8) + addr;
	}
 puts9(b1);
 switch(n-1)
	{
	 case 1:
		puts9(b2);
		break;
	 case 2:
		puts9(b2);
		puts9(b3);
	}
lastloc =+ n-1;
 if (cbyte > 18)
	flushs9();
 return;
}
flushs9()
{
 extern int tbyte, cbyte, csum;
 extern struct filebuf buf4, *iobuf4;
 extern char outbuf[];
 char con[4];
 register int i, j;
 itohf(cbyte, con);
 outbuf[2] = con[2];
 outbuf[3] = con[3];
 csum =+ cbyte;
 cbyte++;
 csum = ~csum;
 itohf(csum, con);
 i = (cbyte << 1);
 outbuf[i++] = con[2];
 outbuf[i++] = con[3];
 outbuf[i] = '\n';
 for (j=0; j <= i; j++)
	putc(outbuf[j], iobuf4);
 cbyte = 0;
 csum = 0;
 return;
}
puts9(arg1)
int arg1;
{
 extern int cbyte, tbyte, csum;
 extern char outbuf[];
 char con[4];
 register int i;
 itohf(arg1,con);
 tbyte++;
 cbyte++;
 i = (cbyte << 1);
 outbuf[i++] = con[2];
 outbuf[i++] = con[3];
 csum =+ arg1;
 return;
}


/*	These are the B...G format output routines
 */

objbg(n,b1,b2,b3)
{
 struct {
     char lobyte,hibyte;
     } *p;

 if (tbyte == 0)
    {
     cbyte = 0;
     outbuf[0] = 'B';
    }

 if (cbyte == 0)
    {
     p = &addr;
     outbuf[2] = p->hibyte;
     outbuf[3] = p->lobyte;
     cbyte = 4;
     csum = (addr >> 8) + addr;
     lastloc = addr - 1;
    }

 if (++lastloc != addr)
    {
     flushbg();
     lastloc = addr;
     p = &addr;
     outbuf[2] = p->hibyte;
     outbuf[3] = p->lobyte;
     cbyte = 4;
     csum = (addr >> 8) + addr;
    }

 putbg(b1);
 switch(n-1)
    {
     case 1:
	 putbg(b2);
	 break;

     case 2:
	 putbg(b2);
	 putbg(b3);
    }

 lastloc =+ n-1;
 if (cbyte > 255)
     flushbg();
}


flushbg()
{
 outbuf[1] = cbyte - 5;
 csum =+ (cbyte - 5);
 outbuf[cbyte] = ~csum;
 write(fildes4,outbuf,cbyte+1);
 cbyte = 0;
 csum = 0;
}


putbg(arg1)
{
 outbuf[cbyte++] = arg1;
 tbyte++;
 csum =+ arg1;
}


/*
 *	check answer from eval(), and print appropriate error message
 */
echk(ans)
int ans[];
{
 if (ans[0] == 0)  return(1);
 if (ans[0] < 0)
	errmes(-5);
 if (ans[0] > 0)
	errmes(-14);
 errput();
 return(-1);
}
/*
 *	compar two strings, return -1, 0, or 1
 *	if s1<s2, s1==s2, or s1>s2, respectively
 *	This routine is used by teh qsort called in pass2() to
 *	sort the symbol table.
 */
comp(s1,s2)
char *s1, *s2;
{
 while ((*s1 != '\0') && (*s2 != '\0'))
	{
	 if (*s1 > *s2)  return(1);
	 if (*s1++ < *s2++)  return(-1);
	}
 if (*s1 == '\0')
	{
	 if (*s2 == '\0')  return(0);
	 return(-1);
	}
 return(1);
}
/*
 *	print out symbol table
 */
outsym(iobuf)
struct buf *iobuf;
{
 struct symtab *head, *last, *p;
 int i,j,k;

 /*  make one big linked list out of hash table */
 head = last = 0;

 for(i=0; i<HASHMOD; i++)
    {
     /* if list empty, ignore */
     if (hashtab[i] == 0) continue;

     /* check for header */
     if (head == 0) head = hashtab[i];

     /* put in old link to head */
     if (last != 0) last->nextp = hashtab[i];

     /* save tail of current list */
     last = endhash[i];
    }

 /*  sort the list  */
 head = sort(head);

 /* print it */
 putc('\n',iobuf);
 j = 0;
 p = head;

 while(p != 0)
    {
     for (k=0; p->symname[k] != '\0'; k++)
	 linebuf[j++] = p->symname[k];

     while(k++ < 10)
	 linebuf[j++] = ' ';

     itohf(p->symval,&linebuf[j]);
     linebuf[j-1] = '=';
     j =+ 4;

     if (j >= 75)
	{
	 linebuf[j] = '\n';
	 for (k=0; k <= j; k++)
	     putc(linebuf[k],iobuf);
	 j = 0;
	 p = p->nextp;
	 continue;
	}

     linebuf[j++] = ' ';
     linebuf[j++] = ' ';

     p = p->nextp;
    }

 for (k=0; k < j; k++)
	 putc(linebuf[k],iobuf);

 putc('\n',iobuf);
 fflush(iobuf);
}


/*  sort routine  */

sort(head)
struct symtab *head;
{
 struct symtab *s1, *i, *lasti;
 register struct symtab *s, *j, *lastj;

 i = head;
 lasti = 0;

 while(i != 0)
    {
     lastj = lasti;
     j = i;
     s = s1 = 0;

     while(j != 0)
	{
	 if (s == 0)
	    {
	     s = j;
	     s1 = lastj;
	     continue;
	    }

	 if (comp(s->symname,j->symname) > 0)
	    {
	     /* switch smallest */
	     s = j;
	     s1 = lastj;
	    }

	 lastj = j;
	 j = j->nextp;
	}

    /*  unlink element */
    if (s != i)
	{
	 if (s1 != 0)
	     s1->nextp = s->nextp;

     /* link onto front of unsorted part */

	 s->nextp = i;
	 if (lasti == 0)
	     head = s;
	 else
	     lasti->nextp = s;

	}

     if (lasti == 0)
	 lasti = head;
     else
	 lasti = lasti->nextp;
     i = lasti->nextp;
    }
 return(head);
}
