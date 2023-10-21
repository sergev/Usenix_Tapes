/* unit.c - 1/17/87 */

/*
        Copyright 1987 Gregory R. Simpson

UUCP: {ihnp4, seismo, decwrl, philabs, ucbvax}!decvax!cwruecmp!ncoast!simpsong
CSNET: ncoast!simpsong@case.CSNET     
ARPA:  ncoast!simpsong%case.CSNET@Csnet-Relay.ARPA
AT&T:  (216)-473-1019

	This notice and any statement of authorship must be reproduced
	on all copies.  The author does not make any warranty expressed
	or implied, or assumes any liability or responsiblity for the
	use of this software.

	Any distributor of copies of this software shall grant the
	recipient permission for further redistribution as permitted
	by this notice.	 Any distributor must distribute this software
	without any fee or other monetary gains, unless expressed written
	permission is granted by the author.

	This software or its use shall not be: sold, rented, leased,
	traded, or otherwise marketed without the expressed written
	permission of the author.

	If the software is modified in a manner creating derivative
	copyrights, appropriate legends may be placed on derivative
	work in addition to that set forth above.

	Permission is hereby granted to copy, reproduce, redistribute or
	otherwise use this software as long as the conditions above
	are meet.

	All rights not granted by this notice are reserved.

 (This copyright notice was written by, Kurt Zeilenga (zeilenga@hc.dspo.gov)
  thanks Kurt... -greg)

*/
/* This program is for Unit Conversion... */

#include <stdio.h>

#define ESC '\033'
#define True 1

/* Definition for clearing Screen */

#define Clear printf("%c[2J%c[1;1f",ESC,ESC)

#define MAXGROUP 20       /* Maximum Number of Unit Groups - 1 */

struct of_units {
       char  *name;       /* Name of Unit */
       float value;       /* Value - Initially set to 0 */
       float conversion;  /* Conversion factor to Primary Unit */
       int   counter;     /* Number of units in this group */
} 

#include "unit_table.h"

main(argc, argv)
int argc;
char *argv[];
{
       int   group, user_unit, counter, unit_counter, sameunit;
       int   comp_unit, ref_unit, chart, screen;
       float user_value, from_value, to_value, temp_value, value, increment;
       float primary;
       char c[10], *cfile, chartfile[100];
       FILE *fpchart;

       /* default values */

       group      = 1;
       user_unit  = 1;
       user_value = 1; 

       sameunit   = 0;  /* New Unit Group */
       chart      = 0;  /* Default is standard conversion */

       /* check for chart option */

       if (argc > 2)   /* usage message if extra command line arguments */
       {
          usage();
       } else if (argc > 1) {
          if ( argv[1][0] == '-' ) {
             if ( argv[1][1] == 'C' || argv[1][1] == 'c' ) {
                chart = 1;
             } else {
                usage();
             }
          } else {
            usage();
          }
       }
       while(True)        /* loop forever... */
       {
           if ( sameunit == 0 ) 
           {
              Clear;
              printf("                Unit Group Selection\n");
              printf("-------------------------------------------------------------\n");
              printf("1.  Length                  2. Mass                \n");
              printf("3.  Speed                   4. Volume              \n");
              printf("5.  Area                    6. Density             \n");
              printf("7.  Time                    8. Force               \n");
              printf("9.  Energy/Heat/Work       10. Pressure            \n");
              printf("11. Angle                  12. Power               \n");
              printf("13. Electric Charge        14. Magnetic Induction  \n");
              printf("15. Light                  16. Thermal Conductivity\n");
              printf("17. Coeff of Heat Transfer 18. Heat Flux           \n");
              printf("19. Viscosity              20. Temperature         \n");
              printf("21. Constants                                      \n");
              printf("---------------------------------------------------(C)grs----\n");


              printf("Which Type of Unit? ");
              getinteger(&group, MAXGROUP);
              group = group - 1;   /* since array starts at 0 */
              
              unit_counter = unit[group][1].counter - 1;
        
           } /* end of sameunit if */

              switch (group)
              {
                 case 19:
                          temperature();
                          sameunit = 0;
                          break;
                 case 20:
                          constants();
                          sameunit = 0;
                          break;
                 default:

              Clear;
              printf("                     Unit Selection\n");
              printf("-------------------------------------------------------------\n");
              for (counter = 0; counter <= unit_counter; counter = counter+2)
              {
                     printf("%2d. %-18s                       %2d. %-18s\n",
                     (counter + 1), unit[group][counter].name, 
                     (counter + 2), unit[group][counter + 1].name);
              }
              printf("-------------------------------------------------------------\n");

/* --------------------   Standard Unit Conversion   ----------------  */

              if (chart == 0) {

                     printf("Your Unit? ");
                     getinteger(&user_unit, unit_counter);
                     user_unit = user_unit - 1;

                     printf("\nHow many %s ? ", unit[group][user_unit].name );
                     getnumber(&user_value, unit_counter);

                     Clear;
                     printf("\n     ============================================================\n");

                     primary=user_value*(1/unit[group][user_unit].conversion);

                     printf("     %16.8g %s is equivalent to:\n",
                     user_value, unit[group][user_unit].name);
                     printf("     ------------------------------------------------------------\n\n");
                     for (counter = unit_counter; counter >= 0; counter--) 
                     {
                            if (counter != user_unit) 
                            {
                                   unit[group][counter].value = 
                                   primary * unit[group][counter].conversion;
                                   printf("                  %16.8g %-18s\n",
                                   unit[group][counter].value,
                                   unit[group][counter].name);
                            }
                     }

                     printf("     ============================================================\n");
                     printf("\n      (R)erun Unit type, (N)ew Unit type, (Q)uit, or (C)hart: ");
                     scanf("%s",c);
                     if ( c[0] == 'Q' || c[0] == 'q' ) {
                            printf("\n    U... Units conversion, by Gregory R. Simpson - Copyright 1987 \n");
                            exit();
                     } 
                     else if ( c[0] == 'R' || c[0] == 'r' ) {
                            sameunit = 1;
                     } 
                     else if ( c[0] == 'N' || c[0] == 'n' ) {
                            sameunit = 0;
                     } 
                     else if ( c[0] == 'C' || c[0] == 'c' ) {
                            chart = 1;
                            sameunit = 0;
                     }
              } 

/* ---------------------  Chart Option   --------------------- */

              else {    

                     printf("\nYour Reference Unit?  ");
                     getinteger(&ref_unit, unit_counter);
                     ref_unit = ref_unit - 1;
                     printf("Your Comparison Unit? ");
                     getinteger(&comp_unit, unit_counter);
                     comp_unit = comp_unit - 1;

                     printf("\nFrom How Many %s?  ", unit[group][ref_unit].name);
                     getnumber(&from_value);
                     printf("To How Many %s?    ", unit[group][ref_unit].name);
                     getnumber(&to_value);
                     printf("By what increment? ");
                     getnumber(&increment);

                     screen = 0;
                     printf("\nFilename for chart? (<CR> for screen): ");
                     cfile = gets(chartfile);
                     if (cfile[0] == '\0') {
                        fpchart = stdout;
                        screen  = 1;
                     } 
                     else if ( ( fpchart = fopen(cfile,"a") ) == NULL ) {
                        fprintf(stderr, "Can't open Chartfile.");
                        exit(1);
                     }                      

                     Clear;
 
                     /* Error Checking and Correction... */

                     if ( from_value > to_value )
                     {
                        fprintf(fpchart,
                        "Your From value is Greater than your To value.\n");
                        fprintf(fpchart,"Therefore, I swapped them.\n");
                        temp_value = from_value;
                        from_value = to_value;
                        to_value   = temp_value;
                     }
                     if ( increment < 0 )
                     {
                       fprintf(fpchart,
                       "Since your From value is Less than your To value,\n");
                       fprintf(fpchart,
                       "I will make your increment positive.\n");
                       increment = -increment;
                     }

                     fprintf(fpchart,
"------------------------------------------------------------------------------\n");

                     for (value = from_value; value <= to_value; 
                          value = value + increment) 
                         {
                           primary=value*(1/unit[group][ref_unit].conversion);
                            unit[group][comp_unit].value = 
                             primary * unit[group][comp_unit].conversion;
                             fprintf(fpchart,
                             "| %16.8g %-18s | %16.8g %-18s |\n",
                             value,unit[group][ref_unit].name,
                             unit[group][comp_unit].value,
                             unit[group][comp_unit].name);
                             fprintf(fpchart,
"------------------------------------------------------------------------------\n");
                     }
                     if (screen == 0) {
                         fclose(fpchart);
                     }
                     printf("\n (R)erun Unit type, (N)ew Unit type, (Q)uit, or (S)tandard Conversions: ");
                     scanf("%s",c);
                     if ( c[0] == 'Q' || c[0] == 'q' ) {
                            printf("\n    U... Unit Conversion, by Gregory R. Simpson - Copyright 1987 \n");
                            exit();
                     } 
                     else if ( c[0] == 'R' || c[0] == 'r' ) {
                            sameunit = 1;
                     } 
                     else if ( c[0] == 'N' || c[0] == 'n' ) {
                            sameunit = 0;
                     } 
                     else if ( c[0] == 'S' || c[0] == 's' ) {
                            chart = 0;
                            sameunit = 0;
                     }

              } /* end of chart if else */
          } /* end of switch */
       } /* end of while */
} /* ---- end of main program ---- */

/* getinteger - Get Positive Integer Routine - G.R.Simpson */

int getinteger(choice, max)

int *choice;
int max;
{
       int status, c;
       while(1) { 
              status = scanf("%d", choice);
              if (status == 0)
              {
                     scanf("%*s");
                     printf("Please Use Positive Integers; try again: ");
              }
              else if (status == 1)
              {
                     while ((c = getchar()) != '\n' && c != EOF)
                            ;
                     if ( c == EOF )
                     {
                            ungetc(c, stdin);
                     }
                     if ( *choice > 0 && *choice <= max+1 ) 
                     {
                             return status;
                     }
                     printf("Please use a number from 1 to %d : ", max+1);
              }
              else  /* status is -1 */
              {
              printf("End of file encountered... \n");
              *choice = 1;
              return status;
              }
}
}

/* getfloat - Get Floating Point Number - G.R.Simpson */

getnumber(fchoice)

float *fchoice;
{
       int status, c;
       while( (status = scanf("%f", fchoice)) == 0)
       {
              scanf("%*s");
              printf("Please Use Numeric Input; try again: ");
       }
       if (status == 1)
       {
              while ((c = getchar()) != '\n' && c != EOF)
                     ;
              if ( c == EOF )
                     ungetc(c, stdin);
       }
       else  /* status is -1 */
       printf("End of file encountered... \n");
       return status;
}

/* usage - Print a usage message - G.R.Simpson */

usage()
{
        printf("\nusage: u [-c] \n");
        printf("-c : unit chart option \n\n");
        printf("U... Unit conversions; Copyright, Gregory R. Simpson 1987\n");
        exit();
}

/* temperature - G.R.Simpson */

temperature()
{
static char *scale[4] = { "Fahrenheit", "Celsius", "Rankine", "Kelvin" };
float fahrenheit, celsius, rankine, kelvin;
float temp;
char c[2];
int    choice, tempagain;

        tempagain = 1;

        while(tempagain) {
              Clear;
              printf("                     Unit Selection\n");
              printf("-------------------------------------------------------------\n");
                     printf("1. Fahrenheit                    2. Celsius \n");
                     printf("3. Rankine                       4. Kelvin \n");
              printf("-------------------------------------------------------------\n");

        printf("Your Temperature Scale? ");
        getinteger(&choice,3);
        printf("\n How many degrees %s? ", scale[choice-1]);
        getnumber(&temp);
        switch(choice) {
              case 1 :
                    fahrenheit = temp;
                    break;
              case 2 :
                    fahrenheit = temp*(9.0/5.0) + 32.0;
                    break;
              case 3 :
                    fahrenheit = temp - 459.67;
                    break;
              case 4 :
                    fahrenheit = ((temp-273.15)*(9.0/5.0)) + 32.0;
                    break;
        }
        celsius = (fahrenheit - 32) * (5.0/9.0);
        rankine = fahrenheit + 459.67;
        kelvin  = celsius + 273.15;

        Clear;
        printf("     %16.8g degrees %s is equivalent to:\n",
                     temp,scale[choice-1]);
        printf("                  ------------------------------------\n");
        if (choice != 1) printf("                  %16.8g degrees Fahrenheit\n", fahrenheit);
        if (choice != 2) printf("                  %16.8g degrees Celsius\n",    celsius);
        if (choice != 3) printf("                  %16.8g degrees Rankine\n",    rankine);
        if (choice != 4) printf("                  %16.8g degrees Kelvin\n",     kelvin);

        printf("                  ------------------------------------\n");
        printf("\n (R)erun Unit type, (N)ew Unit type, or (Q)uit: ");
        scanf("%s",c);
        if ( c[0] == 'Q' || c[0] == 'q' ) {
           printf("\n    U... Unit Conversion, by Gregory R. Simpson - Copyright 1987 \n");
           exit();
        } 
        else if ( c[0] == 'R' || c[0] == 'r' ) {
           tempagain = 1;
        } 
        else if ( c[0] == 'N' || c[0] == 'n' ) {
           tempagain = 0;
        } 
  } /* end while(tempagain) */
        c[0] = '\0';
        return;
}

/* constants - G.R.Simpson */

constants()
{
        char c[10];

        Clear;         
        printf("\npi                      =  3.141592653589793238462643\n");
        printf("e                       =  2.718281828459045235360287\n");
        printf("atomic mass unit        =  1.66053 e-27 kg, e-24 gm \n");
        printf("Avogadro's number N     =  6.02217 e23 mole^-1 \n");
        printf("Boltzmann's constant    =  R/N = 1.3806 e-23 J/K, e-16 erg/K = 8.61708 e-5 eV/K \n");
        printf("gas constant            =  8.3143 J/mole-K = 0.082054 l-atm/mole-K\n");
        printf("gravitational constant  =  6.673 e-11 N-m^2/kg^2, J-m/kg^2, \n");
        printf("mass of electron        =  9.10956 e-31 kg, e-28 gm =  5.48593 e-4 amu \n");
        printf("mass of proton          =  1.67261 e-27 kg, e-24 gm =  1.0072766 amu \n");
        printf("Planck's constant       =  h =  6.62620 e-34 J-sec,  e-27 erg-sec \n");
        printf("h bar                   =  h/2*pi = 1.05459 e-34 J-sec, e-27 erg-sec \n");
        printf("speed of light          =  2.997925 e8 m/sec, e10 cm/sec \n");
        printf("Stefan-Boltzmann constant =  5.670 e-8 W/m^2-K^4 \n");
        printf("\n      <CR> to continue or (Q)uit: ");
        gets(c);
        if ( c[0] == 'Q' || c[0] == 'q' ) {
           printf("\n    U... Unit Conversion, by Gregory R. Simpson - Copyright 1987 \n");
           exit();
        } else {
        c[0] = '\0';
        return;
        }
}
