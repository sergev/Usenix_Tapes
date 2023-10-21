/* label.c --- makes PostScript code for cassette labels, from 
1 to 4 per page.
usage: label n  
Send Bug reports and improvements to:

Scott Anderson   sanders3@ub.D.UMN.EDU
And it's FREE!
*/

#include <stdio.h>
#include <strings.h>

main (argc,argv)
   int argc ;
   char *argv[] ;
{
int num=0 ;
if (argc == 1 ) {
fprintf (stderr,"usage: %s [1234]\n",argv[0]) ;
exit(1) ; }
sscanf (argv[1],"%d " ,&num ) ;
prologue() ;
songs () ;
num-- ;
if (num == 0 ) { printf("showpage\n" ) ;
exit(0) ; }
printf(" initmatrix .5 in 5.1 in translate makeit " ) ;
songs() ;
num-- ;
if (num == 0 ) { printf("showpage\n" ) ;
exit(0) ; }
printf(" initmatrix 4.3 in 1.0 in translate makeit " ) ;
songs() ;
num-- ;
if (num == 0 ) { printf("showpage\n" ) ;
exit(0) ; }
printf(" initmatrix 4.3 in 5.1 in translate makeit " ) ;
songs() ;
printf("showpage \n " ) ;
}
songs ()
{
   char line1[40] ;
   printf (" titlefont\n" ) ;
   fprintf(stderr ,"ENTER ARTIST    | \n") ;
   gets ( line1 ) ;
   printf("title1 \(%s\)show\n",line1 ) ;
   fprintf(stderr ,"ENTER ALBUM     | \n") ;
   gets ( line1 ) ;
   printf("stitle1 \(%s\)show\n",line1 ) ;
   printf (" normalfont\n" ) ;
   fprintf(stderr ,"ENTER SONGS, END W/999 | \n") ;
   printf (" songs1\n") ;
   gets(line1) ;
   while ( strcmp(line1,"999" )) {
   printf("newline \(%s\)show\n",line1 ) ;
   gets(line1) ;
   }
   printf (" titlefont \n " ) ;
   fprintf(stderr ,"ENTER ARTIST #2 | \n") ;
   gets ( line1 ) ;
   printf("title2 \(%s\)show\n",line1 ) ;
   fprintf(stderr ,"ENTER ALBUM #2  | \n") ;
   gets ( line1 ) ;
   printf("stitle2 \(%s\)show\n",line1 ) ;
   printf(" normalfont \n " ) ;
   fprintf(stderr ,"ENTER SONGS, END W/999 | \n") ;
   printf (" songs2\n") ;
   gets(line1) ;
   while ( strcmp(line1,"999" )) {
   printf("new2 \(%s\)show\n",line1 ) ;
   gets(line1) ;
   }
   }
prologue()
{
printf ("\%%\!PS-Adobe-1.0 \n " ) ;
printf (" /in {72 mul } def \n");
printf (" /intt { 2 setlinewidth 0 setgray newpath 0 0 moveto }def\n");
printf (" /drawgraybox { .4 setgray  \n" ) ;
printf (" 0 in 4 in lineto  \n" ) ;
printf (" .625 in 4 in lineto  \n" ) ;
printf (" .625 in 0 in lineto \n" ) ; 
printf (" 0 0 lineto  \n" ) ;
printf (" fill intt } def \n" ) ;
printf (" /drawtapebox { 2 setlinecap .625 in 0 moveto .625 in 4 in lineto stroke \n" ) ;
printf (" 0 0 moveto 0 4 in lineto stroke \n" ) ;
printf (" 1.125 in 0 moveto 1.125 in 4 in lineto stroke \n" ) ;
printf (" 3.75 in 0 moveto 3.75 in 4 in lineto stroke \n" ) ;
printf (" 0 0  moveto 3.75 in 0 lineto  stroke \n" ) ;
printf (" .625 in 2 in moveto 3.75 in 2 in lineto stroke \n" ) ;
printf (" 0 4 in moveto 3.75 in 4 in lineto stroke \n" ) ;
printf (" 0 setlinecap \n") ;
printf (" } def \n") ;
printf (" /makeit { intt drawgraybox drawtapebox 3.75 in 0 in translate 90 rotate } def \n") ;
printf (" /LM 7 def \n") ;
printf (" /LM2 155 def \n") ;
printf (" /title1 { newpath LM 2.9 in moveto } def \n") ;
printf (" /title2 { newpath LM2 2.9 in moveto } def \n") ;
printf (" /stitle1 { newpath LM 2.7 in moveto } def \n") ;
printf (" /stitle2 { newpath LM2 2.7 in moveto } def \n") ;
printf (" /newline { currentpoint 12 sub LM exch moveto pop } def \n") ;
printf(" /new2 { currentpoint 12 sub LM2 exch moveto pop } def \n " ) ;
printf ("/titlefont { /Helvetica-Bold findfont 14 scalefont setfont} def \n") ;
printf ("/normalfont { /Helvetica findfont 10 scalefont setfont} def \n") ;
printf (" /songs1 { newpath LM 2.5 in moveto } def \n") ;
printf (" /songs2 { newpath LM2 2.5 in moveto } def \n") ;
printf (" .5 in 1.0 in translate \n") ;
printf (" makeit \n") ;
}
