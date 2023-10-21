/****************************************************************************/
/* ST COMM by Mark Falleroni Version 1.4 1/18/86                                                        */
/* Maintenance History:                                                                                                         */
/*         - added code to disable the mouse during program execution           */
/*         - added code to disable the cursor during program termination        */
/*         - added word wrap toggle module                                                              */
/*         - added capability to abort file transmission                                        */
/****************************************************************************/
#include <stdio.h>
#include <aes.h>
#define MASK7 0177
#define OFF 0
#define ON 1
#define RD 0
#define WRT 1
#define EOL 13
#define F10 0x440000
#define F1 0x3B0000
#define F2 0x3C0000
#define F3 0x3D0000
#define F4 0x3E0000
#define HELP 0x620000
#define BUFSIZE 10000
#define PBSIZE 8192
#define PFULL 8100
#define DFULL 9940
#define LIMIT 300

int dlay,cnt,ctr;
short psave,dsave,set,wrap;
char buf[BUFSIZE];
char pbuf[PBSIZE];
FILE *fip, *fopen();

main()
{
short go_on,c,g1,g2,g3,g4,hand,lo,go;
psave = OFF;
dsave = OFF;
go_on = 1;
set = 0;
wrap = OFF;

conout(27);
conout(101);   /* ENABLE CURSOR */

hand = graf_handle(&g1,&g2,&g3,&g4);
graf_mouse(256,0);  /* DISABLE MOUSE */

while (go_on == 1)
{
        clrscreen();  /* CLEAR THE SCREEN */
        inv_on();
        printf("         ST COMM by Mark Falleroni Version 1.4 1/18/86     \n\n");
        inv_off();
        red_on();
        printf("        ~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("        ~ MAIN MENU COMMANDS ~\n");
        printf("        ~~~~~~~~~~~~~~~~~~~~~~\n\n");
        printf("        *T*");
        grey_on();
        printf(" TERMINAL MODE\n");
        red_on();
        printf("        *E*");
        grey_on();
        printf(" ERASE A FILE \n");
        red_on();
        printf("        *Q*");
        grey_on();
        printf(" QUIT \n");
        red_on();
        printf("        *S*");
        grey_on();
        printf(" SET TRANSMIT DELAY \n");
        red_on();
        printf("        *D*");
        grey_on();
        printf(" DISK FILE SET-UP, EXIT TO TERMINAL MODE\n");
        red_on();
        printf("        *C*");
        grey_on();
        printf(" CLOSE FILE\n\n");
        green_on();
        printf("        **************************\n");
        printf("        * Terminal Mode Commands *\n");
        printf("        **************************\n\n");
        printf("        *F1  *");
        grey_on();
        printf(" toggle printer save on/off\n");
        green_on();
        printf("        *F2  *");
        grey_on();
        printf(" transmit an informal file \n");
        green_on();
        printf("        *F3  *");
        grey_on();
        printf(" toggle file save on/off \n");
        green_on();
        printf("        *F4  *");
        grey_on();
        printf(" toggle word wrap on/off \n");
        green_on();
        printf("        *F10 *");
        grey_on();
        printf(" exit to MAIN MENU\n");
        green_on();
        printf("        *HELP*");
        grey_on();
        printf(" list terminal mode commands\n\n");

        printf("        Choice = ");
        c = getchar();
        printf("\n");

        switch (c)
        {
        case 'T':
        case 't': put_serial();   /* TERMINAL MODE */
                          break;
        case 'q':
        case 'Q': go_on = 0;      /* EXIT TO OS */
                          break;
        case 'e':
        case 'E': erase();
                          break;   /* DELETE A FILE */
        case 's':
        case 'S': set_time(); /* SET TRANSMISSION DELAY */
                          break;
        case 'd':
        case 'D': dset();  /* CREATE FILE FOR DATA SAVE */
                          break;
        case 'c':
        case 'C': fclose(fip);  /* CLOSE FILE CREATED FOR DATA SAVE */
                          break;
        default : go_on = 1;      /* ILLEGAL CHOICE, CONTINUE */
        }
} /* while */

graf_mouse(257,0); /* ENABLE MOUSE */
conout(27);
conout(102); /* DISABLE CURSOR */
conout(27);
conout(119); /* TURN OFF WORD WRAP */

} /* main */

/*
/* FUNCTION put_serial - processes terminal mode commands/responses
/*
/*
*/

put_serial()
{
unsigned int c;
int wait;
short go_on,n,st;
char name[14];

inv_on();
printf("\n**** F10 = exit to MAIN MENU ****\n");
inv_off();

ctr = 0;
st  = OFF;
wait = 0;
cnt = 0;
go_on = 1;

while (go_on == 1)
{
 while ((n = conis()) < 0)    /* GET CHAR INPUT WITHOUT ECHO */
 {                              /* SCREEN */
        c = necin();
        switch (c)
        {
        case F3 : dsave_on();  /* SAVE TO DISK */
                          break;
        case F2 : transmit();    /* TRANSMIT FILE */
                          break;
        case F1 : psave_on(); /* SAVE FOR PRINTER */
                          break;
        case F4 : wrd_wrap();
                          break;
        case F10: go_on = 0; /* EXIT TO MAIN MENU */
                          if (psave == ON || dsave == ON)
                                dump();
                          break;
        case HELP : help();
                                break;
        default : auxout(c);    /* OUTPUT TO SERIAL PORT */
        }
 }
 while (( n = auxis()) < 0)  /* CHAR IS READY AT SERIAL PORT */
   {                       /* GET CHAR FROM SERIAL PORT    */
                c = auxin();    /* MASK LAST 7 BITS TO GET      */
                c = c & MASK7; /* ASCII VALUE                              */
                putchar(c);
                if (psave == ON)
                  pbuf[cnt++] = c;  /* SAVE TO BUFFER */
                if (dsave == ON)
                  buf[ctr++] = c;
                if (cnt == PFULL || ctr == DFULL)
                {
                  st = ON;
                  c = 19;
                  auxout(c); /* SEND STOP COMMAND */
                }
        }

 if (st == ON)  /* INCREMENT TIMER TO INSURE HOST HAS */
        wait++;         /* STOPPED SENDING */
 if (wait > LIMIT)
        {
        dump();
        st = OFF;
        wait = 0;
        c = 17; /* START HOST AGAIN */
        auxout(c);
        }
}

}

/*
*
* FUNCTION dset - opens file for write, exits to terminal mode
*
*/
dset()
{
char name[15];
int ans;

   printf("\nENTER SAVE FILENAME: ");
   get_name(name);
   if ((fip=fopen(name,"r")) == NULL)
         fip = fopen(name,"w");
   else
         {
         printf("File Already Exists; OK to Overwrite? Y/N: ");
         get_answer(&ans);
         if ( ans = 1)
                fip = fopen(name,"w");
         else
                return;
         }

        put_serial();
}
/*
*
* FUNCTION psave_on - turns on save switch for buffer save
*
*/
psave_on()
{
int n,c;

 if (psave == OFF)   /* F1 TOGGLES SAVE ON AND OFF */
        {
                inv_on();
                psave = ON;
                printf("\n****** PRINTER SAVE ON ******\n");
        }
 else
        {
                printf("\n****** PRINTER SAVE OFF ******\n");
                dump();
                psave = OFF;
                inv_off();
        }

}

/*
/*FUNCTION get_answer - DETERMINES WHETHER USER WANTS TO ERASE
/* EXISTING FILE
*/
get_answer(ans)
int *ans;
{
short c;

if (( c = getchar() ) == 'Y' || c == 'y')  /* OVERWRITE FILE */
   *ans = 1;
else
        {
   *ans = 0;
        inv_on();
        printf("\n** ABORT FILE SAVE **\n");
        inv_off();
        }
}

/*
/*
/*FUNCTION transmit - opens user selected file and transmits
/*file, one char at a time to serial port
/*
*/

transmit()

{
char tname[15];
int  c, c1, i, n, getout;
FILE *fp, *fopen();

green_on();
printf("\nENTER NAME OF FILE TO TRANSMIT: ");
inv_on();
get_name(tname);
inv_off();

if ((fp = fopen(tname,"r")) == NULL)
        {
        printf("\nCan't open %s\n",tname);
        grey_on();
        return;
        }
getout = OFF;
printf("\n\n~~~TYPE 'Q' TO ABORT TRANSMISSION~~~\n");
while ((c = fgetc(fp)) != EOF)
        {
        auxout(c);
        delay();
        if ((n = conis()) < 0)
                {
                 c1 = necin();            /* ABORT TRANSMISSION */
                 c1 = c1 & MASK7;
                 if (c1 == 'q' || c1 == 'Q')
                        getout = ON;
                }
        i = auxin();   /* SHOW FILE AS TRANSMITTED */
        printf("%c",c);
        if (getout == ON)
                break;
        }
fclose(fp);
inv_on();
if (getout == OFF)
        printf("\n*** TRANSMISSION COMPLETE ***\n");
else
        printf("\n*** TRANSMISSION ABORTED ***\n");
inv_off();
grey_on();
}

/*
*
* FUNCTION delay - LOOPS UNTIL VARIABLE DLAY IS ZERO
*
*/
delay()
{
int i,j;
if (set = 1)
  for (i = 0; i < dlay ; i++)
        for (j = 0; j < dlay; j++)
                ;
}

/*
*
* FUNCTION clrscreen - CLEAR THE SCREEN
*
*
*/
clrscreen()
{
conout(27);
conout(69); /* ESCAPE E */
}

/*
*
*
* FUNCTION erase - DELETES USER SPECIFIED FILE
*
*
*/
erase()
{
char name[14];
int c, n;

red_on();
printf("\n\nENTER NAME OF FILE TO BE DELETED: ");
inv_on();
get_name(name);
inv_off();
printf("\n");
delete(name);
}

/*
*
*
* FUNCTION get_name - gets a filename
*
*
*/
get_name(name)
char *name;
{
 int c, n;

  n=0;
  while ((c = getchar()) != EOL)
         name[n++] = c;
  name[n] = '\0';
}

/*
*
* FUNCTION set_time - sets transmission delay between characters
*
*/

set_time()
{
short time[10];
int i, c, l, m;
 l = 0;
 m = 1;
 i = 0;
 inv_on();
 printf("\n\nENTER THE DELAY TIME (1..100): ");
 inv_off();
 green_on();
 while ((c = getchar()) != EOL)
        time[i++] = c-48;
 dlay = 0;
 for (l = i-1; l >= 0; l--)
        {
          dlay = time[l]*m + dlay;
          m = m*10;
        }
 set = 1;          /* TURN ON SET SWITCH */
 grey_on();
}

/*

*
* FUNCTION dump - dumps buffer to printer or disk
*
*
*/
dump()
{
 int i,c;
        if (psave == ON)
        {
          for (i = 0; i < cnt; i++)
                prnout(pbuf[i]);
          cnt = 0;
          clrpbuf();
        }
        if (dsave == ON)
          {
                for(i=0; i<ctr; i++)
                 fputc(buf[i],fip);
                ctr = 0;
                clrbuf();
          }
}

/*
*
* FUNCTION inv_on - turns on inverse video
*
*/
inv_on()
{
conout(27);
conout(112); /* ESCAPE p */
}

/*
*
* FUNCTION inv_off - turns off inverse video
*
*/
inv_off()
{
conout(27);
conout(113); /* ESCAPE q */
}

/*
*
* FUNCTION clrpbuf - clear printer save buffer
*
*/
clrpbuf()
{
int i;
        for (i = 0; i < PBSIZE; i++)
          pbuf[i] = ' ';
}

/*
*
* FUNCTION clrbuf - clear disk save buffer
*
*/
clrbuf()
{
int i;
        for (i = 0; i < BUFSIZE; i++)
         buf[i] = ' ';
}

/*
*
* FUNCTION dsave_on - toggle disk save on/off
*
*/
dsave_on()
{
        if (dsave == OFF)
        {
          red_on();
          printf("\n ** DISK SAVE ON **\n");
          dsave = ON;
        }
        else
        {
          auxout(19);  /* SEND STOP TO HOST */
          printf("\n ** DISK SAVE OFF **\n");
          printf("** FLUSHING SECTOR BUFFER **\n");
          dump();
          printf("done....\n");
          auxout(17);  /* SEND START TO HOST */
          grey_on();
          dsave = OFF;
        }
}

/*
* FUNCTION red_on - set background color to red
*/
red_on()
{
          conout(27);
          conout(99);
          conout(1);   /* SET BACKGROUND COLOR TO RED */
}

/*
* FUNCTION grey_on - restore background color to grey
*/
grey_on()
{
          conout(27);
          conout(99);
          conout(0);  /* GREY BACKGROUND */
}

/*
* FUNCTION help - lists terminal mode commands
*/
help()
{
clrscreen();
inv_on();
printf("\n\nTerminal Mode Commands\n\n");
printf("*F1 * toggle printer save on/off\n");
printf("*F2 * transmit an informal file \n");
printf("*F3 * toggle file save on/off   \n");
printf("*F4 * toggle word wrap on/off   \n");
printf("*F10* exit to MAIN MENU ********\n\n");
printf(" Type Any Key to Continue ");
inv_off();
necin();          /* GET CHAR WITH NO ECHO TO SCREEN */
printf("\n");
}

/*
*
* FUNCTION green_on - turn background color green
*
*/
green_on()
{
  conout(27);
  conout(99);
  conout(2);
}

/****************************************/
/*  Function wrd_wrap toggle word wrap  */
/****************************************/

wrd_wrap()
{
  if (wrap == OFF)
        {
         wrap = ON;
         conout(27);
         conout(118);                    /* TURN ON WORD WRAP - ESCAPE v */
         inv_on();
         printf("** Word Wrap On **\n");
         inv_off();
        }
  else
        {
         wrap = OFF;
         conout(27);
         conout(119); /* TURN OFF WORD WRAP - ESCAPE w */
         inv_on();
         printf("** Word Wrap Off **\n");
         inv_off();
        }
}
