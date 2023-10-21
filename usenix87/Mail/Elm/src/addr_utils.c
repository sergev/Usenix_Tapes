/**			addr_utils.c			**/

/** This file contains addressing utilities 

    (C) Copyright 1986 Dave Taylor 
**/

#include "headers.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#ifdef BSD
#undef tolower
#endif

char *shift_lower(), *get_alias_address(), *get_token(), *strtok(),
     *strchr(), *strcpy(), *strcat(), *strncpy();

int
talk_to(sitename)
char *sitename;
{
	/** If we talk to the specified site, return true, else
	    we're going to have to expand this baby out, so 
	    return false! **/

	struct lsys_rec  *sysname;

	sysname = talk_to_sys;

	if (sysname == NULL) {
	 dprint0(2,
		"Warning - talk_to_sys is currently set to NULL! (talk_to)\n");
	 return(0);
	}

	while (sysname != NULL) {
	  if (strcmp(sysname->name, sitename) == 0)
	    return(1);
	  else
	    sysname = sysname->next;
	}

	return(0);
}

remove_domains(host)
char *host;
{
	/** Remove all entries following the first '.' to ensure that
	    entries like "MIT.ARPA" will match "MIT" in the database
	**/

	register int loc = 0;

	while (host[loc] != '.' && host[loc] != '\0')
	  loc++;

	if (host[loc] == '.') host[loc] = '\0';
}

add_site(buffer, site, lastsite)
char *buffer, *site, *lastsite;
{
	/** add site to buffer, unless site is 'uucp', current machine, or
            site is the same as lastsite.   If not, set lastsite to
            site.
	**/

	char local_buffer[LONG_SLEN];
	char *strip_parens();

	if (strcmp(site, "uucp") != 0)
	  if (strcmp(site, lastsite) != 0) 
	    if (strcmp(site, hostname) != 0) {
	      if (buffer[0] == '\0')
	        strcpy(buffer, strip_parens(site));         /* first in list! */
	      else {
	        sprintf(local_buffer,"%s!%s", buffer, strip_parens(site));
	        strcpy(buffer, local_buffer);
	      }
	      strcpy(lastsite, strip_parens(site)); /* don't want THIS twice! */
	    }
}

#ifdef USE_EMBEDDED_ADDRESSES

get_address_from(prefix, line, buffer)
char *prefix, *line, *buffer;
{
	/** This routine extracts the address from either a 'From:' line
	    or a 'Reply-To:' line...the algorithm is quite simple, too:
	    increment 'line' past header, then check last character of 
	    line.  If it's a '>' then the address is contained within '<>'
	    and if it's a ')' then the address is in the 'clear'... **/

	register int i, j = 0;
	
	no_ret(line);

	line = (char *) (line + strlen(prefix) + 1);

	if (line[strlen(line)-1] == '>') {
	  for (i=strlen(line)-2; i > -1 && line[i] != '<'; i--)
	    buffer[j++] = line[i];
	  buffer[j] = 0;
	  reverse(buffer);
	}
	else {	/* either ')' or address in the clear... */
	  for (i=0; i < strlen(line) && line[i] != '('; i++)
	    buffer[j++] = line[i];
	  if (buffer[j-1] == '(') j--;
	  buffer[j] = 0;
	}
}

#endif

translate_return(addr, ret_addr)
char *addr, *ret_addr;
{
	/** Return ret_addr to be the same as addr, but with the login 
            of the person sending the message replaced by '%s' for 
            future processing... 
	    Fixed to make "%xx" "%%xx" (dumb 'C' system!) 
	**/

	register int loc, loc2, index = 0;
	
	loc2 = chloc(addr,'@');
	if ((loc = chloc(addr, '%')) < loc2)
	  loc2 = loc;

	if (loc2 != -1) {	/* ARPA address. */
	  /* algorithm is to get to '@' sign and move backwards until
	     we've hit the beginning of the word or another metachar.
	  */
	  for (loc = loc2 - 1; loc > -1 && addr[loc] != '!'; loc--)
	     ;
	}
	else {			/* usenet address */
	  /* simple algorithm - find last '!' */

	  loc2 = strlen(addr);	/* need it anyway! */

	  for (loc = loc2; loc > -1 && addr[loc] != '!'; loc--)
	      ;
	}
	
	/** now copy up to 'loc' into destination... **/

	while (index <= loc) {
	  ret_addr[index] = addr[index];
	  index++;
	}

	/** now append the '%s'... **/

	ret_addr[index++] = '%';
	ret_addr[index++] = 's';

	/** and, finally, if anything left, add that **/

	while (loc2 < strlen(addr)) {
	  ret_addr[index++] = addr[loc2++];
	  if (addr[loc2-1] == '%')	/* tweak for "printf" */
	    ret_addr[index++] = '%';
	}
	
	ret_addr[index] = '\0';
}

build_address(to, full_to)
char *to, *full_to;
{
	/** loop on all words in 'to' line...append to full_to as
	    we go along, until done or length > len.  Modified to
	    know that stuff in parens are comments... 
	**/

	register int i, changed = 0, in_parens = 0;
	char word[SLEN], *ptr, buffer[SLEN];
	char new_to_list[LONG_SLEN];
	char *strpbrk(), *expand_system(), *strcat();

	new_to_list[0] = '\0';

	i = get_word(to, 0, word);

	full_to[0] = '\0';

	while (i > 0) {

	  if (word[0] == '(' || in_parens) {
	    in_parens = (word[strlen(word-1)] != ')');
	    strcat(full_to, " ");
	    strcat(full_to, word);
	  }
	  else if (strpbrk(word,"!@:") != NULL)
#ifdef DONT_TOUCH_ADDRESS
	    sprintf(full_to, "%s%s%s", full_to,
                    full_to[0] != '\0'? ", " : "", word);
#else
	    sprintf(full_to, "%s%s%s", full_to,
                    full_to[0] != '\0'? ", " : "", expand_system(word, 1));
#endif
	  else if ((ptr = get_alias_address(word, 1, 0)) != NULL)
	    sprintf(full_to, "%s%s%s", full_to, 
                    full_to[0] != '\0'? ", " : "", ptr);
	  else if (strlen(word) > 0) {
	    if (valid_name(word)) 
	      sprintf(full_to, "%s%s%s", full_to,
                      full_to[0] != '\0'? ", " : "", word);
	    else if (check_only) {
	      printf("(alias \"%s\" is unknown)\n\r", word);
	      changed++;
	    }
	    else if (! isatty(fileno(stdin)) ) {	/* batch mode error! */
	      fprintf(stderr,"Cannot expand alias '%s'!\n\r", word);
	      fprintf(stderr,"Use \"checkalias\" to find valid addresses!\n\r");
	      dprint1(1,
		      "Can't expand alias %s - bailing out! (build_address)\n", 
		      word);
	      emergency_exit();
	    }
	    else {
	      dprint1(2,"Entered unknown address %s (build_address)\n", word);
	      sprintf(buffer, "'%s' is an unknown address.  Replace with: ", 
	              word);
	      word[0] = '\0';

	      if (mail_only) 
	        printf(buffer);
	      else
	        PutLine0(LINES, 0, buffer);
		
	      (void) optionally_enter(word, LINES, strlen(buffer), FALSE);
	      if (strlen(word) > 0) {
	        sprintf(new_to_list, "%s%s%s", new_to_list,
			strlen(new_to_list) > 0? " ":"", word);
	        dprint1(3,"Replaced with %s (build_address)\n", word);
	      }
	      else
		dprint0(3,"Address removed from to list (build_address)\n");
	      if (mail_only) printf("\n\r");
	      changed++;
	      clear_error();
	      continue;
	    }
	  }

	  i = get_word(to, i, word);
	}

	if (changed)
	  strcpy(to, new_to_list);
}

int
real_from(buffer, entry)
char *buffer;
struct header_rec *entry;
{
	/***** Returns true iff 's' has the seven 'from' fields, (or
	       8 - some machines include the TIME ZONE!!!)
	       Initializing the date and from entries in the record 
	       and also the message received date/time.  *****/

	char junk[STRING], timebuff[STRING], holding_from[SLEN];
	int  eight_fields = 0;

	entry->year[0] = '\0';
	junk[0] = '\0';

	/* From <user> <day> <month> <day> <hr:min:sec> <year> */

	sscanf(buffer, "%*s %*s %*s %*s %*s %s %*s %s", timebuff, junk);

	if (timebuff[1] != ':' && timebuff[2] != ':') { 
	  dprint1(3,"real_from returns FAIL [bad time field] on\n-> %s\n", 
		  buffer);
	  return(FALSE);
	}
	if (junk[0] != '\0') {	/* try for 8 field entry */
	  junk[0] = '\0';
	  sscanf(buffer, "%*s %*s %*s %*s %*s %s %*s %*s %s", timebuff, junk);
	  if (junk[0] != '\0') {
	    dprint1(3,"real_from returns FAIL [too many fields] on\n-> %s\n", 
		    buffer);
	    return(FALSE);
	  }
	  eight_fields++;
	}

	/** now get the info out of the record! **/

	if (eight_fields) 
	  sscanf(buffer, "%s %s %s %s %s %s %*s %s",
	            junk, holding_from, entry->dayname, entry->month, 
                    entry->day, entry->time, entry->year);
	else
	  sscanf(buffer, "%s %s %s %s %s %s %s",
	            junk, holding_from, entry->dayname, entry->month, 
                    entry->day, entry->time, entry->year);
	
	strncpy(entry->from, holding_from, STRING);
	resolve_received(entry);
	return(entry->year[0] != '\0');
}

forwarded(buffer, entry)
char *buffer;
struct header_rec *entry;
{
	/** Change 'from' and date fields to reflect the ORIGINATOR of 
	    the message by iteratively parsing the >From fields... 
	    Modified to deal with headers that include the time zone
	    of the originating machine... **/

	char machine[SLEN], buff[SLEN], holding_from[SLEN];

	machine[0] = '\0';

	sscanf(buffer, "%*s %s %s %s %s %s %s %*s %*s %s",
	            holding_from, entry->dayname, entry->month, 
                    entry->day, entry->time, entry->year, machine);

	if (isdigit(entry->month[0])) { /* try for veeger address */
	  sscanf(buffer, "%*s %s %s%*c %s %s %s %s %*s %*s %s",
	            holding_from, entry->dayname, entry->day, entry->month, 
                    entry->year, entry->time, machine);
	}
	if (isalpha(entry->year[0])) { /* try for address including tz */
	  sscanf(buffer, "%*s %s %s %s %s %s %*s %s %*s %*s %s",
	            holding_from, entry->dayname, entry->month, 
                    entry->day, entry->time, entry->year, machine);
	}

	if (machine[0] == '\0')
	  sprintf(buff,"anonymous");
	else
	  sprintf(buff,"%s!%s", machine, holding_from);

	strncpy(entry->from, buff, STRING);
}

parse_arpa_from(buffer, newfrom)
char *buffer, *newfrom;
{
	/** try to parse the 'From:' line given... It can be in one of
	    two formats:
		From: Dave Taylor <hplabs!dat>
	    or  From: hplabs!dat (Dave Taylor)

	    Added: removes quotes if name is quoted (12/12)
	    Added: only copies STRING characters...
	    Added: if no comment part, copy address instead! 
	**/

	char temp_buffer[SLEN], *temp;
	register int i, j = 0;

	temp = (char *) temp_buffer;
	temp[0] = '\0';

	no_ret(buffer);		/* blow away '\n' char! */

	if (lastch(buffer) == '>') {
	  for (i=strlen("From: "); buffer[i] != '\0' && buffer[i] != '<' &&
	       buffer[i] != '('; i++)
	    temp[j++] = buffer[i];
	  temp[j] = '\0';
	}
	else if (lastch(buffer) == ')') {
	  for (i=strlen(buffer)-2; buffer[i] != '\0' && buffer[i] != '(' &&
	       buffer[i] != '<'; i--)
	    temp[j++] = buffer[i];
	  temp[j] = '\0';
	  reverse(temp);
	}

#ifdef USE_EMBEDDED_ADDRESSES

	/** if we have a null string at this point, we must just have a 
	    From: line that contains an address only.  At this point we
	    can have one of a few possibilities...

		From: address
		From: <address>
		From: address ()
	**/
	  
	if (strlen(temp) == 0) {
	  if (lastch(buffer) != '>') {       
	    for (i=strlen("From:");buffer[i] != '\0' && buffer[i] != '('; i++)
	      temp[j++] = buffer[i];
	    temp[j] = '\0';
	  }
	  else {	/* get outta '<>' pair, please! */
	    for (i=strlen(buffer)-2;buffer[i] != '<' && buffer[i] != ':';i--)
	      temp[j++] = buffer[i];
	    temp[j] = '\0';
	    reverse(temp);
	  }
	}
#endif
	  
	if (strlen(temp) > 0) {		/* mess with buffer... */

	  /* remove leading spaces and quotes... */

	  while (whitespace(temp[0]) || quote(temp[0]))
	    temp = (char *) (temp + 1);		/* increment address! */

	  /* remove trailing spaces and quotes... */

	  i = strlen(temp) - 1;

	  while (whitespace(temp[i]) || quote(temp[i]))
	   temp[i--] = '\0';

	  /* if anything is left, let's change 'from' value! */

	  if (strlen(temp) > 0)
	    strncpy(newfrom, temp, STRING);
	}
}

parse_arpa_date(string, entry)
char *string;
struct header_rec *entry;
{
	/** Parse and figure out the given date format... return
	    the entry fields changed iff it turns out we have a
	    valid parse of the date!  **/

	char word[15][NLEN], buffer[SLEN], *bufptr;
	char *aword;
	int  words = 0;

	strcpy(buffer, string);
	bufptr = (char *) buffer;

	/** break the line down into words... **/

	while ((aword = strtok(bufptr," \t '\"-/(),.")) != NULL) {
	  strcpy(word[words++], aword);
	  bufptr = NULL;
	}

	if (words < 6) {	/* strange format.  We're outta here! */
	  dprint1(3,"parse_arpa_date failed [less than six fields] on\n-> %s\n",
		  string);
	  return;
	}

	/* There are now five possible combinations that we could have:
	 
	    Date: day_number month_name year_number time timezone
	    Date: day_name day_number month_name year_number ...
	    Date: day_name month_name day_number time year_number
	    Date: day_name month_name day_number year_number time
	    Date: day_number month_name year_number time timezone day_name

	   Note that they are distinguishable by checking the first
	   character of the second, third and fourth words... 
	*/

	if (isdigit(word[1][0])) {			/*** type one! ***/
	  if (! valid_date(word[1], word[2], word[3])) {
	    dprint4(3,"parse_arpa_date failed [bad date: %s/%s/%s] on\n-> %s\n",
		  word[1], word[2], word[3], string);
	    return;		/* strange date! */
	  }
	  strncpy(entry->day, word[1], 3);
	  strncpy(entry->month, word[2], 3);
	  strncpy(entry->year,  word[3], 4);
	  strncpy(entry->time,  word[4], 10);
	}
	else if (isdigit(word[2][0])) {		        /*** type two! ***/
	  if (! valid_date(word[2], word[3], word[4])) {
	    dprint4(3,"parse_arpa_date failed [bad date: %s/%s/%s] on\n-> %s\n",
		  word[2], word[3], word[4], string);
	    return;		/* strange date! */
	  }
	  strncpy(entry->day, word[2], 3);
	  strncpy(entry->month, word[3], 3);
	  strncpy(entry->year,  word[4], 4);
	  strncpy(entry->time,  word[5], 10);
	}
	else if (isdigit(word[3][0])) {		
	  if (word[4][1] == ':' || 
              word[4][2] == ':') {	               /*** type three! ***/
	    if (! valid_date(word[3], word[2], word[5])) {
	     dprint4(3,
		"parse_arpa_date failed [bad date: %s/%s/%s] on\n-> %s\n",
		    word[3], word[2], word[5], string);
	      return;		/* strange date! */
	    }
	    strncpy(entry->year,  word[5], 4);
	    strncpy(entry->time,  word[4], 10);
	  }
	  else {				       /*** type four!  ***/ 
	    if (! valid_date(word[3], word[2], word[4])) {
	     dprint4(3,"parse_arpa_date failed [bad date: %s/%s/%s] on\n-> %s\n",
		    word[3], word[2], word[4], string);
	      return;		/* strange date! */
	    }
	    strncpy(entry->year,  word[4], 4);
	    strncpy(entry->time, word[5], 10);
	  }
	  strncpy(entry->day, word[3], 3);
	  strncpy(entry->month, word[2], 3);
	}
}

fix_arpa_address(address)
char *address;
{
	/** Given a pure ARPA address, try to make it reasonable.

	    This means that if you have something of the form a@b@b make 
            it a@b.  If you have something like a%b%c%b@x make it a%b@x...
	**/

	register int host_count = 0, i;
	char     hosts[MAX_HOPS][2*NLEN];	/* array of machine names */
	char     *host, *addrptr;

	/*  break down into a list of machine names, checking as we go along */
	
	addrptr = (char *) address;

	while ((host = get_token(addrptr, "%@", 2)) != NULL) {
	  for (i = 0; i < host_count && ! equal(hosts[i], host); i++)
	      ;

	  if (i == host_count) {
	    strcpy(hosts[host_count++], host);
	    if (host_count == MAX_HOPS) {
	       dprint0(2,
              "Can't build return address - hit MAX_HOPS (fix_arpa_address)\n");
	       error("Can't build return address - hit MAX_HOPS limit!");
	       return(1);
	    }
	  }
	  else 
	    host_count = i + 1;
	  addrptr = NULL;
	}

	/** rebuild the address.. **/

	address[0] = '\0';

	for (i = 0; i < host_count; i++)
	  sprintf(address, "%s%s%s", address, 
	          address[0] == '\0'? "" : 
	 	    (i == host_count - 1 ? "@" : "%"),
	          hosts[i]);

	return(0);
}

figure_out_addressee(buffer, mail_to)
char *buffer;
char *mail_to;
{
	/** This routine steps through all the addresses in the "To:"
	    list, initially setting it to the first entry (if mail_to
	    is NULL) or, if the user is found (eg "alternatives") to
	    the current "username".

	    Modified to know how to read quoted names...
	**/

	char *address, *bufptr;
	register int index = 0, index2 = 0;
	
	if (equal(mail_to, username)) return;	/* can't be better! */

	bufptr = (char *) buffer;	/* use the string directly   */

	if (strchr(buffer,'"') != NULL) {	/* we have a quoted string */
	  while (buffer[index] != '"')
	    index++;
	  index++;	/* skip the leading quote */
	  while (buffer[index] != '"' && index < strlen(buffer))
	    mail_to[index2++] = buffer[index++];
	  mail_to[index2] = '\0';
	}
	else while ((address = strtok(bufptr, " ,\t\n\r")) != NULL) {
	  if (! okay_address(address, "don't match me!")) {
	    strcpy(mail_to, username);	/* it's to YOU! */
	    return;
	  }
	  else if (strlen(mail_to) == 0)	/* it's SOMEthing! */
	    get_return_name(address, mail_to, FALSE);

	  bufptr = (char *) NULL;	/* set to null */
	}

	return;
}
