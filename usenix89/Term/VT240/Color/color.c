/*
 *      Color.c ->>     Change colors on a VT240.
 *      Author  ->>     Xev, SUNYA, 1987
 *
 *
 *      Options: [-t text-color] [-c cursor-color] [-b background-color]
 *      colors:  aquamarine, md-aquamarine, black, blue, cadet-blue, 
 *               cornflwr-blue, dk-slate-blue, lt-blue, lt-steel-blue, 
 *               md-blue, md-slate-blue, midnight-blue, navy-blue,
 *               sky-blue, slate-blue, steel-blue, coral, cyan, firebrick, 
 *               gold, goldenrod, md-goldenrod, green, dk-green,
 *               dk-olive-green, forest-green, lime-green, md-forest-green, 
 *               md-sea-green, md-spring-green, pale-green, sea-green,
 *               spring-green, yellow-green, dk-slate-grey, dim-grey, 
 *               lt-grey, khaki, magenta, maroon, orange, orchid, dk-orchid, 
 *               md-orchid, pink, plum, red, indian-red, md-violet-red, 
 *               orange-red, violet-red, salmon, sienna, tan, thistle, 
 *               turquoise, dk-turquoise, md-turquoise, violet, violet-blue,
 *               wheat, white, yellow, yellow-green
 *
 *      If no options are given, the defaults are used, which are set 
 *              in defines at the beginning, and at this time, contain
 *              the following values:
 *              c = White, b = Red, t = Black.
 *
 *      The following is the format of the color command on a VT240:
 *              <ESC>P3s(e)s(m<Type>(<Color>))<ESC>\<ESC>[2H
 *      Where:
 *              <ESC> is escape,
 *              <Type> is 0 for background,
 *                        1 for cursor, and
 *                        2 for text, and,
 *              <Color> is taken from the massive set of if statements.
 *
 *      The commands may be strung together, with just the 
 *              <Type>(<Color>)   repeated.
 *
 *
 */
 
static char rcsid[] = "$Header: color.c,v 1.4 87/03/30 15:47:45 xev Locked $";

/*
 *
 * $Author: xev $
 * $Date: 87/03/30 15:47:45 $
 * 
 * $Log:	color.c,v $
 * Revision 1.4  87/03/30  15:47:45  xev
 * Some more little changes.
 * 
 * Revision 1.3  87/03/30  12:03:13  xev
 * Well, a couple of other little changes.
 * 
 * Revision 1.2  87/03/30  09:57:04  xev
 * Changed printusage() to make it compatible with the vax.
 * Also, changed a lot of strcpy()s to strcat()s.
 * 
 *
 */ 
 
#define DEFB "h280l35s60"       /*  Sea-Green   */
#define DEFC "h120l50s100"      /*  Red         */
#define DEFT "h180l65s60"       /*  Goldenrod   */
#define SAME 0
#define CLEARS  "\033[2J\033[1;1H"
#include <stdio.h>


        char color[20];

main(argc, argv)
int argc;
char **argv;
{
        int c;
        extern int optind;
        extern char *optarg;
        char type[2];
        char cmd[100];

/* Check for 0 args. If so, just set defaults. */

        if (argc == 1) {
                printf ("P3ps(e)s(m0(%s)1(%s)2(%s))\[2H", DEFB, DEFC, DEFT);
                printf ("%s", CLEARS);
                exit (0);
        }

/* Begin to setup the command string. */
        
        strcpy (cmd, "P3ps(e)s(m");
        
/* Get the options, and act upon them. */
        
        while ((c = getopt(argc, argv, "hb:t:c:")) != EOF) {
                switch (c) {
                        case 'b':
                                strcat (cmd,"0(");
                                break;
                        case 'c':
                                strcat (cmd,"1(");
                                break;
                        case 't':
                                strcat (cmd,"2(");
                                break;
                        default:
                                printusage(argv);
                                exit();
                                break;
                }
                getcolor(optarg);
                strcat (cmd, color);
                strcat (cmd, ")");
        }
        strcat (cmd, ")\[2H");
        printf ("%s",cmd);
        printf ("%s", CLEARS);  
}

printusage(argv)
char **argv;
{
	printf ("\n  %8s:  options: [-t text-color] [-c cursor-color] [-b background-color]\n", argv[0]);
	printf ("                          [-h help]\n");
	printf ("               colors:    aquamarine, md-aquamarine, black, blue, \n");
	printf ("                          cadet-blue, cornflwr-blue, dk-slate-blue,\n");
	printf ("                          lt-blue, lt-steel-blue, md-blue, md-slate-blue,\n");
	printf ("                          midnight-blue, navy-blue, sky-blue, slate-blue,\n");
	printf ("                          steel-blue, coral, cyan, firebrick, gold,\n");
	printf ("                          goldenrod, md-goldenrod, green, dk-green,\n");
	printf ("                          dk-olive-green, forest-green, lime-green,\n");
	printf ("                          md-forest-green, md-sea-green, md-spring-green,\n");
	printf ("                          pale-green, sea-green, spring-green, yellow-green,\n");
	printf ("                          dk-slate-grey, dim-grey, lt-grey, khaki, magenta,\n");
	printf ("                          maroon, orange, orchid, dk-orchid, md-orchid, pink,\n");
	printf ("                          plum, red, indian-red, md-violet-red, orange-red,\n");
	printf ("                          violet-red, salmon, sienna, tan, thistle,\n");
	printf ("                          turquoise, dk-turquoise, md-turquoise, violet,\n");
	printf ("                          violet-blue, wheat, white, yellow, yellow-green\n");

}

getcolor(arg)
char *arg;
{
        if ((strcmp (arg, "aqua")) == SAME) {
                strcpy (color, "h260l65s60");
                return;
        }
        if ((strcmp (arg, "md-aqua")) == SAME) {
                strcpy (color, "h280l50s60");  
                return;
        }
        if ((strcmp (arg, "black")) == SAME) {
                strcpy (color, "h0l0s0");   
                return;
        }
        if ((strcmp (arg, "blue")) == SAME) {
                strcpy (color, "h0l50s100");
                return;
        }
        if ((strcmp (arg, "cadet-blue")) == SAME) {
                strcpy (color, "h300l50s25");
                return;
        }
        if ((strcmp (arg, "cornflwr-blue")) == SAME) {
                strcpy (color, "h0l35s25");
                return;
        }
        if ((strcmp (arg, "dk-slate-blue")) == SAME) {
                strcpy (color, "h40l35s60");    
                return;
        }
        if ((strcmp (arg, "lt-blue")) == SAME) {
                strcpy (color, "h300l80s25");   
                return;
        }
        if ((strcmp (arg, "lt-steel-blue")) == SAME) {
                strcpy (color, "h0l65s25");     
                return;
        }
        if ((strcmp (arg, "md-blue")) == SAME) {
                strcpy (color, "h0l50s60");    
                return;
        }
        if ((strcmp (arg, "md-slate-blue")) == SAME) {
                strcpy (color, "h30l50s100");  
                return;
        }
        if ((strcmp (arg, "midnight-blue")) == SAME) {
                strcpy (color, "h0l25s25");    
                return;
        }
        if ((strcmp (arg, "navy-blue")) == SAME) {
                strcpy (color, "h0l35s60");    
                return;
        }
        if ((strcmp (arg, "sky-blue")) == SAME) {
                strcpy (color, "h320l50s60");  
                return;
        }
        if ((strcmp (arg, "slate-blue")) == SAME) {
                strcpy (color, "h330l50s100"); 
                return;
        }
        if ((strcmp (arg, "steel-blue")) == SAME) {
                strcpy (color, "h320l35s60");  
                return;
        }
        if ((strcmp (arg, "coral")) == SAME) {
                strcpy (color, "h150l50s100"); 
                return;
        }
        if ((strcmp (arg, "cyan")) == SAME) {
                strcpy (color, "h300l50s100"); 
                return;
        }
        if ((strcmp (arg, "firebrick")) == SAME) {
                strcpy (color, "h120l35s60");       
                return;
        }
        if ((strcmp (arg, "gold")) == SAME) {
                strcpy (color, "h150l50s60");  
                return;
        }
        if ((strcmp (arg, "goldenrod")) == SAME) {
                strcpy (color, "h180l65s60");
                return;
        }
        if ((strcmp (arg, "md-goldenrod")) == SAME) {
                strcpy (color, "h180l80s60");       
                return;
        }
        if ((strcmp (arg, "green")) == SAME) {
                strcpy (color, "h240l50s100"); 
                return;
        }
        if ((strcmp (arg, "dk-green")) == SAME) {
                strcpy (color, "h240l25s25");       
                return;
        }
        if ((strcmp (arg, "dk-olive-green")) == SAME) {
                strcpy (color, "h180l25s25");       
                return;
        }
        if ((strcmp (arg, "forest-green")) == SAME) {
                strcpy (color, "h240l35s60");       
                return;
        }
        if ((strcmp (arg, "lime-green")) == SAME) {
                strcpy (color, "h240l50s60");       
                return;
        }
        if ((strcmp (arg, "md-forest-green")) == SAME) {
                strcpy (color, "h200l35s60");       
                return;
        }
        if ((strcmp (arg, "md-sea-green")) == SAME) {
                strcpy (color, "h240l35s25");       
                return;
        }
        if ((strcmp (arg, "md-spring-green")) == SAME) {
                strcpy (color, "h210l50s100"); 
                return;
        }
        if ((strcmp (arg, "pale-green")) == SAME) {
                strcpy (color, "h240l65s25");       
                return;
        }
        if ((strcmp (arg, "sea-green")) == SAME) {
                strcpy (color, "h280l35s60");       
                return;
        }
        if ((strcmp (arg, "spring-green")) == SAME) {
                strcpy (color, "h270l50s100"); 
                return;
        }
        if ((strcmp (arg, "yellow-green")) == SAME) {
                strcpy (color, "h200l50s60");       
                return;
        }
        if ((strcmp (arg, "dk-slate-green")) == SAME) {
                strcpy (color, "h300l25s25");       
                return;
        }
        if ((strcmp (arg, "dim-grey")) == SAME) {
                strcpy (color, "h0l33s0");  
                return;
        }
        if ((strcmp (arg, "lt-grey")) == SAME) {
                strcpy (color, "h0l66s0");  
                return;
        }
        if ((strcmp (arg, "khaki")) == SAME) {
                strcpy (color, "h180l50s25");       
                return;
        }
        if ((strcmp (arg, "magenta")) == SAME) {
                strcpy (color, "h60l50s100");       
                return;
        }
        if ((strcmp (arg, "maroon")) == SAME) {
                strcpy (color, "h80l35s60");        
                return;
        }
        if ((strcmp (arg, "orange")) == SAME) {
                strcpy (color, "h120l50s60");     
                return;
        }
        if ((strcmp (arg, "orchid")) == SAME) {
                strcpy (color, "h60l65s60");        
                return;
        }
        if ((strcmp (arg, "dk-orchid")) == SAME) {
                strcpy (color, "h40l50s60");        
                return;
        }
        if ((strcmp (arg, "md-orchid")) == SAME) {
                strcpy (color, "h20l65s60");        
                return;
        }
        if ((strcmp (arg, "pink")) == SAME) {
                strcpy (color, "h120l65s25");       
                return;
        }
        if ((strcmp (arg, "plum")) == SAME) {
                strcpy (color, "h60l80s60");        
                return;
        }
        if ((strcmp (arg, "red")) == SAME) {
                strcpy (color, "h120l50s100"); 
                return;
        }
        if ((strcmp (arg, "indian-red")) == SAME) {
                strcpy (color, "h120l25s25");       
                return;
        }
        if ((strcmp (arg, "md-violet-red")) == SAME) {
                strcpy (color, "h100l65s60");       
                return;
        }
        if ((strcmp (arg, "orange-red")) == SAME) {
                strcpy (color, "h90l50s100");       
                return;
        }
        if ((strcmp (arg, "violet-red")) == SAME) {
                strcpy (color, "h80l50s60");        
                return;
        }
        if ((strcmp (arg, "salmon")) == SAME) {
                strcpy (color, "h120l35s25");       
                return;
        }
        if ((strcmp (arg, "sienna")) == SAME) {
                strcpy (color, "h160l35s60");       
                return;
        }
        if ((strcmp (arg, "tan")) == SAME) {
                strcpy (color, "h140l65s60");       
                return;
        }
        if ((strcmp (arg, "thistle")) == SAME) {
                strcpy (color, "h60l80s25");        
                return;
        }
        if ((strcmp (arg, "turquoise")) == SAME) {
                strcpy (color, "h300l80s60");       
                return;
        }
        if ((strcmp (arg, "dk-turquoise")) == SAME) {
                strcpy (color, "h340l65s60");       
                return;
        }
        if ((strcmp (arg, "md-turquoise")) == SAME) {
                strcpy (color, "h300l65s60");       
                return;
        }
        if ((strcmp (arg, "violet")) == SAME) {
                strcpy (color, "h60l25s25");        
                return;
        }
        if ((strcmp (arg, "violet-blue")) == SAME) {
                strcpy (color, "h60l50s25");        
                return;
        }
        if ((strcmp (arg, "wheat")) == SAME) {
                strcpy (color, "h180l80s25");       
                return;
        }
        if ((strcmp (arg, "white")) == SAME) {
                strcpy (color, "h0l99s0");  
                return;
        }
        if ((strcmp (arg, "yellow")) == SAME) {
                strcpy (color, "h180l50s100"); 
                return;
        }
        if ((strcmp (arg, "yellow-green")) == SAME) {
                strcpy (color, "h220l65s60");       
                return;
        }
        printf ("Color %s not found. Exiting.\n", arg);
        exit (1);
}
