
#include <stdio.h>

/*	Intel (TM) 8038 family single component u-p machine code disassembler
 *	By: Sheu, Der-Ru; March 12, 1985
 *
 *	Command line options:
 *		-s hex#		sets the starting address (default 0)
 *		-e hex#		sets the ending address (default 2K)
 *		-a		sets output in assembly format (default 0)
 *
 *	Stdin and Stdout are always used as in/output, use redirection for
 *	file i/o.  Input should be in the Intel HEX format.
 *
 */

int pc_start = 0;		/* Command options */
int pc_end = 0x800;
int asm = 0;

main(argc, argv)
int argc;
char *argv[];
{
	int count, New, c, i, pc;
	char addr[6];
	int new_pc, byte1, byte2, two_byte;

/*    -1-        parse command line parameters				*/

	for (i=1; i<argc;)
		switch (argv[i++][1])

		{	case 'a':	
			case 'A':
				asm = 1;
				break;

			case 's':	
			case 'S':
				if (i >= argc) usage();
				pc_start = readhex(argv[i++]);
				break;

			case 'e':
			case 'E':
				if (i >= argc) usage();
				pc_end = readhex(argv[i++]);
				break;

			default: usage();
		}

/*    -2-        now begin disassemblying				*/

	for (pc = 0; pc < pc_start;)
		if (!(get_op(pc, &pc, &byte1))) exit(0);

	if (!(get_op(pc, &pc, &byte1))) exit(0);

	for (; pc <= pc_end;)
		if (!(get_op(pc, &new_pc, &byte2))) exit(0);
		else
		{	if (new_pc == (pc+1))
				two_byte = decode(pc, byte1, byte2, asm);
			else
			{	decode(pc, byte1, 0, asm);
				two_byte = 0;
				printf("\n");
			}
			printf("\n");

			if (two_byte)
			{	if (!(get_op(new_pc, &pc, &byte1))) exit(0); }
			else
			{	pc = new_pc;
				byte1 = byte2;
			}
		}
	printf("\n");
}


readhex(string)
char string[];
{
	int c, i, j;

	for (i=j=0; ; i++)
	{	c = string[i];

		if ((c >= '0') && (c <= '9')) c -= '0';
		else if ((c >= 'A') && (c < 'G')) c -= 'A' + 10;
		else if ((c >= 'a') && (c < 'g')) c -= 'a' + 10;
		else break;

		j <<= 4;
		j += c;
	}

	return(j);
}


usage()
{
	fprintf(stderr, "Usage: dee [-a] [-s starting] [-e ending]\n");
	exit(-1);
}

