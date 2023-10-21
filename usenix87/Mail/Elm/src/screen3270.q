From hpccc!mcgregor@hplabs.ARPA Thu Jun  5 11:41:49 1986
Received: from hplabs.ARPA by hpldat ; Thu, 5 Jun 86 11:41:32 pdt
Message-Id: <8606051841.AA16872@hpldat>
Received: by hplabs.ARPA ; Thu, 5 Jun 86 11:38:07 pdt
To: hplabs!taylor@hplabs.ARPA
Date: Thu, 5 Jun 86 11:36:39 PDT
From: hpccc!mcgregor@hplabs.ARPA (Scott McGregor)
Subject: revised screen3270.q
Telephone: (415) 857-5875
Postal-Address: Hewlett-Packard, PO Box 10301, Mail stop 20CH, Palo Alto CA 943X03-0890
X-Mailer: ELM [version 1.0]


/*
 * screen3270.q
 *
 *   Created for ELM, 5/86 to work (hopefully) on UTS systems with 3270
 *   type terminals, by Scott L. McGregor, HP Corporate Computing Center.
 *
 */

#include "headers.h"
#  include <sys/utsname.h>
#  include <sys/tubio.h>

#  include <errno.h>


#  define  TTYIN		0		/* standard input */
#include <stdio.h>
#define MAXKEYS 101
#define OFF 0
#define UNKNOWN 0
#define ON 1
#define FALSE 0
#define TRUE 1

char *error_name();

extern int _line, _col;

 
pfinitialize()
{
	char cp[80];
	
	dprint0(9,"pfinitialize()\n");
	pfinit();
	/*
	 * load the system defined pfkeys
	 */
	pfload("/usr/local/etc/.elmpfrc");
	/*
	 * load the user's keys if any
	 */
	strcat(cp,home);
	strcat(cp,"/.elmpfrc");
	pfload(cp);
	pfprint();
}

/*
 * note, inputs are limited to 80 characters! Any larger amount
 * will be truncated without notice!
 */


/*
 * pfinit() initializes _pftable
 */
pfinit()
{
	int i,j;
	
	dprint0(9,"pfinit()\n");
	for (i=0;i<MAXKEYS;i++) {
		for (j=0;j<80;j++) {
			_pftable[i][j]='\0';
		}
	}
	return(0);
}


/*
 * pfset(key) sets _pftable entries.
 */
pfset(key,return_string)
int key;
char *return_string;
{
	int i;

	dprint2(9,"pfset(%d,%s)\n",key,return_string);
	for (i=0;i<=80;i++) {
		if (i <= strlen(return_string))
			_pftable[key][i] = return_string[i];
		else _pftable[key][i] = '\0';
	}
	dprint2(9,"pfset: %d %s\n",key,_pftable[key]);
       
	return(0);
}

/*
 *  pfload(name) reads file "name" and parses the
 *  key definitions in it into a table used by the
 *  pfreturn.
 */
pfload(name)
char *name;
{
	FILE *pfdefs;
	int i,j,k;
	int key = 0;
	char defn[80];
	char newdefn[80];

	dprint1(9,"pfload(%s)\n",name);
	if ((pfdefs = fopen(name,"r")) == NULL){
		dprint2(2,"%s pfrc open failed, %s \n",name,
			error_name(errno));
		return(0);
	 }

	 /*
	  * This program reads .elmpfrc files which it currently
	  * expects to be in the form:
	  * 
	  * <pfkeynumber> <whitespace> <pfkeyvalue> [<whitespace> <comment>]
	  * 
	  * Pfkeynumber is an integer 1-24.  Whitespace can be one
	  * or more blanks or tabs.  Pfkeyvalue is any string NOT
	  * containing whitespace (however, \b for blank, \t for tab
	  * and \n for newline can be used instead to indicate that
	  * the indicated whitespace character is part of a command.
	  * Note that the EnTER key will NOT be treated as a newline
	  * command, so defining a newline key is a good idea!
	  * Anything else appearing on the line after the pfkey is ignored
	  * and so can be used as a comment.
	  *
	  * This may not be the best form for a file used by
	  * humans to set parms, and if someone comes up with a
	  * better one and a parser to read it, then this can be
	  * replaced.
	  *
	  */
	 
	dprint1(1,"%s pfrc opened\n",name);
	k = 0;
	while ( fscanf(pfdefs,"%d%s",&key,defn) != EOF ) {
		dprint2(9,"pfkey definition 1: %d %s\n",key,defn);
		if ((key < 0) || (key > MAXKEYS)) {
			dprint2(9,"pfkey defn failed: key=%d MAXKEYS=%d\n",key,MAXKEYS);
			k++;
		} else {
			dprint2(9,"pfkey definition 2: %d %s\n",key,defn);
			for (i=0,j=0;i<strlen(defn);i++) {
				if (defn[i]=='\\') {
					if (defn[i+1]=='n') {
						newdefn[j++]='\n'; i++;
					}
					if (defn[i+1]=='t') {
						newdefn[j++]='\t'; i++;
					}
					if (defn[i+1]=='0') {
						newdefn[j++]='\0'; i++;
					}
					if (defn[i+1]=='1') {
						newdefn[j++]='\001'; i++;
					}
					if (defn[i+1]=='b') {
						newdefn[j++]=' '; i++;
					}
				}
				 else {
					newdefn[j++]=defn[i];
				}
			}
			dprint2(9,"pfkey new definition: %d %s\n",key,newdefn);
			pfset(key,newdefn);
		}
	}
	dprint1(9,"pfkey definition table:  %s\n",_pftable);
	return(k);
}


/*
 * pfreturn(key) returns the stored string for that pfkey.
 */
pfreturn(key,string)
int key;
char string[];
{
	int i;
	
	dprint2(9,"pfreturn(%d,%s)\n",key,string);
	for (i=0;i<80;i++) {
		string[i] = _pftable[key][i];
	}
	dprint1(9,"pfreturn string=%s\n",string);
	return;
}


/*
 * pfprint() prints all pfkey definitions
 */
pfprint()

{
	int i;
	char string[80];

	for (i=0;i<MAXKEYS;i++) {
		if (strlen(_pftable[i]) != 0)
		dprint2(9,"%d pf table entry=%s\n",i+1,_pftable[i]);
	}
}

/*
 * rowcol2offset(row,col) takes the row and column numbers and
 * returns the offset into the array.
 * row and column are assumed to vary from 0 to LINES-1, and COLUMNS-1
 * respectively.
 */
rowcol2offset(row,col)
int row, col;
{
	dprint2(9,"rowcol2offset(%d,%d)\n",row,col);
	
	if ((row <= LINES) && (row >= 0)) {
		if ((col <= COLUMNS) && (col >=0)) {
			return(row*COLUMNS+col);
		}
		else return(0);
	}
	else return(0);
}

/*
 * offset2row(offset) takes the offset returnes the row.
 * row is assumed to vary from 0 to LINES-1.
 */
offset2row(offset)
int offset;
{
	int i;
	
	dprint1(9,"offset2row(%d)\n",offset);
	i =  (int) (offset / COLUMNS);
	dprint1(9,"offset2row returns= %d)\n",i);
	return(i);
}

/*
 * offset2col(offset) takes the offset returnes the col.
 * col is assumed to vary from 0 to COLUMNS-1.
 */
offset2col(offset)
int offset;
{
	int i;
	
	dprint1(9,"offset2col(%d)\n",offset);
	i =  (int) (offset % COLUMNS);
	dprint1(9,"offset2col returns= %d)\n",i);
	return(i);
}

/*
 * Row(row) takes the row in 0 <= row < LINES and returns
 * row in 0 < row <= LINES.
 */
Row(row)
int row;
{
	dprint1(9,"Row(%d)\n",row);
	return(row+1);
}

/*
 * Col(Col) takes the col in 0 <= col < COLUMNS and returns
 * col in 0 < col <= COLUMNS.
 */
Col(col)
int col;
{
	dprint1(9,"Col(%d)\n",col);
	return(col+1);
}


gethostname(hostname,size) /* get name of current host */
int size;
char *hostname;
{
	/** Return the name of the current host machine.  UTS only **/

	/** This routine compliments of Scott McGregor at the HP
	    Corporate Computing Center **/
     
	int uname();
	struct utsname name;

        dprint2(9,"gethostname(%s,%d)\n",hostname,size);
	(void) uname(&name);
	(void) strncpy(hostname,name.nodename,size-1);
	if (strlen(name.nodename) > SLEN)
	  hostname[size] = '\0';
}

int
isa3270()
{
	/** Returns TRUE and sets LINES and COLUMNS to the correct values
	    for an Amdahl (IBM) tube screen, or returns FALSE if on a normal
	    terminal (of course, next to a 3270, ANYTHING is normal!!) **/

	struct tubiocb tubecb;
	
	dprint0(9,"isa3270()\n");
	if (ioctl(TTYIN, TUBGETMOD, &tubecb) == -1){
	  return(FALSE);	/* not a tube! */
	}
	LINES   = tubecb.line_cnt - 2;
	COLUMNS = tubecb.col_cnt;
	if (!check_only && !mail_only) {
		isatube = TRUE;
		return(TRUE);
	}
	 else {
		 isatube = FALSE;
		 return(FALSE);
	}
}

/*
 * ReadCh3270() reads a character from the 3270.
 */
int ReadCh3270()
{
        /** read a character from the display! **/

	register int x;
	char tempstr[80];
        char ch;
	
	dprint0(9,"ReadCh3270()\n");
	if ((_input_buf_ptr > COLUMNS) ||
        (_input_buffer[_input_buf_ptr] == '\0')) {
		WriteScreen3270();
		for (x=0; x < COLUMNS; x++) _input_buffer[x] = '\0';
		panel (noerase, read) {
			#@ LINES+1,1# #INC,_input_buffer,COLUMNS#
		}
		dprint1(9,"ReadCh3270 _input_buffer=%s\n",_input_buffer);
		x=strlen(_input_buffer);
		pfreturn(qskp,tempstr);
		if (!strcmp(tempstr,"\001")) {
			if (strlen(_input_buffer) == 1) {
				tempstr[0]='\0';
			}
			 else {
				tempstr[0]='\n';
				tempstr[1]='\0';
			}
		}
		dprint1(9,"ReadCh3270 pfkey=%s\n",tempstr);
		strcat(_input_buffer,tempstr);
		dprint1(9,"ReadCh3270 _input_buffer+pfkey=%s\n",_input_buffer);
		ch = _input_buffer[0];
		dprint1(9,"ReadCh3270 returns(%c)\n",ch);
		_input_buf_ptr = 1;
		return(ch);
	}
	 else {
		 ch = _input_buffer[_input_buf_ptr];
		 dprint1(9,"ReadCh3270 returns(%c)\n",ch);
		 _input_buf_ptr = _input_buf_ptr + 1;
		 return(ch);
	}
}


/*
 * WriteScreen3270() Loads a screen to the buffer.
 *
 */
WriteScreen3270()
{
	register int x;
	int currcol;
	int currrow;
	int i;
	int state = OFF;
	int prevrow = 1;
	int prevcol = 1;
	int prevptr = 0;
	int clear_state = ON;
	char tempstr[80];
	char copy_screen[66*132];
	
	dprint0(9,"WriteScreen3270()\n");
	prevrow = 1;
	prevcol = 1;
	prevptr = 0;
	state = OFF;
	for (x=0; x < LINES*COLUMNS; x++){
		if ((_internal_screen[x] == '\016')
		&& (state == OFF)) {
			currrow = (x / COLUMNS ) + 1;
			currcol = (x % COLUMNS ) + 1 ;
			i = x - prevptr - 1;
			strncpy(copy_screen, (char *) (_internal_screen+(prevptr)),i);
			panel(erase=clear_state,write,noread) {
				#@ prevrow, prevcol # #ON,copy_screen,i #
			}
			clear_state = OFF;
			state = ON;
		     /* prevrow = currrow; */
		     /* prevcol = currcol; */
			prevrow = currrow + 1;
			prevcol = 0;
			prevptr = x+1;
		}
		else if ((_internal_screen[x] == '\017')
		&& (state == ON)) {
			currrow = (x / COLUMNS ) + 1;
			currcol = (x % COLUMNS ) + 1;
			i = x - prevptr - 1;
			strncpy(copy_screen, (char *) (_internal_screen+(prevptr)),i);
			panel(erase = clear_state,write,noread) {
				#@ prevrow,prevcol # #OH,copy_screen,i #
			}
			clear_state = OFF;
			state = OFF;
		     /* prevrow = currrow; */
		     /* prevcol = currcol; */
			prevrow = currrow + 1;
			prevcol = 0;
		}
	       else if (_internal_screen[x] < ' ') {
			_internal_screen[x] = ' ';
			prevptr = x + 1;
	       }
	}
	/* write remainder of buffer */
	if (state == OFF) {
		currrow = (LINES) + 1 ;
		currcol = (COLUMNS ) + 1;
		i = x - prevptr  ;
		strncpy(copy_screen, (char *) (_internal_screen+(prevptr)),i);
		panel(erase=clear_state,write,noread) {
			#@ prevrow,prevcol # #ON,copy_screen,i #
		}
	}
	else {
		currrow = (LINES ) + 1 ;
		currcol = (COLUMNS ) + 1 ;
		i = x - prevptr ;
		strncpy(copy_screen, (char *) (_internal_screen+(prevptr)),i);
		panel(erase=clear_state,write,noread) {
			#@ prevrow,prevcol # #OH,copy_screen,i #
		}
	}
}


/*
 * Clear3270
 */
Clear3270()
{
	int  i,j;
	
	dprint0(9,"Clear3270()\n");
	j = rowcol2offset(LINES,COLUMNS);
	for (i = 0; i < j; i++) {
		_internal_screen[i] = ' ';
	}
	return(0);
}

/*
 *  WriteChar3270(row,col) writes a character at the row and column.
 */
WriteChar3270(row,col,ch)
int row, col;
char ch;
{
	dprint3(9,"WriteChar3270(%d,%d,%c)\n",row,col,ch);
	_internal_screen[rowcol2offset(row,col)] = ch;
}

/*
 *  WriteLine3270(row,col) writes a line at the row and column.
 */
WriteLine3270(row,col,line)
int row, col;
char *line;
{
	int i, j, k;
	dprint3(9,"WriteLine3270(%d,%d,%s)\n",row,col,line);
        _line = row;
	_col = col;
	k = strlen(line);
	i=rowcol2offset(row,col);
	for (j=0; j<k; i++, j++) {
		if ((line[j] >= ' ')   ||
		   (line[j] == '\016') ||
		   (line[j] == '\017'))
		_internal_screen[i] = line[j];
		else _internal_screen[i] = ' ';
	}
   /*   _line = offset2row(i-1);  calling program updates location */
   /*   _col  = offset2col(i-1); */
	
}


/*
 * ClearEOLN3270() clears the remainder of the current line on a 3270.
 */
ClearEOLN3270()
{
	int i,j ;
	
	dprint0(9,"ClearEOLN3270()\n");
	j = rowcol2offset(_line,COLUMNS);
	for (i=rowcol2offset(_line,_col); i < j; i++) {
		_internal_screen[i] = ' ';
	}
}

/*
 * ClearEOS3270() clears the remainder of the current screen on a 3270.
 */
ClearEOS3270()
{
	int i,j;
	
	dprint0(9,"ClearEOS3270()\n");
	j = rowcol2offset(LINES,COLUMNS);
        for (i = rowcol2offset(_line,_col); i < j; i++) {
		_internal_screen[i] = ' ';
	}
}

