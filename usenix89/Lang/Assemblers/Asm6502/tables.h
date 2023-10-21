
static char *opcode[] = {
 "brk","ora","002","003","004","ora","asl","007","php","ora","asl","011",
 "012","ora","asl","015","bpl","ora","018","019","020","ora","asl",
 "023","clc","ora","026","027","028","ora","asl","031","jsr","and","034",
 "035","bit","and","rol","039","plp","and","rol","043","bit","and",
 "rol","047","bmi","and","050","051","052","and","rol","055","sec","and",
 "058","059","060","and","rol","063","rti","eor","066","067","068","eor",
 "lsr","071","pha","eor","lsr","075","jmp","eor","lsr","079","bvc","eor",
 "082","083","084","eor","lsr","087","cli","eor","090","091","092","eor",
 "lsr","095","rts","adc","098","099","100","adc","ror","103","pla","adc",
 "ror","107","jmp","adc","ror","111","bvs","adc","114","115","116","adc",
 "ror","119","sei","adc","122","123","124","adc","ror","127","128","sta",
 "130","131","sty","sta","stx","135","dey","137","txa","139","sty","sta",
 "stx","143","bcc","sta","146","147","sty","sta","stx","151","tya","sta",
 "txs","155","156","sta","158","159","ldy","lda","ldx","163","ldy","lda",
 "ldx","167","tay","lda","tax","171","ldy","lda","ldx","175","bcs","lda",
 "178","179","ldy","lda","ldx","183","clv","lda","tsx","187","ldy","lda",
 "ldx","191","cpy","cmp","194","195","cpy","cmp","dec","199","iny","cmp",
 "dex","203","cpy","cmp","dec","207","bne","cmp","210","211","212","cmp",
 "dec","215","cld","cmp","218","219","220","cmp","dec","223","cpx","sbc",
 "226","227","cpx","sbc","inc","231","inx","sbc","nop","235","cpx","sbc",
 "inc","239","beq","sbc","242","243","244","sbc","inc","247","sed","sbc",
 "250","251","252","sbc","inc","255"
};

static short mode[] = {
  0,10,13,13,13,                          3,3,13,0,2,
  1,13,13,6,6,                            13,9,11,13,13,
  13,4,4,13,0,                            8,13,13,13,7,
  7,13,6,10,13,                           13,3,3,3,13,
  0,2,1,13,6,                             6,6,13,9,11,
  13,13,13,4,4,                           13,0,8,13,13,
  13,7,7,13,0,                            10,13,13,13,3,
  3,13,0,2,1,                             13,6,6,6,13,
  9,11,13,13,13,                          4,4,13,0,8,
  13,13,13,7,7,                           13,0,10,13,13,
  13,3,3,13,0,                            2,1,13,12,6,
  6,13,9,11,13,                           13,13,4,4,13,
  0,8,13,13,13,                           7,7,13,13,10,
  13,13,3,3,3,                            13,0,13,0,13,
  6,6,6,13,9,                             11,13,13,4,4,
  5,13,0,8,0,                             13,13,7,13,13,
  2,10,2,13,3,                            3,3,13,0,2,
  0,13,6,6,6,                             13,9,11,13,13,
  4,4,5,13,0,                             8,0,13,7,7,
  8,13,2,10,13,                           13,3,3,3,13,
  0,2,0,13,6,                             6,6,13,9,11,
  13,13,13,4,4,                           13,0,8,13,13,
  13,7,7,13,2,                            10,13,13,3,3,
  3,13,0,3,0,                             13,6,6,6,13,
  9,11,13,13,13,                          4,4,13,0,8,
  13,13,13,7,7,13
};

static short length[] = {  1,1,2,2,2,2,3,3,3,2,2,2,3,1 };

static char *before[] = {
  ""," a "," #$"," $"," $"," $"," $"," $"," $"," $",
  " ($", " ($", " ($",""
};

static char *after[] = {
  "        ","     ","   ","    ",
 ",x  ",",y  ","  ",",x",",y","  ",",x)","),y",")","        "
};

static char *zero_page[] = {
  "D6510 -- 6510 Data Direction Register",  /*  0 */
  "R6510 -- 6510 Data Register",            /*  1 */
  "BANK -- token 'search' looks for, or bank #", /* 2 */
  "PC_HI -- Address for BASIC sys command or monitor and long call/jump routines", /* 3 */
  "PC_LO -- Address for BASIC sys or monitor long call/jumps", /* 4 */
  "S_REG -- status register temp",		/* 5 */
  "A_REG -- .A reg temp",			/* 6 */
  "X_REG -- .X reg temp",			/* 7 */
  "Y_REG -- .Y reg temp",			/* 8 */
  "STKPTR,INTEGR,CHARAC -- Stack pointer temp, BASIC search character",	/* 9 */
  "ENDCHR -- flag: scan for quote at end of string", /* a */
  "TRMPOS -- Screen column from last tab",	/* b */
  "VERCK -- flag: 0=load, 1=verify",		/* c */
  "COUNT -- Input buf. ptr/# of subscripts",	/* d */
  "DIMFLG -- Flag: default array dimension",	/* e */
  "VALTYP -- Data type: $ff=string,$00=numeric", /* f */
  "INTFLG -- Data type: $00 = float,$80= integer",	      /* 10 */   
  "GARBFL -- flag: data scan/list quote/garbage collection",  /* 11 */
  "DORES,SUBFLG -- flag: subscript ref./user func. call",     /* 12 */
  "INPFLG -- Flag: $00=input,$40=get,$98=read",		      /* 13 */
  "DOMASK,TANSGN -- ??, Flag: tan sign/comparision result",   /* 14 */
  "CHANNL",						      /* 15 */
  "POKER,LINNUM -- temp integer value",			      /* 16 */
  "POKER+1,LINNUM+1 -- temp integer value",		      /* 17 */
  "TEMPPT -- pointer: temp string stack",		      /* 18 */
  "LASTPT -- last temp string address",			      /* 19 */
  "LASTPT+1 -- last temp string address",		      /* 1a */
  "TEMPST -- stack for temp strings",			      /* 1b */
  "TEMPST+1 -- stack for temp strings",			      /* $1C */
  "TEMPST+2 -- stack for temp strings",			      /* $1D */
  "TEMPST+3 -- stack for temp strings",			      /* $1E */
  "TEMPST+4 -- stack for temp strings",			      /* $1F */
  "TEMPST+5 -- stack for temp strings",			      /* $20 */
  "TEMPST+6 -- stack for temp strings",			      /* $21 */
  "TEMPST+7 -- stack for temp strings",			      /* $22 */
  "TEMPST+8 -- stack for temp strings",			      /* $23 */
  "INDEX -- utility pointer area",			      /* $24 */
  "INDEX+1 -- utility pointer area",			      /* $25 */
  "INDEX2 -- utility pointer area",			      /* 26 */
  "INDEX2+1 -- utility pointer area",			      /* 27 */
  "RESHO -- float. pt. product of multiply",		      /* 28 */
  "RESMOH -- float. pt. product of multiply",		      /* 29 */
  "ADDEND -- float pt. product of multiply",		      /* 2a */
  "RESMO,RESLO -- float point product of multiply",	      /* 2b */
  "??? -- floating point something",			      /* 2c */
  "TXTTAB -- pointer: start of BASIC text",		      /* 2d */
  "TXTTAB+1 -- pointer: start of BASIC text",		      /* 2e */
  "VARTAB -- pointer: start of BASIC variables",	      /* 2f */
  "VARTAB+1 -- pointer: start of BASIC variables",	      /* 30 */
  "ARYTAB -- pointer: start of BASIC arrays",		      /* 31 */
  "ARYTAB+1 -- pointer: start of BASIC arrays",		      /* 32 */
  "STREND -- pointer: end of bASIC arrays + 1",		      /* 33 */
  "STREND+1 -- pointer: end of BASIC arrays +1",	      /* 34 */
  "FRETOP -- pointer: bottom of string storage",	      /* 35 */
  "FRETOP+1 -- pointer: bottom of string storage",	      /* 36 */
  "FRESPC -- utility string pointer",			      /* 37 */
  "FRESPC+1 -- utility string pointer +1",		      /* 38 */
  "MAX_MEM_1 -- top of string/variable bank (bank 1)",	      /* 39 */
  "MAX_MEM_1+1 -- top of string/variable bank (bank 1)",      /* 3a */
  "CURLIN -- current BASIC line number",		      /* 3b */
  "CURLINE+1 --current BASIC line number",		      /* 3c */
  "TXTPTR  -- pointer to BASIC text used by CHRGET etc.",     /* 3d */
  "TXTPTR+1 -- pointer to BASIC text used by CHRGET etc.",    /* 3e */
  "FORM,FNDPNT -- used by print using, pointer to item found by search.", /* 3f  */
  "FORM+1,FNDNT+1 -- used by print using, pointer to item found by search.", /* 40 */
  "DATLIN -- current data line number",			      /* 41 */
  "DATLIN+1 -- current data line number",		      /* 42 */
  "DATPTR -- current data line address",		      /* 43 */
  "DATPTR+1 -- current data line address",		      /* 44 */
  "INPPTR -- Vector: input routine",			      /* 45 */
  "INPPTR+1 -- Vector: input routine",			      /* 46 */
  "VARNAM -- current BASIC variable name",		      /* 47 */
  "VARNAM+1 -- current BASIC variable name",		      /* 48 */
  "FDECPT,VARPNT -- ???, Pointer: current BASIC variable data",	/* 49 */
  "FDECPT+1,VARPNT+1 -- ???, Pointer: current BASIC variable data", /* 4a */
  "LSTPNT,FORPNT -- ???, pointer: index variable for for/next",	/* 4b */
  "ANDMSK,EORMSK",					      /* 4c */
  "VARTXT,OPPTR",					      /* 4d */
  "VARTXT+1,OPPTR+1",					      /* 4e */
  "OPMASK",						      /* 4f */
  "GRBPNT,TEMPF3,DEFPNT",				      /* 50 */
  "GRBPNT+1,TEMPF3+1,DEFPNT+1",				      /* 51 */
  "DSCPNT",						      /* 52 */
  "DSCPNT+1",						      /* 53 */
  "???",						      /* 54 */
  "HELPER -- flags 'help' or 'list'",			      /* 55 */
  "JMPER",						      /* 56 */
  "JMPER+1???",						      /* 57 */
  "OLDOV",						      /* 58 */
  "TEMPF1,PTARG1 -- multiply defined for instr.",	      /* 59 */
  "ARYPNT,HIGHDS",					      /* 5a */
  "PTARG2",						      /* 5b */
  "PTARG2+1,HIGHTR",					      /* 5c */
  "STR1",						      /* 5d */
  "STR1+1,TEMPF2",					      /* 5e */
  "STR1+2,DECCNT -- Number of digits after the decimal point", /* 5f */
  "STR2,TENEXP,T0 -- ??,??, ML Monitor Z.P. storeage in FAC", /* 60 */
  "STR2+1,GRBTOP,DPTFLG,LOWTR -- ??,??, Decimal point flag, ??", /* 61 */
  "STR2+2,EXPSGN -- ??, part of FAC???",		      /* 62 */
  "POSITN,DSCTMP,LEFT_FLAG,FACEXP,T1 -- ??,??,paint-lft flg,FAC#1 exp.,mon. thingy", /* 63 */
  "MATCH,RIGHT_FLAG,FACHO -- ??,paint-right flag,FAC#1 Mantissa", /* 64 */
  "FACMOH -- FAC#1 mantissa high order",		      /* 65 */
  "INDICE,FACMO,T2: ??,FAC#1 mantissa,mon. thingy",	      /* 66 */
  "FACLO -- FAC#1 mantissa",				      /* 67 */
  "FACSGN -- FAC#1 sign",				      /* 68 */
  "DEGREE,SGNFLG: ??,pointer:series-eval constant",	      /* 69 */
  "ARGEXP -- FAC#2 exponent",				      /* 6a */
  "ARGHO -- FAC#2 MAntissa",				      /* 6b */
  "ARGMOH,INIT_AS_0 -- FAC#2 mantissa, just a count for init", /* 6c */
  "ARGMO -- FAC#2 mantissa middle order",		      /* 6d */
  "ARGLO -- FAC#2 mantissa lo order",			      /* 6e */
  "ARGSGN -- FAC#2 sign",				      /* 6f */
  "STRNG1,ARISGN -- ??,sign comparison result: FAC#1 vs #2",  /* 70 */
  "FACOV  -- FAC#1 low-order (rounding)",		      /* 71 */
  "STRNG2,POLYPT,CURTOL -- ?? FBUFPT -- ptr: cassette buffer", /* 72 */
  "STRNG2+1,POLYPT+1,CURTOL+1-- ?? FBUFPT+1 -- ptr: cassette buffer", /* 73 */
  "AUTINC -- incrememt val. for auto (0=off)",		      /* 74 */
  "AUTINC+1 -- incrememt val. for auto (0=off)",	      /* 75 */
  "MVDFLG -- flag if 10k hires area allocated",		      /* 76 */
  "Z_P_TEMP_1 -- print using's leading zero cnter, sprite temp, mid$ temp", /* 77 */
  "HULP,KEYSIZ -- counter,??",				      /* 78 */
  "SYNTMP -- used as temp for indirect loads",		      /* 79 */
  "DSDESC,TXTPTR -- descriptor for ds$, monitor ZP storage",  /* 7a */
  "DSDESC+1,TXTPTR+1 -- descriptor for ds$, monitor ZP storage", /* 7b */
  "DSDESC+2,TXTPTR+2 -- descriptor for ds$, monitor ZP storage", /* 7c */
  "TOS -- top of run time stack",			      /* 7d */
  "TOS+1 -- top of run time stack",			      /* 7e */
  "RUNMOD -- flags run/direct mode",			      /* 7f */
  "PARSTS -- DOS parser status word  POINT -- using's pointer to dec.pt", /* 80 */
  "PARSTX",						      /* 81 */
  "OLDSTK",						      /* 82 */
  "COLSEL  -- current color selected",			      /* 83 */
  "MULTICOLOR_1",					      /* 84 */
  "MULTICOLOR_2",					      /* 85 */
  "FOREGROUND",						      /* 86 */
  "SCALE_X -- Scale factor in X",			      /* 87 */
  "SCALE_X+1 -- Scale factor in X",			      /* 88 */
  "SCALE_Y -- Scale factor in Y",			      /* 89 */
  "SCALE_Y+1 -- Scale factor in Y",			      /* 8a */
  "STOPNB -- stop paint if not background/not same color",    /* 8b */
  "GRAPNT",						      /* 8c */
  "GRAPNT+1",						      /* 8d */
  "VTEMP1",						      /* 8e */
  "VTEMP2",						      /* 8f */
  "STATUS -- I/O operation status byte",		      /* 90 */
  "STKEY -- stop key flag",				      /* 91 */
  "SVXT -- tape temporary",				      /* 92 */
  "VERCK -- load or verify flag",			      /* 93 */
  "C3PO -- serial buffered char flag",			      /* 94 */
  "BSOUR -- char buffer for serial",			      /* 95 */
  "SYNO -- Cassette sync #",				      /* 96 */
  "XSAV -- temp for BASIN",				      /* 97 */
  "LDTND -- index to logical file",			      /* 98 */
  "DFLTN -- default input device #",			      /* 99 */
  "DFLTO -- default output device #",			      /* 9a */
  "PRTY -- cassette parity",				      /* 9b */
  "DPSW -- cassette dipole switch",			      /* 9c */
  "MSGFLG -- OS message flag",				      /* 9d */
  "PTR1,T1 -- cassette error pass1, temporary 1",	      /* 9e */
  "PTR2,T2 -- cassette error pass 2, temporary 2",	      /* 9f */
  "TIME -- 24 hour clock in 1/60th seconds",		      /* a0 */
  "TIME+1 -- 24 hour clock in 1/60th seconds",		      /* a1 */
  "TIME+2 -- 24 hour clock in 1/60th seconds",		      /* a2 */
  "R2D2 -- serial bus usage   PCNTR -- cassette",	      /* a3 */
  "BSOUR1,FIRT -- temp used by serial routine",		      /* a4 */
  "COUNT,CNTDN -- temp used by serial routine, cassette sync countdown", /* a5 */
  "BUFPT -- cassette buffer pointer",			      /* a6 */
  "INBIT,SHCNL -- RS232 rcvr input bit storage,cassette short count", /* a7 */
  "BITCI,RER -- RS232 rcvr bit count,cassette read error",    /* a8 */
  "RINONE -- RS232 rcvr flag for start bit check\n                                REZ -- Cassette reading zeros", /* a9 */
  "RIDATA,RDFLG -- RS232 rcvr byte buffer, cassette read mode",	  /* aa */
  "RIPRTY -- RS232 rcvr parity storage,  SHCNH -- Cassette short cnt.",	/* ab */
  "SAL -- pointer: tape buffer/screen scrolling",	      /* ac */
  "SAH -- pointer: tape buffer/screen scrolling",	      /* ad */
  "EAL -- tape end addresses / end of program",		      /* ae */
  "EAH -- tape end addresses / end of program",		      /* af */
  "CMP0 -- tape timing constants",			      /* b0 */
  "TEMP",						      /* b1 */
  "TAPE1 -- address of tape buffer",			      /* b2 */
  "TAPE1+1 -- address of tape buffer",			      /* b3 */
  "BITTS,SNSW1 -- RS232 trns bit count",		      /* b4 */
  "NXTBIT,DIFF -- RS232 TRNS next bit to be sent",	      /* b5 */
  "RODATA,PRP -- rs232 trns byte buffer",		      /* b6 */
  "FNLEN -- length current file name str",		      /* b7 */
  "LA -- current file logical addr",			      /* b8 */
  "SA -- current file 2ndary addr",			      /* b9 */
  "FA -- current file device addr",			      /* ba */
  "FNADR -- addr current file name str",		      /* bb */
  "FNADR+1 -- addr current file name str",		      /* bc */
  "ROPRTY,OCHAR -- rs232 trns parity buffer",		      /* bd */
  "FSBLK -- cassette read block count",			      /* be */
  "DRIVE,MYCH -- serial word buffer",			      /* bf */
  "CAS1 -- Cassette manual/cntrled switch (updated during irq)", /* c0 */
  "TRACK,STAL -- ??, I/O start address (lo)",		      /* c1 */
  "SECTOR,STAH -- ??, I/O start address (hi)",		      /* c2 */
  "MEMUSS,TMP2 -- Cassette load temps (2 bytes)",	      /* c3 */
  "MEMUSS,TMP2 -- Cassette load temps (2 bytes)",	      /* c4 */
  "DATA -- tape read/write data",			      /* c5 */
  "BA -- bank for current load/save/verify operation",	      /* c6 */
  "FNBANK -- bank where current FN is found (at 'fnadr')",    /* c7 */
  "RIBUF -- RS232 input buffer pointer",		      /* c8 */
  "RIBUF+1 -- RS232 input buffer pointer",		      /* c9 */
  "ROBUF -- rs232 output buffer pointer",		      /* ca */
  "ROBUF+1 -- rs232 output buffer pointer",		      /* cb */
  "KEYTAB -- keyscan table pointer",			      /* cc */
  "KEYTAB+1 -- keyscan table pointer",			      /* cd */
  "IMPARM -- PRIMM utility string pointer",		      /* ce */
  "IMPARM+1 -- PRIMM utility string pointer",		      /* cf */
  "NDX -- index to keyboard queue",			      /* d0 */
  "KYNDX -- pending function key flag",			      /* d1 */
  "KEYIDX -- index into pending function key string",	      /* d2 */
  "SHFLAG -- keyscan shift key status",			      /* d3 */
  "SFDX -- keyscan current key index",			      /* d4 */
  "LSTX -- keyscan last key index",			      /* d5 */
  "CRSW -- <CR> input flag",				      /* d6 */
  "MODE -- 40/80 column mode flag",			      /* d7 */
  "GRAPHM -- text/graphic mode flag",			      /* d8 */
  "CHAREN -- RAM/ROM vic character fetch flag (bit-2)",	      /* d9 */
  "KEYSIZ,SEDSAL -- pointers for movlin",		      /* da */
  "KEYLEN,BITMSK -- temporary for tab & line wrap routines",  /* db */
  "KEYNUM,SAVER -- another temporary place to save a reg.",   /* dc */
  "KEYNXT,SEDEAL",					      /* dd */
  "KEYBNK,SEDT1 -- savpos",				      /* de */
  "KEYTMP,SEDT2 -- ?savpos?",				      /* df */
  "PNT -- pointer to current line (text)",		      /* e0 */
  "PNT+1 -- pointer to current line (text)",		      /* e1 */
  "USER -- pointer to current line (attribute)",	      /* e2 */
  "USER+1 -- pointer to current line (attribute)",	      /* e3 */
  "SCBOT -- window lower limit",			      /* e4 */
  "SCTOP -- window upper limit",			      /* e5 */
  "SCLF -- window left margin",				      /* e6 */
  "SCRT -- window right margin",			      /* e7 */
  "LSXP -- current input column start",			      /* e8 */
  "LSTP -- current input line start",			      /* e9 */
  "INDX -- current input line end",			      /* ea */
  "TBLX -- current cursor line",			      /* eb */
  "PNTR -- current cursor column",			      /* ec */
  "LINES -- maximum number of screen lines",		      /* ed */
  "COLUMNS -- maximum number of screen columns",	      /* ee */
  "DATAX -- current data to print",			      /* ef */
  "LSTCHR -- previous char printed (for <esc> test)",	      /* f0 */
  "COLOR -- curr attribute to print (default fgnd color)",    /* f1 */
  "TCOLOR -- saved attrib to print ('insert' and 'delete')",  /* f2 */
  "RVS -- reverse mode flag",				      /* f3 */
  "QTSW -- quote mode flag",				      /* f4 */
  "INSRT -- insert mode flag",				      /* f5 */
  "INSFLG -- auto-insert mode flag",			      /* f6 */
  "LOCKS -- disables <shift><C=>,<ctrl>s",		      /* f7 */
  "SCROLL -- disables screen scroll, line linker",	      /* f8 */
  "BEEPER -- disables <ctrl>G",				      /* f9 */
  "user zpg",					/* $fa */
  "user zpg",					/* $fb */
  "user zpg",					/* $fc */
  "user zpg",					/* $fd */
  "user zpg",					/* $fe */
  "LOFBUF"
};
