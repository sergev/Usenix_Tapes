%{
/*
 * datesub.l: Rog-O-Matic XIV (CMU) Fri Feb  1 10:23:56 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 */

# define yywrap() 1
%}
%%
"Jan"				printf ("1"); 
"Feb"				printf ("2"); 
"Mar"				printf ("3"); 
"Apr"				printf ("4"); 
"May"				printf ("5"); 
"Jun"				printf ("6"); 
"Jul"				printf ("7"); 
"Aug"				printf ("8"); 
"Sep"				printf ("9"); 
"Oct"				printf ("10"); 
"Nov"				printf ("11"); 
"Dec"				printf ("12"); 
%%

main ()
{ while (yylex ())
    ; 
}
