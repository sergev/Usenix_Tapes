/**			domains.c			**/

/** This file contains all the code dealing with the expansion of 
    domain based addresses in Elm.  It uses the file "domains" as
    defined in the sysdefs.h file.

    (C) Copyright 1986 Dave Taylor

    From a file format and idea in "uumail" - designed by Stan Barber.
**/

#include <stdio.h>
#include <ctype.h>

#include "headers.h"

#ifdef BSD
# undef toupper
# undef tolower
#endif

/** define the various characters that we can encounter after a "%" sign
    in the template file...
**/

#define USERNAME	'U'	/* %U = the name of the remote user */
#define HOSTNAME	'N'	/* %N = the remote machine name     */
#define FULLNAME	'D'	/* %D = %N + domain info given      */
#define NPATH		'R'	/* %R = path to %N from pathalias   */
#define PPATH		'P'	/* %P = path to 'P' from pathalias  */
#define OBSOLETE	'S'	/* %S = (used to be suffix string)  */

/** and finally some characters that are allowed in user/machine names **/

#define okay_others(c)	(c == '-' || c == '^' || c == '$' || c == '_')

/** and some allowed ONLY in the username field **/

#define special_chars(c)	(c == '%' || c == ':')

char *find_path_to(), *expand_domain(), *match_and_expand_domain();
char *strcpy(), *strcat(), *strtok();
unsigned long sleep();
void rewind();
	      
open_domain_file()
{
	if ((domainfd = fopen(domains, "r")) == NULL) {
	  dprint1(1, "Can't open file %s as domains file (open_domain_file)\n", 
		  domains);
	}
	else {
	  dprint1(2,
            "\nOpened '%s' as the domain database. (open_domain_file)\n\n", 
		  domains);
	}
	
	/* if it fails it'll instantiate domainfd to NULL which is
	   exactly what we want to have happen!! */
}

char *expand_domain(buffer)
char *buffer;
{
	/** Expand the address 'buffer' based on the domain information, 
	    if any.  Returns NULL if it can't expand it for any reason.
	**/

	char name[2*NLEN], address[2*NLEN], domain[2*NLEN];
	char *match_and_expand_domain();

	if (domainfd == NULL) return(NULL);	/* no file present! */

	if (explode(buffer, name, address, domain)) 
	  return( match_and_expand_domain(domain, name, address) );
	else {	/* invalid format - not "user@host.domain" */
	  dprint1(3, 
		 "Invalid format for domain expansion: %s (expand_domain)\n", 
		   buffer);
	  return(NULL);
	}
}	

int
explode(buffer, name, address, domain)
char *buffer, *name, *address, *domain;
{
	/** Break buffer, if in format name@machine.domain, into the
	    component parts, otherwise return ZERO and don't worry
	    about the values of the parameters!
	**/
	
	register int i, j = 0;

	/** First get the name... **/

	for (i=0; buffer[i] != '@'; i++) {
	  if (! isalnum(buffer[i]) && ! okay_others(buffer[i]) && ! 
		special_chars(buffer[i]))
	    return(0);			/* invalid character in string! */
	  name[i] = buffer[i];
	}

	name[i++] = '\0';

	/** now let's get the machinename **/

	while (buffer[i] != '.') {
	  if (! isalnum(buffer[i]) && ! okay_others(buffer[i]))
	     return(0);			/* invalid character in string! */
	  address[j++] = buffer[i++];
	}
	address[j] = '\0';

	j = 0;

	/** finally let's get the domain information (there better be some!) **/

	while (buffer[i] != '\0') {
	  if (! isalnum(buffer[i]) && ! okay_others(buffer[i]) && 
	        buffer[i] != '.')
	    return(0);		      /* an you fail again, bozo! */
	  domain[j++] = toupper(buffer[i]);
	  i++;
	}

	domain[j] = '\0';
	
	return(j);		/* if j == 0 there's no domain info! */
}
        
char *match_and_expand_domain(domain, name, machine)
char *domain, *name, *machine;
{
	/** Given the domain, try to find it in the domain file and
   	    if found expand the entry and return the result as a 
	    character string...
	**/

	static char address[SLEN];
	char   buffer[SLEN], domainbuff[NLEN];
	char   field1[2*NLEN], field2[2*NLEN], field3[2*NLEN];
	char   *path, *template, *expanded, *mydomain, *strtok();
	int    matched = 0, in_percent = 0;
	register int i, j = 0;

	address[j] = '\0';

	domainbuff[0] = '\0';
	mydomain = (char *) domainbuff;		    /* set up buffer etc */

	do { 
	  rewind(domainfd);		           /* back to ground zero! */

	  if (strlen(mydomain) > 0) {		   /* already in a domain! */
	    mydomain++;		 		      /* skip leading '.' */
	    while (*mydomain != '.' && *mydomain != ',') 
	      mydomain++;	 		      /* next character   */
	    if (*mydomain == ',')
	      return (NULL);  			  /* didn't find domain!  */
	  }
	  else
	    sprintf(mydomain, "%s,", domain);		/* match ENTIRELY! */

	/* whip through file looking for the entry, please... */

	while (fgets(buffer, SLEN, domainfd) != NULL) {
	  if (buffer[0] == '#')				  /* skip comments */
	    continue;
	  if (strncmp(buffer, mydomain, strlen(mydomain)) == 0) { /* match? */
	     matched++;	/* Gotcha!  Remember this momentous event! */
	     break;
	  }
	}

	if (! matched) 
	   continue;		/* Nothing.  Not a sausage!  Step through! */

	/** We've matched the domain! **/

	no_ret(buffer);

	(void) strtok(buffer, ",");	/* skip the domain info */

	strcpy(field1, strtok(NULL, ","));	/* fun 		*/
	strcpy(field2, strtok(NULL, ","));	/*    stuff     */
	strcpy(field3, strtok(NULL, ","));	/*       eh?    */
	
	path = (char *) NULL;

	/* now we merely need to figure out what permutation this is! */

	if (field3 == NULL || strlen(field3) == 0)
	  if (field2 == NULL || strlen(field2) == 0)
	    template = (char *) field1;
	  else {
	    path     = (char *) field1;
	    template = (char *) field2;
	  }
	else {
	  dprint1(2,"Domain info for %s from file broken into THREE fields!!\n",
		   domain);
	  dprint3(2, "-> %s\n-> %s\n-> %s\n", field1, field2, field3);
	  error1("Warning: domain %s uses a defunct field!!", domain);
	  sleep(2);
	  path     = (char *) field1;
	  template = (char *) field3;
	}

	if (strlen(path) > 0 && path[0] == '>') 
	   path++;	/* skip the '>' character, okay? */

	j = 0;		 	/* address is zero, right now, right?? */
	address[j] = '\0';	      /* make sure string is too! */

	for (i=0; i < strlen(template); i++) {
	  if (template[i] == '%') {
	    if (! in_percent)			   /* just hit a NEW percent! */
	      in_percent = 1;
	    else {		  /* just another percent sign on the wall... */
	      address[j++] = '%';
	      address[j] = '\0';		     /* ALWAYS NULL terminate */
	      in_percent = 0;
	    }
	  }
	  else if (in_percent) {	       /* Hey! a real command string */
	    in_percent = 0;
	    switch (template[i]) {
	      case USERNAME: strcat(address, name);		break;
	      case HOSTNAME: strcat(address, machine);		break;
	      case FULLNAME: strcat(address, machine);
			     strcat(address, domain);		break;
	      case NPATH   : 

		 if ((expanded = find_path_to(machine, FALSE)) == NULL) {
		    dprint2(3,"\nCouldn't expand system path '%s' (%s)\n\n",
			    machine, "domains");
	            error1("Couldn't find a path to %s!", machine);
	            sleep(2);
	            return(NULL);	/* failed!! */
	         }
	         strcat(address, expanded);	/* isn't this fun??? */

	         break;

	      case PPATH   : 

		 if ((expanded = find_path_to(path, FALSE)) == NULL) {
		    dprint2(3,"\nCouldn't expand system path '%s' (%s)\n\n",
			    path, "domains");
	            error1("I Couldn't find a path to %s!", path);
	            sleep(2);
	            return(NULL);	/* failed!! */
	         }
	         strcat(address, expanded);	/* isn't this fun??? */

	         break;

	      case OBSOLETE:	/* fall through.. */
	      default      : dprint2(1,
	     "\nError: Bad sequence in template file for domain '%s': %%%c\n\n",
			domain, template[i]);
	    }
	    j = strlen(address);
	  }
	  else {
	    address[j++] = template[i];
	    address[j] = '\0';			/* null terminate */
	  }
	}	

	address[j] = '\0';

	} while (strlen(address) < 1);

	return( (char *) address);
}
