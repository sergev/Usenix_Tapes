/******************************************************************************/
/*                                                                            */
/*  THE GREASED PIG CONTEST : ... Our body shelters both a soul and a pig ... */
/*                                                                            */
/*  Place : University of Lowell.                                             */
/*  Programmer : ONG QUOC TRI.                                                */
/*  Date-written : Oct 19, 1986.                                              */
/*                                                                            */
/*  Description : This program is an interactive game simulation of a greased */
/*                pig contest with the following rules :                      */
/*                   1. The contestant is called 'C'.                         */
/*                   2. The sow is called 'S'.                                */
/*                   3. The hog is called 'H'.                                */
/*                   4. The boar is called 'B'.                               */
/*                   5. The child pig, hoggy is called 'Z'.                   */
/*                   6. The child pig, piggy is called 'Y'.                   */
/*                   7. The child pig, porky is called 'K'.                   */
/*                   8. The child pig, hoglet is called 'L'.                  */
/*                   9. The child pig, piglet is called 'E'.                  */
/*                  10. The child pig, porker is called 'R'.                  */
/*                  11. The child pig, hogling is called 'I'.                 */
/*                  12. The child pig, pigling is called 'N'.                 */
/*                  13. The child pig, porkling is called 'G'.                */
/*                  14. The hidden piggybanks are marked by '$'.              */
/*                  15. The hidden porkybombs are marked by '@'.              */
/*                  16. Obstacles (such as the slop trough) are '*'.          */
/*                  17. The number of moves for 'C' is limited to 25.         */
/*                  18. Each captured pig is scored 20 points but 1 for sow.  */
/*                  19. 11 piggybanks bonuses 78 points.                      */
/*                  20. Foul when hitting the porkybomb.                      */
/*                  21. The pigpen is an 11 X 16 playing board. The screen is */
/*                      updated after each move.                              */
/*                  22. Moves off the edge of the pigpen will continue in the */
/*                      same direction but from the opposite edge.            */
/*                  23. Both 'C' and the pigs move after 'C' has entered his  */
/*                      move. (The program makes the move for 'H', 'Z', 'L',  */
/*                      and 'I' according to the values from a random number  */
/*                      generator). 'C' and the pigs stop for obstacles.      */
/*                  24. The game ends when 'C' quits, dies, runs out of moves */
/*                      ('C' loses) or 'C' and the female pig 'S' are in the  */
/*                      same square ('C' wins).                               */
/*                                                                            */
/*  Reference : Introduction to PASCAL and Structured Design, pp 322-333,     */
/*              NELL DALE and DAVID ORSHALICK.                                */
/*                                                                            */
/*  System : DEC VAX/VMS 8650.                                                */
/*  Language : C.                                                             */
/*                                                                            */
/******************************************************************************/

#include curses                           /* Curses Screen Management Package */
#include ctype                        /* Character Type Classification Macros */
#include descrip                                /* VMS Descriptor Definitions */
#include dvidef/* Get Device & Volume Information Data Identifier Definitions */
#include math                               /* RTL Math Function Declarations */
#include jpidef            /* $GETJPI System Service Request Code Definitions */
#include signal                              /* UNIX Signal Value Definitions */
#include syidef  /* $GETSYI Get Sytem Information Data Identifier Definitions */
#include ssdef               /* System Service Return Status Code Definitions */
#include time                     /* RTL Routine Return Structure Definitions */

#define LIB$M_CLI_CTRLT 0X00100000 
#define LIB$M_CLI_CTRLY 0X02000000
#define Height 11                                 /* number of rows in pigpen */
#define Width 16                               /* number of columns in pigpen */
#define Award 25                                               /* pig's value */

enum Boolean
     { False, True };

enum PlayType
     { Quit, Die, Lose, Win, Play };

enum HandType
     { Contestant, Sow, Hog, Boar, Hoggy, Piggy, Porky, Hoglet, Piglet, Porker,
       Hogling, Pigling, Porkling, OldMan, OldSow } Hand;

struct Position
       {
       int          Y, X;                                      /* row, column */
       enum Boolean Free;
       };

typedef struct Position Player;
typedef char Matrix [25][81];

FILE   *file_ptr, *fopen();
WINDOW *PlayWin[13], *HideWin[13], *MarkWin[13], *TagWin[13], *SlopWin[37];
WINDOW *PenWin, *ConWin, *PadWin[2],*KeyWin[14], *BusWin, *IWin[2], *MessWin[3];

static char *file_score = "Pig-Score.DAT"; 
static char *file_play  = "Pig-Play.DAT";

char Alias [9], UserName [9], NodeName [16], ProcessName [16], DeviceName [256];
char OldName [16], BaseDigit (), GetHandID (), GetHideID (), Next ();

short int Channel;
int CPULIM, NumberOfAlarm, Control_C = 0, DeadEnd = 0, pid;

int LIB$GETJPI (), LIB$GETSYI (), LIB$GETDVI ();
int LIB$DISABLE_CTRL (), LIB$ENABLE_CTRL ();
int SYS$ASSIGN (), SYS$SETPRN ();
int Interrupt(), NoInterrupt(), TimeOut();

int ContestantMove (), SowMove (), HogMove (), BoarMove ();
int HoggyMove (), PiggyMove (), PorkyMove ();
int HogletMove (), PigletMove (), PorkerMove ();
int HoglingMove (), PiglingMove (), PorklingMove ();

typedef (*Fun) ();
Fun PigFun []
    = { ContestantMove, SowMove, HogMove, BoarMove, 
        HoggyMove, PiggyMove, PorkyMove, HogletMove, PigletMove,
        PorkerMove, HoglingMove, PiglingMove, PorklingMove };

static int PigWay [18][8]                          /* direction of best moves */
           = { { 1, 2, 4, 3, 7, 6, 8, 9 },                      /* seeker set */
               { 2, 3, 1, 6, 4, 9, 7, 8 }, { 3, 6, 2, 9, 1, 8, 4, 7 },
               { 4, 1, 7, 2, 8, 3, 9, 6 }, { 5, 5, 5, 5, 5, 5, 5, 5 },
               { 6, 9, 3, 8, 2, 7, 1, 4 }, { 7, 4, 8, 1, 9, 2, 6, 3 },
               { 8, 7, 9, 4, 6, 1, 3, 2 }, { 9, 8, 6, 7, 3, 4, 2, 1 },
               { 9, 8, 6, 7, 3, 4, 2, 5 },                       /* hider set */
               { 8, 7, 9, 4, 6, 1, 3, 5 }, { 7, 4, 8, 1, 9, 2, 6, 5 },   
               { 6, 9, 3, 8, 2, 7, 1, 5 }, { 6, 1, 8, 7, 3, 2, 9, 4 }, /* m-s */
               { 4, 1, 7, 2, 8, 3, 9, 5 }, { 3, 2, 6, 1, 9, 4, 8, 5 },
               { 2, 1, 3, 4, 6, 7, 9, 5 }, { 1, 4, 2, 7, 3, 8, 6, 5 } };

static int Bon [12] [13]                                      /* hidden bonus */
           = { { 1,1, 2,2,2,4,8, 3, 2,2,2,5,7 },
               { 1,1, 2,2,2,5,7, 3, 2,2,2,6,6 },
               { 1,1, 2,2,2,6,6, 3, 2,2,2,9,3 },
               { 1,1, 2,2,2,9,3, 3, 3,3,3,2,7 },
               { 1,1, 3,3,3,2,7, 3, 3,3,3,3,6 },
               { 1,1, 3,3,3,3,6, 3, 3,3,3,4,5 },
               { 1,1, 3,3,3,4,5, 2, 3,3,3,2,8 },
               { 1,1, 3,3,3,2,8, 2, 4,4,4,2,4 },
               { 1,1, 4,4,4,2,4, 2, 4,4,4,2,5 },
               { 1,1, 4,4,4,2,5, 2, 4,4,4,3,3 },
               { 1,1, 4,4,4,3,3, 2, 5,5,5,2,2 },
               { 1,1, 5,5,5,2,2, 2, 2,2,2,4,8 } };

char Marker [13][10]
     = { "         ", " sow     ", " hog     ", " boar    ", 
                      " hoggy   ", " piggy   ", " porky   ", 
                      " hoglet  ", " piglet  ", " porker  ",
                      " hogling ", " pigling ", " porkling" };

struct Candidate
       {
       char Home [38];                              /* user's login directory */
       char User [35];                                           /* user name */
       char Base [28];                                            /* sort key */
       char Name [4];                                          /* winner name */
       int  Score, Number;                         /* score and player number */
       int  Year, Month, Day;                                 /* winning date */
       int  Hour, Minute, Second;                              /* coding time */
       int  Member, Group;                             /* member and group id */
       };

struct Candidate GameMaster [11] 
                 = { { "OngGameMaster$Disk:[Phu_Xuyen]030000\n",
                       "PHU_XUYENvt100-80 0100000019OPX00\n",
                       "271003Mon110000XPO00000100\n", "XPO",
                        10, 1, 27, 10, 3, 11, 10, 1, 1, 1 },
                     { "OngGameMaster$Disk:[Thi_Dang] 029100\n",
                       "THI_DANG vt100-80 0090000028TTD00\n",
                       "270424Sun180000DTT00000200\n", "DTT",
                        9, 2, 27, 4, 24, 18, 9, 2, 2, 1 },
                     { "OngGameMaster$Disk:[Quoc_Vinh]030200\n",
                       "QUOC_VINHvt100-80 0080000039OQV00\n",
                       "461008Tue050000VQO00000300\n", "VQO",
                        8, 3, 46, 10, 8, 5, 8, 3, 3, 1 },
                     { "OngGameMaster$Disk:[Quoc_Tri] 029300\n",
                       "QUOC_TRI vt100-80 0070000048OQT00\n",
                       "540323Tue110000TQO00000400\n", "TQO",
                        7, 4, 54, 3, 23, 11, 7, 4, 4, 1 },
                     { "OngGameMaster$Disk:[Thi_Thuan]030400\n",
                       "THI_THUANvt100-80 0060000059PTT00\n",
                       "560923Sun160000TTP00000500\n", "TTP",
                        6, 5, 56, 9, 23, 16, 6, 5, 5, 1 },
                     { "OngGameMaster$Disk:[Mong_Thuy]030500\n",
                       "MONG_THUYvt100-80 0050000069OMT00\n",
                       "571026Sat040000TMO00000600\n", "TMO",
                        5, 6, 57, 10, 26, 4, 5, 6, 6, 1 },
                     { "OngGameMaster$Disk:[N_N_Thach]030600\n",
                       "N_N_THACHvt100-80 0040000079NNT00\n",
                       "581210Sun000000TNN00000700\n", "TNN",
                        4, 7, 58, 12, 10, 0, 4, 7, 7, 1 },
                     { "OngGameMaster$Disk:[Ut_Nguyen]030700\n",
                       "UT_NGUYENvt100-80 0030000089OKN00\n",
                       "640419Sun150000NKO00000800\n", "NKO",
                        3, 8, 64, 4, 19, 15, 3, 8, 8, 1 },
                     { "OngGameMaster$Disk:[Bao_Chau] 029800\n",
                       "BAO_CHAU vt100-80 0020000098OBC00\n",
                       "670910Sun100000CBO00000900\n", "CBO",
                        2, 9, 67, 9, 10, 10, 2, 9, 9, 1 },
                     { "OngGameMaster$Disk:[My_Linh]  028900\n",
                       "MY_LINH  vt100-80 0010000107DML00\n",
                       "761219Sun160000LMD00001000\n", "LMD",
                        1, 10, 76, 12, 19, 16, 1, 10, 10, 1 },
                     { "                              100 00\n",
                       "                  0000000000   00\n",
                       "000000   000000   00000000\n", "   ",
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

static char Rule [21] [66]
       = { "Welcome to 'THE GREASED PIG CONTEST'                             ",
           "                                                                 ",
           "The rules are simple :                                           ",
           "                                                                 ",
           "o  You are the contestant, 'C' you want the hog, boar, porkling, ",
           "   hoggy, piggy, porky, hoglet, piglet, porker, hogling, pigling,",
           "   and hidden piggybanks '$', but the sow 'S' is chasing you.    ",
           "                                                                 ",
           "o  After each move use the auxiliary keypad (located on the right",
           "   most part of the keyboard) to give the direction of move (from",
           "   1 to 9). Enter direction '5' you will move nowhere.           ",
           "                                                                 ",
           "o  You have 25 moves to catch a pig. Each captured swine scores  ",
           "   25 points but 1 for the sow. Stepping on the hidden porkybomb ",
           "   '@', your next move will come to a halt.                      ",
           "                                                                 ",
           "o  Both you and the pigs move simultaneously but stop for slop   ",
           "   troughs, '*'. If you or the pig go off the edge of the pigpen,",
           "   you continue from the opposite side.                          ",
           "                                                                 ",
           "o  You win when you occupy the same square as the sow 'S'.       "};

static char Help [21] [66]
       = { "------------------------------------- Sow desperately wants you. ",
           "|        |        |        |        | Hog always jumps randomly. ",
           "| UNUSED | UNUSED | UNUSED | UNUSED | Boar retains his distance. ",
           "|        |        |        |        | Hoggy crosses the piggery. ",
           "|--------+--------+--------+--------| Piggy traces Sow/Hog/Boar. ",
           "|    7   |    8   |    9   |    -   | Porky roams in the pigpen. ",
           "|        |        |        |        | Hoglet divides the pigsty. ",
           "|        |   Up   |        |  Rule  | Piglet allures you to Sow. ",
           "|--------+--------+--------+--------| Porker runs away from you. ",
           "|    4   |    5   |    6   |    ,   | Hogling confuses his move. ",
           "|        |        |        |        | Pigling guides Sow to you. ",
           "|  Left  | Center |  Right |  Help  | Porkling goes to the edge. ",
           "|--------+--------+--------+--------| 2 Porkybombs halt 2 moves. ",
           "|    1   |    2   |    3   |        | 11 Piggybanks cost 39 pts. ",
           "|        |        |        |        | <1..9> directions of move. ",
           "|        |  Down  |        |        | <5> stand-by, re-initiate. ",
           "|-----------------+--------|  List  | <0> quit game, store name. ",
           "|        0        |    .   |        | <.> select, reset, rename. ",
           "|                 |        |        | <-> rules of this GP game. ",
           "|       Exit      | Select |        | <,> help plays, hint wins. ",
           "------------------------------------- < > list ten best players. "};

char  List [21] [66]
      = {  "   WINNER'S LOGIN DIRECTORY        DATE     NAME   SCORE   NUMBER",
           "==============================   ========   ====   =====   ======",
           "", "", "", "", "", "", "", "", "", "",
           "=================================================================",
           "Note:  _ Winner will be enlisted; if his/her score is higher than",
           "         the lowest score in the list or his/her previous score. ",
           "", "", "", "", "", "" };

main ()
   {
   struct Candidate TopTen [10];
   Player           Players [15], Hider [13];         /* the family of Suidae */
   int              Bo, Total [5], Trial = 1, OldK = 0, EmptyFile = 0;
   enum PlayType    Winner = Play;
   Matrix           PigPen;

   signal (SIGINT, NoInterrupt);                         /* disable control/C */
   DisableControl (LIB$M_CLI_CTRLT);                     /* disable control/T */
   DisableControl (LIB$M_CLI_CTRLY);                     /* disable control/Y */
   initscr ();                                           /* initialize screen */
   OffCursor ();
   DefineWindow (Players, Hider);
   GetLast ();
   pid = getpid ();                                         /* get process id */
   noecho ();
   printf ("\033>");                            /* select Numeric Keypad Mode */
   srand ( time () );  /* initialize the random number generator by the clock */

   ShowKeyPad (&OldK);
   Alert (" W A I T \15");
   MakePigPen (PigPen);
   ShowPigPen (PigPen);
   MakeBigWord ();                           /* define double width character */
   CPULIM = GetCPUtime (JPI$_CPULIM, pid, "CPU limit"); /* get CPU time limit */
   GetPigName (Alias, OldName, ProcessName, UserName);        /* select alias */
   MoveString ( Marker[0], Alias, 0, strlen (Alias) - 1, strlen (Alias) < 8 );
   GetNodeName (NodeName);
   AssignChannel (&Channel);
   GetDeviceName (Channel, DeviceName);
   BackFire (TopTen, Total, &EmptyFile);     /* restart the game upon request */
   SYS$SETPRN (0);                                     /* set no name process */
   if ( Control_C )
      Interrupt ();             /* interrupt upon previous CTRL/C user action */
   signal (SIGINT, Interrupt);                            /* interrupt enable */
   
   while ( (Winner != Quit) && (Winner != Win) && (CheckCPU () == True) )
      {
      GetTopTen (TopTen, Total, EmptyFile);
      ShowBigWord (" MENU: <0>quit <.>reset <->rule <,>help ", False, 0, 6);
      ShowScore (Award);
      Replay (Bon [ Bo = rand () % 12 ]);              /* select hidden bonus */
      ShowMarker (Players, Hider, Alias);
      DisplayKey (5, &OldK);
      Reset (Players, Hider, PigPen, False, Trial == 1, True, Trial == 1, True);
      ShowGame (Trial++); 
      MakeList (TopTen, Total);
      Winner = Play;
      Prompt ("DIRECTION");
      ShowBigWord ("       GREASED PIG CONTEST              ", False, 0, 1);
      PlayGame (TopTen, Players, Hider, &OldK, Total, Bon[Bo], &Winner, PigPen);
      }

   SetProcessName ();                 /* return original process name to user */
   DeleteWindow (Players, Hider);
   EnableControl (LIB$M_CLI_CTRLT);                       /* enable control/T */
   EnableControl (LIB$M_CLI_CTRLY);                       /* enable control/Y */
   } /* main */   

GetLast ()
   {                             
   char buffer [82], last [81];

   if ((file_ptr = fopen (file_play, "r")) != NULL)
      {
      while (fgets (buffer, 82, file_ptr) != NULL)
         sprintf (last, "%-80s", buffer);
      standout ();
      mvaddstr (0, 0, last);
      refresh (); 
      fclose (file_ptr);
      }
   else
      CreateShell (file_play, "P");
   } /* get last */



PutLast (Winner, Score, PlayerNumber)
enum PlayType Winner;
int           Score, PlayerNumber;
   {
   static char *Result [4] = { "QUIT", "DIE", "LOSE", "WIN" };
   long        time_value;

   if ((file_ptr = fopen (file_play, "a")) == NULL)
      GetOut (" ERROR: P-P file can't open for updating", 5);

   time (&time_value);
   fprintf (file_ptr, "%06d %-4s> %-28s %-9s %03d %s", PlayerNumber,
        Result [Winner], getenv ("HOME"), Alias, Score, ctime (&time_value));
   fclose (file_ptr);
   } /* put last */



GetTopTen (TopTen, Total, EmptyFile)
struct Candidate TopTen [];
int              Total [], EmptyFile;
   {
   if ( EmptyFile != -1 )
      {
      ReadTopTen (TopTen, Total);
      Decode (TopTen, Total);
      Extract (TopTen, Total);
      CheckTopTen (TopTen, Total);
      }
   } /* get top ten */

BackFire (TopTen, Total, EmptyFile)
struct Candidate TopTen [];
int              Total [], *EmptyFile;
   {
   static char *Question [4]
                = { " WHERE:                                 ",
                    " WHEN:                                  ",
                    " WHO:                                   ",
                    " WHAT:                                  " };
   static char *Answer [4]
                = { "UNIVERSITY OF LOWELL",
                    "SUN OCT 19, 1986",
                    "ONG QUOC TRI'",
                    "450 49 6717" };
   static char *BadWord [4]
                = { " ERROR: unknown location                ",
                    " ERROR: invalid date                    ",
                    " ERROR: unauthorized official           ",
                    " ERROR: illegal password                " };
   char         String [80];
   int          XPos [4] = { 8, 7, 6, 7 }, i, j;
   long         timer;

   if ( (file_ptr = fopen ("Pig-Start.DAT", "r+")) != NULL )
      {
      if ( (*EmptyFile = getc (file_ptr)) == -1 ) 
         {
         News (" RESTART ");
         echo ();
         for (i = 0; i <= 3; i++)
            {
            ShowBigWord (Question [i], False, 0, 0);
            NumberOfAlarm = 10;
            LeaveGraphic ();
            GetWindowString (stdscr, 0, XPos [i], String);
            for (j = 0; j <= strlen (String) - 1; j++)
               String [j] = toupper (String [j]); 
            if (strcmp (String, Answer [i]) != 0)
               GetOut (BadWord [i], 5);
            }
         noecho ();                              
         EnterGraphic ();
         Alert (" W A I T \15");
         Prompt ("         ");
         CopyTopTen (TopTen, GameMaster);
         Totalize (Total);
         time (&timer);
         fprintf (file_ptr, "Starting Date %s", ctime(&timer));
         fprintf (file_ptr, "Default Device & Directory %s\n", getenv ("PATH"));
         fprintf (file_ptr, "Process Name '%s'\n", ProcessName);
         fprintf (file_ptr, "Process ID %X\n", pid);
         fprintf (file_ptr, "Controlling Terminal %s\n", ctermid (0));
         }
      fclose (file_ptr);
      }

   if (*EmptyFile == - 1)
      CreateShell (file_score, "S");
   } /* back fire */

GetWindowString (Wind, Y, X, S)
WINDOW *Wind;
int    Y, X;
char   S [];
   {
   CheckCPU ();
   signal (SIGALRM, TimeOut);           /* pass signal and function to SIGNAL */
   alarm (1);                                 /* set alarm clock for 1 second */
   OnCursor ();
   mvwgetstr (Wind, Y, X, S);
   OffCursor ();
   alarm (0);                                         /* cancel pending alarm */
   CheckCPU ();
   } /* get window string */



CopyTopTen (A, B)
struct Candidate A [], B [];
   {
   int Index;

   for (Index = 0; Index <= 9; Index++)
       A [Index] = B [Index];
   } /* copy top ten */



Extract (TopTen, Total)
struct Candidate TopTen [];
int              Total [];
   {
   int Index;

   for (Index = 0; Index <= 9; Index++)
      if (strcmp (TopTen [Index].Home, GameMaster [Index].Home) == 0)
          strcpy (TopTen [Index].Name, GameMaster [Index].Name);
      else
          MoveString (TopTen [Index].Name, TopTen [Index].Base, 15, 17, 0);
   } /* extract */



CheckTopTen (TopTen, Total)
struct Candidate TopTen [];
int              Total [];
   {
   CheckTotal (Total);
   CheckSize (TopTen);
   CheckDefault (TopTen);
   CheckUserName (TopTen);
   CheckWinnerName (TopTen);
   CheckScore (TopTen);
   CheckNumber (TopTen);
   CheckDate (TopTen);
   CheckTime (TopTen);
   } /* check top ten */

CheckTotal (Total)
int Total [];
   {
   if ( Total [Quit] != (Total[Play] - Total[Die] - Total[Lose] - Total[Win]) )
      Counterfeit ();
   if ( Total [Die] != (Total[Play] - Total[Quit] - Total[Lose] - Total[Win]) )
      Counterfeit ();
   if ( Total [Lose] != (Total[Play] - Total[Quit] - Total[Die] - Total[Win]) )
      Counterfeit ();
   if ( Total [Win] != (Total[Play] - Total[Quit] - Total[Die] - Total[Lose]) )
      Counterfeit ();
   if ( Total [Play] != (Total[Quit] + Total[Die] + Total[Lose] + Total[Win]) )
      Counterfeit ();
   } /* check total */



CheckSize (TopTen)
struct Candidate TopTen [];
   {
   int i;
 
   for (i = 0; i <= 9; i++)
      {
      if ( strlen (TopTen [i].Home) != 37 )
         Counterfeit ();
      if ( strlen (TopTen [i].User) != 34 )
         Counterfeit ();
      if ( strlen (TopTen [i].Base) != 27 )
         Counterfeit ();
      if ( TopTen [i].Score < 1 || TopTen [i].Score > 300 )
         Counterfeit ();
      if ( TopTen [i].Number < 1 || TopTen [i].Number > 999999 )
         Counterfeit ();
      if ( TopTen [i].Year < 0 || TopTen [i].Year > 99 )
         Counterfeit ();
      if ( TopTen [i].Month < 1 || TopTen [i].Month > 12 )
         Counterfeit ();
      if ( TopTen [i].Day < 1 || TopTen [i].Day > 31 )
         Counterfeit ();
      if ( TopTen [i].Hour < 0 || TopTen [i].Hour > 23 )
         Counterfeit ();
      if ( TopTen [i].Minute < 0 || TopTen [i].Minute > 59 )
         Counterfeit ();
      if ( TopTen [i].Second < 0 || TopTen [i].Second > 59 )
         Counterfeit ();
      }
   } /* check size */

CheckDefault (TopTen)
struct Candidate TopTen [];
   {
   int i, j;
   
   for (i = 0; i <= 9; i++)
       if (TopTen [i].Home [30] == '0')                            /* default */
          if ( isdigit (TopTen [i].Home [33]) )
             {
             j = CTI (TopTen [i].Home, 33, 33);
             if ( strcmp (TopTen [i].Home, GameMaster [j].Home) != 0 )
                Counterfeit ();
             if ( strcmp (TopTen [i].User, GameMaster [j].User) != 0 )
                Counterfeit ();
             if ( strcmp (TopTen [i].Base, GameMaster [j].Base) != 0 )
                Counterfeit ();
             if ( TopTen [i].Score != GameMaster [j].Score )
                Counterfeit ();
             if ( TopTen [i].Number != GameMaster [j].Number )
                Counterfeit ();
             if ( TopTen [i].Year != GameMaster [j].Year )
                Counterfeit ();
             if ( TopTen [i].Month != GameMaster [j].Month )
                Counterfeit ();
             if ( TopTen [i].Day != GameMaster [j].Day )
                Counterfeit ();
             if ( TopTen [i].Member != GameMaster [j].Member )
                Counterfeit ();
             if ( TopTen [i].Group != GameMaster [j].Group )
                Counterfeit ();
             }
          else
             Counterfeit ();
       else
       if ( TopTen [i].Home [30] != '1' )                           /* hacker */
          Counterfeit ();
   } /* check default */



CheckUserName (TopTen)
struct Candidate TopTen [];
   {
   int i, j, UL, HL;

   for (i = 0; i <= 9; i++)
      {
      UL = CTI (TopTen [i].User, 27, 27);
      HL = CTI (TopTen [i].Home, 31, 32) - UL - 1;
      for (j = 0; j <= (UL - 1); j++)
         if ( toupper (TopTen [i].Home [j + HL]) != TopTen [i].User [j] )
            Counterfeit ();
      }
   } /* check user name */

CheckWinnerName (TopTen)
struct Candidate TopTen [];
   {
   int Index;
  
   for (Index = 0; Index <= 9; Index++)
      if ( (TopTen [Index].User [28] != TopTen [Index].Name [2]) ||
           (TopTen [Index].User [29] != TopTen [Index].Name [1]) ||
           (TopTen [Index].User [30] != TopTen [Index].Name [0]) )
         Counterfeit ();
   } /* check winner name */     



CheckScore (TopTen)
struct Candidate TopTen [];
   {
   int i;

   for (i = 0; i <= 9; i++)
      if ( TopTen [i].Score != CTI (TopTen [i].User, 18, 20) )
         Counterfeit ();
   } /* check score */



CheckNumber (TopTen)
struct Candidate TopTen [];
   {
   int i, US, BS;

   for (i = 0; i <= 9; i++)
      {
      US = CTI (TopTen [i].User, 21, 26);
      BS = CTI (TopTen [i].Base, 18, 23);
      if ( TopTen [i].Number != US )
         Counterfeit ();
      if ( TopTen [i].Number != BS )
         Counterfeit ();
      if ( US != BS )
         Counterfeit ();
      }
   } /* check number */



char BaseDigit (Number)
   {
   static char Digit [10]
               = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
   
   return (Digit [Number]);
   } /* base digit */

CheckDate (TopTen)
struct Candidate TopTen [];
   {
   int i;

   for (i = 0; i <= 9; i++)
      {
      if ( TopTen [i].Year  != CTI (TopTen [i].Base, 0, 1) )
         Counterfeit ();
      if ( TopTen [i].Month != CTI (TopTen [i].Base, 2, 3) )
         Counterfeit ();
      if ( TopTen [i].Day   != CTI (TopTen [i].Base, 4, 5) )
         Counterfeit ();
      }
   } /* check date */



CheckTime (TopTen)
struct Candidate TopTen [];
   {
   int i, N;

   for (i = 0; i <= 9; i++)
      {
      N = CTI (TopTen [i].Base, 9, 10);                /* hour */
      if ( N < 0 || N > 23 )
         Counterfeit ();
      N = CTI (TopTen [i].Base, 11, 12);             /* minute */
      if ( N < 0 || N > 59 )
         Counterfeit ();
      N = CTI (TopTen [i].Base, 13, 14);             /* second */
      if ( N < 0 || N > 59 )
         Counterfeit ();
      }
   } /* check time */



Counterfeit ()
   {
   GetOut (" ERROR: invalid data file               ", 6); 
   } /* counterfeit */



CTI (String, Start, End)
char String [];
int  Start, End;
   {
   char C [7];

   MoveString (C, String, Start, End, 0);   
   C [(End - Start) + 1] = '\0';

   return ( atoi (C) );
   } /* character to integer */

ReadTopTen (C, Total)
struct Candidate C [];
int              Total [];
   {
   int Draft [10], i;

   if ((file_ptr = fopen (file_score, "r")) == NULL)
      GetOut (" ERROR: P-S file can't open for reading ", 5);

   fscanf (file_ptr, "%d %o %X %o %d %d %o %X %o %d %d %o %X %o %d\n", 
      &Draft [0],    &Draft [1],   &Draft [2],    &Draft [3],   &Draft [4],
      &Total [Quit], &Total [Die], &Total [Lose], &Total [Win], &Total [Play],
      &Draft [5],    &Draft [6],   &Draft [7],    &Draft [8],   &Draft [9]);
   for (i = 0; i <= 9; i++)
      {
      fgets (C [ Draft[i] ].Home, 38, file_ptr); 
      fgets (C [ Draft[i] ].User, 35, file_ptr);
      fgets (C [ Draft[i] ].Base, 28, file_ptr);
      fscanf (file_ptr, "%x %o %d %x %o %o %x %d %o %x\n", 
         &C [ Draft[i] ].Score,  &C [ Draft[i] ].Number,
         &C [ Draft[i] ].Year,   &C [ Draft[i] ].Month,  &C [ Draft[i] ].Day,
         &C [ Draft[i] ].Hour,   &C [ Draft[i] ].Minute, &C [ Draft[i] ].Second,
         &C [ Draft[i] ].Member, &C [ Draft[i] ].Group);
      }
   fclose (file_ptr);
   } /* read top ten */



Replay (Bonus)
int Bonus [];
   {
   int Count = 0;
   
   for (Hand = Contestant; Hand <= Porkling; Hand++)
      {
      wmove (IWin [1], Hand, 0);
      wprintw (IWin [1], "%d ", Bonus [Hand]);
      Count = Count + Bonus [Hand];
      }
   if (Count != 41)
      GetOut (" ERROR: invalid number of hidden points ", 5);  
   } /* replay */



RingBell ()
   {
   move (0, 40);
   putchar ('\7');
   } /* ring bell */

PlayGame (TopTen, Players, Hider, OldKey, Total, Bon, Winner, PigPen)
struct Candidate TopTen [];
Player           Players [], Hider [];      
int              *OldKey, Total [], Bon [];
enum PlayType    *Winner;
Matrix           PigPen;
   {
   int          MovesRemaining = Award, Score, Count = -1;
   int          Bank [14] = { 0,1, 2,3,4,5,6,7,8,9,10,11,12, 0 };
   enum Boolean Foul = False, PreGame = True, Obstructed;

   MakeDraft (Bank, 13);
   do
      {
      if (!Foul)
         ContestantMove (Players, Hider, PigPen, Winner, PreGame, OldKey);
      else
         {
         Alert (" H A L T \15");
         Foul = False;
         }
      PreGame = False;
      if (*Winner != Quit)
         {
         Score = MovesRemaining;
         for (Hand = Hog; Hand <= Porkling; Hand++)
            if (Players [Hand].Free)
               {
               CheckMove (BusWin, Hand);
               (*PigFun [Hand]) (Players, PigPen, &Obstructed);
               if (SameSquare (Players[Contestant], Players[Hand]))
                  ShowBanner (Hand, PlayWin, Players, &Score, *Winner, PigPen);
               else
                  {
                  ShowPlayer (Players [Hand], PlayWin [Hand], Obstructed);
                  CheckHide (BusWin, Hand, Players [Hand], PigPen);
                  }
               } /* herd play */
         SowMove (Players, PigPen, &Obstructed);
         ShowPlayer (Players [Sow], PlayWin [Sow], Obstructed);
         if (SameSquare (Players[Contestant], Players[Sow]))          
            if (Census (Players, Sow) == 1)
               ShowBanner (Sow, PlayWin, Players, &Score, *Winner=Win, PigPen);
            else
               ShowBanner (0, PlayWin, Players, &Score, *Winner=Die, PigPen);
         else
            CheckHide (BusWin, Sow, Players [Sow], PigPen);
         if ( (Count < 12) && (Score == MovesRemaining) && (*Winner == Play) )
            XHide (&Score, &Count, Bon, Bank, &Foul, Players[0], Hider, PigPen);
         MovesRemaining = Score - 1;       /* decrement the moves left by one */
         } /* fooling around */
      else
         News (" Q U I T ");
      ShowScore (MovesRemaining);
      } 
   while ( (*Winner == Play) && (MovesRemaining > 0) );

   Finish (Winner, MovesRemaining, OldKey, Total, TopTen);
   } /* play game */

MakeList (TopTen, Total)
struct Candidate TopTen [];
int              Total [];
   {
   char *Line[5] 
        = { "       _ Total number of deserters :",
            "       _ Total number of deaths :",
            "       _ Total number of losers :",
            "       _ Total number of winners :",
            "       _ Total number of players :" };
   enum PlayType Winner;
   int  Index;

   for (Index = 0; Index <= 9; Index++)
      sprintf (List [Index+2], "%.30s   %2d/%02d/%02d    %3s    %3d    %6d",
              TopTen [Index].Home, 
              TopTen [Index].Month, TopTen [Index].Day, TopTen [Index].Year,
              TopTen [Index].Name, 
              TopTen [Index].Score, TopTen [Index].Number);
   sprintf (List [15], "       _ Winning score ranges from %d to 300.", 
                                                            TopTen [9].Score+1);
   for (Winner = Quit; Winner <= Play; Winner++)
      sprintf (List [Winner+16], "%s %d", Line [Winner], Total [Winner]);

   if (Total [Play] == 999999)
      Totalize (Total);
   } /* make list */



Totalize (Total)
int Total [];
   {
   Total [Quit] = Total [Die]  = Total [Lose] = 0;
   Total [Win]  = Total [Play] = 10;
   } /* totalize */



CreateShell (file_desc, message)
char *file_desc, *message;
   {
   delete (file_desc);
   if ((file_ptr = fopen (file_desc, "w")) == NULL)
      GetOut (strcat (" ERROR: can't open for writing file P-", message), 5);

   if (chmod (file_desc, 0400|0200|0100| 0040|0020|0010| 0004|0002|0001) == -1)
      GetOut (strcat (" ERROR: can not set protection file P-", message), 5);
   fclose (file_ptr);
   } /* create shell file */   

Reset (Players, Hider, PigPen, Notice, Select, PreGame, Sloppy, Hiding)
Player       Players [], Hider [];
Matrix       PigPen;
enum Boolean Notice, Select, PreGame, Sloppy, Hiding;
   {
   int Row, Column;
   
   if (Notice)
      {
      ClearError ();
      News ("R E S E T");
      }

   for (Row = 1; Row <= (2 * Height + 1); Row++)
      for (Column = 1; Column <= (4 * Width + 1); Column++)
         switch (PigPen [Row] [Column])
            {
            case '@' : 
               if (PreGame == True && Select == True)
                  PigPen [Row] [Column] = ' ';
               break;
            case '#' : 
               if (PreGame == True && Select == True)
                  PigPen [Row] [Column] = '$';
               break;
            case '*' :
               if (Notice)
                  PigPen [Row] [Column] = ' ';
               break;
            }

   if (Hiding)
      GoHide (Players [Contestant], Hider, PigPen);
   if (PreGame)
      MakePlayer (Players, PigPen, Select);
   Players [OldSow] = Players [Sow];
   if (Select)
      ShowSlop (Players [Contestant], PigPen, Sloppy);

   if (CountSymbol (PigPen, "**") != 36)
      GetOut (" ERROR: invalid number of slop troughs  ", 5);  
   if (CountSymbol (PigPen, "$#") != Census (Hider, Contestant))
      GetOut (" ERROR: invalid number of piggy banks   ", 5); 
   if (CountSymbol (PigPen, "@#") != Census (Players, Sow))
      GetOut (" ERROR: invalid number of game players  ", 5);

   if (Notice)
      Prompt ("DIRECTION");
   } /* reset */



SameSquare (You, Pig)
Player You, Pig;
   {
   return ( (You.Y == Pig.Y) && (You.X == Pig.X) );
   } /* same square */

CheckMove (BusWin, Swine)
WINDOW        *BusWin;
enum HandType Swine;
   {
   mvwin (BusWin, Swine+2, 66);
   wrefresh (BusWin);
   } /* check move */



CheckHide (BusWin, Swine, Pig, PigPen)
WINDOW        *BusWin;
enum HandType Swine;
Player        Pig;
Matrix        PigPen;
   {
   if (PigPen [2*Pig.Y][4*Pig.X-2] == '#' || PigPen [2*Pig.Y][4*Pig.X-2] == '$')
      {
      mvwin (BusWin, Swine+2, 78);
      wrefresh (BusWin);
      }
   } /* check hide */



ShowTime (TimeWin, Subscript)
WINDOW *TimeWin;
int    Subscript;
   {
   char A [14], B [14];
   long timer;

   wmove (TimeWin, 0, 0);
   switch (Subscript)
      {
      case 1 :
         wprintw (TimeWin, "%s        %s        %s",
                  "University of Lowell", "Sun Oct 19, 1986", "ONG QUOC TRI'");
         break;
      case 2 :
         ConvertCPUtime (GetCPUtime (JPI$_CPUTIM, pid, "elapsed CPU"), B);
         if (CPULIM)
            ConvertCPUtime (CPULIM, A);
         else
            strcpy (A, "Infinite     ");
         wprintw (TimeWin, "CPU limit: %s          Elapsed CPU time: %s", A, B);
         break;
      case 3 :
         time (&timer);
         wprintw (TimeWin, "%-15s %11s %-12s %24s", 
                             ProcessName, NodeName, DeviceName, ctime (&timer));
         break;
      }
   wrefresh (TimeWin); 
   } /* show time */

XHide (Score, Count, Bonus, Bank, Foul, Piece, Hider, PigPen)
int          *Score, *Count, Bonus [], Bank [];
enum Boolean *Foul;
Player       Piece, Hider [];
Matrix       PigPen;
   {
   char   Message [40];
   enum HandType Swine;

   if ( PigPen [2*Piece.Y] [4*Piece.X - 2] == '$' )
      {
      Swine = Bank [ *Count += 1 ];
      if (Bonus [Swine] < 1 || Bonus [Swine] > 9)
         GetOut (" ERROR: bonus points out of range       ", 5);
      else
      if (Bonus [Swine] == 1)
         strcpy (Message, "BOOMS! Hold your position for one move  ");
      else
         sprintf (Message, "BONUS! You have found a %d-pts piggy bank", 
                  Bonus [Swine]);
      delwin (HideWin [Swine]);
      ShowBigWord (Message, True, 1, 0);
      *Score += Bonus [Swine];
      *Foul = Bonus [Swine] == 1;
      Hider [Swine].Free = False;
      PigPen [2*Piece.Y] [4*Piece.X - 2] = ' ';               /* clear pigpen */
      wsetattr (MessWin [0], _BOLD);
      ShowBigWord ("       GREASED PIG CONTEST              ", False, 0, 1);
      }
   } /* show hidden score */



HandWin (PieceWin, Hand, Row, Column, ID)
WINDOW        *PieceWin [];
enum HandType Hand;
int           Row, Column;
char          ID;
   {
   PieceWin [Hand] = newwin (1, 1, Row, Column);
   switch (Hand)
      {
      case Contestant : case Sow :
         wsetattr (PieceWin [Hand], _BOLD | _BLINK | _REVERSE);
         break;
      default :
         wsetattr (PieceWin [Hand], _BLINK | _REVERSE);
         break;
      }
   waddch (PieceWin [Hand], ID);
   } /* set player window */

DefineWindow (Players, Hider)
Player Players [], Hider [];
   {
   wsetattr ((ConWin = newwin (23, 15, 1, 65)), _REVERSE);
   wclear (ConWin);
   wrefresh (ConWin); 

   wsetattr ((MessWin [0] = newwin (1, 9, 1, 68)), _REVERSE);        /* score */
   wclear (MessWin [0]);
   wrefresh (MessWin [0]); 

   wsetattr ((IWin [0] = newwin (13, 2, 2, 66)), _BOLD);
   wsetattr ((IWin [1] = newwin (13, 2, 2, 77)), _BOLD);

   for (Hand = Contestant; Hand <= Porkling; Hand++)
      {
      mvwaddstr (IWin [0], Hand, 0, " g");                       /* tombstone */
      Players [Hand].Free = Hider [Hand].Free = False;            /* freedom! */
      }

   wsetattr ((BusWin = newwin (1, 1, 2, 66)), _BOLD);
   waddch (BusWin, '\140');                                        /* diamond */

   wsetattr ((MessWin [1] = newwin (1, 9, 15, 68)), _REVERSE);
   wclear (MessWin [1]);
   wrefresh (MessWin [1]); 

   wsetattr ((PadWin [0] = newwin (7, 13, 16, 66)), _BOLD);
   PadWin [1] = newwin (7, 9, 16, 68);

   wsetattr ((MessWin [2] = newwin (1, 9, 23, 68)), _REVERSE);
   wclear (MessWin [2]);
   wrefresh (MessWin [2]); 
   } /* define window */



MakePigPen (PigPen)
Matrix PigPen;
   {
   char Symbol;
   int  Row, Column;                              
                                      /* initialize pigpen using scale factor */
   for (Row = 1; Row <= (2 * Height + 1); Row++)
      {
      if (Row % 2 == 1)
         Symbol = '-';                                /* set horizontal walls */
      else
         Symbol = ' ';                            /* set pigpen square blanks */
      for (Column = 1; Column <= (4 * Width + 1); Column++)
         if (Column % 4 == 1)
            if (Row % 2 == 1)
               PigPen [Row][Column] = '+';                 /* set cross walls */
            else
               PigPen [Row][Column] = '|';              /* set vertical walls */
         else
            PigPen [Row][Column] = Symbol;    /* set horizontal wall or blank */
      }
   } /* make pigpen */

View (Text, Subscript, OldKey, Winner)
char          Text [][66];
int           Subscript, *OldKey;
enum PlayType Winner;
   {
   char *Wish [15]
        = { "The swine will show you the true nature of the FORCE.            ",
            "Your journey towards the dark side will be completed.            ",
            "You will be seduced by the dark side of the FORCE.               ",
            "Soon you will witness the triumph of pig power.                  ",
            "Never underestimated the power of the FORCE.                     ",
            "Your venture will be supported by the FORCE.                     ",
            "Our body shelters both a soul and a pig.                         ",
            "The FORCE will be with you, always.                              ",
            "Let the FORCE flow through you.                                  ",
            "Strong are you with the FORCE.                                   ",
            "May the FORCE be with you.                                       ",
            "Beware of the sow.                                               ",
            " R U L E ", " H E L P ", " L I S T " };
   WINDOW *ViewWin, *CRWin, *TimeWin;
   char   Dummy [1] = "";
   int    Row;

   ClearError ();
   News (Wish [Subscript + 11]);
   DisplayKey (Subscript + 10, OldKey);

   LeaveGraphic ();
   wsetattr ((TimeWin = newwin (1, 65, 1, 0)), _REVERSE);
   ShowTime (TimeWin, Subscript);

   ViewWin = newwin (21, 65, 2, 0);                      
   wsetattr ((CRWin = newwin (1, 65, 23, 0)), _BOLD);
   for (Row = 0; Row <= 20; Row++)
      mvwaddstr (ViewWin, Row, 0, Text [Row]); 
   wrefresh (ViewWin);

   mvwaddstr (CRWin, 0, 0, "To exit, press key <0>");
   wrefresh (CRWin);
   Row = 100 - (40 * (Winner == Win));
   while ( strcmp (Dummy, "0") != 0 )
      {
      NumberOfAlarm = Row;
      GetWindowString (BusWin, 0, 0, Dummy);
      }
   ClearError ();

   Subscript = rand () % 12;
   mvwaddstr (CRWin, 0, 0, Wish [Subscript]);
   wrefresh (CRWin);
   EnterGraphic ();

   delwin (TimeWin);
   delwin (ViewWin);
   delwin (CRWin);
   Prompt ("DIRECTION");
   } /* view */

ShowSlot (PigPen, Row, Column)                      /* VT101 line drawing set */
Matrix PigPen;
int    Row, Column;
   {
   char Symbol;

   switch (PigPen [Row][Column])
      {
      case ' ' : 
         Symbol = '\137';                                            /* blank */
         break;
      case '|' :
         Symbol = '\170';                                     /* vertical bar */
         break;
      case '-' :
         Symbol = '\161';                          /* horizontal line, scan 5 */
         break;
      case '+' :
         if (Row == 1)
            if (Column == 1)
               Symbol = '\154';                          /* upper left corner */
            else
            if (Column == (4 * Width + 1))
               Symbol = '\153';                         /* upper right corner */
            else                        
               Symbol = '\167';                                    /* top 'T' */
         else
         if (Row == (2 * Height + 1))
            if (Column == 1)
               Symbol = '\155';                          /* lower left corner */
            else
            if (Column == (4 * Width + 1))
               Symbol = '\152';                         /* lower right corner */
            else
               Symbol = '\166';                                 /* bottom 'T' */
         else
            if (Column == 1)
               Symbol = '\164';                                   /* left 'T' */
            else
            if (Column == (4 * Width + 1))
               Symbol = '\165';                                  /* right 'T' */
            else
               Symbol = '\156';                             /* crossing lines */
            break;
      } /* switch */                       
   mvwaddch (PenWin, (Row-1), (Column-1), Symbol);
   } /* show slot */



Occupied (Piece, PigPen)
Player Piece;
Matrix PigPen;
   {
   return (SameBed (PigPen, Piece) || PigPen [2*Piece.Y][4*Piece.X - 2] == '$');
   } /* Occupied */

ShowLine (PigPen, R, C, Row, Column, Slots)                   /* line drawing */
Matrix PigPen;
int    *R, *C, Row, Column, Slots;
   {
   int Loop;
   
   for (Loop = 1; Loop <= (Slots * 2); Loop++)
      {
      if ( (*R >= 1 && *R <= (2*Height+1)) && (*C >= 1 && *C <= (4*Width+1)) )
         ShowSlot (PigPen, *R, *C);
      *R = *R + Row;
      *C = *C + Column;
      }
   wrefresh (PenWin);
   } /* show line */



ShowPigPen (PigPen)                     /* display the pigpen from inside out */
Matrix PigPen;
   {
   int Row = ((2 * Height) / 2) + 1, Column = ((4 * Width) / 2) + 1;
   int Radius;

   PenWin = newwin ((2*Height+1), (4*Width+1), 1, 0); /* define PigPen window */
   ShowSlot (PigPen, Row, Column);                             /* show center */
   wrefresh (PenWin);

   for (Radius = 1; Radius <= 32; Radius++)
      {
      Row = Row + 1;
      Column = Column + 1;
      ShowLine (PigPen, &Row, &Column, -1,  0, Radius);                 /* up */
      ShowLine (PigPen, &Row, &Column,  0, -1, Radius);               /* left */
      ShowLine (PigPen, &Row, &Column,  1,  0, Radius);               /* down */
      ShowLine (PigPen, &Row, &Column,  0,  1, Radius);              /* right */
      } 
   } /* show pigpen */



char GetHideID (Hand)
enum HandType Hand;
   {
   char *Hide = "@@$$$$$$$$$$$";

   return (Hide [Hand]);
   } /* get hidden ID */



char GetHandID (Hand)
enum HandType Hand;
   {
   char *Token = "CSHBZYKLERING";

   return (Token [Hand]);
   } /* get hand ID */

ShowSlop (Piece, PigPen, Create)
Player       Piece;
Matrix       PigPen;
enum Boolean Create;
   {
   Player Bar [36];                                  /* obstruction positions */
   int    Row;

   for (Row = 0; Row <= 35; Row++)
      {
      do
         GetPosition (&Bar [Row]);
      while ( Occupied (Bar [Row], PigPen) || SameSquare (Piece, Bar [Row]) );
                                         /* enter obstruction into the pigpen */
      PigPen [2 * Bar [Row].Y] [4 * Bar [Row].X - 2] = '*';
      if (Create)
         DefineSlop (SlopWin, Bar [Row], Row, 0);
      else
         mvwin ( SlopWin [Row], 2 * Bar [Row].Y, 4 * Bar [Row].X - 2 );
      wrefresh (SlopWin [Row]);
      } 
   } /* show slop */



DefineSlop (SlopWin, Piece, Row, Blink)
WINDOW *SlopWin [];
Player Piece;
int    Row, Blink;
   {
   SlopWin [Row] = newwin ( 1, 1, 2 * Piece.Y, 4 * Piece.X - 2 ); 
   wsetattr (SlopWin [Row], _BOLD);
   if (Blink)
      wsetattr (SlopWin [Row], _BLINK | _UNDERLINE);
   waddch (SlopWin [Row], '*');
   } /* define slop */



GoHide (Piece, Hider, PigPen)
Player Piece, Hider [];
Matrix PigPen;
   {
   for (Hand = Contestant; Hand <= Porkling; Hand++)
      if ( !Hider [Hand].Free )
         {
         do
            GetPosition (&Hider [Hand]); 
         while ( Occupied (Hider [Hand], PigPen) );
         PigPen [2 * Hider [Hand].Y] [4 * Hider [Hand].X - 2] = '$';
         Hider [Hand].Free = True;
         }   
   } /* go hide */

MakePlayer (Players, PigPen, Select)
Player       Players [];
Matrix       PigPen;
enum Boolean Select;
   {
   for (Hand = Contestant; Hand <= Porkling; Hand++)         
      if (Select == True || !Players [Hand].Free)
         {
         do
            GetPosition (&Players [Hand]); 
         while ( Occupied (Players [Hand], PigPen) );
                                              /* enter player into the pigpen */
         PigPen [2*Players [Hand].Y] [4*Players [Hand].X - 2] = '@';
         if ( !Players [Hand].Free )
            {
            HandWin (PlayWin, Hand, 2*Players [Hand].Y, 4*Players [Hand].X-2, 
                                                              GetHandID (Hand));
            if (Hand == Contestant)
               {
               wclrattr (PlayWin [Contestant], _BLINK);
               box (PlayWin [Contestant], 'C', 'C');
               }
            Players [Hand].Free = True;
            }
         else
            mvwin (PlayWin [Hand], 2*Players [Hand].Y, 4*Players [Hand].X - 2);
         wrefresh (PlayWin [Hand]);
         }
   PigPen [2*Players [Contestant].Y] [4*Players [Contestant].X - 2] = ' ';
   } /* make player */



MakeBigWord ()                                           /* use VT101 command */
   {
   printf ("\033[1;1H");                                           /* go home */
   printf ("\033[0K");                    /* erase from cursor to end of line */
   printf ("\033[24A");                            /* move cursor up 24 lines */
   printf ("\033#6");                                     /* set double width */
   printf ("\033[2A");                              /* move cursor up 2 lines */
   } /* make big word */



Interrupt ()
   {
   GetOut (" EMERGENCY CLOSE: VMS CTRL/C interrupt  ", 5);
   } /* interrupt */



NoInterrupt ()
   {
   Control_C = True;
   Alert (" W A I T \15");
   } /* no interrupt */

GetPosition (Object)
Player *Object;
   {
   Object->Y = (rand () % Height) + 1;                        /* row position */
   do
      Object->X = (rand () % (Width + 1)) + 1;             /* column position */
   while (Object->X > Width);                                      /* rebound */
   } /* get position */



ShowMarker (Players, Hider, String)
Player Players [], Hider [];
char String [];
   {
   LeaveGraphic ();
   for (Hand = Contestant; Hand <= Porkling; Hand++)
      {
      if ( !Players [Hand].Free )
         {
         HandWin (MarkWin, Hand, Hand+2, 67, GetHandID (Hand));     
         wrefresh (MarkWin [Hand]); 
         TagWin [Hand] = subwin (ConWin, 1, 9, Hand+2, 68);
         if (Hand == Contestant)
            wsetattr (TagWin [Contestant], _BOLD);
         mvwaddstr (TagWin [Hand], 0, 0, Marker [Hand]);
         wrefresh (TagWin [Hand]);
         }
      if ( !Hider [Hand].Free )
         {
         HandWin (HideWin, Hand, Hand+2, 77, GetHideID (Hand));     
         wrefresh (HideWin [Hand]);
         }
      }
   wrefresh (IWin [1]);
   wrefresh (IWin [0]);
   wrefresh (ConWin);
   EnterGraphic ();
   } /* show marker */



ShowScore (MovesRemaining)
int MovesRemaining;
   {
   wprintw (MessWin [0], "SCORE %03d\15", MovesRemaining); 
   wrefresh (MessWin [0]);
   wclrattr (MessWin [0], _BOLD);
   } /* show moves remaining */

ShowKeyPad (OldKey)
int OldKey;
   {
   static char KeyPad [7] [9]
               = { " 7 8 9-R ",
                   "  \\|/  | ",
                   " 4-5-6-H ",
                   "  /|\\  | ", 
                   " 1 2 3\\  ",
                   " \\ / | L ",
                   "  0 -S/  " };
   Player KeyPos [14]
          = { { 22, 69 },
              { 20, 69 }, { 20, 71 }, { 20, 73 },
              { 18, 69 }, { 18, 71 }, { 18, 73 },
              { 16, 69 }, { 16, 71 }, { 16, 73 },
              { 22, 73 }, { 16, 75 }, { 18, 75 }, { 20, 75 } };
   int  Row, Column, Key;


   EnterGraphic ();
   for (Row = 1; Row <= 5; Row++)
      {
      mvwaddch (PadWin [0], Row, 1, '\170');                  /* vertical bar */
      mvwaddch (PadWin [0], Row, 11, '\170');                 /* vertical bar */
      }
   mvwaddch (PadWin [0], 0, 1, '\154');                  /* upper left corner */
   mvwaddch (PadWin [0], 0, 11, '\153');                /* upper right corner */
   mvwaddch (PadWin [0], 6, 1, '\155');                  /* lower left corner */
   mvwaddch (PadWin [0], 6, 11, '\152');                /* lower right corner */
   wrefresh (PadWin [0]);
   LeaveGraphic ();

   for (Key = 0; Key <= 13; Key++)
      {
      switch (Key)
         {
         case 0 :
            KeyWin [Key] = newwin (1, 3, KeyPos [Key].Y, KeyPos [Key].X);
            break;
         case 13 :
            KeyWin [Key] = newwin (3, 1, KeyPos [Key].Y, KeyPos [Key].X);
            break;
         default :
            KeyWin [Key] = newwin (1, 1, KeyPos [Key].Y, KeyPos [Key].X);
            break;
         }
      wsetattr (KeyWin [Key], _BOLD);
      }
   for (Key = 0; Key <= 13; Key++)
      DisplayKey (Key, OldKey);
   DisplayKey (5, OldKey);

   for (Row = 0; Row <= 6; Row++)
      mvwaddstr (PadWin [1], Row, 0, KeyPad [Row]);
   wrefresh (PadWin [1]);
   EnterGraphic ();
   } /* show keypad */

MakeDraft (A, N)
int A [], N;
   {
   int Index, Bound, Choice, Value;

   for (Bound = N; Bound >= 1; Bound--)
      {
      Choice = rand () % Bound;
      Value = A [Choice];
      for (Index = Choice; Index <= (N - 1); Index++)
         A [Index] = A [Index + 1];
      A [N] = Value;
      }
   Value = A [0];
   A [0] = A [N];
   A [N] = Value; 
   } /* make draft */



ContestantMove (Players, Hider, PigPen, Winner, PreGame, OldKey)
Player        Players [], Hider [];
Matrix        PigPen;
enum PlayType *Winner;
enum Boolean  PreGame;
int           *OldKey;
   {
   Player       Stone;
   int          Direction;
   enum Boolean Obstructed;

   CheckMove (BusWin, Contestant);
   if (DeadEnd)
      delwin (SlopWin [36 + (DeadEnd = 0)]);
   while ((Direction = GetKey (PlayWin [Contestant], OldKey, Winner)) == 10)
      Reset (Players, Hider, PigPen, True, True, PreGame, False, False);
   if (Direction == 0)
      *Winner = Quit;
   else
      if (Direction == 5)
         ClearError ();
      else
         {
         Players [OldMan] = Players [Contestant];
         MakeMove (&Players [Contestant], 1, Direction, PigPen, &Obstructed);
         
         if (Obstructed)
            {
            Alert ("WRONG WAY\15");
            Stone = Players [Contestant];
            MoveASquare (&Stone, Direction);
            DefineSlop (SlopWin, Stone, 36, DeadEnd = 1);
            wrefresh (SlopWin [36]);
            }
         else
            ClearError ();
         ShowPlayer (Players [Contestant], PlayWin [Contestant], Obstructed);
         CheckHide (BusWin, Contestant, Players [Contestant], PigPen);
         }
   } /* contestant move */

SowMove (Players, PigPen, Obstructed)
Player       Players [];
Matrix       PigPen;
enum Boolean *Obstructed;
   {
   Player       Pig;
   int          Row, Column = 0;                   
   int          Direction, Distance = 1, NOF;
   enum Boolean TryAgain, Leader;

   CheckMove (BusWin, Sow);
   Row = Locator (Players [Sow], Players [Contestant]) - 1;           /* seek */

   if ((*Obstructed = (Row == 4)) == False)
      { 
      Players [OldSow] = Players [Sow];
      Leader = Players [Pigling].Free;
      NOF = Census (Players, Sow);
      if ((GetDistant (Players [Sow], Players [Contestant]) > 9) || (NOF == 1))
         Row += 9;                                                    /* hide */

      do                                               /* make the sow's move */
         {
         Pig = Players [Sow];    /* assign sow's position to temporary player */
         Direction = PigWay [Row][Column++];
         MakeMove (&Pig, Distance, Direction, PigPen, Obstructed);
         if ( !*Obstructed )
            TryAgain = *Obstructed = SameBed (PigPen, Pig);
         else
         if ((TryAgain = (Column < 3)) == False)
            if ((TryAgain = (NOF <= 4)) == False)          /* alternate route */
               if ((TryAgain = (Row > 8)) == True)
                  {
                  Row -= 9;
                  Column = 0;
                  }
               else
               if ((TryAgain = Leader) == True) 
                  {
                  Leader = False;
                  Column = 0;
                  Row = Locator (Players [Sow], Players [Pigling]) - 1;
                  }
               else
                  TryAgain = Players [Pigling].Free == False;            
         }
      while ( TryAgain == True && Column < (3 + ((NOF <= 4) * 5))  );

      UpdatePigPen (PigPen, *Obstructed, &Players [Sow], Pig); 
      }
   } /* sow move */

HogMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Distance, Way [9] = { 1, 2, 3, 4, 6, 7, 8, 9, 0 };

   MakeDraft (Way, 8);
   Distance = (rand () % 15) + 1;                                  /* 1 .. 15 */
   MovePig (&Pig, Players, Hog, Way, Distance, 0, 8, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Hog], Pig, PlayWin [Hog]); 
   } /* hog move */



BoarMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column = 0;
   
   switch ( GetDistant (Players [Boar], Players [Contestant]) )
      {
      case 0 : /* escape */
         if (GetDistant (Players [Boar], Players [Sow]) > 2)
            {
            Row = Locator (Players [Boar], Players [Sow]) - 1;      
            Column = (Row == Locator (Players [Boar], Players [OldMan]) - 1);
            }
         else
            Row = UnderAttack (Boar, Players);
         break;
      case 1 : case 2 : case 3 : case 4 : /* hide */
         Row = Locator (Players [Boar], Players [Contestant]) + 8;  
         break;
      default : /* seek */
         Row = Locator (Players [Boar], Players [Contestant]) - 1; 
         break;
      }
 
   MovePig (&Pig, Players, Boar, PigWay [Row], 1, Column, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Boar], Pig); 
   } /* boar move */

HoggyMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column;
   int          LacThu [2] [5] = { { 1, 3, 7, 9, 0 }, { 2, 4, 6, 8, 0 } };
   
   if (GetDistant (Players [Hoggy], Players [Contestant]) == 0)
      Row = Locator (Players [Hoggy], Players [OldMan]) - 1;  
   else
      Row = Locator (Players [Hoggy], Players [Contestant]) - 1;  

   Column = (Row % 2) == 0;
   MakeDraft (LacThu [Column], 4);
   
   MovePig (&Pig, Players, Hoggy, LacThu [Column], 1, 0, 4, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Hoggy], Pig); 
   } /* hoggy move */



PiggyMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, PS, PH, PB;
                                                      /* decide direction set */
   switch ( GetDistant (Players [Piggy], Players [Contestant]) )
      {
      case 0 : /* escape */
         Row = UnderAttack (Piggy, Players);                       
         break;
      case 1 : /* hide */
         Row = Locator (Players [Piggy], Players [Contestant]) + 8;  
         break;
      default : /* seek */
         PS = GetDistant (Players [Piggy], Players [Sow]);
         PH = GetDistant (Players [Piggy], Players [Hog]);
         PB = GetDistant (Players [Piggy], Players [Boar]);
         if ( (PS <= PH) && (PS <= PB) )
            Row = Locator (Players [Piggy], Players [OldSow]) - 1;
         else
         if ( PH < PB )
            Row = Locator (Players [Piggy], Players [Hog]) - 1;
         else
            Row = Locator (Players [Piggy], Players [Boar]) - 1;
         break;
      }

   MovePig (&Pig, Players, Piggy, PigWay [Row], 1, 0, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Piggy], Pig); 
   } /* piggy move */

PorkyMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column = 0;
   
   switch ( GetDistant (Players [Porky], Players [Contestant]) )
      {
      case 0 : /* escape */
         if (GetDistant (Players [Porky], Players [Sow]) > 2)
            {
            Row = Locator (Players [Porky], Players [Sow]) - 1;      
            Column = (Row == Locator (Players [Porky], Players [OldMan]) - 1);
            }
         else
            Row = UnderAttack (Porky, Players);
         break;
      case 1 : /* hide */
         Row = Locator (Players [Porky], Players [Contestant]) + 8;
         break;
      default : /* wander */
         if (Players [Porky].X == 1)
            if (Players [Porky].Y == 1)
               Row = 5;                                               /* East */
            else
               Row = 7;                                              /* North */
         else
         if (Players [Porky].X == Width)
            if (Players [Porky].Y == Height)
               Row = 3;                                               /* West */
            else
               Row = 1;                                              /* South */
         else
            if (Players [Porky].Y < 6)
               Row = 5;                                               /* East */
            else
            if (Players [Porky].Y > 6)
               Row = 3;                                               /* West */
            else
               if (Players [Porky].X < 9)
                  Row = 3;                                            /* West */
               else
                  Row = 5;                                            /* East */
         break;
      }      
   
   MovePig (&Pig, Players, Porky, PigWay [Row], 1, Column, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Porky], Pig); 
   } /* porky move */

HogletMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row;
   int          HaDo [2] [5] = { { 1, 2, 3, 4, 0 }, { 6, 7, 8, 9, 0 } };

   if (GetDistant (Players [Hoglet], Players [Contestant]) == 0)
      Row = Locator (Players [Hoglet], Players [OldMan]) < 5;       /* escape */
   else
   if (GetDistant (Players [Hoglet], Players [Contestant]) < 9)
      Row = Locator (Players [Hoglet], Players [Contestant]) < 5;     /* hide */
   else   
      Row = Locator (Players [Hoglet], Players [Contestant]) > 5;     /* seek */
   
   MakeDraft (HaDo [Row], 4);
   MovePig (&Pig, Players, Hoglet, HaDo [Row], 1, 0, 4, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Hoglet], Pig, PlayWin [Hoglet]); 
   } /* hoglet move */



PigletMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column = 0;
   
   switch ( GetDistant (Players [Piglet], Players [Contestant]) )
      {
      case 0 : /* hide */
         if (GetDistant (Players [Piglet], Players [Sow]) > 2)
            {
            Row = Locator (Players [Piglet], Players [Sow]) - 1;      
            Column = (Row == Locator (Players [Piglet], Players [OldMan]) - 1);
            }
         else
            Row = UnderAttack (Piglet, Players);
         break;
      case 1 : /* allures */
         Row = Locator (Players [Piglet], Players [Contestant]) - 1;  
         Column = 1;
         break;
      default : /* extradite */
         if (GetDistant (Players [Piglet], Players [Sow]) > 2)
            Row = Locator (Players [Piglet], Players [Contestant]) - 1;
         else
            Row = Locator (Players [Piglet], Players [Sow]) - 1; 
         break;
      }

   MovePig (&Pig, Players, Piglet, PigWay [Row], 1, Column, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Piglet], Pig); 
   } /* piglet move */

PorkerMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column;

   if ( GetDistant (Players [Porker], Players [Contestant]) == 0)
      Row = UnderAttack (Porker, Players);                          /* escape */
   else
      Row = Locator (Players [Porker], Players [Contestant]) + 8;     /* hide */
   Column = GetDistant (Players [Porker], Players [Contestant]) > 9;

   MovePig (&Pig, Players, Porker, PigWay [Row], 1, Column, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Porker], Pig); 
   } /* porker move */



HoglingMove (Players, PigPen, Bar)
Player Players [];
Matrix PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column, Way [8];

   if (GetDistant (Players [Hogling], Players [Contestant]) == 0)
      Row = UnderAttack (Hogling, Players);                         /* escape */
   else
   if (GetDistant (Players [Hogling], Players [Contestant]) < 9)
      Row = Locator (Players [Hogling], Players [Contestant]) + 8;    /* hide */
   else   
      Row = Locator (Players [Hogling], Players [Contestant]) - 1;    /* seek */

   for (Column = 0; Column <= 6; Column++)
      Way [Column] = PigWay [Row][Column];
   Way [7] = 0;

   MakeDraft (Way, 7);
   MovePig (&Pig, Players, Hogling, Way, 1, 0, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Hogling], Pig, PlayWin [Hogling]); 
   } /* hogling move */

PiglingMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column = 0;
   
   switch ( GetDistant (Players [Pigling], Players [Contestant]) )
      {
      case 0 : /* extradite */
         if ( (SameSquare (Players [OldSow], Players [Sow]))
         && (GetDistant (Players [Pigling], Players [Sow]) > 2) )
            {
            Row = Locator (Players [Pigling], Players [Sow]) - 1;
            Column = (Row == Locator (Players [Pigling], Players [OldMan])- 1);
            }
         else        
            Row = UnderAttack (Pigling, Players);                   
         break;
      case 1 : /* hide */
         Row = Locator (Players [Pigling], Players [Contestant]) + 8;
         Column = (Row == Locator (Players [Pigling], Players [Sow]) - 1);
         if (Column && SameSquare (Players [OldSow], Players [Sow]))
            Row = Locator (Players [Pigling], Players [Sow]) - 1;
         break;
      case 2 : /* guide */  
         if (SameSquare (Players [OldSow], Players [Sow]))
            if (GetDistant (Players [Pigling], Players [Sow]) > 1) 
               Row = Locator (Players [Pigling], Players [Sow]) - 1;   
            else
               Row = Locator (Players [Pigling], Players [Contestant]) + 8;
         else
            Row = Locator (Players [Pigling], Players [Contestant]) - 1;
         break;
      default : /* seek */
         if (GetDistant (Players [Pigling], Players [Sow]) > 3) 
            Row = Locator (Players [Pigling], Players [Sow]) - 1;   
         else
            Row = Locator (Players [Pigling], Players [Contestant]) - 1;
         break;
      }

   MovePig (&Pig, Players, Pigling, PigWay [Row], 1, Column, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Pigling], Pig); 
   } /* pigling move */

PorklingMove (Players, PigPen, Bar)
Player       Players [];
Matrix       PigPen;
enum Boolean *Bar;
   {
   Player       Pig;
   int          Row, Column = 0, North, South, East, West;
   
   switch ( GetDistant (Players [Porkling], Players [Contestant]) )
      {
      case 0 : /* escape */
         if (GetDistant (Players [Porkling], Players [Sow]) > 2) 
            {
            Row = Locator (Players [Porkling], Players [Sow]) - 1;
            Column = (Row == Locator (Players [Porkling], Players [OldMan])- 1);
            }
         else        
            Row = UnderAttack (Porkling, Players);                   
         break;
      case 1 : /* hide */
         Row = Locator (Players [Porkling], Players [Contestant]) + 8;
         break;
      case 2 : case 3 : case 4 : case 5 : case 6 : case 7 : case 8 : case 9 :
         North = Players [Porkling].Y - 1;
         South = Height - Players [Porkling].Y;
         East  = Width - Players [Porkling].X;
         West  = Players [Porkling].X - 1;
         if (North < South)
            if (North)
               if (East < West)
                  Row = 8;                                       /* NorthEast */
               else
                  Row = 6;                                       /* NorthWest */
            else
               Row = 7;                                          /* North     */
         else
         if (North = South)
            if (East < West)
               Row = 5;                                               /* East */
            else
               Row = 3;                                               /* West */
         else
            if (South == 0)
               if (East < West)
                  Row = 2;                                       /* SouthEast */
               else
                  Row = 0;                                       /* SouthWest */
            else
               Row = 1;                                          /* South     */
         break;
      default : /* seek */
         Row = Locator (Players [Porkling], Players [Contestant]) - 1;
         break;
      }

   MovePig (&Pig, Players, Porkling, PigWay [Row], 1, Column, 7, Bar, PigPen);
   UpdatePigPen (PigPen, *Bar, &Players [Porkling], Pig); 
   } /* porkling move */

Locator (You, Pig)
Player You, Pig;
   {
   int dY, dX, Row;
                                                          /* decide direction */
   dY = You.Y - Pig.Y;
   dX = You.X - Pig.X;
   if (dY < 0)
      if (dX > 0)
         Row = 1;                                   /* object is at SouthWest */
      else
      if (dX == 0)
         Row = 2;                                       /* object is at South */
      else
         Row = 3;                                   /* object is at SouthEast */
   else
   if (dY == 0)
      if (dX > 0)
         Row = 4;                                        /* object is at West */
      else
      if (dX == 0)
         Row = 5;                                      /* object is at Center */
      else
         Row = 6;                                        /* object is at East */
   else
      if (dX > 0)
         Row = 7;                                   /* object is at NorthWest */
      else
      if (dX == 0)
         Row = 8;                                       /* object is at North */
      else
         Row = 9;                                   /* object is at NorthEast */
                                                      
   return (Row);
   } /* locator */



GetDistant (Pig, Contestant)
Player Pig, Contestant;
   {
   double dY, dX;

   dY = (double) Pig.Y - (double) Contestant.Y;
   dX = (double) Pig.X - (double) Contestant.X;

   return (sqrt ( (dY * dY) + (dX * dX) ));
   } /* get distant */

UnderAttack (Hand, Players)
enum HandType Hand;
Player        Players [];
   {
   if (GetDistant (Players [Hand], Players [OldMan]) > 1)
      return ( Locator (Players [Hand], Players [OldMan]) - 1 );      /* seek */
   else
      return ( Locator (Players [Hand], Players [OldMan]) + 8 );      /* hide */
   } /* under attack */



UpdatePigPen (PigPen, Bar, Piece, Pig)
Matrix       PigPen;
enum Boolean Bar;
Player       *Piece, Pig;
   {
   if ( !Bar )
      {
      LeavePigPen (PigPen, *Piece);
      EnterPigPen (PigPen, *Piece = Pig);                     /* move the pig */
      }
   } /* update pigpen */



EnterPigPen (PigPen, Piece)
Matrix PigPen;
Player Piece;
   {
   switch ( PigPen [2*Piece.Y] [4*Piece.X - 2] )
      {
      case ' ' :
         PigPen [2*Piece.Y] [4*Piece.X - 2] = '@';
         break;
      case '$' :
         PigPen [2*Piece.Y] [4*Piece.X - 2] = '#';
         break;
      }
   } /* enter pigpen */



LeavePigPen (PigPen, Piece)
Matrix PigPen;
Player Piece;
   {
   switch ( PigPen [2*Piece.Y] [4*Piece.X - 2] )
      {
      case '#' :
         PigPen [2*Piece.Y] [4*Piece.X - 2] = '$';
         break;
      case '@' :
         PigPen [2*Piece.Y] [4*Piece.X - 2] = ' ';
         break;
      }
   } /* leave pigpen */

SameBed (PigPen, Piece)
Matrix PigPen;
Player Piece;
   {
   enum Boolean Bar;

   switch ( PigPen [2*Piece.Y] [4*Piece.X - 2] )
      {
      case '#' : case '*' : case '@' :
         Bar = True;
         break;
      default :
         Bar = False;
         break;
      }
   return ( Bar );
   } /* same bed different dreams */



Census (Gamblers, Swine)
Player        Gamblers [];
enum HandType Swine;
   {
   int Count = 0;
   
   for (Hand = Swine; Hand <= Porkling; Hand++)
      Count = Count + Gamblers [Hand].Free;
  
   return ( Count );
   } /* census */



MovePig (Pig, Players, Swine, Way, Distance, Column, MaxMove, Bar, PigPen)
Player        *Pig, Players [];
enum HandType Swine;
int           Way [], Distance, Column, MaxMove;
enum Boolean  *Bar;
Matrix        PigPen;
   {
   int Direction;

   do                                                    /* make the pig move */
      {
      *Pig = Players [Swine];
      Direction = Way [ Column++ ];
      if ( (Direction < 1) || (Direction > 9) )
         GetOut (strcat (" ERROR: invalid direction of go", Marker [Swine]), 5);
      MakeMove (Pig, Distance, Direction, PigPen, Bar);
      }
   while ( (*Bar = SameBed (PigPen, *Pig)) == True && Column < MaxMove );
   } /* move the pig */

MakeMove (Piece, Distance, Direction, PigPen, Obstructed)
Player       *Piece;
int          Distance, Direction;
Matrix       PigPen;
enum Boolean *Obstructed;
   {
   Player Temp;                                           /* temporary player */

   *Obstructed = False;
   Temp = *Piece;             /* assign player's position to temporary player */
   while ( ((Distance > 0) && (!*Obstructed)) )
      {
      MoveASquare (&Temp, Direction);     /* move temporary player one square */
      Distance--;                         /* decrement distance by one square */
      *Obstructed = (PigPen [ 2 * Temp.Y ][ 4 * Temp.X - 2 ] == '*');
      if ( !*Obstructed)
         *Piece = Temp;                         /* move the player one square */
      } /* while */
   } /* make a move */



MoveASquare (Temp, Direction)             /* move a player a square at a time */
Player *Temp;
int    Direction;
   {
   switch (Direction)                                 /* get new row position */
      {
      case 7 : case 8 : case 9 :
         if (Temp->Y == 1)
            Temp->Y = Height;
         else
            Temp->Y--;
         break;
      case 1 : case 2 : case 3 :
         if (Temp->Y == Height)
            Temp->Y = 1;
         else
            Temp->Y++;
         break;
      }; /* switch */

   switch (Direction)                              /* get new column position */
      {
      case 1 : case 4 : case 7 :
         if (Temp->X == 1)
            Temp->X = Width;
         else
            Temp->X--;
         break;
      case 3 : case 6 : case 9 :
         if (Temp->X == Width)
            Temp->X = 1;
         else
            Temp->X++;
         break;
      }; /* switch */
   } /* move a square */

GetKey (PieceWin, OldKey, Winner)
WINDOW        *PieceWin;
int           *OldKey;
enum PlayType Winner;
   {
   char String [1];
   int  Key = 14;

   do
      {
      NumberOfAlarm = 60 + (30 * (Winner == Win));
      GetWindowString (PieceWin, 0, 0, String);

      if (isdigit (String [0]))
         DisplayKey (Key = atoi (String), OldKey);
      else
      if ( strcmp (String, ".") == 0 )
         DisplayKey (Key = 10, OldKey); /* reset */
      else
      if ( strcmp (String, "-") == 0 )
         View (Rule, 1, OldKey, Winner);
      else
      if ( strcmp (String, ",") == 0 )
         View (Help, 2, OldKey, Winner);
      else
      if ( String[0] == '\0' ) 
         View (List, 3, OldKey, Winner);
      else 
         Alert (" BAD KEY \15"); 
      }
   while (Key == 14);
   
   return (Key);
   } /* get key */



DisplayKey (Key, OldKey)
int Key, *OldKey;
   {
   wclrattr (KeyWin [*OldKey], _BLINK | _REVERSE);
   TurnKey (KeyWin, *OldKey);
   wsetattr (KeyWin [Key], _BLINK | _REVERSE);
   TurnKey (KeyWin, Key);
   *OldKey = Key;
   } /* display key */



ClearError ()
   {
   wclrattr (MessWin [1], _BOLD);
   wclrtobot (MessWin [1]);
   wrefresh (MessWin [1]); 
   } /* clear error */

TurnKey (KeyWin, Key)
WINDOW *KeyWin [];
int    Key;
   {
   static char KeyID [14]
          = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
              'S', 'R', 'H', 'L' };

   switch (Key)
      {
      case 0 :
         mvwaddstr (KeyWin [0], 0, 0, " 0 ");
         break;
      case 13 :
         mvwaddch (KeyWin [13], 0, 0, ' ');
         mvwaddch (KeyWin [13], 1, 0, 'L');
         mvwaddch (KeyWin [13], 2, 0, ' ');
         break;
      default :
         waddch (KeyWin [Key], KeyID [Key]);
         break;
      }
   wrefresh (KeyWin [Key]);   
   } /* turn key */



Prompt (S)
char *S;
   {
   wclrattr (MessWin [2], _BOLD);
   mvwaddstr (MessWin [2], 0, 0, S);
   wrefresh (MessWin [2]);
   HideCursor ();
   } /* prompt to user */



News (String)
char String [];
   {
   wsetattr (MessWin [2], _BOLD);
   mvwaddstr (MessWin [2], 0, 0, String);
   wrefresh (MessWin [2]);
   HideCursor ();
   } /* news */



HideCursor ()
   {
   wmove (BusWin, 0, 0);
   wrefresh (BusWin);
   } /* hide cursor */

Alert (String)
char String [];
   {
   RingBell ();
   wsetattr (MessWin [1], _BOLD);
   mvwaddstr (MessWin [1], 0, 0, String);
   wrefresh (MessWin [1]);
   } /* Red alert */



ShowGame (Trial)
int Trial;
   {
   wclrattr (MessWin [1], _BOLD);
   wprintw (MessWin [1], " GAME #%d \15", Trial);
   wrefresh (MessWin [1]);
   }



TimeOut ()
   {
   wsetattr (MessWin [1], _BOLD); 
   wprintw (MessWin [1], "CLOCK %03d\15", NumberOfAlarm);
   wrefresh (MessWin [1]);

   if (--NumberOfAlarm)
      {
      signal (SIGALRM, TimeOut);  
      alarm (1);
      }
   else
      GetOut ("            T I M E   O U T             ", 4);
   } /* time out */



GetOut (String, Attribute)
char String [];
int  Attribute;
   {
   OffCursor ();
   ShowBigWord (String, True, 0, Attribute);
   touchwin (stdscr);                                   /* clear the overlaps */
   LeaveGraphic ();
   OnCursor ();
   SetProcessName ();                                     /* back to old name */
   EnableControl (LIB$M_CLI_CTRLT);                       /* enable control/T */
   EnableControl (LIB$M_CLI_CTRLY);                       /* enable control/Y */
   if (Attribute != 4)
      _exit (); 
   printf ("\n\n");
   for (Attribute = 1; Attribute <= 1947792; Attribute++);          /*  sleep */
   SYS$DASSGN (Channel);                                /* deassign a channel */
   SYS$DELPRC (0, 0);                                /* delete process itself */
   } /* get out */

ShowBanner (Hand, PlayWin, Players, MovesRemaining, Winner, PigPen)
enum HandType Hand;
WINDOW        *PlayWin [];
Player        Players [];
int           *MovesRemaining;
enum PlayType Winner;
Matrix        PigPen;
   {
   static char *Greeting [13]
               = { " WHOP!... WHAM!... WHANG!... OOUCH! ... ", 
                   " BRAVO! You have caught the tough sow   ",
                   " WUPS! You have caught the horned hog   ",
                   " EEGAH! You have caught the wild boar   ",
                   " JAB! You have caught the greedy hoggy  ",
                   " HUMMM! You have caught the dumb piggy  ",
                   " AHH! You have caught the stupid porky  ",
                   " GEE! You have caught the wacky hoglet  ",
                   " YEAH! You have caught the fair piglet  ",
                   " WOW! You have caught the nasty porker  ",
                   " OOPS! You have caught the wart hogling ",
                   " BANG! You have caught the wise pigling ",
                   " ACK! You have caught the smart porkling" };
 
   delwin (PlayWin [Hand]);
   ShowBigWord (Greeting [Hand], True, (Hand > Sow), 0);
   LeaveGraphic ();
   delwin (TagWin [Hand]); 
   EnterGraphic ();
   delwin (MarkWin [Hand]);   
   if (Winner != Die)
      LeavePigPen (PigPen, Players [Hand]);
   Players [Hand].X = 67;                               /* tombstone position */
   Players [Hand].Y = Hand + 2;                            /* marker position */
   Players [Hand].Free = False;
   *MovesRemaining += ( (Award - (24 * (Hand == Sow)) ) * (Winner != Die) );
   wsetattr (MessWin [0], _BOLD);
   if (Winner == Play) 
      ShowBigWord ("       GREASED PIG CONTEST              ", False, 0, 1);
   } /* show banner */



ShowPlayer (Piece, PieceWin, Obstructed)
Player       Piece;
WINDOW       *PieceWin;
enum Boolean Obstructed; 
   {
   if ( !Obstructed )
      {
      mvwin (PieceWin, (2 * Piece.Y), (4 * Piece.X - 2) );
      wrefresh (PieceWin);
      }
   } /* show player */

CheckCPU ()
   {
   if (CPULIM)                                                /* infinite = 0 */
      if ((CPULIM - GetCPUtime (JPI$_CPUTIM, pid, "elapsed CPU")) <= 1001)
         GetOut (" EMERGENCY CLOSE: CPU time limit expired", 4);
   return (True);
   } /* check CPU time */



Finish (Winner, Score, OldKey, Total, TopTen)
enum PlayType    *Winner;
int              Score, OldKey, Total [];
struct Candidate TopTen [];
   {
   static char *Message [4]
               = { "OH! OH! 'Cold Feet' Chicken, we miss you",        /* quit */
                   "HA! HA! HA! Death Meat, catch you later ",        /* die  */
                   "OINK! OINK! You have run out of moves   ",        /* lose */
                   "CONGRATULATIONS! You won the GP contest " };      /* win  */
   struct Candidate TenTop [10];

   if ((*Winner == Play) && (Score < 1))
      *Winner = Lose;
   ShowBigWord (Message [*Winner], True, (*Winner != Quit), 3+(*Winner==Quit));
   ClearError ();
   if ( *Winner != Die )
      {
      wsetattr (PlayWin [Contestant], _BLINK);
      waddch (PlayWin [Contestant], 'C');
      wrefresh (PlayWin [Contestant]);
      }

   Total [*Winner] = Total [*Winner] + 1;
   Total [Play] = Total [Play] + 1;
   if (*Winner == Win && Score > TopTen[9].Score)
      if (Score > TopTen [9].Score)
         UpdatePigScore (TopTen, GameMaster [10], Total [Play], Score, OldKey);
      else
         GetOut (" CANCEL: your score isn't high enough   ", 5);
   if (*Winner != Quit)
      GameOver ();

   CopyTopTen (TenTop, TopTen);
   Encode (TenTop, Total);
   signal (SIGINT, NoInterrupt);
   WriteTopTen (TenTop, Total);
   PutLast (*Winner, Score, Total [Play]);
   if (Control_C)
      Interrupt ();
   signal (SIGINT, Interrupt);
   } /* finish */

WriteTopTen (C, Total)
struct Candidate C [];
int              Total [];
   {
   int Draft [11] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
   int i;

   if ((file_ptr = fopen (file_score, "r+")) == NULL)
      GetOut (" ERROR: P-S file can't open for updating", 5);

   MakeDraft (Draft, 10);

   fprintf (file_ptr, "%d %o %X %o %d %d %o %X %o %d %d %o %X %o %d\n", 
           Draft [0],    Draft [1],   Draft [2],    Draft [3],   Draft [4],
           Total [Quit], Total [Die], Total [Lose], Total [Win], Total [Play],
           Draft [5],    Draft [6],   Draft [7],    Draft [8],   Draft [9]);
   for (i = 0; i <= 9; i++)
      {
      fprintf (file_ptr, "%s", C [ Draft[i] ].Home); 
      fprintf (file_ptr, "%s", C [ Draft[i] ].User); 
      fprintf (file_ptr, "%s", C [ Draft[i] ].Base); 
      fprintf (file_ptr, "%x %o %d %x %o %o %x %d %o %x\n", 
         C [ Draft[i] ].Score,  C [ Draft[i] ].Number,
         C [ Draft[i] ].Year,   C [ Draft[i] ].Month,  C [ Draft[i] ].Day,
         C [ Draft[i] ].Hour,   C [ Draft[i] ].Minute, C [ Draft[i] ].Second,
         C [ Draft[i] ].Member, C [ Draft[i] ].Group);
      }
   fclose (file_ptr);
   } /* write top ten */


CountSymbol (PigPen, S)
Matrix PigPen;
char   S [];
   {
   int R, C, Count = 0;

   for (R = 1; R <= (2 * Height + 1); R++)
      for (C = 1; C <= (4 * Width + 1); C++)
         Count = Count + (PigPen [R][C] == S[0] || PigPen [R][C] == S[1]);
   return (Count);
   } /* count symbol */

GetCPUtime (item_code, pid, message)
int item_code, pid;
char *message;
   {
   int out_value;

/* LIB$GETJPI item-code, [pid], [prcnam], [out value], [out strng], [out len] */
   if (LIB$GETJPI (&item_code, &pid, 0, &out_value, 0, 0) != SS$_NORMAL)
      GetOut (strcat (" ERROR: $GETJPI can't get ", message), 5);
   else
      return (out_value); 
   } /* get CPU time */



SetProcessName ()                                      /* SYS$SETPRN [prcnam] */
   {
   $DESCRIPTOR (OldPN_desc, OldName);
   if (SYS$SETPRN (&OldPN_desc) != SS$_NORMAL)
      GetOut ("ERROR: SYS$SETPRN can't set process name", 5);
   } /* set process name */



GetProcessName (out_string)
char out_string [];
   {
   char pcnam_dest [16];
   $DESCRIPTOR (pcnam_desc, pcnam_dest);
   int item_code = JPI$_PRCNAM, out_len;

/* LIB$GETJPI item-code, [pid], [prcnam], [out value], [out strng], [out len] */
   if (LIB$GETJPI (&item_code, &pid, 0, 0, &pcnam_desc, &out_len) != SS$_NORMAL)
      GetOut (" ERROR: $GETJPI can not get process name", 5);
   else
      pcnam_dest [out_len] = '\0'; 
   strcpy (out_string, pcnam_dest); 
   } /* get process name */



GetNodeName (out_string)
char out_string [];
   {
   int item_code = SYI$_NODENAME, out_len;
   char nodenam_dest [16]; 
   $DESCRIPTOR (nodenam_desc, nodenam_dest);

/* LIB$GETSYI item-code, [out valu], [out str], [out len], [csid], [node nam] */
   if (LIB$GETSYI (&item_code, 0, &nodenam_desc, &out_len, 0, 0) != SS$_NORMAL)
      GetOut (" ERROR: LIB$GETSYI can not get node name", 5);
   else
      nodenam_dest [out_len] = '\0'; 
   strcpy (out_string, nodenam_dest); 
   } /* get node name */

AssignChannel (Channel)        /* SYS$ASSIGN devnam, chan, [acmode], [mbxnam] */
short int *Channel;
   {
   char terminal_dest [20] = "                   "; 
   $DESCRIPTOR (terminal_desc, terminal_dest);
   
   ctermid (terminal_dest);
   if (SYS$ASSIGN (&terminal_desc, Channel, 0, 0) != SS$_NORMAL)
      GetOut (" ERROR: SYS$ASSIGN can't get a channel  ", 5);
   } /* assign channel */



GetDeviceName (Channel, out_string)
short int Channel;
char out_string [];
   {
   int item = DVI$_DEVNAM, out_len;
   char devnam_dest [256]; 
   $DESCRIPTOR (devnam_desc, devnam_dest);

/* LIB$GETDEV item-code, [chan], [dev name], [out valu], [out str], [out len] */
   if (LIB$GETDVI (&item, &Channel, 0, 0, &devnam_desc, &out_len) != SS$_NORMAL)
      GetOut (" ERROR: LIB$GETDVI can't get device name", 5);
   else
      devnam_dest [out_len] = '\0'; 
   strcpy (out_string, devnam_dest); 
   } /* get device name */



DisableControl (disable_msk)       /* LIB$DISABLE_CTRL disable-msk, [old-msk] */
long disable_msk;
   {
   int ret;

   ret = LIB$DISABLE_CTRL (&disable_msk, 0);
   if (ret != SS$_NORMAL)
      {
      fprintf (stderr, "LIB$DISABLE_CTRL can't set no control\n");
      exit (ret);
      }
   } /* disable control */



EnableControl (enable_msk)           /* LIB$ENABLE_CTRL enable-msk, [old-msk] */
long enable_msk;
   {
   int ret;

   ret = LIB$ENABLE_CTRL (&enable_msk, 0);
   if (ret != SS$_NORMAL)
      {
      fprintf (stderr, "LIB$ENABLE_CTRL can't set control\n");
      exit (ret);
      }
   } /* enable control */

UpdatePigScore (TopTen, Best, PlayerNumber, MovesRemaining, OldKey)
struct Candidate TopTen [], Best;
int              PlayerNumber, MovesRemaining, OldKey;
   {
   WINDOW *BestWin [2];
   int    Index = -1;

   Best.Score  = MovesRemaining;
   Best.Number = PlayerNumber;
   Best.Member = getuid ();
   Best.Group  = getgid ();
   MakeBest (&Best);

   while ( (strcmp (Best.Name, "   ") == 0) && (++Index <= 9) )
      if (strcmp (TopTen [Index].Home, Best.Home) == 0)              /* match */
         strcpy (Best.Name, TopTen [Index].Name);

   if ( ((strcmp (Best.Name, "   ") != 0) && (Best.Score > TopTen[Index].Score))
     || (strcmp (Best.Name, "   ") == 0) )
      {
      Alert (" W A I T \15");
      if (strcmp (Best.Name, "   ") == 0)
         TopTen [9] = Best;                               /* last user's data */
      else
         TopTen [Index] = Best;                            /* old user's data */
      BubbleSort (TopTen, 10);
      ShowBigWord ("MENU: <0>store <5>undo <.>rename <,>help", False, 0, 6);
      Index = 0;
      while ((strcmp (TopTen [Index].Home, Best.Home) != 0) && (Index++ <= 9));
      if (strcmp (Best.Name, "   ") == 0)                      /* new winnner */
         strcpy (TopTen [Index].Name, "AAA");
      ShowTopTen (BestWin, TopTen);
      GetPlayerName (BestWin [0], Index, OldKey, TopTen [Index].Name);

      MoveString (TopTen [Index].Base, TopTen [Index].Name, 0, 2, 15);
      TopTen [Index].User [28] = TopTen [Index].Name [2];
      TopTen [Index].User [29] = TopTen [Index].Name [1];
      TopTen [Index].User [30] = TopTen [Index].Name [0];
      BubbleSort (TopTen, 10);
      }
   else
      GetOut (" CANCEL: new score isn't higher than old", 5);
   } /* update pig scores */

MakeBest (Best)
struct Candidate *Best;
   {
   char      String [32];
   struct tm *time_structure;
   long      time_val;

   strncpy (String, getenv ("HOME"), 30);       /* the user's login directory */
   sprintf (Best->Home, "%-30.30s1%2d 00\n", String, strlen (String));

   sprintf (Best->User, "%-9.9s%-9.9s%03d%06d%d   00\n",
       UserName, getenv ("TERM"), Best->Score, Best->Number, strlen (UserName));

   time (&time_val);
   time_structure = localtime (&time_val);
   Best->Year  = time_structure->tm_year;
   Best->Month = time_structure->tm_mon + 1;
   Best->Day   = time_structure->tm_mday;       
   sprintf (Best->Base, "%02d%02d%02d%.3s%02d%02d%02d   %06d00\n", 
        Best->Year, Best->Month, Best->Day, ctime (&time_val), 
        time_structure->tm_hour, time_structure->tm_min, time_structure->tm_sec,
        Best->Number);
   } /* make best */



GetPigName (Alias, OldName, ProcessName, UserName)
char Alias [], OldName [], ProcessName [], UserName [];
   {
   int i = -1, j, k = 0;
   char prcnam [16], A [16];

   strncpy (UserName, getenv ("USER"), 9);
   GetProcessName (ProcessName);
   j = strlen (ProcessName);
   while ( (i <= j) && (ProcessName [++i] == ' ') );    /* skip leading blank */
   while ( (j >= 1) && (ProcessName [--j] == ' ') );   /* skip trailing blank */
   MoveString (prcnam, ProcessName, i, j, 0);
   prcnam [(j-i) + 1] = '\0';
   sprintf (OldName, "%-15s", prcnam);        /* copy process name for rename */
   j = strlen (prcnam);
   if ( j > 9 )
      for (i = 0; i <= (j - 1); i++)
         if (prcnam [i] != ' ')
            A [k++] = prcnam [i];                      /* skip internal blank */
   A [k] = '\0';
   if ( j > 0 && j < 9 )
      strcpy (Alias, prcnam);
   else
   if (strlen (A) == 0 || strlen (A) > 9)
      strcpy (Alias, UserName);
   else
      strcpy (Alias, A);
   } /* get pig name */

GetPlayerName (BestWin, Row, OldKey, BestName)
WINDOW        *BestWin;
int           Row, OldKey;
char          BestName [];
   {
   static char *Initials [3]
               = { " enter your FIRST initial               ", 
                   " enter your MIDDLE initial              ",
                   " enter your LAST initial                " };
   WINDOW      *MyWin;
   char        MyName [3];
   int         Column, Key = 10;

   MyWin = subwin (BestWin, 1, 1, (Row + 8), 28); 
   do
      {
      switch ( Key )
         {
         case 1 : case 2 : case 3 : /* down */
            if (MyName [Column] == 'A')
               MyName [Column] = 'Z';
            else
               MyName [Column]--;
            Key = Key + 3;
            break;
         case 5 : /* re-initiate */
            MyName [Column] = BestName [Column];
            break;
         case 7 : case 8 : case 9 : /* up */
            if (MyName [Column] == 'Z')
               MyName [Column] = 'A';
            else
               MyName [Column]++;
            Key = Key - 3;
            break;
         case 10 : /* rename */
            strcpy (MyName, BestName);
            mvwaddstr (BestWin, (Row + 1), (Column = 0) + 1, MyName);
            break;
         }
      if (Key == 4) /* left */
         Column = Column - (Column > 0) + (2 * (Column == 0));     
      else
      if (Key == 6) /* right */
         Column = Column + (Column < 2) - (Column * (Column == 2));

      ShowBigWord (Initials [Column], False, 0, 2);
      mvwin (MyWin, (Row + 8), (Column + 28));
      waddch  (MyWin, MyName [Column]); 
      wrefresh (MyWin);
      wrefresh (BestWin); 
      }
   while ( (Key = GetKey (MyWin, OldKey, Win)) != 0 );
   ClearError ();
   strcpy (BestName, MyName);
   delwin (MyWin);
   News (" E X I T ");
   } /* get player name */

ShowTopTen (BestWin, TopTen)
WINDOW           *BestWin [2];
struct Candidate TopTen [];
   {
   WINDOW     *GoldWin[2];
   static int S[2] = { -2, 2 };
   int        Index, Column;

   for (Index = 0; Index <= 1; Index++)
      {
      GoldWin[Index] = newwin (13, 7, 6, (36 - ((14 * Index))));
      wsetattr (GoldWin[Index], _REVERSE);
      box (GoldWin[Index], ' ', ' ');
      BestWin[Index] = newwin (11, 6, 7, (37 - ((15 * Index))));
      wsetattr (BestWin[Index], _BOLD);                     
      }
   mvwaddstr (BestWin[0], 0, 0, "  TOP ");
   mvwaddstr (BestWin[1], 0, 0, " TEN  ");

   for (Index = 0; Index <= 9; Index++)
      {
      wmove (BestWin [0], Index+1, 0);
      wprintw (BestWin [0], " %s  ", TopTen[Index].Name);
      wmove (BestWin [1], Index+1, 0);
      wprintw (BestWin [1], "  %3d ", TopTen[Index].Score);
      }
   for (Column = 0; Column <= 5; Column++)
      for (Index = 0; Index <= 1; Index++)
         {
         mvwin (GoldWin [Index], 6, ((36 - (14*Index)) + (S[Index]*Column)));
         wrefresh (GoldWin [Index]);
         mvwin (BestWin [Index], 7, ((37 - (15*Index)) + (S[Index]*Column)));
         wrefresh (BestWin [Index]); 
         }
   } /* show top ten */



BubbleSort (C, n)                                 /* sort in descending order */
struct Candidate C [];
int              n;                                 /* number of best players */
   {
   struct Candidate Temp;
   int              i, j;

   for (i = 0; i < (n - 1); ++i)
      for (j = (i + 1); j < n; ++j)
         if ((C[i].Score < C[j].Score) || 
            ((C[i].Score == C[j].Score) && (strcmp (C[i].Base, C[j].Base) > 0)))
            { 
            Temp = C [i];
            C[i] = C[j];
            C[j] = Temp;
            }
   } /* bubble sort */

Decode (TopTen, Total)
struct Candidate TopTen [];
int              Total [];
   {
   int Index;

   for (Index = 0; Index <= 9; Index++)
      {
      TopTen [Index].Score  = TopTen [Index].Score  + Total [Quit];
      TopTen [Index].Number = TopTen [Index].Number + Total [Die];
      TopTen [Index].Year   = TopTen [Index].Year   - Total [Lose];
      TopTen [Index].Month  = TopTen [Index].Month  - Total [Win];
      TopTen [Index].Day    = TopTen [Index].Day    - Total [Play];

      Crypt (&TopTen [Index], Total, Index, -1);

      if ( TopTen [Index].Home [34] == '1' )
         Reverse (TopTen [Index].Home, '0', 34);
      else
      if ( TopTen [Index].Home [34] != '0' )
         Counterfeit ();
      if ( TopTen [Index].User [31] == '1' )
         Reverse (TopTen [Index].User, '0', 31);
      else
      if ( TopTen [Index].User [31] != '0' )
         Counterfeit ();
      if ( TopTen [Index].Base [24] == '1' )
         Reverse (TopTen [Index].Base, '0', 24);
      else
      if ( TopTen [Index].Base [24] != '0' )
         Counterfeit ();

      if ( TopTen [Index].Home [35] > '0' && TopTen [Index].Home [35] <= '9' )
         Mix (TopTen [Index].Home, '0', 35, CTI (TopTen [Index].Home, 35, 35));
      else
      if ( TopTen [Index].Home [35] != '0' )
         Counterfeit ();
      if ( TopTen [Index].User [32] > '0' && TopTen [Index].User [32] <= '9' )
         Mix (TopTen [Index].User, '0', 32, CTI (TopTen [Index].User, 32, 32));
      else
      if ( TopTen [Index].User [32] != '0' )
         Counterfeit ();
      if ( TopTen [Index].Base [25] > '0' && TopTen [Index].Base [25] <= '9' )
         Mix (TopTen [Index].Base, '0', 25, CTI (TopTen [Index].Base, 25, 25));
      else
      if ( TopTen [Index].Base [25] != '0' )
         Counterfeit ();

      Decipher (TopTen [Index].Home, GameMaster [Index].Home,  33);
      Decipher (TopTen [Index].User, GameMaster [Index].User,  30);
      Decipher (TopTen [Index].Base, GameMaster [Index].Base,  23);
      }
   } /* decode */

Encode (TenTop, Total)
struct Candidate TenTop [];
int              Total [];
   {
   struct tm *time_structure;
   long      time_val;
   int       Index, m;

   for (Index = 0; Index <= 9; Index++)
      {
      time (&time_val);
      time_structure = localtime (&time_val);
      TenTop [Index].Second = time_structure->tm_sec;
      TenTop [Index].Minute = time_structure->tm_min;
      TenTop [Index].Hour   = time_structure->tm_hour;

      Encipher (TenTop [Index].Home, GameMaster [Index].Home,  33);
      Encipher (TenTop [Index].User, GameMaster [Index].User,  30);
      Encipher (TenTop [Index].Base, GameMaster [Index].Base,  23);

      if ((m = rand () % 10) > 0)
         Mix (TenTop [Index].Home, BaseDigit (m), 35, m);
      if ((m = rand () % 7) > 0)
         Mix (TenTop [Index].User, BaseDigit (m), 32, m);
      if ((m = rand () % 5) > 0)
         Mix (TenTop [Index].Base, BaseDigit (m), 25, m);

      if ( rand () % 2 )
         Reverse (TenTop [Index].Home, '1', 34);
      if ( rand () % 2 )
         Reverse (TenTop [Index].User, '1', 31);
      if ( rand () % 2 )
         Reverse (TenTop [Index].Base, '1', 24);

      Crypt (&TenTop [Index], Total, Index, 1);

      TenTop [Index].Score  = TenTop [Index].Score  - Total [Quit];
      TenTop [Index].Number = TenTop [Index].Number - Total [Die];
      TenTop [Index].Year   = TenTop [Index].Year   + Total [Lose];
      TenTop [Index].Month  = TenTop [Index].Month  + Total [Win];
      TenTop [Index].Day    = TenTop [Index].Day    + Total [Play];
      }
   } /* encode */

Decipher (Text, Key, End)
char Text [], Key [];
int  End;
  {
  int i;
  char c, k;

  for (i = 0; i <= End; i++)
     if ((Text [i] != ' ') && (Key [i] != ' '))
        {
        k = Key [i];
        for (c = '!'; k != Text [i]; c++)
           k = Next (k);
        Text [i] = c;
        }
  } /* decipher */



Encipher (Text, Key, End)
char Text [], Key [];
int  End;
  {
  int i;
  char c, k;

  for (i = 0; i <= End; i++)
     if ((Text [i] != ' ') && (Key [i] != ' '))
        {
        k = Key [i];
        for (c = '"'; c <= Text [i]; c++)
           k = Next (k);
        Text [i] = k;
        }
  } /* encipher */



ConvertCPUtime (CPUtime, S)
int  CPUtime;
char S [];
   {
   int d, h, m, s, t;
   
   d = CPUtime / 8640000;
   t = CPUtime % 8640000;
   h = t / 360000;
   t = t % 360000;
   m = t / 6000;
   t = t % 6000;
   s = t / 100;
   t = t % 100;
   sprintf (S, "%d %02d:%02d:%02d.%02d", d, h, m, s, t);
   } /* convert CPU time into string */

Crypt (Best, Total, i, AccessMode)
struct Candidate *Best;
int              Total [], i, AccessMode;
   {
   Disguise (Best->Home, Key (Best->Hour),     0,  5, AccessMode);
   Disguise (Best->Home, Key (Total [Quit]),   6, 11, AccessMode);
   Disguise (Best->Home, Key (Best->Group),   12, 17, AccessMode);
   Disguise (Best->Home, Key (Total [Die]),   18, 23, AccessMode);
   Disguise (Best->Home, Key (Best->Day),     24, 33, AccessMode);

   Disguise (Best->User, Key (Best->Minute),   0,  8, AccessMode);
   Disguise (Best->User, Key (Best->Score),    9, 17, AccessMode);
   Disguise (Best->User, Key (Total [Play]),  18, 20, AccessMode);
   Disguise (Best->User, Key (Best->Number),  21, 26, AccessMode);
   Disguise (Best->User, Key (Best->Month),   27, 30, AccessMode);

   Disguise (Best->Base, Key (Best->Second),   0,  5, AccessMode);
   Disguise (Best->Base, Key (Total [Win]),    6,  8, AccessMode);
   Disguise (Best->Base, Key (Best->Member),   9, 14, AccessMode);
   Disguise (Best->Base, Key (Total [Lose]),  15, 17, AccessMode);
   Disguise (Best->Base, Key (Best->Year),    18, 23, AccessMode);

   Disguise (Best->Home, Key (Total [Play]),   0, 33, AccessMode);
   Disguise (Best->User, Key (Total [Play]),   0, 30, AccessMode);
   Disguise (Best->Base, Key (Total [Play]),   0, 23, AccessMode); 
   } /* crypt */



Disguise (Text, Key, Start, End, AccessMode)
char Text [];
int  Key, Start, End, AccessMode;
   {                                           
   int i;

   for (i = Start; i <= End; i++)
      Text [i] = Text [i] + (Key * AccessMode); 
   } /* Disguise */



char Next (c)
char c;
  {
  if (c == '~')
     c = '!';
  else
     c++;
  return (c);
  } /* next */

Mix (S, C, Index, M)
char S [], C;
int  Index, M;
   {
   int L [9] = { 3, 8, 14, 19, 23, 29, 30, 32, 33 }, i;

   for (i = 0; i <= L [M - 1]; i++)
      if (S [i] >= '!' && S [i] <= '/')                         
         S [i] = S [i] + 79;                                   /* from p to ~ */
      else
      if (S [i] >= '0' && S [i] <= '?')                         
         S [i] = S [i] + 48;                                   /* from ` to o */
      else
      if (S [i] >= '@' && S [i] <= 'O')                          
         S [i] = S [i] + 16;                                   /* from P to _ */
      else
      if (S [i] >= 'P' && S [i] <= '_')                          
         S [i] = S [i] - 16;                                   /* from @ to O */
      else
      if (S [i] >= '`' && S [i] <= 'o')                         
         S [i] = S [i] - 48;                                   /* from 0 to ? */
      else
      if (S [i] >= 'p' && S [i] <= '~')                        
         S [i] = S [i] - 79;                                   /* from ! to / */
      else
      if (S [i] != ' ')
         Counterfeit ();
   S [Index] = C;
   } /* mix */



Reverse (S, C, End)     /* Brian W. Kernighan, The C progamming language, p59 */
char S [], C;
int  End;
   {
   int c, i, j;

   for (i = 0, j = (End - 1); i < j; i++, j--)
      {
      c = S [i];
      S [i] = S [j];
      S [j] = c;
      }
   S [End] = C;
   } /* reverse */



Key (Number)
int Number;
   {
   static int MagicSquare [11] = { 0, 6, 1, 8, 7, 5, 3, 2, 9, 4 };

   return ( MagicSquare [Number % 10] );
   } /* Key */

EnterGraphic ()
   {
   move (0, 40); 
   printf ("\033(0");         /* select special character set as G0, shift in */
   } /* enter line drawing character set */



LeaveGraphic ()
   {
   move (0, 40); 
   printf ("\033(B");              /* select US character set as G0, shift in */
   } /* enter default character set */



OnCursor ()                                                  /* VT200 command */
   {
   move (0, 40);
   printf ("\033[?25h");
   } /* cursor enable */



OffCursor ()                                                 /* VT200 command */
   {
   move (0, 40);
   printf ("\033[?25l");
   } /* cursor disable */



GameOver ()
   {
   static char *Game_Over [6]
               = { "<<<<< >>>>>", "<<<<E O>>>>", 
                   "<<<ME OV>>>", "<<AME OVE>>", 
                   "<GAME OVER>", " GAME OVER " };
   int Row, Column;

   ShowBigWord ("                                        ", False, 0, 0);
   standout ();                              /* active the boldface attribute */
   for (Row = 1; Row <= 3; Row++)
      {
      for (Column = 5; Column >= 0; Column--)
         {
         mvaddstr (0, 11, Game_Over[Column]);
         refresh ();
         } /* back */
      HideCursor ();
      for (Column = 0; Column <= 5; Column++)
         {
         mvaddstr (0, 11, Game_Over[Column]);
         refresh ();
         } /* forth */
      HideCursor ();
      } 
   standend ();                            /* deactive the boldface attribute */
   } /* game over */

MoveString (A, B, Start_B, End_B, Start_A)
char A [], B [];
int  Start_B, End_B, Start_A;
   {
   int Index;

   for (Index = Start_B; Index <= End_B; Index++)
      A [Start_A++] = B [Index];
   } /* copy string */



ShowBigWord (String, Bell, Second, Attribute)
char         String [];
enum Boolean Bell;
int          Second, Attribute;
   {
   refresh ();
   if (Bell)
      RingBell ();
   switch (Attribute)
      {
      case 1 :
         setattr (_BOLD);
         break;
      case 2 :
         setattr (_BLINK);
         break;
      case 3 :
         setattr (_REVERSE);
         break;
      case 4 :
         setattr (_BLINK | _BOLD);
         break;
      case 5 :
         setattr (_BOLD | _REVERSE);
         break;
      case 6 :
         setattr (_BOLD | _BLINK | _REVERSE);
         break;
      }
   mvaddstr (0, 0, String); 
   LeaveGraphic ();
   refresh ();
   EnterGraphic ();
   clrattr (_BOLD | _BLINK | _REVERSE);
   HideCursor ();
   for (Attribute = 1; Attribute <= (Second * 1947792); Attribute++);/* sleep */
   } /* show big word */

DeleteWindow (Players, Hider) 
Player Players [], Hider [];
   {
   int Index;
    
   EnterGraphic ();
   for (Hand = Contestant; Hand <= Porkling; Hand++)
      if (Players [Hand].Free)
         delwin (PlayWin [Hand]);
    
   for (Index = 0; Index <= 35; Index++)
      delwin (SlopWin [Index]);
   if (DeadEnd)
      delwin (SlopWin [36]);

   for (Index = 1; Index <= (2 * Height + 1); Index++)
      {
      scroll (PenWin);
      if (Index % 2 == 0)
         wrefresh (PenWin);
      } /* rolling thunder */ 
   wrefresh (PenWin);
   LeaveGraphic ();

   delwin (MessWin [0]);
   delwin (BusWin);
   delwin (IWin [0]); 
   delwin (IWin [1]); 
   for (Hand = Contestant; Hand <= Porkling; Hand++)
      {
      if (Players [Hand].Free)
         {
         delwin (TagWin [Hand]);
         delwin (MarkWin [Hand]);
         }
      if (Hider [Hand].Free)
         delwin (HideWin [Hand]);
      }

   delwin (MessWin [1]);
   for (Hand = Contestant; Hand <= OldMan; Hand++)
      delwin (KeyWin [Hand]);
   delwin (PadWin [1]);
   delwin (PadWin [0]);
   delwin (MessWin [2]);
   delwin (ConWin);

   touchwin (stdscr);                                   /* clear the overlaps */
   endwin ();                                     /* terminates curse session */
   OnCursor ();
   SYS$DASSGN (Channel);                                /* deassign a channel */
   } /* delete window */
