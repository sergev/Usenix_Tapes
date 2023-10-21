#include <ctype.h>
#include "dis.h"

dumpitout()
{
	int i;

	for(i = 0; i<0x10000;) {
		if (f[i] & LOADED) {

			if (f[i] & SREF && f[i] & ISOP)
				printf("\n\n\n");

			printf("%04x  ",i);
			print_bytes(i);
			print_label(i);
			if (f[i] & ISOP)
				i += print_inst(i);
			else
				i += print_data(i);
			printf("\n");

		} else {
			i++;
		}
	}

	print_refs();
}

pchar(c)
int c;
{
	if (isascii(c) && isprint(c))
		return(c);
	return('.');
}

char *
lname(i)
int i;
{
	static char buf[20];
	char t;

	if (f[i] & NAMED) 
		return(get_name(i));
	if ((i > 0) && ((f[i-1] & (NAMED | DREF)) == (NAMED | DREF))) {
		strcpy(buf, get_name(i-1));
		strcat(buf, "+1");
		return (buf);
	}
	if (f[i] & SREF)
		t = 'S';
	else if (f[i] & JREF)
		t = 'L';
	else if (f[i] & DREF)
		t = 'D';
	else 
		t = 'X';
	
	sprintf(buf, "%c%x", t, i);
	return (buf);
}

print_label(i)
{
	if (f[i] & (NAMED | JREF | SREF | DREF)) 
		printf("%-10s", lname(i));
	else
		printf("%10s"," ");
}

print_bytes(addr)
int addr;
{
	register struct info *ip; 

	if ((f[addr] & ISOP) == 0) {
		printf("           ");
		return;
	}

	ip = &optbl[getbyte(addr)];

	switch (ip->nb) {
		case 1:
			printf("%02x         ", getbyte(addr));
			break;
		case 2:
			printf("%02x %02x      ", getbyte(addr), getbyte(addr+1));
			break;
		case 3:
			printf("%02x %02x %02x   ", getbyte(addr), getbyte(addr+1), getbyte(addr+2));
			break;
	}
}
		

print_inst(addr)
int addr;
{
	int opcode;
	register struct info *ip; 
	int operand;
	int istart;

	istart = addr;
	opcode = getbyte(addr);
	ip = &optbl[opcode];

	printf("%s  ", ip->opn);

	addr++;

	switch(ip->nb) {
		case 1:
			break;
		case 2:
			operand = getbyte(addr);
			break;
		case 3:
			operand = getword(addr);
			break;
	}

	if (ip->flag & REL) {
		if (operand > 127) 
			operand = (~0xff | operand);
		operand = operand + ip->nb + addr - 1;
	}

	switch (ip->flag & ADRMASK) {
		case IMM:
			printf("#$%02x                        * %d %c", operand, operand, pchar(operand));
			break;
		case ACC:
		case IMP:
			break;
		case REL:
		case ABS:
		case ZPG:
			printf("%s ", lname(operand));
			break;
		case IND:
			printf("(%s) ", lname(operand));
			break;
		case ABX:
		case ZPX:
			printf("%s,X ", lname(operand));
			break;
		case ABY:
		case ZPY:
			printf("%s,Y ", lname(operand));
			break;
		case INX:
			printf("(%s,X) ", lname(operand));
			break;
		case INY:
			printf("(%s),Y", lname(operand));
			break;
		default:
			break;
	}

	return(ip->nb);

}

print_data(i)
{
	int count;
	int j;
	int start;

	start = i;
	printf(".DB  %02x ", getbyte(i));
	count = 1;
	i++;

	for (j = 1; j < 8; j++) {
		if (f[i] & (JREF | SREF | DREF) || ((f[i] & LOADED) == 0)) 
			break;
		else
			printf("%02x ", getbyte(i));
		i++;
		count++;
	}
	for (j = count; j < 8; j++)
		printf("   ");

	printf("    * ");

	for (j = start; j < i ; j++) 
			printf("%c", pchar(getbyte(j)));

	return (count);
}

print_refs()
{
	char tname[50];
	char cmd[200];
	FILE *fp;
	register struct ref_chain *rp;
	register int i;

	sprintf(tname, "dis.%d", getpid());
	sprintf(cmd, "sort %s; rm %s", tname, tname);

	fp = fopen(tname, "w");
	if (!fp) 
		crash("Cant open %s/n", tname);

	for (i = 0; i<0x10000; i++) {
		if(f[i] & (JREF|SREF|DREF)) {
			rp = get_ref(i);
			if (!rp) {
				fprintf(stderr, "No ref %d\n", i);
				break;
			}

			fprintf(fp, "%-8s  %04x   ", lname(i), i);
			while (rp) {
				fprintf(fp, "%04x ", rp->who);
				rp = rp->next;
			}
			fprintf(fp, "\n");
		}

	}

	fclose(fp);

	printf("\n\n\n\n\nCross References\n\n");
	printf("%-8s  Value  References\n", "Symbol");
	fflush (stdout);

	system(cmd);
}
