#include "ops.h"

/*
 * create dispatch table for interpreter from EM1-tables
 */

struct mnems {
	char	m_name[3];	/* ascii name */
	char	m_nminis;	/* # of operands that fit in the opcode */
	char	m_mbase;	/* lowest of these */
	char	m_nshorties;	/* # of operand-groups with hibyte in opcode */
	char	m_sbase;	/* lowest of these */
	char	m_obase;	/* opcode of long instruction */
	int	m_flags;	/* see flags */
};

/* definition of m_flags */
#define MN0		0	/* pseudo flag to indicate normal case */
#define MNS		01	/* shorties escaped */
#define MNE		MNS	/* instruction without operands escaped */
#define MNL		02	/* long instruction escaped */
#define MNXYZ		014	/* determines legal values of operand */
#define		MNX	004	/* only even operands allowed */
#define		MNY	010	/* only 1,2,4,6 etc.. allowed */
#define		MNZ	014	/* no operands */
#define MNABMCP		0160	/* a variety of instructions exists */
#define		MNA	0020	/* alternate context mark */
#define		MNB	0040	/* branch instruction */
#define		MNM	0060	/* normal context mark */
#define		MNC	0100	/* call instruction */
#define		MNP	0120	/* pseudo instruction */
#define	MNO		0200	/* minis start at 1 not at 0 */

#define ESC	0


struct dispatch {
	char d_ins;
	char d_typ;
	int  d_num;
} disp1[256],disp2[256];

main() {

	fill_tables();
	print_tables();
}

fill_tables() {
	register struct mnems *m;
	register struct dispatch *d;
	register i;
	extern struct mnems mnemon[];

	for(m= &mnemon[1];m->m_name[0];m++) {
		if ((m->m_flags&MNABMCP)==MNP)
			continue;
		i = m - mnemon;
		if (i==op_lab || i==op_nul)
			continue;
		if ((m->m_flags&MNXYZ)!=MNZ) {
			if (m->m_nminis && (m->m_flags&MNABMCP)!=MNC)
				minis(m-mnemon,4,m->m_mbase,m->m_nminis,disp1);
			if (m->m_nshorties)
				minis(m-mnemon,3,m->m_sbase,m->m_nshorties,
					m->m_flags&MNS ? disp2 : disp1);
			d = m->m_flags&MNL ? disp2 : disp1;
			d =+ (m->m_obase&0377);
			enter(d,m-mnemon,2,0);
		} else {
			d = m->m_flags&MNE ? disp2 : disp1;
			d =+ (m->m_obase&0377);
			enter(d,m-mnemon,1,0);
		}
	}
	enter(&disp1[ESC],0,5,0);
}

minis(ins,typ,base,count,dtab) struct dispatch *dtab; {
	register struct dispatch *d;
	register i,j;
	extern struct mnems mnemon[];

	d = &dtab[base&0377];
	i = count&0377;
	j = 0;
	if ( typ==4 && (mnemon[ins&0377].m_flags&MNO))
		j = 1;
	do
		enter(d++,ins,typ,j++);
	while (--i);
}

enter(dtab,ins,typ,num) {
	register struct dispatch *d;
	extern struct mnems mnemon[];

	d = dtab;
	if (d->d_typ)
		printf("COLLISION: %3.3s(%d,%d) WITH %3.3s(%d,%d)\n",
			mnemon[ins&0377].m_name,typ,num,
			mnemon[d->d_ins&0377].m_name,d->d_typ,d->d_num);
	d->d_ins = ins;
	d->d_typ = typ;
	d->d_num = num;
}

print_tables() {

	print(disp1);
	print(disp2);
}

print(table) struct dispatch *table; {
	register struct dispatch *d;
	register i;
	extern struct mnems mnemon[];

	for(i=0;i<256;i++) {
		d = &table[i];
		switch(d->d_typ) {
		default:	printf("?????");break;
		case 0:	printf("illins");break;
		case 1:
		case 2:	printf("%3.3s.%c",mnemon[d->d_ins&0377].m_name,
				d->d_typ == 1 ? 'z' : 'l');
			break;
		case 3:
		case 4:	printf("%3.3s.%s%d",mnemon[d->d_ins&0377].m_name,
				d->d_typ == 3 ? "s" : "", d->d_num);
			break;
		case 5: printf("escape"); break;
		}
		if ( i%8 == 7) printf("\n");
		else printf(";\t");
	}
	printf("\n\n\n\n\n");
}
