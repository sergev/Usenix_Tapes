
#include <stdio.h>

#include "tables.h"			/* Mnemonis and opcode tables     */

int byte1, byte2, Hnibble, Lnibble, i, c;

/*
 * Routine to decode an instruction "byte1 byte2" with address "pc".
 *      calls Dumpbyte to dump a byte
 *      calls lookup to look up in the opcode table
 */

decode(pc, byte1, byte2, asm)
int pc, byte1, byte2, asm;
{
	if (!asm)
	{
		printf("    ");

		Dumpbyte( pc >> 8 );		/* Dump address */
		Dumpbyte( pc & 0X0FF );
		printf("     ");

		Dumpbyte(byte1);		/* Dump code */
		printf("  ");
		if (_Byte[byte1])
			Dumpbyte(byte2);
		else printf("  ");
	}

	Hnibble = byte1 >> 4;			/* Extract the higher  */
	Lnibble = byte1 & 0X0F;			/* and lower nibbles   */

	printf("           ");
	lookup();				/* and look it up      */

	if (_Byte[byte1])
		if (_Byte[byte1] == 1) Dumpbyte(byte2);
		else
		{	byte2 += ((pc--) & 0XF00);
			Dump(byte2>>8);
			Dumpbyte(byte2 & 0X0FF);
		}
	return(_Byte[byte1]);
}

lookup()
/*  Lookup: look up the opcode table
 *  input - Lnibble and Hnibble
 *  output - mnemonics are printed
 */
{
	if ( (!(Lnibble & 0X08) && (Lnibble & 0X02)) || (Lnibble == 5) )
		printf("%s",Row[trans1[Lnibble]][Hnibble]);	
						/* REGULAR opcodes */

	else if (Lnibble == 4) {		/* CALL or JUMP */
		if (Hnibble & 0X01) printf("CALL ");
		else printf("JMP ");
		Dump( (Hnibble & 0X0F) >> 1 );
	}

	else if ( (byte1 & 0XFE) == 0X0C0 || (byte1 & 0XFE) == 0X0E0 )
		printf("???");			/* unknown opcodes */

	else if ( ((Hnibble != 0) && (Hnibble != 3) && (Hnibble != 8) &&
		(Hnibble != 9) ) || (Lnibble > 11 ) ) {
		i = 0;
		while ( (c = Col[Hnibble][i++]) != '\0') {
			switch (c) {
			case 'r':
				Dump (Lnibble & 0X07);
				break;
			case '@':
				if (Lnibble < 8) printf("@");
				break;
			case 'p':
				Dump(Lnibble & 0X03);
				break;
			default:
				putc( c, stdout );
				break;
			}
		}
	}

	else printf("%s", ir[ trans2[Hnibble]][trans3[Lnibble]]);
}

Dumpbyte( i )
/* Dumpbyte: dump a byte stored in i		*/
int i;
{
	Dump( i >> 4 );
	Dump( i & 0X0F );
}

Dump(nibble)
/* Dump: dump a nibble (4 bits, a hex digit)	*/
int nibble;
{
	if( (nibble >= 0) && (nibble <= 9)) putchar( nibble + '0');
	else putchar(nibble + 'A' -10);
}
