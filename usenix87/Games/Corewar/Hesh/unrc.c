/*
** unrc.c -	disassemble rc binaries
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
**
static	char	rcsid[] = "$Header: unrc.c,v 7.0 85/12/28 14:36:48 ccohesh Dist $";
**
*/

#include <stdio.h>
#include "cw.h"


char	*argv0;

main (argc, argv)
reg	int	argc;
reg	char	**argv;
{
	auto	memword	i;

	argv0 = *argv++, argc--;
	if (argc == 0)
		*argv = RC_OUT;
	else if (argc != 1)
		usage();
	if (!freopen(*argv, "r", stdin))
	{
		perror(*argv);
		exit(1);
	}
	while (fread((char *) &i, sizeof(i), 1, stdin) == 1)
		unrc(&i);
	fclose(stdin);
}

/*
** unrc() -	unassemble the given instruction
*/
unrc (i)
reg	memword	*i;
{
	reg	int	incnt;
	static	int	icnt = 0;

	icnt++;
	switch (i->op)
	{
		when DAT:
		case JMP:
		case RND:
			incnt = 1;
		when MOV:
		case ADD:
		case SUB:
		case JMZ:
		case DJZ:
		case CMP:
		case MUL:
		case DIV:
			incnt = 2;
		otherwise:
			fprintf(stderr, "Inst. %d: bad opcode: %d\n",
				icnt, i->op);
			return;
	}
	switch (i->modb)
	{
		case IMM:
		case REL:
		case IND:
		otherwise:
			fprintf(stderr, "Inst. %d: bad address mode for B\n", icnt);
			return;
	}
	if (incnt == 1)
	{
		if (printit(stdout, i))
			fprintf(stderr, "Inst. %d: printit() problem\n", icnt);
		return;
	}
	switch (i->moda)
	{
		case IMM:
		case REL:
		case IND:
		otherwise:
			fprintf(stderr, "Inst. %d: bad address mode for A\n", icnt);
			return;
	}
	if (printit(stdout, i))
		fprintf(stderr, "Inst. %d: printit() problem\n", icnt);
}

/*
** usage() -	print a usage message and exit
*/
void	usage ()
{
	fprintf(stderr, "Usage: %s file\n", argv0);
	exit(1);
}
