#define TRUE 1
#define FALSE 0
#define NULL 0
#define CLEAR 0
#define EMPTY -2
#define NORETAIN 0
#define RETAIN 1
#define XLATE2 2
#define XFORM2 3
#define PICK 0
#define KEYBOARD 1
#define BUTTON 2
#define LOCATOR 3
#define VALUATOR 4
#define LEFT 0
#define RIGHT 1
#define SEGNUM 5
#define MAXVSURF 2
#define VSURFNUM 2
#define PICKS 1
#define LOCATORS 1
#define KEYBORDS 1
#define VALUATRS 4
#define BUTTONS 8
#define ASSOCNUM 20
#define LOW 0
#define MEDIUM 1
#define HIGH 2
#define PMODE 0755  /*** RWE FOR OWNER,RE FOR GROUP, OTHERS ***/
#define SOLID 0
#define DOTTED 1
#define DASHED 2
#define DOTDASHED 3
#define INFINITY 1024

/****************************************************************************/
/*                                                                          */
/*     PROJECT: CORE SYSTEM GRAPHICS SUBROUTINE PACKAGE                     */
/*                                                                          */
/*     DEVELOPERS: MICHAEL T. GARRETT R531-8845s                            */
/*                 ANDREW G. GREENHOLT R531-8845s                           */
/*                                                                          */
/*     PURPOSE: DEVELOP A STANDARD GRAPHICS PACKAGE USING THE 'C'           */
/*              PROGRAMMING LANGUAGE. JUSTIFICATION FOR A 'CORE SYSTEM'     */
/*              IS THE PROMOTION OF PROGRAM PORTABILITY AND DEVICE          */
/*              INDEPENDENCE.                                               */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     C PREPROCESSOR EXTENSIONS                                            */
/*                                LAST NINE ARE TEMPORARY                   */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* #define CLEAR 0                                                          */
/* #define INITIAL 1                                                        */
/* #define TRUE 1                                                           */
/* #define FALSE 0                                                          */
/* #define NULL 0                                                           */
/* #define NORETAIN 0                                                       */
/* #define RETAIN 1                                                         */
/* #define XLATE2 2                                                         */
/* #define XFORM2 3                                                         */
/* #define PICK 0                                                           */
/* #define KEYBOARD 1             VERSION 7'S DEFINES WILL                  */
/* #define BUTTON 2               BE PLACED HERE!                           */
/* #define LOCATOR 3                                                        */
/* #define VALUATOR 4                                                       */
/* #define LEFT 0                                                           */
/* #define RIGHT 1                                                          */
/* #define SEGNUM 5                                                         */
/* #define MAXVSURF 2                                                       */
/* #define VSURFNUM 2                                                       */
/* #define PICKS 1                                                          */
/* #define LOCATORS 1                                                       */
/* #define KEYBORDS 1                                                       */
/* #define VALUATRS 4                                                       */
/* #define BUTTONS 8                                                        */
/* #define ASSOCNUM 20                                                      */
/* #define LOW 0                                                            */
/* #define MEDIUM 1                                                         */
/* #define HIGH 2                                                           */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     COMMON DATA AREA                                                     */
/*                                                                          */
/****************************************************************************/

/***************************************************/
/*** typedef int bool;  VERSION 7 IMPLEMENTATION ***/
/***************************************************/

/***************************************/
/***                                 ***/
/*** CONTROL RELATED STATE VARIABLES ***/
/***                                 ***/
/***************************************/

int level,sysinit,batchupd,ndcinvkd; /*** BOOL UNDER VERSION 7 ***/
int dfaltset; /*** BOOL UNDER VERSION 7 ***/
int erreport;
char *filename;
int pdfd;
int rastererase,explicit;

/**********************************************/
/* Version 7 of this declaration:             */
/*                      int level = 4;        */
/*                      int sysinit = FALSE;  */
/**********************************************/

/**************************************************/
/***                                            ***/
/*** LOGICAL DEVICE NAME ASSOCIATIONS STRUCTURE ***/
/*** FOR FUTURE CONSIDERATION                   ***/
/***                                            ***/
/****************************************************************************/
/***                                                                      ***/
/*** struct lnamstruc                                                     ***/
/***   {                                                                  ***/
/***   char *usernam;                                                     ***/
/***   char *sysnam;                                                      ***/
/***   }lognames[MAXVSURF + PICKS + LOCATORS + VALUATRS + KEYBORDS + BUTTONS];
									  ***/
/***   struct lnamstruc *lognmptr;                                        ***/
/***                                                                      ***/
/****************************************************************************/

/******************************/
/***                        ***/
/*** VIEW SURFACE VARIABLES ***/
/***                        ***/
/******************************/

struct viewsurf
   {
   char *name;
   int imupdate;   /********************************************************/
   int vshardwr;   /*** CAN SUPPORT VISIBILITY                           ***/
   int hlhardwr;   /*** CAN SUPPORT HIGHLIGHTING              ALL        ***/
   int dthardwr;   /*** CAN SUPPORT DETECTABILITY           BOOLEAN      ***/
   int trhardwr;   /*** CAN SUPPORT TRANSLATION              UNDER       ***/
   int schardwr;   /*** CAN SUPPORT SCALING                 VERSION      ***/
   int rohardwr;   /*** CAN SUPPORT ROTATION                   7         ***/
   int dehardwr;   /*** CAN SUPPORT DELETING A SEGMENT      EXCEPT       ***/
   int idhardwr;   /*** CAN SUPPORT PRIMITIVE PICK ID       SELECTED.    ***/
   int lshardwr;   /*** CAN SUPPORT LINE STYLE                           ***/
   int lwhardwr;   /*** CAN SUPPORT LINE WIDTH                           ***/
   int inhardwr;   /*** CAN SUPPORT INTENSITY                            ***/
   int clhardwr;   /*** CAN SUPPORT COLOR                                ***/
   int txhardwr;   /*** CAN SUPPORT HARDWARE TEXT GENERATION             ***/
   int erasure;    /*** DELETES BY ERASURE                               ***/
   int segopclo;   /*** NEEDS TO HAVE SEGMENTS OPENED AND CLOSED         ***/
   int nwframdv;   /*** NEW FRAME ACTION REQUIRED WHEN CHANGE IS MADE    ***/
   int nwframnd;   /*** NEW FRAME ACTION NEEDEDWHEN END BATCH OF UPDATES ***/
   int vinit;      /***                                                  ***/
   int selected;   /********************************************************/
   int (*dev1drive)();   /*** VERSION 7: DEVDRIVE ***/
   } surface[MAXVSURF];

   struct viewsurf *surfptr;
   struct viewsurf *recentvs;
   int slectnxt;

struct                   /********************************************/
   {                     /*  structure for passing arguments to the  */
   int opcode;           /*  device drivers, which will use only the */
   int logical;          /*  parts they need.                        */
   char *string;         /********************************************/
   int int1;
   float float1;
   float float2;
   float float3;
   } ddstruct;

   int gensco1();

/*************************/
/***                   ***/
/*** SEGMENT VARIABLES ***/
/***                   ***/
/*************************/

/**********************************************/
/* Version 7 of this declaration:             */
/*                      int segnum = ???;     */
/*                      int maxvsurf = ???;   */
/**********************************************/

struct segstruc
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

   struct segstruc *segptr;
   int segnum;

/******************************/
/***                        ***/
/*** INPUT DEVICE VARIABLES ***/
/***                        ***/
/******************************/

/*** int assocnum = ???; VERSION 7 IMPLEMENTATION ***/

/**************************************************/
/*                                                */
/* FOLLOWING STRUCTURE NEEDED WITH INITIALIZATION */
/* IN VERSION 7.                                  */
/*                                                */
/* struct                                         */
/*    {                                           */
/*    int picks;                                  */
/*    int locators;                               */
/*    int keybords;                               */
/*    int valuatrs;                               */
/*    int buttons;                                */
/*    } devnumbr;                                 */
/*                                                */
/**************************************************/

struct device
   {
   char devname;
   int enable;       /*** TWO VARIABLES BOOL ***/
   int echo;         /*** UNDER VERSION 7    ***/
   float echopos[2];
   char *echosurf;
   int (*dev2drive)();   /*** VERSION 7: DEVDRIVE ***/
   };

struct device pick[PICKS];

   struct device *pickptr;

struct locatstr
   {
   struct device subloc;
   float setpos[2];
   } locator[LOCATORS];

   struct locatstr *locatptr;

struct keybstr
   {
   struct device subkey;
   int bufsize;
   int maxbufsz;
   } keybord[KEYBORDS];

   struct keybstr *keybptr;

struct valstr
   {
   struct device subval;
   float vlinit;
   float vlmin;
   float vlmax;
   } valuatr[VALUATRS];

   struct valstr *valptr;

struct butnstr
   {
   struct device subbut;
   int prompt;
   } button[BUTTONS];

   struct butnstr *butnptr;

struct
   {
   int evenclas;
   int eventnam;
   int sampl1cls;   /*** VERSION 7: SAMPLCLS ***/
   int sampl1nam;   /*** VERSION 7: SAMPLNAM ***/
   } assocs[ASSOCNUM];

/*****************************/
/***                       ***/
/*** INPUT QUEUE VARIABLES ***/
/***                       **/
/*****************************/

int qsize;

struct reportpk
   {
   int seg2name;  /*** VERSION 7: SEGNAME ***/
   int pick1id;   /*** VERSION 7: PICKID ***/
   };

struct reportky
   {
   int length;
   char *text;
   };

struct events
   {
   struct reportpk pickrept;
   struct reportky keybrept;
   };

struct samples
   {
   float location[2];
   float value;
   };

struct smpldata
   {
   int sampl2cls;  /*** VERSION 7: SAMPLCLS ***/
   int sampl2nam;  /*** VERSION 7: SAMPLNAM ***/
   struct samples onefsmpl;
   };

struct inqueue
   {
   int evenclas;
   int eventnam;
   struct events onefevnt;
   int samplnum;
   struct smpldata smplinfo;
   } inputq;

/*********************************/
/***                           ***/
/*** PRIMITIVE STATE VARIABLES ***/
/***                           ***/
/*********************************/

int stdmarkr;
int intflag,clrflag,lsflag,lwflag,fntflag,szeflag,angflag,qualflag,picflag,cpchang;  /*** BOOLEAN UNDER VERSION 7 ***/
float cp[2];

struct aspect
   {
   float width;
   float height;
   };

struct worldcor
   {
   float dx;
   float dy;
   };

struct primattr
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

struct primattr pdfcurrent;
struct primattr current;
struct primattr defaultt;
struct primattr minimum;
struct primattr maximum;

/*******************************/
/***                         ***/
/*** SEGMENT STATE VARIABLES ***/
/***                         ***/
/*******************************/

struct segstruc *openseg;
int opsegemp,osexists,prevseg;               /*** BOOL UNDER VERSION 7 ***/
int csegtype,sgtypmin,sgtypmax;
int segused;
int pdfnext;

struct segattr
   {
   int visbil2ty;   /********************************************/
   int detect2bl;   /*** THREE VARIABLES BOOL UNDER VERSION 7 ***/
   int high2lght;   /*** ALSO DELETE NUMBERS WITHIN NAMES.    ***/
   float scale2[2];  /********************************************/
   float trans2lat[2];
   float rotate2;
   } defsegat;

/*******************************/
/***                         ***/
/*** VIEWING STATE VARIABLES ***/
/***                         ***/
/*******************************/

int coordsys,idenflag;
float ndcspace[2],viewup[2],imxform[3][2],vwxform[2][2],modxform[3][2],compxfrm[3][2];
int ndcspuse,wndwclip,vfinvokd,vwrotate,corsyset;  /*** BOOL UNDER VERSION 7 ***/
float wldsidx,wldsidy,ndcsidx,ndcsidy;

struct
   {
   float xmin;
   float xmax;
   float ymin;
   float ymax;
   } viewport;

struct
   {
   float xmin;
   float xmax;
   float ymin;
   float ymax;
   } window;

/*****************************/
/***                       ***/
/*** INPUT STATE VARIABLES ***/
/***                       ***/
/*****************************/

int cervalid;             /*** BOOL UNDER VERSION 7 ***/
struct inqueue cereport;

/****************************************************************************/
/*                                                                          */
/*     FUNCTIONS FOLLOW                                                     */
/*                                                                          */
/*     NOTE1: EACH SUBROUTINE WILL EITHER RETURN A '0' UPON SUCCESSFUL      */
/*            COMPLETION OR THE NUMBER OF THE ERROR LISTED IN DOCUMENTATION.*/
/*                                                                          */
/*     NOTE2: AN ERROR MESSAGE WILL BE DISPLAYED BY THE ERROR HANDLER       */
/*            "errhand" IF NECESSARY. VALUE PASSED TO "errhand" CORRESPONDS */
/*            TO THE OFFSET INTO THE INTERNAL ERROR TABLE OF THAT ROUTINE,  */
/*            WHICH DOESN'T MATCH THE ERROR NUMBER LISTED IN THE            */
/*            DOCUMENTATION.                                                  */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     FUNCTION: initcore                                                   */
/*                                                                          */
/*     PURPOSE: GUARANTEES THAT THE CORE SYSTEM IS IN A STANDARD STATE WITH */
/*              THE DEFAULT SETTINGS OF ALL THE CORE SYSTEM PARAMETERS      */
/*              ESTABLISHED.                                                */
/*                                                                          */
/****************************************************************************/

initcore(levelnum)
   int levelnum;  /*** WON'T ALLOW REGISTER CLASS, TRY IN VERSION 7 ***/
   {
   char *funcname;
   int errnum,i;

   funcname = "initcore";
/****************************************************************************/
/* The following statement should not be incorporated in this routine.      */
/* Rather, it should be an initializer in COMMON definition above.          */
/****************************************************************************/
   level = 4;
   if(sysinit)     /*** ROUTINE PREVIOUSLY CALLED? ***/
      {
      errnum = 0;
      errhand(funcname,errnum);
      return(1);
      }
   if(levelnum > level)     /*** LEVEL SUPPORTED? ***/
      {
      errnum = 1;
      errhand(funcname,errnum);
      return(2);
      }
/**********************************************/
/***                                        ***/
/*** INPUT LOGICAL DEVICE NAME ASSOCIATIONS ***/
/*** FUTURE CONSIDERATION                   ***/
/***                                        ***/
/**********************************************/

/****************************************************************************/
/****                                                                    ****/
/****                     INITIALIZE VARIABLES                           ****/
/****                                                                    ****/
/****************************************************************************/
/***************************************/
/***                                 ***/
/*** CONTROL RELATED STATE VARIABLES ***/
/***                                 ***/
/***************************************/
   sysinit = TRUE;
   batchupd = FALSE;
   ndcinvkd = FALSE;
   dfaltset = FALSE;
   rastererase = FALSE;
   explicit = FALSE;

   erreport = 0;

   filename = "pseudodisplay";

/*************************/
/***                   ***/
/*** SEGMENT VARIABLES ***/
/***                   ***/
/*************************/

   for(segptr = &segment[0];segptr < &segment[SEGNUM];segptr++)
      {
      segptr->seg1name = ' ';
      segptr->type = EMPTY;
      segptr->visbil1ty = TRUE;
      segptr->detect1bl = FALSE;
      segptr->high1lght = FALSE;
      segptr->scale1[0] = 1.0;segptr->scale1[1] = 1.0;
      segptr->trans1lat[0] = 0.0;segptr->trans1lat[1] = 0.0;
      segptr->rotate1 = 0.0;
      segptr->vsurfnum = 0;
      for(i = 0;i < VSURFNUM;i++)
	 {
	 segptr->vsurfptr[i] = NULL;
	 }
      segptr->pdfptr = NULL;
      segptr->redraw = FALSE;
      segptr->segsize = 0;
      }
   segnum = 0;

/******************************/
/***                        ***/
/*** VIEW SURFACE VARIABLES ***/
/***                        ***/
/******************************/
   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      surfptr->name = " ";
      surfptr->imupdate = TRUE;
      surfptr->vshardwr = FALSE;
      surfptr->hlhardwr = FALSE;
      surfptr->dthardwr = FALSE;
      surfptr->trhardwr = FALSE;
      surfptr->schardwr = FALSE;
      surfptr->rohardwr = FALSE;
      surfptr->dehardwr = FALSE;
      surfptr->idhardwr = FALSE;
      surfptr->lshardwr = FALSE;
      surfptr->lwhardwr = FALSE;
      surfptr->inhardwr = FALSE;
      surfptr->clhardwr = FALSE;
      surfptr->txhardwr = FALSE;
      surfptr->erasure = FALSE;
      surfptr->segopclo = FALSE;
      surfptr->nwframdv = FALSE;
      surfptr->nwframnd = FALSE;
      surfptr->vinit = FALSE;
      surfptr->selected = 0;
      surfptr->dev1drive = NULL;
      }

   recentvs = NULL;
   slectnxt = 1;

   /*********************************************************/
   /***                                                   ***/
   /*** PRESET VIEW SURFACE FLAGS INDICATING CAPABILITIES ***/
   /***                                                   ***/
   /*********************************************************/

   /****************/
   /***          ***/
   /*** GENISCO1 ***/
   /***          ***/
   /****************/

   surface[0].name = "vwsurf1";
   surface[0].clhardwr = TRUE;
   surface[0].txhardwr = TRUE;
   surface[0].erasure = TRUE;
   surface[0].dev1drive = gensco1;

   surface[1].name = "vwsurf2";

   ddstruct.opcode = NULL;
   ddstruct.logical = FALSE;
   ddstruct.string = "";
   ddstruct.int1 = 0;
   ddstruct.float1 = 0.0;
   ddstruct.float2 = 0.0;
   ddstruct.float3 = 0.0;

/******************************/
/***                        ***/
/*** INPUT DEVICE VARIABLES ***/
/***                        ***/
/******************************/
   for(pickptr = &pick[0];pickptr < &pick[PICKS];pickptr++)
      {
      pickptr->devname = " ";
      pickptr->enable = FALSE;
      pickptr->echo = FALSE;
      pickptr->echopos[0] = 0.0;pickptr->echopos[1] = 0.0;
      pickptr->echosurf = " ";
      pickptr->dev2drive = NULL;
      }

   for(locatptr = &locator[0];locatptr < &locator[LOCATORS];locatptr++)
      locatptr->devname = " ";
      locatptr->enable = FALSE;
      locatptr->echo = FALSE;
      locatptr->echopos[0] = 0.0;locatptr->echopos[1] = 0.0;
      locatptr->echosurf = " ";
      locatptr->dev2drive = NULL;
      locatptr->setpos[0] = 0.0;locatptr->setpos[1] = 1.0;
   for(keybptr = &keybord[0];keybptr < &keybord[KEYBORDS];keybptr++)
      {
      keybptr->devname = " ";
      keybptr->enable = FALSE;
      keybptr->echo = FALSE;
      keybptr->echopos[0] = 0.0;keybptr->echopos[1] = 0.0;
      keybptr->echosurf = " ";
      keybptr->dev2drive = NULL;
      keybptr->bufsize = 0;
      keybptr->maxbufsz = 80;
      }
   for(valptr = &valuatr[0];valptr < &valuatr[VALUATRS];valptr++)
      {
      valptr->devname = " ";
      valptr->enable = FALSE;
      valptr->echo = FALSE;
      valptr->echopos[0] = 0.0;valptr->echopos[1] = 0.0;
      valptr->echosurf = " ";
      valptr->dev2drive = NULL;
      valptr->vlinit = 0.0;
      valptr->vlmin = 0.0;
      valptr->vlmax = 1.0;
      }
   for(butnptr = &button[0];butnptr < &button[BUTTONS];butnptr++)
      {
      butnptr->devname = " ";
      butnptr->enable = FALSE;
      butnptr->echo = FALSE;
      butnptr->echopos[0] = 0.0;butnptr->echopos[1] = 0.0;
      butnptr->echosurf = " ";
      butnptr->dev2drive = NULL;
      butnptr->prompt = 0;
      }

/*********************************/
/***                           ***/
/*** PRIMITIVE STATE VARIABLES ***/
/***                           ***/
/*********************************/
   stdmarkr = 5;

   intflag = FALSE;
   clrflag = FALSE;
   lsflag = FALSE;
   lwflag = FALSE;
   fntflag = FALSE;
   szeflag = FALSE;
   angflag = FALSE;
   qualflag = FALSE;
   picflag = FALSE;
   cpchang = FALSE;

   cp[0] = 0;cp[1] = 0;

   for(i = 0;i < 3;i++)
      {
      defaultt.color[i] = 1.0;
      minimum.color[i] = 0.0;
      maximum.color[i] = 1.0;
      }

   defaultt.intensty = 1.0;
   minimum.intensty = 0.0;
   maximum.intensty = 1.0;

   defaultt.linwidth = 1.0;
   minimum.linwidth = -INFINITY;
   maximum.linwidth = INFINITY;

   defaultt.linestyl = SOLID;
   minimum.linestyl = SOLID;
   maximum.linestyl = DOTDASHED;

   defaultt.font = 0;
   minimum.font = 0;
   maximum.font = 1;

   defaultt.charsize.width = 1.0;
   minimum.charsize.width = -INFINITY;
   maximum.charsize.width = INFINITY;

   defaultt.charsize.height = 1.0;
   minimum.charsize.height = -INFINITY;
   maximum.charsize.height = INFINITY;

   defaultt.chrspace.dx = 1.0;
   minimum.chrspace.dx = -INFINITY;
   maximum.chrspace.dx = INFINITY;

   defaultt.chrspace.dy = 0.0;
   minimum.chrspace.dy = -INFINITY;
   maximum.chrspace.dy = INFINITY;

   defaultt.chqualty = LOW;
   minimum.chqualty = LOW;
   maximum.chqualty = HIGH;

   defaultt.pick2id = 0;
   minimum.pick2id = 0;
   maximum.pick2id = 32767;

/*******************************/
/***                         ***/
/*** SEGMENT STATE VARIABLES ***/
/***                         ***/
/*******************************/
   defsegat.visbil2ty = TRUE;
   defsegat.detect2bl = FALSE;
   defsegat.high2lght = FALSE;
   defsegat.scale2[0] = 1.0;defsegat.scale2[1] = 1.0;
   defsegat.trans2lat[0] = 0.0;defsegat.trans2lat[1] = 0.0;
   defsegat.rotate2 = 0.0;

   openseg = NULL;
   opsegemp = TRUE;
   osexists = FALSE;
   prevseg = FALSE;
   csegtype = NORETAIN;
   sgtypmin = NORETAIN;
   sgtypmax = XFORM2;
   segused = 0;
   pdfnext = 0;

/*******************************/
/***                         ***/
/*** VIEWING STATE VARIABLES ***/
/***                         ***/
/*******************************/
   viewport.xmin = 0.0;
   viewport.xmax = 1.0;
   viewport.ymin = 0.0;
   viewport.ymax = 1.0;

   window.xmin = 0.0;
   window.xmax = 1.0;
   window.ymin = 0.0;
   window.ymax = 1.0;

   wldsidx = 1.0;
   wldsidy = 1.0;
   ndcsidx = 1.0;
   ndcsidy = 1.0;

   ndcspace[0] = 1.0;ndcspace[1] = 1.0;
   coordsys = LEFT;
   viewup[0] = 0.0;viewup[1] = 1.0;
   vwxform[0][0] = 1.0;vwxform[0][1] = 0.0;vwxform[1][0] = 1.0;vwxform[1][1] = 0.0;
   identity(imxform);
   idenflag = TRUE;

   identity(modxform);
   identity(compxfrm);

   ndcspuse = FALSE;
   wndwclip = FALSE;
   vfinvokd = FALSE;
   vwrotate = FALSE;
   corsyset = FALSE;

/*****************************/
/***                       ***/
/*** INPUT STATE VARIABLES ***/
/***                       ***/
/*****************************/
   cervalid = FALSE;

/*****************************************/
/*                                       */
/* INITIALIZE PSEUDO DISPLAY FILE SYSTEM */
/*                                       */
/*****************************************/

   if(( pdfd = creat(filename,PMODE)) == -1)
      {
      printf("initerm: can't create %s\n",filename);
      }
   if(( pdfd = open(filename,2)) == -1)
      {
      printf("initerm: can't open %s\n",filename);
      }

   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: termcore                                                   */
/*                                                                          */
/*     PURPOSE: TERMINATE ALL VIEW SURFACES AND RELEASE INPUT DEVICES       */
/*              USED BY THE CORE SYSTEM.                                    */
/*                                                                          */
/****************************************************************************/

termcore()
   {
   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if(surfptr->vinit)
	 {
	 if(surfptr->selected)
	    {
	    deselectvwsurface(surfptr->name);
	    }
	 termvwsurface(surfptr->name);
	 }
      }

/* disablall();    BECOMES OPERATIVE UNDER LEVEL-3  IMPLEMENTATION */

   /*****************************************/
   /***                                   ***/
   /*** FILES ARE CLOSED BY SYSTEM UPON.  ***/
   /*** COMPLETION OF APPLICATION PROGRAM ***/
   /*** ERASE FILENAME FROM SYSTEM.       ***/
   /***                                   ***/
   /*****************************************/

   unlink(filename);

   sysinit = FALSE;
   return(0);
   }
