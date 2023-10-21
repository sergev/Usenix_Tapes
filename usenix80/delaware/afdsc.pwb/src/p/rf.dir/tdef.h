#define NBLIST 768 /*	blocks available for allocation (3*256) */
#define BLK 128    /*	words per block */
#define SEEKDIST 128	/* size of a maximal seek in blocks */
#define MAXSEEK 32767	/* size of maximal seek in bytes */
#define ENDWORD 0777
#define FONT_CNTRL 003000
#define PT_CNTRL 074000
#ifdef NROFF /*	NROFF*/
#define EM t.Em
#define HOR t.Hor
#define VERT t.Vert
#define INCH 240 /*	increments per inch*/
#define SPS INCH/10 /*	space size*/
#define SS INCH/10 /*	 " */
#define TRAILER 0
#define UNPAD 0227
#define PO 0 /*page offset*/
#define ASCII 1
#define PTID 1
#define LG 0
#define DTAB 0 /*	set at 8 Ems at init time*/
#define ICS 2*SPS
#endif
#ifndef NROFF /*	TROFF*/
#define UNPAD 027
#define HOR 1
#define ASCII 0
#define PTID 0
#define LG 1
#define DTAB (INCH/2)
#define ICS 3*SPS
#endif

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGFPE 8
#define SIGKILL 15
#define SIGPIPE 13
#define ECHO 010 /*	tty echo mode*/
#define NARSP 0177 /*	narrow space*/
#define HNSP 0226 /*	half narrow space*/
#define PS 10 /*	default point size*/
#define FT 0 /*	default font position*/
#define VS INCH/6 /*	vert space; 12points*/
#define NN 132 /*	number registers*/
#define NNAMES 14 /*predefined reg names*/
#define NIF 5 /*	if-else nesting*/
#define NS 64 /*	name buffer*/
#define NTM 256 /*	tm buffer*/
#define NEV 3 /*	environments*/
#define EVLSZ 10 /*	size of ev stack*/
#define EVS 3*256 /*	environment size in words*/
#define NM 250 /*	requests + macros*/
#define DELTA 512 /*	delta core bytes*/
#define STKSIZE 11 /*	words*/
#define NHYP 10 /*	max hyphens per word*/
#define NHEX 128 /*	byte size of exception word list*/
#define NTAB 35 /*	tab stops*/
#define NSO 5 /*	"so" depth*/
#define WDSIZE 170 /*	word buffer size*/
#define LNSIZE 480 /*	line buffer size*/
#define NDI 5 /*	number of diversions*/
#define MOT 0100000 /*	motion character indicator*/
#define MOTV 0170000 /*	clear for motion part*/
#define VMOT 0040000 /*	vert motion bit*/
#define MAXMOT 07777 /*	 maximum allowable motion in motion char */
#define NMOT 0020000 /*	 negative motion indicator*/
#define RMOT 0010000 /*	 rule motion indicator */
#define MMASK 0100000 /*	macro mask indicator*/
#define CMASK 0100377
#define RULE 1
#define NRULE 0
#define RSIZE 7 /*	size of rule thickness in units */
#define HM 0
#define VM 1
#define UL 3 /* size of vertical motion for underrule */
#define VRI 02
#define HRI 01
#define MAXESC 13107 /*	 maximum esc motion per pty command */
#define MESCHAR  MAXESC*5 /*	 maximum escape per pty command */
#define ZBIT 0400 /*	zero width char*/
#define BMASK 0377
#define BYTE 8
#define IMP 004 /*	impossible char*/
#define FILLER 037
#define PRESC 026
#define HX 0376 /*	High-order part of xlss*/
#define LX 0375 /*	low-order part of xlss*/
#define CONT 025
#define COLON 013
#define XPAR 030
#define ESC 033
#define FLSS 031
#define RPT 014
#define JREG 0374
#define NTRAP 20 /*	number of traps*/
#define NPN 20 /*	numbers in "-o"*/
#define T_PAD 0101 /*	cat padding*/
#define T_INIT 0100
#define T_IESC 16 /*initial offset*/
#define T_STOP 0111
#define NPP 10 /*	pads per field*/
#define FBUFSZ 256 /*	field buf size words*/
#define OBUFSZ 512 /*	bytes*/
#define IBUFSZ 512 /*	bytes*/
#define NC 256 /*	cbuf size words*/
#define NOV 10 /*	number of overstrike chars*/
#define LONG0 long /*	long flag for atoi0,1*/
#define ZONE 5 /*	5hrs for EST*/
#define TDELIM 032
#define LEFT 035
#define RIGHT 036
#define LEADER 001
#define TAB 011
#define TMASK  037777
#define RTAB 0100000
#define CTAB 0040000
#define OHC 024

/***********  videocomp 500 commands  ****************/
#define FULFACE 060 /*	 030  define page full face */
#define ROT90 065 /*	035 define location and rotate 90 degrees */
#define JOBID 020 /*	010 job id */
#define ENDREC 021 /*	011 end of record */
#define ENDPG 063 /*	 033  end page */
#define JOBID 020 /*	 010  job id  */
#define ENDRCD 021 /*	 011  end record */
#define FONTFCH 044 /*  024  font fetch */
#define SFCOM 0106 /*	 046  space forward */
#define ADCOM 0114 /*	 04C  advance  */
#define UPCOM 0116 /*	 04E  up       */
#define SHTAB 0122 /*	052  save horizontal tab */
#define MHTAB 0121 /*	051  move horizontal tab */
#define SRCOM 0144 /*  064  set rule */
