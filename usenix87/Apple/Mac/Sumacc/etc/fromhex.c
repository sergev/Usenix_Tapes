#include "quickdraw.h"
#include "osintf.h"
#include "toolintf.h"

#define bufsize 512
#define InName "C"
#define OutName "C prog"

int rout, rin, io, i;
int l, val, bytes, sum, getsum;
char bin[bufsize], bout[bufsize];
int count, szin, bini, bouti;
FInfo fInfo;

putflush()
   {
   if (bouti)
      {
      count = bouti;
      io = FSWrite(rout, &count, bout); 
      bouti = 0;
      }
   }

putchar(ch)
   int ch;
   {
   bout[bouti++] = (char) ch;
   if (bouti == bufsize)
      putflush();
   }

int getchar()
   {
   if (bini == bufsize)
      {
      bini = 0;
      count = bufsize;
      io = FSRead(rin, &count, bin);
      }
   return ((int) bin[bini++]);
   }

badnews()
   {
   int j;

   io = FSClose(rin);
   io = FSClose(rout);
   SysBeep(1);
   for (j = 0; j <= 20000; j++) 
      l = 0;
   SysBeep(1);
   ExitToShell();
   }

main()
   {
   bouti = 0;
   bini = bufsize;

   io = FSDelete(OutName, 0);
   io = Create(OutName, 0, "CCOM", "APPL");
   io = GetFInfo(OutName, 0, &fInfo);
   fInfo.fdFlags |= fHasBundle << 8;
   io = SetFInfo(OutName, 0, &fInfo);
   io = OpenRF(OutName, 0, &rout);
   if (io)
      badnews();

   io = FSOpen(InName, 0, &rin);
   if (io)
      badnews();
   io = GetEOF(rin, &szin);
   count = szin >> 1;
   io = Allocate(rout, &count);
   if (io)
      badnews();
   val = 0;
   bytes = 0;
   sum = 0;
   getsum = 0;

   while (szin && !getsum)
      {
      l = getchar() & 127;
      szin--;
      if ((l >= 64) && (l < 80))
         {
	 bytes++;
         val = (val << 4) | (l - 64);
	 if (!(bytes & 1))
	    {
	    putchar(val);
	    sum += val;
	    val = 0;
 	    }
	 }
      if (l == 124)
         getsum = 1;
      }
   putflush();
   io = FSClose(rout);
   if (!getsum)
      badnews();

   sum += bytes >> 1;
   val = 0;
   for (i = 1; i <= 8; i++)
      val = (val << 4) | (getchar() & 15);
   io = FSClose(rin);
   if (val != sum)
      badnews();
   ExitToShell();
   }
