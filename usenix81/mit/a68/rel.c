#include "mical.h"
#include "/usr/nu/include/b.out.h"

/*  Handle output file processing for b.out files */

FILE *tout;		/* text portion of output file */
FILE *dout;		/* data portion of output file */
FILE *rtout;		/* text relocation commands */
FILE *rdout;		/* data relocation commands */

long rtsize;		/* size of text relocation area */
long rdsize;		/* size of data relocation area */

char rname[STR_MAX];	/* name of file for relocation commands */

struct bhdr filhdr;	/* header for b.out files, contains sizes */

/* Initialize files for output and write out the header */

Rel_Header()
{
	long Sym_Write();

	if ((tout = fopen(Rel_name, "w")) == NULL ||
		(dout = fopen(Rel_name, "a")) == NULL)
		Sys_Error("open on output file %s failed", Rel_name);

	Concat(rname, Source_name, ".tmpr");
	if ((rtout = fopen(rname, "w")) == NULL
	 || (rdout = fopen(rname, "a")) == NULL)
		Sys_Error("open on output file %s failed", rname);
	filhdr.fmagic = FMAGIC;
	filhdr.tsize = tsize;
	filhdr.dsize = dsize;
	filhdr.bsize = bsize;
	fseek(tout, (long)(SYMPOS), 0);
	filhdr.ssize = Sym_Write(tout);
	filhdr.rtsize = rtsize;
	filhdr.rdsize = rdsize;
	filhdr.entry = 0;

	fseek(tout, 0L, 0);
	fwrite(&filhdr, sizeof(filhdr), 1, tout);

	fseek(tout, (long)(TEXTPOS), 0);	/* seek to start of text */
	fseek(dout, (long)(DATAPOS), 0);	
	fseek(rdout, rtsize, 0);
	rtsize = 0;
	rdsize = 0;
}

/*
 * Fix_Rel -	Fix up the object file
 *	For .b files, we have to
 *	1)	append the relocation segments
 *	2)	fix up the rtsize and rdsize in the header
 *	3)	delete the temporary file for relocation commands
 */
Fix_Rel()
{
	long ortsize;
	long i;
	register FILE *fin, *fout;

	ortsize = filhdr.rtsize;
	filhdr.rtsize = rtsize;
	filhdr.rdsize = rdsize;
	fclose(rtout);
	fclose(rdout);
	if ((fin = fopen(rname, "r")) == NULL)
		Sys_Error("cannot reopen relocation file %s", rname);

	fout = tout;

	/* first write text relocation commands */

	fseek(fout, (long)(RTEXTPOS), 0);
	for (i=0; i<rtsize; i++)
		putc(getc(fin), fout);

	/* seek to start of data segment relocation commands */

	fseek(fin, ortsize, 0);
	for (i=0; i<rdsize; i++)
		putc(getc(fin), fout);

	/* now re-write header */

	fseek(fout, 0, 0);
	fwrite(&filhdr, sizeof(filhdr), 1, fout);
	fclose(fin);
	unlink(rname);
}

/* rel_val -	Puts value of operand into next bytes of Code
 * updating Code_length. Put_Rel is called to handle possible relocation.
 * If size=L a longword is stored, otherwise a word is stored 
 */
rel_val(opnd,size)
register struct oper *opnd;
{	register int i;
	register struct sym_bkt *sp;
	long val;
	char *CCode;			/* char version of this */

	i = Code_length>>1;	/* get index into WCode */
	if (sp = opnd->sym_o)
		Put_Rel(opnd, size, Dot + Code_length);
	val = opnd->value_o;
	switch(size)
	{
	case L:
		WCode[i++] = val>>16;
		Code_length += 2;
	case W:
		WCode[i] = val;
		Code_length += 2;
		break;
	case B:
		CCode = (char *)WCode;
		CCode[Code_length++] = val;
	}
 }

/* Version of Put_Text which puts whole words, thus enforcing the mapping
 * of bytes to words.
 */

#ifdef mc68000
Put_Words(code,nbytes)
  char *code;
  {	if (nbytes & 1) Sys_Error("Put_Words given odd nbytes=%d",nbytes);
	Put_Text(code,nbytes);
}
#endif

#ifndef mc68000
Put_Words(code,nbytes)
register char *code;
{	register char *cc, ch;
	register int i;
	char tcode[100];

	cc = tcode;
	for (i=0; i<nbytes; i++) tcode[i] = code[i];
	i = nbytes>>1;
	if (nbytes & 1) Sys_Error("Put_Words given odd nbytes=%d\n",nbytes);
	while (i--) { ch = *cc; *cc = cc[1]; *++cc = ch; cc++; }
	Put_Text(tcode,nbytes);
}
#endif

/* Put_Text -	Write out text to proper portion of file */

Put_Text(code,length)
 register char *code;
 {	if (Pass != 2) return;
	if (Cur_csect == Text_csect) fwrite(code, length, 1, tout);
	else if (Cur_csect == Data_csect) fwrite(code, length, 1, dout);
	else return;	/* ignore if bss segment */
 }

/* set up relocation word for operand:
 *  opnd	pointer to operand structure
 *  size	0 = byte, 1 = word, 2 = long/address
 *  offset	offset into WCode & WReloc array
 */

Put_Rel(opnd,size,offset)
struct oper *opnd;
int size;
long offset;
{
	struct reloc r;
	if (opnd->sym_o == 0) return;	/* no relocation */
	if (Cur_csect == Text_csect)
		rtsize += rel_cmd(&r, opnd, size, offset, rtout);
	else if (Cur_csect == Data_csect)
		rdsize += rel_cmd(&r, opnd, size, offset - tsize, rdout);
	else return;	/* just ignore if bss segment */
}


/* rel_cmd -	Generate a relocation command and output */

rel_cmd(rp, opnd, size, offset, file)
register struct reloc *rp;
struct oper *opnd;
int size;
long offset;
FILE *file;
{
	int csid;			/* reloc csect identifier */
	register struct csect *csp;	/* pointer to csect of sym */
	register struct sym_bkt *sp;	/* pointer to symbol */

	sp = opnd->sym_o;
	csp = sp->csect_s;
	if (Pass == 2) {
		rp->rsymbol = 0;
		if (!(sp->attr_s & S_DEF)
		 && (sp->attr_s & S_EXT)) {
			rp->rsegment = REXT;
			rp->rsymbol = sp->id_s;
		}
		else if (csp == Text_csect) rp->rsegment = RTEXT;
		else if (csp == Data_csect) rp->rsegment = RDATA;
		else if (csp == Bss_csect) rp->rsegment = RBSS;
		else Prog_Error(E_RELOCATE);
		rp->rpos = offset;
		rp->rsize = size;
		rp->rdisp = 0;
		fwrite(rp, sizeof *rp, 1, file);
	}
	return(sizeof *rp);
}
