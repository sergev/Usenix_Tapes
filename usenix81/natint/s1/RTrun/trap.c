#include "../hd/param.h"
#include "../hd/systm.h"
#include "../hd/user.h"
#include "../hd/proc.h"
#include "../hd/reg.h"
#include "../hd/seg.h"

#define EBIT    1               /* user error bit in PS: C-bit */
#define UMODE   0170000         /* user-mode bits in PS word */
#define SETD    0170011         /* SETD instruction */
#define SYS     0104400         /* sys (trap) instruction */
#define USER    020             /* user-mode flag added to dev */

/*
 * structure of the system entry table (sysent.c)
 */
struct sysent   {
	int     count;          /* argument count */
	int     (*call)();      /* name of handler */
} sysent[64];

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char    regloc[9]
{
	R0, R1, R2, R3, R4, R5, R6, R7, RPS
};

/*
 * Called from l40.s or l45.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * dev is the kind of trap that occurred.
 */
trap(dev, sp, r1, nps, r0, pc, ps)
{
	register i, a;
	register struct sysent *callp;

	savfp();
	if ((ps&UMODE) == UMODE)
		dev =| USER;
	u.u_ar0 = &r0;
	switch(dev) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 * The numbers printed are used to
	 * find the hardware PS/PC as follows.
	 * (all numbers in octal 18 bits)
	 *      address_of_saved_ps =
	 *              (ka6*0100) + aps - 0140000;
	 *      address_of_saved_pc =
	 *              address_of_saved_ps - 2;
	 */
	default:
		panicking++;
		printf("ka6 = %o\n", *ka6);
		printf("aps = %o\n", &ps);
		printf("trap type %o\n", dev);
		panic("trap");

	case 0+USER: /* bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case 1+USER: /* illegal instruction */
		if(fuiword(pc-2) == SETD && u.u_signal[SIGINS-1] == 0)
			goto out;
		i = SIGINS;
		break;

	case 2+USER: /* bpt or trace */
		i = SIGTRC;
		break;

	case 3+USER: /* iot */
		i = SIGIOT;
		break;

	case 5+USER: /* emt */
		i = SIGEMT;
		break;

	case 6+USER: /* sys call */
		u.u_error = 0;
		ps =& ~EBIT;
		if(u.u_signal[SIGSYS-1] && (char *)pc<=u.u_window[1] && (char *)pc>=u.u_window[0])
			goto out6;
		callp = &sysent[fuiword(pc-2)&077];
		if (callp == sysent) { /* indirect */
			a = fuiword(pc);
			pc =+ 2;
			i = fuword(a);
			if ((i & ~077) != SYS)
				goto out6;      /* illegal */
			callp = &sysent[i&077];
			for(i=0; i<callp->count; i++)
				u.u_arg[i] = fuword(a =+ 2);
		} else {
			for(i=0; i<callp->count; i++) {
				u.u_arg[i] = fuiword(pc);
				pc =+ 2;
			}
		}
		u.u_dirp = u.u_arg[0];
		trap1(callp->call);
		if(u.u_intflg)
			u.u_error = EINTR;
		if(u.u_error < 100) {
			if(u.u_error) {
				ps =| EBIT;
				r0 = u.u_error;
			}
			goto out;
		}
	out6:   i = SIGSYS;
		break;

	/*
	 * Since the floating exception is an
	 * imprecise trap, a user generated
	 * trap may actually come from kernel
	 * mode. In this case, a signal is sent
	 * to the current process to be picked
	 * up later.
	 */
	case 8: /* floating exception */
		psignal(u.u_procp, SIGFPT);
		return;

	case 8+USER:
		i = SIGFPT;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 11/40 this is not the case and
	 * the routine backup/l40.s may fail.
	 * The classic example is on the instruction
	 *      cmp     -(sp),-(sp)
	 */
	case 9+USER: /* segmentation exception */
		a = sp;
		if(backup(u.u_ar0) == 0)
		if(grow(a))
			goto out;
		i = SIGSEG;
		break;

	/*
	 * kernel memory parity error: panic
	 */
	case 10:for(i=0, a=0; a<16; a++) if(parity[a] && (i= *parity[a])<0) break;
		panicking++;
		printf("error addr= %o000\n", (i&07740) >> 3);
		panic("kernel parity error");
	/*
	 * user memory parity error: kill off user with bus error.
	 */
	case 10+USER:
		for(i=0, a=0; a<16; a++)
			if(parity[a] && (i= *parity[a])<0){
				*parity[a]= 1; break; }
		printf("user mode parity error, addr= %o000\n", (i&07740) >> 3);
		i= SIGBUS;
		break;

	/*
	 * This is probably a bunged up interrupt which vectored through 0.
	 * Watch out for hangs on disk completion.
	 */
	case 15:
	case 15+USER:
		printf("spurious interrupt ignored\n");
		return;
	}
	psignal(u.u_procp, i);

out:
	while(issig())
		psig();
	setpri(u.u_procp);
}

/*
 * Call the system-entry routine f (out of the
 * sysent table). This is a subroutine for trap, and
 * not in-line, because if a signal occurs
 * during processing, an (abnormal) return is simulated from
 * the last caller to savu(qsav); if this took place
 * inside of trap, it wouldn't have a chance to clean up.
 *
 * If this occurs, the return takes place without
 * clearing u_intflg; if it's still set, trap
 * marks an error which means that a system
 * call (like read on a typewriter) got interrupted
 * by a signal.
 */
trap1(f)
int (*f)();
{

	u.u_intflg = 1;
	savu(u.u_qsav);
	(*f)();
	u.u_intflg = 0;
}

/*
 * nonexistent system call-- set fatal error code.
 */
nosys()
{
	u.u_error = 100;
}

/*
 * Ignored system call
 */
nullsys()
{
}
