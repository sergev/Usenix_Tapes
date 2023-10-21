#include "dca2troff.h"

struct fnts {
	int gfid;		/* GFID i.e. font number */
	char *dfnt;		/* use this troff font for now */
	char *fntname;		/* font name */
} fnttab[] = {
	1,	"1",	"Advocate",
	2,	"1",	"Delegate",
	3,	"1",	"OCR-B",
	4,	"1",	"Polygo Pica",
	5,	"1",	"Orator",
	6,	"2",	"Light Italic 10",
	7,	"1",	"OCR-M",
	8,	"1",	"Scribe 10",
	9,	"1",	"Large Pica",
	10,	"1",	"Cyrillic 22",
	11,	"1",	"Courier 10",
	12,	"1",	"Prestige Pica",
	13,	"1",	"Artisan 10",
	14,	"1",	"Manifold",
	15,	"1",	"Bookface Academic",
	16,	"1",	"Latin 10 High Speed",
	17,	"1",	"1403 OCR",
	18,	"2",	"Courier Italic 10",
	19,	"1",	"OCR-A",
	20,	"1",	"PICA",
	21,	"1",	"Katakana Light",
	22,	"1",	"Printing & Publishing 12 Number 3",
	23,	"2",	"Light Italic 10 Mod",
	24,	"1",	"Presentor",
	80,	"1",	"Scribe",
	81,	"1",	"Artisan 12",
	82,	"1",	"Auto Elite",
	83,	"1",	"Elite",
	84,	"1",	"Script",
	85,	"1",	"Courier 12",
	86,	"1",	"Prestige Elite",
	87,	"1",	"Letter Gothic",
	88,	"1",	"High Speed Latin 12",
	89,	"1",	"Large Elite",
	90,	"1",	"Dual Gothic",
	91,	"2",	"Light Italic 12",
	92,	"2",	"Courier 12 Italic",
	93,	"1",	"Polygo Elite",
	94,	"1",	"Diplomat",
	95,	"1",	"Adjutant",
	96,	"1",	"Olde World",
	97,	"2",	"Light Italic 12 Mod",
	155,	"2",	"Boldface Italic",
	156,	"1",	"Thesis",
	157,	"1",	"Title",
	158,	"1",	"Modern",
	159,	"1",	"Boldface",
	160,	"1",	"Essay",
	161,	"1",	"Arcadia",
	162,	"2",	"Essay Italic",
	163,	"3",	"Essay Bold",
	165,	"1",	"High Speed Latin PSM",
	221,	"1",	"Prestige 15",
	222,	"1",	"Gothic 15",
	223,	"1",	"Courier 15",
	224,	"1",	"Rotated Data 1 15",
	225,	"1",	"Scribe 15",
	0,	"",	""
};

/* multi byte class d1 */
do_mbd1()
{
/* the "???????" entries were found in some dw3 files but are not
	documented */

    switch (mb_type)
	{
	case 0x8a:			/* ???????? */
	case 0x8e:			/* ???????? */
	    do_flush(mb_count);
	    return;
	case 0x01:			/* SCG   - Set GCGID thru GCID */
	    itemp = get1num();			/* get GCID1 */
	    jtemp = get1num();			/* get GCID2 */
	    mb_count = mb_count -4;
	    sprintf(tline, "\\\" graphic character set request: GCID1=%d, GCID2=%d\n", itemp, jtemp);
	    do_flush(mb_count);
	    return;
	case 0x05:			/* SFG   - Set CFID thru GFID */
	    itemp = get1num();			/* get GFID */
	    jtemp = get1num();			/* get font width */
	    ctemp = get1ch();			/* get font attribute */
	    mb_count = mb_count - 5;
	    for(ltemp=0;fnttab[ltemp].gfid != itemp; ltemp++)
		{
		if(fnttab[ltemp].gfid == 0)
		    {
	    	    sprintf(tline, "\\\" font request: GFID=%d, width=%d, ",itemp, jtemp);
		    }
		}
	    if(fnttab[ltemp].gfid == itemp)
		    {
	    	    sprintf(tline, "\\\" font request: font='%s', width=%d, ",fnttab[ltemp].fntname, jtemp);
		    }
	    outstr(tline);
	    if(ctemp == 1)
		outstr("monospace\n");
	    else
		outstr("proportionally spaced\n");
	    if(fnttab[ltemp].gfid == itemp)
		    {
		    sprintf(tline, "\\f%s", fnttab[ltemp].dfnt);
	    	    outstr(tline);
		    }
	    do_flush(mb_count);
	    return;
	case 0x15:			/* IEG   - Insert Escaped Graphic */
	    if(mb_count >= 3)
		{
		itemp = get1num();		/* get code page */
		ctemp = get1ch();		/* get character code */
		mb_count = mb_count - 3;
		sprintf(tline, "\\\" special character request: code page=%d, character code=x%02x\n", itemp, ctemp);
		outstr(tline);
		}
	    do_flush(mb_count);
	    return;
	default:
	    fprintf(stderr, "unknown mb d1 type (%02x)\n", mb_type);
	    do_flush(mb_count);
	    return;
	}
}
