
/* 
    8085/8048 disassembler	Paul Fox, 1982 or so... */
    This disassembler is offered as is, no warranty expressed or implied.
    It is now in the public domain-- think of me as you use it.  :-)

    The code is not beautiful, nor was it ever intended to be.  It 
    does do the job.  I'm not cleaning it up before sending it out 
    because I don't feel like testing it again.  
    
    The idea is that you start with an Intel hex format file, and just 
    dive in and disassemble.  The targets of jumps and calls are 
    automatically turned into symbols of the form "LxNNNN" where NNNN 
    is the hex value of the symbol.  Note that it takes at least two 
    disassemmbly passes to get all the symbols printed as labels, 
    since labels appearing before their reference won't be in the 
    table yet on the first pass.  As you figure out what the code 
    does, you add symbol names and values to the symbol table, and 
    change the names of the "LxNNNN" ones to more meaningful values.  
    Symbols beginning with "DB" and "DE" cause a switch to data dump 
    and opcode dump formats, respectively.  Useful when you discover 
    where all the data was stored in the original code.  

    I'm not sure the 8085 code has been used since I added the 8748 stuff.

*/
#define DIS48	/* define DIS48 or DIS85 for 8048 or 8085 disassembler */

#include <stdio.h>
#define SYMLEN 9
#define NMEMLEN 10
#define SYMTABSIZE 800
#define MEMSIZE 0x2000  /* max size of disassembled program */
#define BYTE 0xff

struct symbol_struct {
    int value;
    char name[SYMLEN];
} symbol[SYMTABSIZE];

struct symbol_struct *sym;

struct opc {
    char nmem[NMEMLEN];
    int operands;
} opcode[] = {

#ifdef DIS48
# include "opc48"
#else
# include "opc85"
#endif

};


int first, last, lastsym;
char byte[MEMSIZE];

main()
{
    char strng1[20], strng2[20], listfile[20];
    char error, filing, opcoding;
    int lowbyte, highbyte, cmnd, addr, const, ans;
    FILE *list_fd;
    struct symbol_struct *getsym();

    error = filing = 0;
    opcoding = 1;
    list_fd = stdout;
    lastsym = -1;

    load_hexfile();
    printf("\n Type 'h' for help...");
    while (cmnd = get_command())
    {
	switch (cmnd)
	{
	    case 'h':
		printf("The available commands are:\n");
		printf("	l - load symbol table\n");
		printf("	s - save symbol table\n");
		printf("	d - disassemble\n");
		printf("	o - toggle disassembly output between stdout and a listfile\n");
		printf("	b - toggle between opcodes and '.byte 0xXX' output\n");
		printf("	r - set range of addresses for disassembly\n");
		printf("	h - this message\n");
		printf("	q - quit\n");
		printf("	v - run vi on a file (useful for editing symbol table)\n");
		break;
	    case 'l':
		load_symtab();
		printf(" %d symbols loaded.\n", lastsym);
		error = 0;
		break;

	    case 'r':
		get_range();
		break;

	    case 'o':
		if (filing) 
		{
		    printf(" Listfile closed.\n");
		    fclose(list_fd);
		}
		filing = !filing;
		if (filing)
		{
		    list_fd = NULL;
		    while (list_fd == NULL)
		    {
			printf("List file name: ");
			scanf(" %s ", listfile);
			list_fd = fopen(listfile, "w");
		    }
		    printf("Listfile %s opened.",listfile);
		}
		else
		    list_fd = stdout;
		break;

	    case 'b':
		if (opcoding)
		{
		    printf("Will now default to data bytes...\n");
		    opcoding = 0;
		}
		else
		{
		    printf("Will now default to opcode nmemonics...\n");
		    opcoding = 1;
		}
		break;

	    case 'd':
		for (addr = first; addr <= last && !error ; addr++)
		{
		    fprintf(list_fd, "\n");
		    sym = getsym(addr);
		    if (sym == NULL || sym->name[0] == '0')
		    {
			fprintf(list_fd, "\t");
		    }
		    else
		    {
			if (sym->name[0] == 'D' && sym->name[1] == 'B')
			    opcoding = 0;
			else if (sym->name[0] == 'D' && sym->name[1] == 'E')
			    opcoding = 1;
			fprintf(list_fd,"%s:\t",sym->name);
		    }
		    if (opcoding)
		    {
			fprintf(list_fd, "%s", opcode[byte[addr] & BYTE].nmem);
			switch( opcode[byte[addr] & BYTE].operands )
			{
			    case 0:
				break;

			    case 1:
#ifdef DIS48
				if ((byte[addr] & 0x0f) == 0x04)
				{	/* then its a "call" or direct "jmp" */
				    highbyte = (byte[addr] & 0xe0) << 3;
				    lowbyte = byte[++addr] & BYTE;
				    const = highbyte + lowbyte;
				    if (sym = getsym(const))
					fprintf(list_fd, "	%s", sym->name);
				    else
				    {
					fprintf(list_fd, "	0x%x", const);
					error = !putsym(const);
				    }
				}
				else if ((byte[addr] & 0x0f) == 0x06 ||
					(byte[addr] & 0x1f) == 0x12)
				{	/* it's a page relative jump */
				    highbyte = (addr & ~0xff); /* this page */
				    lowbyte = byte[++addr] & BYTE;
				    const = highbyte + lowbyte;
				    if (sym = getsym(const))
					fprintf(list_fd, "	%s", sym->name);
				    else
				    {
					fprintf(list_fd, "	0x%x", const);
					error = !putsym(const);
				    }
				    
				}
				else
				{
				    fprintf(list_fd, " 0x%x", 
						byte[ ++addr] & BYTE);
				}
#else
 				fprintf(list_fd, " 0x%x", byte[ ++addr] & BYTE);
#endif
				break;
			    
#ifdef DIS85
			    case 2:
				lowbyte = byte[++addr] & BYTE;
				highbyte = byte[++addr] & BYTE;
				const = highbyte * 256 + lowbyte;
				if (sym = getsym(const))
				    fprintf(list_fd, " %s", sym->name);
				else
				{
				    fprintf(list_fd, " 0x%x", const);
				    error = !putsym(const);
				}
				break;
#endif

			    default:
				fprintf(stderr,"Opcode table error!\n");
			}
		    }
		    else
		    {
			fprintf(list_fd,".byte 0x%x",byte[addr] & BYTE);
		    }
		}
		fprintf(list_fd,"\n");
		break;

	    case 's':
		save_symtab();
		printf(" %d symbols saved.\n", lastsym);
		break;

	    case 'v':
		printf("File to edit (with vi): ");
		scanf( "%s", strng1);
		strcpy(strng2,"vi ");
		strcat(strng2,strng1);
		system(strng2);
		break;

	    case 'q':
		printf("Are you sure you want to quit? ");
		ans = getchar();
		if ( ans == 'y' ) 
		    return;
		break;

	}
	fflush(stdout);
    }
}


load_hexfile()
{
    char hexfile[20];
    FILE *hex_fd;
    int line;		/* Current input line number */
    int bytes;		/* Number of bytes to be read on a line */
    int sum;		/* Checksum of bytes on a line */
    int addr;		/* Address for input data to be loaded into */
    int value;		/* Value of data to be loaded */
    int mode;		/* Mode = 1 means start address is given */
    int i;		/* Scratch variables */
    char c;		/* Character scratch variables */

    hex_fd = NULL;
    while (hex_fd == NULL)
    {
	printf(" Hex file for disassembly: ");
	scanf("%s", hexfile);
	hex_fd = fopen( hexfile, "r" );
	if (hex_fd == NULL)
	    printf("	file %s not found...\n",hexfile);
    }

/* Mike's code. */

    printf("Reading from file %s\n",hexfile);

				/* 
				 * Read data from file.  Find number of bytes
				 * to read from a line, read them, and xor
				 * to get the checksum.  Continue until a
				 * line with a 0 byte count is read.  If the
				 * mode on the last line is 1, read the
				 * start address and put it into the PC.
				 */
    line = 0;
    bytes = 1;
    while (bytes > 0)
    {
	line++;			
	if (fscanf(hex_fd,":%2x%4x%2x",&bytes,&addr,&mode) <= 0) 
	{
	    printf("Improper program format in line %d\n",line);
	    fclose(hex_fd);
	    if (line == 1) first = addr;
	    return;
	}
	if (addr >= (MEMSIZE - bytes))
	{
	    printf("Address %x outside allocated memory.\n", addr);
	    return;
	}
	if (line == 1) first = addr;
	sum = bytes + ((addr/256) & BYTE) + (addr & BYTE) + mode;
	for (i = addr; i < addr + bytes; i++)
	{
	    if (fscanf(hex_fd,"%2x",&value) <= 0)
	    {
		printf("Improper program format in line %d at byte %d\n",line,
							i - addr);
		fclose(hex_fd);
		return;
	    }
	    sum += value;
	    byte[i] = value;
	    last = i;
	}
	if (fscanf(hex_fd,"%2x",&value) <= 0)
	{
	    printf("Improper program format in line %d at byte %d\n",line,
							      i - addr);
	    fclose(hex_fd);
	    return;
	}
	sum += value;
	if (sum & BYTE) 
	{
	    printf("Checksum error in line %d\n",line);
	    fclose(hex_fd);
	    return;
	}
	c = getc(hex_fd);
	while (c != '\n') c = getc(hex_fd);
    }
}

get_command()
{
    char c;
    do {
	putchar('-');
	c = getchar();
    } while (c== '\n');
    while (getchar() != '\n') ;
    return(c);
}

load_symtab()
{
    char symfile[20], symname[20];
    int rtncode, intval;
    FILE *sym_fd;

    sym_fd = NULL;
    while (sym_fd == NULL)
    {
	printf(" Symbol table from file: ");
	scanf("%s", symfile);
	sym_fd = fopen( symfile, "r" );
    }

    lastsym = -1;
    while (( rtncode = fscanf(sym_fd, "0x%x %s\n", &intval, symname)) > 0 )
    {
	lastsym++;
	symbol[lastsym].value = intval;
	strcpy(symbol[lastsym].name, symname);
    }
    if (rtncode != EOF)
	printf("!!! Error reading symbol table from %s", symfile);
    fclose(sym_fd);
}

save_symtab()
{
    char symfile[20];
    int i;
    FILE *sym_fd;

    sym_fd = NULL;
    while (sym_fd == NULL)
    {
	printf(" Symbol table to file: ");
	scanf("%s", symfile);
	sym_fd = fopen( symfile, "w" );
    }
    
    i = 0;
    while ( i <= lastsym)
    {
	fprintf( sym_fd, "0x%x\t%s\n", symbol[i].value, symbol[i].name);
	i++ ;
    }
    fclose(sym_fd);
}

get_range()
{
    printf(" Enter start address (hex): ");
    scanf("%x", &first);
    printf(" Enter finish address (hex): ");
    scanf("%x", &last);
}

struct symbol_struct
*getsym(val)
    int val;
{
    int low, high, mid;

    low = 0;
    high = lastsym;

    while (low <= high)
    {
	mid = (low + high)/2;
	if (val > symbol[mid].value )
	    low = mid + 1;
	else if (val < symbol[mid].value )
	    high = mid - 1;
	else return( &symbol[mid] );
    }

    return(NULL);
}

putsym(val)
    int val;
{
    int i, j;


    for (j = 0; j <= lastsym; j++)
	if( symbol[j].value > val)
	    break;


    if ( ++lastsym == SYMTABSIZE-1 )
    {
	printf("!!! Too many symbols !!!");
	return(0);
    }

    for (i = lastsym; i >= j; i--)
	symbol[i+1] = symbol[i];
    symbol[i+1].value = val;
    sprintf(symbol[i+1].name, "Lx%x", val);
    return(1);
}


