#define GETCP 13
#define SETPID 14
#define SETCOL 15
#define SETWIDTH 16
#define SETSTYL 17
#define SETINT 18
#define MOVE 19
#define LINE 20
#define TEXT 21
#define MARK 23
#define SETFONT 24
#define SETSIZE 25
#define SETANGLE 26
#define TRUE 1
#define FALSE 0
#define QUESTION 63
#define MINASCII 32
#define MAXASCII 127
#define LOW 0
#define MEDIUM 1
#define RETAIN 1
#define XLATE2 2
#define XFORM2 3
#define SEGNUM 5
#define MAXVSURF 2
#define VSURFNUM 2
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define UPRIGHT 4
#define DOWNRIGHT 5
#define DOWNLEFT 6
#define UPLEFT 7
#define PENUP 8
#define PENDOWN 9
#define UP2 10
#define DOWN2 11
#define LEFT2 12
#define RIGHT2 13
#define STOP 14
#define DOWN4 15
#define SOLID 0
#define DOTDASHED 3
#define PDFENDSEGMENT 0
#define PDFMOVE 1
#define PDFLINE 2
#define PDFTEXT 4
#define PDFMARKER 6
#define PDFCOLOR 11
#define PDFINTENSITY 12
#define PDFLINESTYLE 13
#define PDFLINEWIDTH 14
#define PDFFONT 15
#define PDFSIZE 16
#define PDFSPACE 17
#define PDFCHARQUALITY 18
#define PDFPICKID 20
#define SHORT 1
#define FLOAT 4
#define EMPTY -2
#define DELETED -1
#define NORETAIN 0
#define CLEAR 0
#define INITIAL 1
#define TERMINATE 10
#define NULL 0
#define NDCSP2 12
#define DELETE 5
#define OPENSEG 9
#define CLOSEG 11
#define SETVSBL 2
#define SETHILIT 3
#define SETDTCT 4
#define ROTATE2 6
#define SCALE2 7
#define TRANS2LATE 8

extern int intflag,clrflag,lsflag,lwflag,fntflag,szeflag,angflag,qualflag,picflag,cpchang,idenflag;

extern struct aspect
   {
   float width;
   float height;
   };

extern struct worldcor
   {
   float dx;
   float dy;
   };

extern struct primattr
   {
   float color[3];
   float intensty;
   float linwidth;
   int linestyl;
   int font;
   struct aspect charsize;
   struct worldcor chrspace;
   int chqualty;
   int pick2id;   /*** VERSION 7: PICKID ***/
   };

extern struct primattr current;
extern struct primattr defaultt;
extern struct primattr minimum;
extern struct primattr maximum;
extern struct primattr pdfcurrent;

extern struct viewsurf
   {
   char *name;
   int imupdate;   /*******************************************/
   int vshardwr;   /*** can support visibility              ***/
   int hlhardwr;   /*** can support highlighting            ***/
   int dthardwr;   /*** can support detectability           ***/
   int trhardwr;   /*** can support translation             ***/
   int schardwr;   /*** can support scaling                 ***/
   int rohardwr;   /*** can support rotation                ***/
   int dehardwr;   /*** can support deleting a segment      ***/
   int idhardwr;   /*** can support primitive pick id       ***/
   int lshardwr;   /*** can support line style              ***/
   int lwhardwr;   /*** can support line width              ***/
   int inhardwr;   /*** can support intensity               ***/
   int clhardwr;   /*** can support color                   ***/
   int txhardwr;   /*** can support hardware text generation***/
   int erasure;    /*** deletes by redrawing with 0 intensity and color ***/
   int segopclo;   /*** needs to have segments opened and closed ***/
   int nwframdv;   /*** new frame action required when change is made ***/
   int nwframnd;   /*** new frame action needed when end BOU***/
   int vinit;      /***                                     ***/
   int selected;   /*******************************************/
   int (*dev1drive)();   /*** VERSION 7: DEVDRIVE ***/
   } surface[MAXVSURF];

extern struct viewsurf *surfptr;
extern struct viewsurf *recentvs;

extern struct segstruc
   {
   char *seg1name;     /*** VERSION 7: SEGNAME ***/
   int type;
   int visbil1ty;      /********************************************/
   int detect1bl;      /*** THREE VARIABLES BOOL UNDER VERSION 7 ***/
   int high1lght;      /*** ALSO DELETE NUMBERS WITHIN NAMES     ***/
   float scale1[2];     /********************************************/
   float trans1lat[2];
   float rotate1;
   int vsurfnum;           /*** = initial; IN VERSION 7 ***/
   struct viewsurf *vsurfptr[VSURFNUM];
   int pdfptr;
   int redraw;
   int segsize;
   } segment[SEGNUM];


extern struct segstruc *openseg;
extern int opsegemp; /*** indicates whether open segment is empty still ***/
extern int osexists;               /*** BOOL UNDER VERSION 7 ***/

extern float cp[2];
extern float viewup[2],vwxform[2][2],imxform[3][2];
extern int wndwclip,vwrotate;
extern stdmarkr;

extern struct            /********************************************/
   {                     /*  structure for passing arguments to the  */
   int opcode;           /*  device drivers, which will use only the */
   int logical;          /*  parts they need.                        */
   char *string;         /********************************************/
   int int1;
   float float1;
   float float2;
   float float3;
   } ddstruct;

extern struct
   {
   float xmin;
   float xmax;
   float ymin;
   float ymax;
   } viewport;

extern struct
   {
   float xmin;
   float xmax;
   float ymin;
   float ymax;
   } window;

extern int batchupd;

extern struct segattr
   {
   int visbil2ty;   /********************************************/
   int detect2bl;   /*** THREE VARIABLES BOOL UNDER VERSION 7 ***/
   int high2lght;   /*** ALSO DELETE NUMBERS WITHIN NAMES.    ***/
   float scale2[2]; /********************************************/
   float trans2lat[2];
   float rotate2;
   } defsegat;


extern int dfaltset;  /*** TWO VARIABLES BOOLEAN ***/
extern int prevseg;   /*** UNDER VERSION 7       ***/

extern int coordsys;
extern float ndcspace[2];

extern struct segstruc *segptr;
extern int segnum;

extern int sysinit;
extern int slectnxt;

extern int csegtype;
extern int ndcspuse; /*** BOOL UNDER VERSION 7 ***/
extern int pdfnext;  /*** points to next free byte in pdf ***/

extern short ptype;

extern int sgtypmin,sgtypmax;
extern int rastererase; /*** flag to let segdraw know not to change color ***/
			/*** or intensity because doing raster erasure    ***/
extern int explicit; /*** BOOL UNDER VERSION 7 ***/

extern int pdfd;

extern int ndcinvkd,vfinvokd; /*** BOOL UNDER VERSION 7 ***/

extern float wldsidx,wldsidy,ndcsidx,ndcsidy;

extern int corsyset;

extern int erreport;
