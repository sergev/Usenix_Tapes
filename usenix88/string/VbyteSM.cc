/* Copyright (c) 1987 Peter A. Buhr */
	
#include <stream.h>
#include "VbyteSM.h"

extern VbyteHeap HeapArea;

/* Add a new HandleNode node n after the current HandleNode node.  */

void HandleNode::AddThisAfter(HandleNode *n) {
	// This routine is called from HandleNode which is a constructor, hence it cannot have I/O in it
	flink = n->flink;
	blink = n;
	n->flink->blink = this;
	n->flink = this;
} // AddThisAfter

/* Delete the current HandleNode node.  */

void HandleNode::DeleteNode() {
	//cerr << "enter:DeleteNode, this:" << (int)this << "\n";
	flink->blink = blink;
	blink->flink = flink;
	//cerr << "exit:DeleteNode\n";
} //  DeleteNode

/* Move an existing HandleNode node n somewhere after the current
HandleNode node so that it is in ascending order by the address in the
byte string area */

void HandleNode::OrderedMoveThisAfter(HandleNode *h) {
	//cerr << "enter:OrderedMoveThisAfter, this:" << (int)this << " h:" << (int)h << "\n";
	//cerr << "HandleList:\n"; cerr << "node:" << (int)&HeapArea.Header << " lnth:" << HeapArea.Header.lnth << " s:" << (int)HeapArea.Header.s << " flink:" << (int)HeapArea.Header.flink << " blink:" << (int)HeapArea.Header.blink << "\n"; {for (HandleNode *i = HeapArea.Header.flink; i != &HeapArea.Header; i = i->flink) cerr << "node:" << (int)i << " lnth:" << i->lnth << " s:" << (int)i->s << " flink:" << (int)i->flink << " blink:" << (int)i->blink << "\n";}
	for (HandleNode *i = h->flink; i->s != 0 && (int)s > (int)i->s; i = i->flink); // find the position for this node after h
	MoveThisAfter(i->blink);
	//cerr << "exit:OrderedMoveThisAfter\n";
} // OrderedMoveThisAfter

/* Move an existing HandleNode node n somewhere before the current
HandleNode node so that it is in ascending order by the address in the
byte string area */

void HandleNode::OrderedMoveThisBefore(HandleNode *h) {
	//cerr << "enter:OrderedMoveThisBefore, this:" << (int)this << " h:" << (int)h << "\n";
	//cerr << "HandleList:\n"; cerr << "node:" << (int)&HeapArea.Header << " lnth:" << HeapArea.Header.lnth << " s:" << (int)HeapArea.Header.s << " flink:" << (int)HeapArea.Header.flink << " blink:" << (int)HeapArea.Header.blink << "\n"; {for (HandleNode *i = HeapArea.Header.flink; i != &HeapArea.Header; i = i->flink) cerr << "node:" << (int)i << " lnth:" << i->lnth << " s:" << (int)i->s << " flink:" << (int)i->flink << " blink:" << (int)i->blink << "\n";}
	for (HandleNode *i = h->blink; (int)s == (int)i->s; i = i->blink); // find the position for this node after h
	MoveThisAfter(i);
	//cerr << "exit:OrderedMoveThisBefore\n";
} // OrderedMoveThisBefore

/* Move characters from one location in the byte-string area to
another. The routine handles the following situations:
	if the |Src| > |Dst|   => truncate
	if the |Dst|   > |Src| => pad Dst with blanks */
	

void VbyteHeap::ByteCopy(char *Dst, int DstStart, int DstLnth, char *Src, int SrcStart, int SrcLnth) {
#ifndef VAX
	for (int i = -1; i < DstLnth - 1; i += 1) {
		if (i == SrcLnth - 1) {				// |Dst| > |Src|
				for ( ; i < DstLnth - 1; i += 1) {	// pad Dst with blanks
					Dst[DstStart + i] = ' ';
				} // for
				break;
			} // exit
		Dst[DstStart + i] = Src[SrcStart + i];
	} // for
#else
	// On a Vax, the above code can be replaced by the following assembler code.
	asm(" subl3 $1,12(ap),12(ap)");	// DstStart -= 1
	asm(" addl3 12(ap),8(ap),r0");	// r1 = Dst + DstStart
	asm(" subl3 $1,24(ap),24(ap)");	// SrcStart -=1
	asm(" addl3 24(ap),20(ap),r1");	// r0 = Src + SrcStart
 	asm(" movc5 28(ap),(r1),$32,16(ap),(r0)"); // Dst = Src, padding with blanks if necessary
#endif
} // ByteCopy

/* Compare two byte strings in the byte-string area. The routine returns the following values:
	 1 => Src1-byte-string > Src2-byte-string
	 0 => Src1-byte-string = Src2-byte-string
	-1 => Src1-byte-string < Src2-byte-string */

int VbyteHeap::ByteCmp(char *Src1, int Src1Start, int Src1Lnth, char *Src2, int Src2Start, int Src2Lnth) {
	int cmp;
	//cerr << "enter:ByteCmp, Src1Start:" << Src1Start << " Src1Lnth:" << Src1Lnth << " Src2Start:" << Src2Start << " Src2Lnth:" << Src2Lnth << "\n";
#ifndef VAX
	for (int i = -1; ; i += 1) {
	if (i == Src2Lnth - 1) {
			for ( ; ; i += 1) {
			if (i == Src1Lnth - 1) {
					cmp = 0;
					goto Finish;
				} // exit
			if (Src1[Src1Start + i] != ' ') {
					cmp = 1;
					goto Finish;
				} // exit
			} // for
		} // exit
	if (i == Src1Lnth - 1) {
			for ( ; ; i += 1) {
			if (i == Src2Lnth - 1) {
					cmp = 0;
					goto Finish;
				} // exit
			if (Src2[Src2Start + i] != ' ') {
					cmp = -1;
					goto Finish;
				} // exit
			} // for
		} // exit
	if (Src2[Src2Start + i] != Src1[Src1Start+ i]) {
			cmp = Src1[Src1Start + i] > Src2[Src2Start + i] ? 1 : -1;
			break;
		} // exit
	} // for
    Finish:
#else
	// On a Vax, the above code can be replaced by the following assembler code.
	asm(" subl3 $1,12(ap),12(ap)");	// Src1Start -= 1
	asm(" addl3 12(ap),8(ap),r0");	// r1 = Src1 + Src1Start
	asm(" subl3 $1,24(ap),24(ap)");	// Src2Start -=1
	asm(" addl3 24(ap),20(ap),r1");	// r0 = Src2 + Src2Start
	asm(" cmpc5 28(ap),(r1),$32,16(ap),(r0)"); // Compare Src1 & Src2, padding with blanks if necessary
	asm(" jneq L1"); // not equal ?
	asm(" clrl -4(fp)"); // cmp = 0
	asm(" jbr  L3"); // jump to finish
	asm("L1: jleq L2"); // less than or equal ?
	asm(" movl $1,-4(fp)"); // cmp = 1
	asm(" jbr  L3"); // jump to finish
	asm("L2: mnegl $1,-4(fp)"); // cmp = -1
	asm("L3: "); // finish
#endif
	//cerr << "exit:ByteCmp, cmp:" << cmp << "\n";
	return(cmp);
} // ByteCmp

/* Allocates specified storage for a string from byte-string area. If
not enough space remains to perform the allocation, the garbage
collection routine is called and a second attempt is made to allocate
the space. If the second attempt fails, a further attempt is made to
create a new, larger byte-string area.  */

char *VbyteHeap::VbyteAlloc(int size) {
	int NoBytes;
	char *r;
	//cerr << "enter:VbyteAlloc, size:" << size << "\n";
	NoBytes = (int)EndVbyte + size;
	if (NoBytes > (int)ExtVbyte) {			// enough room for new byte-string ?
		garbage();				// firer up the garbage collector
		NoBytes = (int)EndVbyte + size;		// try again
		if (NoBytes > (int)ExtVbyte) {		// enough room for new byte-string ?
			extend(size);			// extend the byte-string area
		} // if
	} // if
	r = EndVbyte;
	EndVbyte += size;
	//cerr << "exit:VbyteAlloc, r:" << (int)r << " EndVbyte:" << (int)EndVbyte << " ExtVbyte:" << (int)ExtVbyte << "\n";
	return(r);
} // VbyteAlloc

/* The compaction moves all of the byte strings currently in use to
the beginning of the byte-string area and modifies the handles to
reflect the new positions of the byte strings. Compaction assumes that
the handle list is in ascending order by pointers into the byte-string
area.  The strings associated with substrings do not have to be moved
because the containing string has been moved. Hence, they only require
that their string pointers be adjusted.  */

void VbyteHeap::compaction() {
	HandleNode *h;
	char *obase, *nbase, *limit;

	NoOfCompactions += 1;
	EndVbyte = StartVbyte;
	h = Header.flink;				// ignore header node
	for (;;) {
		ByteCopy(EndVbyte, 1, h->lnth, h->s, 1, h->lnth);
		obase = h->s;
		h->s = EndVbyte;
		nbase = h->s;
		EndVbyte += h->lnth;
		limit = obase + h->lnth;
		h = h->flink;

		// check if any substrings are allocated within a string

		for (;;) {
		if (h == &Header) break;		// end of header list ?
		if (h->s >= limit) break;		// outside of current string ?
			h->s = nbase + ((int)h->s - (int)obase);
			h = h->flink;
		} // for
	if (h == &Header) break;			 // end of header list ?
	} // for
} // compaction

/* Garbage determines the amount of free space left in the heap and
then reduces, leave the same, or extends the size of the heap.  The
heap is then compacted in the existing heap or into the newly
allocated heap */

void VbyteHeap::garbage() {
	int AmountUsed, AmountFree;
	//cerr << "enter:garbage\n";
	//cerr << "HandleList:\n"; cerr << "node:" << (int)&Header << " lnth:" << Header.lnth << " s:" << (int)Header.s << " flink:" << (int)Header.flink << " blink:" << (int)Header.blink << "\n"; {for (HandleNode *i = Header.flink; i != &Header; i = i->flink) cerr << "node:" << (int)i << " lnth:" << i->lnth << " s:" << (int)i->s << " flink:" << (int)i->flink << " blink:" << (int)i->blink << "\n";}
	AmountUsed = 0;
	for (HandleNode *i = Header.flink; i != &Header; i = i->flink) { // calculate amount of byte area used
		AmountUsed += i->lnth;
	} // for
	AmountFree = (int)ExtVbyte - (int)StartVbyte - AmountUsed;

	if (AmountFree < (int)(CurrSize * 0.1)) {	// free space less than 10% ?
		extend(CurrSize);			// extend the heap
// This needs work before it should be used.
//	} else if (AmountFree > CurrSize / 2) {		// free space greater than 3 times the initial allocation ? 
//		reduce((AmountFree / CurrSize - 3) * CurrSize);	// reduce the memory
	} // if
	compaction();					// compact the byte area, in the same or new heap area
	//cerr << "HandleList:\n"; cerr << "node:" << (int)&Header << " lnth:" << Header.lnth << " s:" << (int)Header.s << " flink:" << (int)Header.flink << " blink:" << (int)Header.blink << "\n"; {for (HandleNode *i = Header.flink; i != &Header; i = i->flink) cerr << "node:" << (int)i << " lnth:" << i->lnth << " s:" << (int)i->s << " flink:" << (int)i->flink << " blink:" << (int)i->blink << "\n";}
	//cerr << "exit:garbage\n";
} // garbage

/* Extend the size of the byte-string area by creating a new area and
copying the old area into it. The old byte-string area is deleted.  */

void VbyteHeap::extend(int size) {
	char *OldStartVbyte;
	int OldCurrSize;
	//cerr << "enter:extend, size:" << size << "\n";
	NoOfExtensions += 1;
	OldStartVbyte = StartVbyte;			// save previous byte area
	OldCurrSize = CurrSize;				// & size

	CurrSize += size > InitSize ? size : InitSize;	// minimum extension, initial size
	StartVbyte = EndVbyte = new char[CurrSize];
	ExtVbyte = (void *)(StartVbyte + CurrSize);
	compaction();					// copy from old heap to new & adjust pointers to new heap
	delete[OldCurrSize] OldStartVbyte;		// release old heap
	//cerr << "exit:extend, CurrSize:" << CurrSize << "\n";
} // extend

/* Extend the size of the byte-string area by creating a new area and
copying the old area into it. The old byte-string area is deleted.  */

void VbyteHeap::reduce(int size) {
	char *OldStartVbyte;
	int OldCurrSize;
	//cerr << "enter:reduce, size:" << size << "\n";
	NoOfReductions += 1;
	OldStartVbyte = StartVbyte;			// save previous byte area
	OldCurrSize = CurrSize;				// & size

	CurrSize -= size;
	StartVbyte = EndVbyte = new char[CurrSize];
	ExtVbyte = (void *)(StartVbyte + CurrSize);
	compaction();					// copy from old heap to new & adjust pointers to new heap
	delete[OldCurrSize] OldStartVbyte;		// release old heap
	//cerr << "exit:reduce, CurrSize:" << CurrSize << "\n";
} // reduce
