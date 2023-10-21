
 /*********************************************************\
 * This program exemplifies the soundex algorithm.	  *
 *							  *
 * You type in a word and it spits out the soundex string  *
 * that was produced for that word.			  *
 \*********************************************************/
 
 #include <stdio.h>
 
 char table[] = {
     '0',		    /* A */
     '1',		    /* B */
     '2',		    /* C */
     '3',		    /* D */
     '0',		    /* E */
     '1',		    /* F */
     '2',		    /* G */
     '0',		    /* H */
     '0',		    /* I */
     '2',		    /* J */
     '2',		    /* K */
     '4',		    /* L */
     '5',		    /* M */
     '5',		    /* N */
     '0',		    /* O */
     '1',		    /* P */
     '2',		    /* Q */
     '6',		    /* R */
     '2',		    /* S */
     '3',		    /* T */
     '0',		    /* U */
     '1',		    /* V */
     '0',		    /* W */
     '2',		    /* X */
     '0',		    /* Y */
     '2'			    /* Z */
 };
 
 main()
 {
     char line[81], reduced[25];
     
     printf("type quit to terminate\n");
     do
     {
 	gets(line);
 	if(strcmp(line,"quit")==0)
 	    break;
 	soundex(reduced,line);
 	printf("%s = %s\n",line,reduced);
     } while(1);
     printf("done\n");
 }
 
 soundex(d,s)
 char *d, *s;
 {
     char last_char='#';
     int c;
     char *dorig, *ptr, *ptr2;
     
     dorig = ptr = d;
     
     /* pick up the first char in the string */
     
     *d++ = *s++;
 
     /* for the rest of characters in the string */
     while(*s)
     {
 	/* throw away nonalphabetic characters */
 	if(isalpha(*s)==0)
 	    continue;
 
 	/* convert to upper case */
 	*s = toupper(*s);
     
 	/* convert to group code and place into destination string */
 	
 	c = (int) ( *s++ - 'A') ;
 	*d++ = table[c];
     }
 
     *d = 0;
     
     do
     {
 	/* if character is '0' or character is same as last one */
 	if(*ptr=='0' || *ptr==last_char)
 	{
 	    /* get rid of the character */
 	    ptr2 = ptr;
 	    while(*ptr2=*(ptr2+1)) ptr2++;
 	}
 	else
 	{
 	    /* set last character seen */
 	    last_char = *ptr++;
 	}
     } while(*ptr);    /* while still a character left in the string */
     
     /* make sure the string isn't more than 4 characters */
     
     *(dorig+4) = 0;
      
 }
     
 	    
