/*
 *  Morse Code Program for Suns    Version 1.0
 *
 *  Here is a little program I wrote that converts standard input
 *  to morse code.  The Sun's bell is used to beep the code.
 *  The speed of the generated beeps vary, depending on the model of Sun.
 *  Speed can be changed with the optional first argument on the command
 *  line: 
 *      
 *           % morse 2     # is twice as fast 
 *           % morse 4     # is 3 times a fast
 *           % morse -2    # is  twice as slow 
 *           % morse -3    # is  3 times a slow 
 *
 *  It can be used across a network with remote shell:
 *
 *           % rsh remote_host morse
 * 
 *  Or with pipes: 
 *
 *           % fortune | morse 
 * 
 *  I have not include all the character set. ( ?, !, -, etc. )
 *  New characters can be easily added to the big case statement in main.
 *   
 *  I don't have time to work with this anymore so don't send the bugs to me.
 *  Send me better versions though :^)
 *
 *  Oh yeah, this works on Suns version 3.2 of the UNIX4.3bsd operating system.
 *  compile with: 
 *                    % cc morse.c -o morse -O
 *  
 *
 *   Have fun!
 *   John S. Watson  4/29/87
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sundev/kbd.h>
#include <sundev/kbio.h>
 
int keyboard;
int scale = 3000;
int factor = 1;

unit_pause( number)
   int number;
{
   int i;
  
   for ( i = 0; i < number* (int) scale; i++);
}

dit()
{
   int on = KBD_CMD_BELL;
   int off = KBD_CMD_NOBELL;
   int i;

   ioctl(keyboard, KIOCCMD, &on);
   unit_pause(1);
   ioctl(keyboard, KIOCCMD, &off);
   unit_pause(1);
}

 
da()
{
   int on = KBD_CMD_BELL;
   int off = KBD_CMD_NOBELL;
   int i;

   ioctl(keyboard, KIOCCMD, &on);
   unit_pause( 3);
   ioctl(keyboard, KIOCCMD, &off);
   unit_pause( 1);
}


read_line( buffer)
  char buffer[];
{
  char character;
  int i = 0;
  
  do 
    {
    character = getchar();
    if (  feof(stdin) ) exit(0);
    buffer[i] = character;
    ++i;
    }
  while( character != '\n' );
  
  buffer[ i - 1 ] = '\0';
}



init_keyboard()
{
  keyboard = open("/dev/kbd", O_RDWR, 0666);
  if (keyboard < 0) {
     perror("/dev/kbd");
     exit(1);
     }   
  return( keyboard);
}

main(argc, argv)
 int argc;
 char **argv;
{
  int i;
  char line[81];
  
  if ( argc == 2 )
     {

     factor = atoi( argv[1]);
     if ( factor > 0) 
        scale /= factor;
      else
        scale *= -factor;

     if ( (int) scale == 0) {
        fprintf(stderr, "can't go that fast\n");
        exit(0);
        }
     }

  init_keyboard();


  while ( 1) {
      read_line( line);
      for ( i = 0; i < strlen( line) ; i++) {
      unit_pause( 2);
      switch( line[i] )
            {
             case 'A' : 
             case 'a' :  dit(); da();
                         break;
             case 'B' : 
             case 'b' :  da(); dit(); dit();
                         break;
             case 'C' :
             case 'c' :  da(); dit(); da(); dit();
                         break;
             case 'D' :
             case 'd' :  da(); dit(); dit();
                         break;
             case 'E' :
             case 'e' :  dit();
                         break;
             case 'F' :
             case 'f' :  dit(); dit(); da(); dit();
                         break;
             case 'G' :
             case 'g' :  da(); da(); dit();
                         break;
             case 'H' :
             case 'h' :  dit(); dit(); dit(); dit();
                         break;
             case 'I' :
             case 'i' :  dit(); dit();
                         break;
             case 'J' :
             case 'j' :  dit(); da(); da(); da();
                         break;
             case 'K' :
             case 'k' :  da(); dit(); da();
                         break;
             case 'L' :
             case 'l' :  dit(); da(); dit(); dit();
                         break;
             case 'M' :
             case 'm' :  da(); da();
                         break;
             case 'N' :
             case 'n' :  da(); dit();
                         break;
             case 'O' :
             case 'o' :  da(); da(); da();
                         break;
             case 'P' :
             case 'p' :  dit(); da(); da(); dit();
                         break;
             case 'Q' :
             case 'q' :  da(); da(); dit(); da();
                         break;
             case 'R' :
             case 'r' :  dit(); da(); dit();
                         break;
             case 'S' :
             case 's' :  dit(); dit(); dit();
                         break;
             case 'T' :
             case 't' :  da();
                         break;
             case 'U' :
             case 'u' :  dit(); dit(); da();
                         break;
             case 'V' :
             case 'v' :  dit(); dit(); dit(); da();
                         break;
             case 'W' :
             case 'w' :  dit(); da(); da();
                         break;
             case 'X' :
             case 'x' :  da(); dit(); dit(); da();
                         break;
             case 'Y' :
             case 'y' :  da(); dit(); da(); da();  
                         break;
             case 'Z' :
             case 'z' :  da(); da(); dit(); dit();
                         break;

             case '0' :  da(); da(); da(); da(); da();
                         break;
 
             case '1' :  dit(); da(); da(); da(); da();
                         break;
 
             case '2' :  dit(); dit(); da(); da(); da();
                         break;
 
             case '3' :  dit(); dit(); dit(); da(); da();
                         break;
 
             case '4' :  dit(); dit(); dit(); dit(); da();
                         break;
 
             case '5' :  dit(); dit(); dit(); dit(); dit();
                         break;
 
             case '6' :  da(); dit(); dit(); dit(); dit();
                         break;
 
             case '7' :  da(); da(); dit(); dit(); dit();
                         break;
 
             case '8' :  da(); da(); da(); dit(); dit();
                         break;
 
             case '9' :  da(); da(); da(); da(); dit();
                         break;
 
             case ',' :  da(); da(); dit(); dit(); da(); da();
                         break;
 
             case '.' :  dit(); da(); dit(); da(); dit(); da();
                         break;

             /*
              * add new characters here!
              */
 
             case ' ' : 
             default  :  unit_pause(5);
                         break;
             }
     }
  }
}
