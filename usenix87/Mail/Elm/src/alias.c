/**			alias.c			**/

/** This file contains alias stuff

    (C) Copyright 1986 Dave Taylor
**/

#include "headers.h"
#include <errno.h>

char *expand_group(), *get_alias_address(), *expand_system(), *get_token();
char *error_name(), *error_description();

unsigned long sleep();

extern int errno;

extern int  findnode_has_been_initialized;

read_alias_files()
{
	/** read the system and user alias files, if present.
	    Set the flags 'systemfiles' and 'userfiles' accordingly.
	**/

	char fname[SLEN];
	int  hash;

	if ((hash = open(system_hash_file, O_RDONLY)) == -1) {
	 dprint1(1,
		"Warning: Can't read system hash file %s (read_alias_files)\n",
		system_hash_file);
	  goto user;
	}

	read(hash, system_hash_table, sizeof system_hash_table);
	close(hash);

	/* and data file opened.. */

	if ((system_data = open(system_data_file, O_RDONLY)) == -1) {
	 dprint1(1, 
	         "Warning: Can't read system data file %s (read_alias_files)\n",
		 system_data_file);
	  goto user;
	}

	system_files++;	/* got the system files! */

user:   sprintf(fname,  "%s/.alias_hash", home); 

	if ((hash = open(fname, O_RDONLY)) == -1) {
	 dprint1(1,"Warning: Can't read user hash file %s (read_alias_files)\n",
		  fname);
	  return;
	}

	read(hash, user_hash_table, sizeof user_hash_table);
	close(hash);

	sprintf(fname,  "%s/.alias_data", home); 

	if ((user_data = open(fname, O_RDONLY)) == -1) {
	 dprint1(1,
	         "Warning: can't read user data file %s  (read_alias_files)\n",
		 fname);
	  return;
	}

	user_files++;	/* got user files too! */
}

int
add_alias()
{
	/** add an alias to the user alias text file.  Return zero 
	    if alias not added in actuality **/

	char name[SLEN], *address, address1[LONG_STRING];
	char comment[SLEN];
	char *strcpy();

	PutLine0(LINES-2,0,"Enter alias name: ");
	CleartoEOLN();
	Raw(OFF);
	gets(name);
	Raw(ON);
	if (strlen(name) == 0) 
	  return(0);
	if ((address = get_alias_address(name, 0, 0)) != NULL) {
	  dprint1(2, "Attempt to add a duplicate alias [%s] (add_alias)\n",
		   address); 
	  if (address[0] == '!') {
	    address[0] = ' ';
	    error1("already a group with that name:%s", address);
	  }
	  else
	    error1("already an alias for that: %s", address);
	  return(0);
	}
	PutLine1(LINES-2,0,"Full name for %s: ", name);
	CleartoEOLN();
	Raw(OFF);
	gets(comment);
	Raw(ON);
	if (strlen(comment) == 0) strcpy(comment, name);  
	PutLine1(LINES-2,0,"Enter address for %s: ",name);
	CleartoEOLN();
	Raw(OFF);
	gets(address1);
	Raw(ON);
	if (strlen(address1) == 0) {
	  error("No address specified!");
	  return(0);
	}
	add_to_alias_text(name, comment, address1);
	return(1);
}

int
add_current_alias()
{
	/** alias the current message to the specified name and
	    add it to the alias text file, for processing as
	    the user leaves the program.  Returns non-zero iff
	    alias actually added to file **/

	char name[SLEN], address1[LONG_STRING], buffer[LONG_STRING], *address;
	char comment[SLEN];

	if (current == 0) {
	 dprint0(3,"Add current alias called without any current message!\n");
	 error("No message to alias to!");
	 return(0);
	}
	
	PutLine0(LINES-2,0,"Current message address aliased to: ");
	CleartoEOLN();
	Raw(OFF);
	gets(name);
	Raw(ON);
	if (strlen(name) == 0)	/* cancelled... */
	  return(0);
	if ((address = get_alias_address(name, 0, 0)) != NULL) {
	 dprint1(3,
	         "Attempt to add a duplicate alias [%s] (add_current_alias)\n",
		 address); 
	  if (address[1] == '!') {
	    address[0] = ' ';
	    error1("already a group with that name:%s", address);
	  }
	  else 
	    error1("already an alias for that: %s", address); 
          return(0);
	}
	PutLine1(LINES-2,0,"Full name of %s: ", name);
	CleartoEOLN();
	Raw(OFF);
	gets(comment);
	Raw(ON);
	get_return(buffer);	/* grab the return address of this message */
	strcpy(address1, strip_parens(buffer));	/* remove parens! */
#ifndef DONT_OPTIMIZE_RETURN
	optimize_return(address1);
#endif
	PutLine3(LINES-2,0,"%s (%s) = %s", comment, name, address1);
	CleartoEOLN();
	add_to_alias_text(name, comment, address1);
	return(1);
}

add_to_alias_text(name, comment, address)
char *name, *comment, *address;
{
	/** Add the data to the user alias text file.  Return zero if we
	    succeeded, 1 if not **/
	
	FILE *file;
	char fname[SLEN];
	
	sprintf(fname,"%s/.alias_text", home);
	
	if ((file = fopen(fname, "a")) == NULL) {
	  dprint2(2, "FILE Failure attempting to add alias to file %s (%s)",
		   fname, "add_to_alias_text");
	  dprint2(2, "** %s - %s **\n", error_name(errno), 
		   error_description(errno));
	  error1("couldn't open %s to add new alias!", fname);
	  return(1);
	}

	fprintf(file,"%s : %s : %s\n", name, comment, address);
	fclose(file);

	chown(fname, userid, groupid);

	return(0);
}

show_alias_menu()
{
	MoveCursor(LINES-7,0); CleartoEOLN();	
	MoveCursor(LINES-6,0); CleartoEOLN();	
	MoveCursor(LINES-5,0); CleartoEOLN();
	
	PutLine0(LINES-7,COLUMNS-45, "Alias commands");
	Centerline(LINES-5,
"A)lias current msg, Check a P)erson or S)ystem, M)ake new alias, or R)eturn"
	);
}

alias()
{
	/** work with alias commands... **/
	char name[NLEN], *address, ch, buffer[SLEN];
	int  newaliases = 0;

	if (mini_menu)
	  show_alias_menu();

	/** now let's ensure that we've initialized everything! **/

#ifndef DONT_TOUCH_ADDRESSES
	
	if (! findnode_has_been_initialized) {
	  if (! mail_only)
	    error("initializing internal tables...");
#ifndef USE_DBM
	  get_connections();
	  open_domain_file();
#endif
	  init_findnode();
	  clear_error();
          findnode_has_been_initialized = TRUE;
	}

#endif

	define_softkeys(ALIAS);

	while (1) {
	  prompt("Alias: ");
	  CleartoEOLN();
	  ch = ReadCh();
	  MoveCursor(LINES-1,0); CleartoEOS();
	  
	  dprint1(2,"\n-- Alias command: %c\n\n", ch);

	  switch (tolower(ch)) {
	    case '?': alias_help();				break;

	    case 'a': newaliases += add_current_alias();	break;
	    case 'm': newaliases += add_alias(); 		break;

	    case RETURN:
	    case LINE_FEED:
	    case 'q':
	    case 'x':
	    case 'r': if (newaliases) install_aliases();
		      return;
	    case 'p': if (newaliases) 
			error("Warning: new aliases not installed yet!");
		      PutLine0(LINES-2,0,"Check for person: ");
		      CleartoEOLN();
		      Raw(OFF);
	              gets(name);
		      Raw(ON);
		      if ((address = get_alias_address(name, 0, 0))!=NULL) {
	                if (address[0] == '!') {
	                  address[0] = ' ';
	                  PutLine1(LINES-1,0,"Group alias:%-60.60s", address);
	                  CleartoEOLN();
		        }
			else
			  PutLine1(LINES-1,0,"Aliased address: %-60.60s", 
			  address);
		      }
	              else 
			error("not found");
		      break;

	    case 's': PutLine0(LINES-2,0,"Check for system: ");
		      CleartoEOS();
		      Raw(OFF);
	              gets(name);
		      Raw(ON);
		      if (talk_to(name)) 
#ifdef INTERNET_ADDRESS_FORMAT
			PutLine1(LINES-1,0,
		"You have a direct connection - the address is (user)@%s", 
			name);
#else
			PutLine1(LINES-1,0,
		"You have a direct connection - the address is %s!(user)", 
			name);
#endif
		      else {
		        sprintf(buffer, "(user)@%s", name);
#ifdef DONT_TOUCH_ADDRESS
	 	        strcpy(address, buffer);
#else
	 	        address = expand_system(buffer, FALSE);
#endif
		        if (strlen(address) > strlen(name) + 7)
		          PutLine1(LINES-1,0,"Address is: %.65s", address);
		        else
		          error1("couldn't expand system '%s'", name);
		      }
		      break;

	    case '@': PutLine0(LINES-2,0,"Fully expand alias: ");
		      CleartoEOS();
		      Raw(OFF);
	              gets(name);
		      Raw(ON);
		      if ((address = get_alias_address(name, 1, 0)) != NULL) {
	                ClearScreen();
			PutLine1(3,0,"Aliased address:\n\r%s", address);
	                PutLine0(LINES-1,0,"Press <return> to continue: ");
			(void) getchar();
		      }
	              else 
			error("not found");
		      if (mini_menu) show_alias_menu();
		      break;
	    default : error("Invalid input!");
	  }
	}
}

install_aliases()
{
	/** run the 'newalias' program and install the newly
	    added aliases before going back to the main
	    program! 
	**/


	error("Adding new aliases...");
	sleep(2);

	if (system_call(newalias, SH) == 0) {
	  error("Re-reading the database in...");
	  sleep(2);
	  read_alias_files();
	  set_error("New aliases installed successfully");
	}
	else
	  set_error("'newalias' failed.  Please check alias_text");
}

alias_help()
{
	/** help section for the alias menu... **/

	char ch;

	MoveCursor(LINES-3, 0);	CleartoEOS();

	if (! mini_menu)
	  lower_prompt("Key you want help for : ");
	else {
	  Centerline(LINES-3, 
"Enter key you want help for, '?' for list or '.' to leave help");
	  lower_prompt("Key : ");
	}

	while ((ch = tolower(ReadCh())) != '.') {
	  switch(ch) {
	    case '?' : display_helpfile(ALIAS_HELP);	
	               if (mini_menu) show_alias_menu();	return;
	    case 'a': error(
"a = Add return address of current message to alias database");	break;
	    case 'm': error(
"m = Make new user alias, adding to alias database when done");	break;

	    case RETURN:
	    case LINE_FEED:
	    case 'q':
	    case 'x':
	    case 'r': error("return from alias menu");		break;
		      
	    case 'p': error(
"p = check for a person in the alias database");		break;
	
	    case 's': error(
"s = check for a system in the host routing/domain database");	break;
	
	    default : error("That key isn't used in this section");	break;
	    case '.': return;
	  }
	  if (! mini_menu)
	    lower_prompt("Key you want help for : ");
	  else 
	    lower_prompt("Key : ");
	}
}
