
/* utility module:
 *
 * GET_OP: get the next opcode in the Intel Hex format.
 * KEEP: keep addresses.
 * SAVE: save the addresses.
 */

#include <stdio.h>


static int check_sum = 0;
static int power_up = 1;
static int byte_max = 0;              /* Force read new line */
static int byte_count = 1;


get_nibble()
{
	int i;

	i = getchar();
	if ((i >= '0') && (i <= '9')) return (i - '0');
	if ((i >= 'A') && (i <= 'F')) return (i - 'A' + 10);
	if ((i >= 'a') && (i <= 'f')) return (i - 'a' + 10);
	printf("\nUnexpected character found in the input file!!!\n");
	return(0);
}

get_byte()
{	int i;

	i = get_nibble() << 4;
	i += get_nibble();
	check_sum += i;
	return(i);
}


get_pc(pc_)
int *pc_;
{	int i;

	if (!power_up)
	{
		i = get_byte();
		if (!check_sum) fprintf(stderr, "Check sum error at PC = $%4x", *pc_);
		check_sum = 0;
	}
	power_up = 0;

	while (1)
	{
		while ( (i = getchar()) != ':')
			if (i == EOF) return(0);
		byte_count = 1;

		byte_max = get_byte();
		*pc_ = get_byte() << 8;
		*pc_ += get_byte();

		if ( !(i = get_byte()) ) return(1);
        if ( i == 1 ) return(0);

		fprintf(stderr, "Unknown record type found at PC = $%4x", *pc_);
	}
}

get_op(pc, pc_, op_)
int *pc_, *op_;
{

	if (byte_count >= byte_max)
	{	if (!get_pc(pc_)) return(0);  }
	else
	{	*pc_ = ++pc;
		byte_count++;
	}

	*op_ = get_byte();
	return(1);
}


static char buff[400][6];
static int ptr = 0;

keep(addr)
char addr[6];
{
	int i;
	for (i=0; i<6; i++) buff[ptr][i] = addr[i];
	ptr++;
}

save(fp)
FILE *fp;
{
	int i;
	for(i = 0; i <= ptr; i++) fprintf( fp, "%s", buff[i] );
}
