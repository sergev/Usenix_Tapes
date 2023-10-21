 /* Copyright, 1986 by Eric Green
    All Rights Reserved, with following exceptions:

   1) Right of duplication: Right of duplication granted to all, under
the condition that 
      a) source code to this file must be provided, and cannot be
packaged seperately from the binary code
      b) this copyright notice is left intact
      c) right of duplication cannot be denied to any who recieve this file.

   2) Right of derivative works: I hereby grant permission for all to
freely modify this work. However, note that all derivative works
become property of Eric Green under copyright law, and thereby must
abide by the conditions of this copyright.

*/

#include <stdio.h>
#include "tables.h"

#ifndef lint
static char *copyright_notice = "Copyright 1986 by Eric Green";
#endif

/* 
  program: dsm.c
  purpose: disassemble a 6502 machine code file, generating C-128-specific
    comments (although that is alterable by changing the .labels file or
    for zero page, changing tables.h).
  Author: Eric Green ( {akgua,ut-sally}!usl!{elg,usl-pc!jpdres10} )
  program history: 
   Version 1.0:
   This is the second "C" program I've ever written (the first was a "grep").
   It was programmed in its entirety on July 25, 1986, in one massive
   hack attack. The .labels file, however, wasn't complete -- I can't type
   that fast! Thus, if you think it looks sloppy, it probably is.

   July 26 - added in to print hash table statistics (I'm paranoid),
             print labels in front of their addresses, etc.,
	     print out when we're in various buffers

   July 28 - add in to print zero-page stuff for 3-byte instructions, too.

*/


static  int pc;
static  int temp_mode;
static  int temp_length;
static  int op_lobyte;
static  int op_hibyte;
static  int op;
static  int op_address;


/* note: if you ever expand this, it must be a large prime number */
#define MAX_HASH_SIZE 2003

static int hashtab[MAX_HASH_SIZE];	    /* addresses to hash into */
static char *hashstring[MAX_HASH_SIZE];	    /* strings describing those
					       addresses */

/* hash table statistics -- I'm curious to know how efficient my algorithm
   is. */
static int hash_calls = 0;                /* total calls to hash function */
static int total_hashes = 0;              /* calls + collisions */

main()
{
  /* We shouldn't have any arguments -- the beginning address is the first
     two bytes of the file, we just read from std. input, write to std.
     output. */
  
  int index;

  init_hash();				    /* init the hash table */
  read_hash();				    /* fill it from ".hash" */

  pc =  getchar();
  pc = pc+getchar()*256;
    
  while ( (op=getchar()) != EOF)
    {
      index = hash (pc);
      if (hashtab[index] != -1)
	printf("         %s",hashstring[index]);
      else
	check_page(pc>>8);
      temp_mode = mode[op];
      temp_length = length[temp_mode];
      if (temp_length == 3)
	{
	  op_lobyte = getchar();
	  op_hibyte = getchar();
	  op_address = op_hibyte*256+op_lobyte;
	  printf ("%04x:%02x %02x %02x  %s %s%04x%s ;",pc,op,op_lobyte,op_hibyte,opcode[op],before[temp_mode],op_address,after[temp_mode]);
	}
      else if (temp_length == 2)
	{
	  op_lobyte = getchar();
	  op_address = op_lobyte;
	  if (temp_mode == 9)
	    { do_rel(); }
	  else
	    printf ("%04x:%02x %02x     %s %s%02x%s ;",pc,op,op_lobyte,opcode[op],before[temp_mode],op_lobyte,after[temp_mode]);
	}
      else
	printf ("%04x:%02x        %s %s%s ;",pc,op,opcode[op],before[temp_mode],after[temp_mode]);
      print_chars();
      if (temp_length == 2)
	{
	  if ( (temp_mode != 2) && (temp_mode != 13) && (temp_mode != 9) )
	    printf (" %s\n",zero_page[op_lobyte]);
	  else
	    putchar ('\n');
	}
      else if (temp_length == 3)
	hash_thingy(op_address);
      else
	putchar ('\n');
      if ( (strcmp(opcode[op],"jmp")==0) || (strcmp(opcode[op],"rts")==0))
	puts("-----------------------------------------\n");
      pc = pc + temp_length;
    }
  printf("\n Hash calls=%d, total hashes=%d.\n",hash_calls,total_hashes);
}

print_chars()
{
  int ch;
  int count;

  ch = convert(op);
  putchar(ch);
  if (temp_length > 1)
    {
      ch = convert(op_lobyte);
      putchar(ch);
    }
  else
    putchar(' ');
  if (temp_length > 2)
    {
      ch = convert(op_hibyte);
      putchar(ch);
    }
  else
    putchar(' ');
  putchar(' ');
}

int convert (ch)
     int ch;
{
  int new;
  new = '.';
  if ( (ch > 31) && (ch < 65))
    { new=ch;}
  else if ( (ch > 64) && (ch < 91))
    { new=ch+32;}
  else if ( (ch > 90) && (ch < 96))
    { new = ch;}
  else if ( (ch>96) && (ch<123))
    { new = ch-32;}
  else if ( (ch>192) && (ch<219))
    new = ch-96;
  return(new);
}

do_rel()
{
  if (op_lobyte < 128)
    {
      op_address = op_lobyte+pc+2;
    }
  else
    {
      op_address = pc-(256-op_lobyte)+2;
    }

  printf ("%04x:%02x %02x     %s %s%04x%s ;",pc,op,op_lobyte,opcode[op],before[temp_mode],op_address,after[temp_mode]);
}

init_hash()				    /* init hashtab to -1 */
{
  int counter;
  
  for (counter=0;counter<MAX_HASH_SIZE;counter++)
    hashtab[counter]= (-1);
}

int hash(address)
     int address;
{
  int guard;
  int index;

  hash_calls++;
  guard = 0;
  index = (address*3) % MAX_HASH_SIZE;
  while ( (hashtab[index] != address) && (hashtab[index] != -1) &&
	  (++guard < MAX_HASH_SIZE*2 ))
    {
      index = (index*3) % MAX_HASH_SIZE;
    }
  if (guard == MAX_HASH_SIZE*2)
    {
      printf("Error! HAsh_thingy overflow!\n");
      exit(2);
    }
  total_hashes = total_hashes + 1 + guard;
  return(index);
}

hash_thingy(address)
     int address;
{
  int index;

  index = hash(address);
  if (hashtab[index]==-1)
    {
      if (check_page(op_hibyte) == 0)
	putchar('\n');
    }
  else
    printf(" %s",hashstring[index]);
}


int check_page(page)
{
  int returnflag = 1;

  switch (page)
    {
    case 0:
      printf(" %s\n",zero_page[op_lobyte]); break;
    case 1:
      puts(" 8510 CPU stack"); break;
    case 4:
    case 5:
    case 6:
    case 7:
      puts(" VICSCN -- VIC 40 column text screen"); break;
    case 8:
    case 9:
      puts(" BASIC run-time stack"); break;
    case 0x0c:
      puts(" rs-232 input buffer"); break;
    case 0x0d:
      puts(" rs-232 output buffer"); break;
    case 0x10:
      puts(" Programmable function key data"); break;
    case 0xd8:
    case 0xd9:
    case 0xda:
    case 0xdb:
      puts(" VIC color matrix (nybbles)"); break;
    default:
      returnflag = 0;
    }
  return(returnflag);
}


static char input_line[256];		    /* line we're reading in to
					       put into the hash table */

read_hash()
{
  int index;
  FILE *input_file;
  int address;
  char number[5];
  char *rest;

  input_file = fopen (".labels","r");
  if (input_file == NULL)
    return(0);
  while ( fgets(input_line,255,input_file) != NULL)
    {
      if (input_line[0] != '#')
	{
	  input_line[4]=0;
	  sscanf (input_line,"%x",&address);
	  index = hash(address);
	  hashtab[index]=address;
	  rest = &input_line[5];
  /*      printf ("%04x %d %s\n",address,index,rest); */
	  hashstring[index]=malloc(strlen(rest)+1);
	  strcpy (hashstring[index],rest);
	}
    }
  printf("\nHash calls=%d, total hashes = %d\n\n",hash_calls,total_hashes);
}    

