#include "Process.h"
#include "Scheduler.h"
#include <math.h>

extern const int OOPS_BADTRACETBL;

const int ARG_SAVE_SZ     =4;
const int LINK_SAVE_SZ    =5;

// save registers
#define	SAVE_REGS	register r1=1,r2=2,r3=3,r4=4,r5=5,r6=6,r7=7,r8=abs((r1*r2)+(r3*r4)+r5+r6+r7)

// save floating point registers
#define SAVE_FREGS	register double f1=1.0,f2=2.0,f3=3.0,f4=4.0,f5=sin((f1*f2)+(f3*f4)+(f1*f4))

struct TT_D_COM {	// structure of ACIS 4.2 trace table
	unsigned magic1:8,	// = 0xDF
		code:8,		// = 7
		magic2:8,	// = 0xDF
		first_gpr:4,	// first general register saved
		optw:1, optx:1, opty:1, optz:1;	// option flags
	char	npars:4,	// number of parameters
		frame_reg:4;	// frame pointer register number
	char	first_fpr:4,	// first fp register saved (2,3,4, or 5)
		:4;		// present only if optx==1
	char	lcl_off_size:2,	// size of lcl_offset
		lcl_offset1:6,	// # words to top of stack frame from frame_reg
		lcl_offsetn[3];	// 6, 14, 22, or 30 bits
};

void Process::create(void** stack)
{
//	register int* pcp;		// data area pointer	(r0)
	register int* sp =(int*)1;	// stack pointer	(r1)
	register int* fp =(int*)13;	// frame pointer	(r13)
//	register int* dp;		// data pointer		(r14)
	register int* link =(int*)15;	// link register	(r15)
// find caller's trace table
	register short* p = (short*)link;
	while (((*p++ & 0xff00) != 0xdf00) || ((*p & 0xff00) != 0xdf00));
	register TT_D_COM* ttp = (TT_D_COM*)--p;
	if (ttp->code != 7) {
		setOOPSerror(OOPS_BADTRACETBL,FATAL,ttp,className(),this);
		return;
	}
// calculate pointer to saved caller's fp
	register int** fpp = &((int**)(&stack))[-LINK_SAVE_SZ-(15-ttp->frame_reg)-2];
// find address of caller's last parameter
	register int* osp = *fpp+(ttp->lcl_offset1+(ttp->npars-5)+1);
	register n = osp-sp;			// number of words to copy
	register int* nsp = (int*)stack;	// pointer to new stack
// copy current stack to new stack
	while (n--) *--nsp = *--osp;
// adjust frame link on new stack
	*(fpp+(nsp-osp)) += nsp-osp;
	saved_fp = fp+(nsp-osp);	// save new sp
	SAVE_FREGS;		// force saving of floating point registers
}

void Process::exchj()
{
	register void* fp =(int*)13;	// frame pointer (r13)
	scheduler.previous_process->saved_fp = fp;
	fp = saved_fp;
	SAVE_REGS;		// force saving of registers
	SAVE_FREGS;		// force saving of floating point registers
}
