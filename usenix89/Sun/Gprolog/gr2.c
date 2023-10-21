
/*
 * Gprolog 1.4/1.5
 *
 * plgraphics - interface from CProlog to CORE graphics package
 *
 * Usage:	plgraphics(<func_name>)
 *    or:	plgraphics(<func_name>(arg1,arg2,...,argn))
 *
 * Barry Brachman
 * Dept. of Computer Science
 * Univ. of British Columbia
 * Vancouver, B.C. V6T 1W5
 *
 * .. {ihnp4!alberta, uw-beaver}!ubc-vision!ubc-cs!brachman
 * brachman@cs.ubc.cdn
 * brachman%ubc.csnet@csnet-relay.arpa
 * brachman@ubc.csnet
 */

#include "pl.h"
#include "gr.h"

#include <signal.h>
#include <setjmp.h>

struct {
	PTR space;
	int size;
} sp[MAXARGS];

typedef union {
	char aschar;
	int asint;		/* chars are passed as  INTS */
	char *ascharp;
	PTR asPTR;
	double asdouble;	/* floats are passed as DOUBLES! */
} Mixed;

typedef union {
	int asint;
	float asfloat;
	PTR asPTR;
	char *ascharp;
} Argspace;

extern struct Core_info Core_info[];
extern struct Surface Surface[];

static short noerr;
static nspaces = -1;		/* This must be initialized only once! */
static PTR p;

static jmp_buf jmpbuf;
int (*oldbussignal)();
int (*oldsegvsignal)();
char *fnm;

/*
 * Entry from CProlog to CORE graphics routines
 * Prolog usage:
 *	plgraphics(<CORE_NAME>)
 *   or
 *	plgraphics(<CORE_NAME>(<ARG1>,<ARG2>,...,<ARGn>))
 *
 * Note that getsp(arg) returns a pointer to enough space for arg PTR's,
 * each PTR requires 4 bytes
 *
 */

plgraphics()
{
	register int i,j,arity;
	register PTR ax,g,k,t;
	register struct Core_info *cp;
	PTR a,e,f,e1,f1,t1;
	PTR argptr[MAXARGS];
	Mixed argvec[MAXARGS];
	Argspace *spp;
	struct Surface *s;
	char argtypes[MAXARGS];
	struct Core_info *lookup_core();
	int catchbus(),catchsegv();

	k = ARG1;
	oldbussignal = signal(SIGBUS,catchbus);
	oldsegvsignal = signal(SIGSEGV,catchsegv);
	if (setjmp(jmpbuf)) {
		resetsigs();
		return(0);
	}
	if (IsPrim(k)) {		/* Number or DB reference */
		err1("Improper function name");
		resetsigs();
		return(0);
	}
	if (IsAtomic(k)) {
		cp = lookup_core(AtomP(k)->stofae);
		if (cp == 0) {
			err2("Unknown function - %s",AtomP(k)->stofae);
			resetsigs();
			return(0);
		}
		if (cp->Core_arity != 0) {
			err();
			sprintf(OutBuf,"Arity of %s should be %d",AtomP(k)->stofae,cp->Core_arity);
			PutString(OutBuf);
			resetsigs();
			return(0);
		}
		i = (*cp->Core_func)();
		return(!i);
	}
	if (!IsRef(k)) {
		err1("Improper function name");
		resetsigs();
		return(0);
	}
	g = MolP(k)->Env;
	t = MolP(k)->Sk;
	f = SkelP(t)->Fn;
	arity = FunctorP(f)->arityoffe;
	if ((cp = lookup_core((FunctorP(f)->atoffe)->stofae)) == 0) {
		err2("Unknown function - %s",(FunctorP(f)->atoffe)->stofae);
		resetsigs();
		return(0);
	}
	fnm = cp->Core_name;
	if (cp->Core_arity != arity) {
		err();
		sprintf(OutBuf,"Arity of %s should be %d",fnm,cp->Core_arity);
		PutString(OutBuf);
		resetsigs();
		return(0);
	}
	noerr = TRUE;
	for (i = 0; i < arity && noerr == TRUE; i++) {
		ax = argv(++t,g,&f);
		switch (cp->Core_arg_type[i]) {
		case INT_PTR:
		case ADDR_PTR:
		case FLOAT_PTR:
			if (!IsRef(ax) || !Undef(*ax)) {
				err3(i+1,fnm,"uninstantiated variable");
				noerr = FALSE;
				break;
			}
			argvec[i].asPTR = v1;
			GrowGlobal(1);
			argtypes[i] = 4;
			argptr[i] = ax;
			break;
		case STRING_PTR:
			if (!IsRef(ax) || !Undef(*ax)) {
				err3(i+1,fnm,"uninstantiated variable");
				noerr = FALSE;
				break;
			}
			argvec[i].asPTR = sp[++nspaces].space = getsp(64);
			sp[nspaces].size = 64;
			argtypes[i] = 4;
			argptr[i] = ax;
			break;
		case INT_ARG:
			if (!IsInt(ax)) {
				err3(i+1,fnm,"integer");
				noerr = FALSE;
				break;
			}
			argvec[i].asint = XtrInt(ax);
			argtypes[i] = 4;
			break;
		case FLOAT_ARG:
			if (!IsFloat(ax)) {
				err3(i+1,fnm,"float");
				noerr = FALSE;
				break;
			}
			argvec[i].asdouble = (double)(XtrFloat(ax));
			argtypes[i] = 8;
			break;
		case INT_VEC_ARG:
			p = arg(t,g);
			if (IsInp(p) || Undef(*p)) {
				err3(i+1,fnm,"list of integers/1");
				noerr = FALSE;
				break;
			}
			p = MolP(p)->Sk;
			if (IsAtomic(p) || IsVar(p) ||
				SkelP(p)->Fn != listfunc) {
				err3(i+1,fnm,"list of integers/2");
				noerr = FALSE;
				break;
			}
			e1 = f;
			f1 = SkelP(ax)->Fn;
			if (SkelP(ax)->Fn != listfunc) {
				err3(i+1,fnm,"list of integers/3");
				noerr = FALSE;
				break;
			}
			j = 0;
			p = ax;
			while (IsComp(p) && (MolP(p)->Sk == listfunc)) {
				j++;
				p = argv(Addr(SkelP(p)->Arg2),e1,&e1);
			}
			if (p != atomnil) {
				err3(i+1,fnm,"list of integers/4");
				noerr = FALSE;
				break;
			}
			if (j == 0) {
				err3(i+1,fnm,"(non-empty) list of integers");
				noerr = FALSE;
				break;
			}
			spp = (j == 1 ? getsp(2) : getsp(j));
			sp[++nspaces].space = spp;
			sp[nspaces].size = j;
			argvec[i].asPTR = (PTR)spp;
			argtypes[i] = 4;
			e1 = f;
			f1 = SkelP(ax)->Fn;
			p = ax;
			while (j--) {
				a = arg(Addr(SkelP(p)->Arg1),e1);
				if (!IsPrim(a) || !IsInt(a)) {
					err3(i+1,fnm,"list of integers/5");
					noerr = FALSE;
					break;
				}
				/* sprintf(OutBuf,"%d\n",XtrInt(a)); PutString(OutBuf); */
				spp->asint = XtrInt(a);
				spp++;
				p = argv(Addr(SkelP(p)->Arg2),e1,&e1);
			}
			break;
		case FLOAT_VEC_ARG:
			p = arg(t,g);
			if (IsInp(p) || Undef(*p)) {
				err3(i+1,fnm,"list of floats/1");
				noerr = FALSE;
				break;
			}
			p = MolP(p)->Sk;
			if (IsAtomic(p) || IsVar(p) ||
				SkelP(p)->Fn != listfunc) {
				err3(i+1,fnm,"list of floats/2");
				noerr = FALSE;
				break;
			}
			e = f;
			f1 = SkelP(ax)->Fn;
			p = ax;
			j = 0;
			while (IsComp(p) && (MolP(p)->Sk == listfunc)) {
				j++;
				p = argv(Addr(SkelP(p)->Arg2),e,&e);
			}
			if (p != atomnil) {
				err3(i+1,fnm,"list of floats/3");
				noerr = FALSE;
			}
			if (noerr == FALSE)
				break;
			if (j == 0) {
				err3(i+1,fnm,"(non-empty) list of floats");
				noerr = FALSE;
				break;
			}
			/* Each double needs 8 bytes */
			spp = sp[++nspaces].space = getsp(j*2);
			sp[nspaces].size = j * 2;
			argvec[i].asPTR = (PTR)spp;
			argtypes[i] = 4;
			e = f;
			p = ax;
			while (j--) {
				a = arg(Addr(SkelP(p)->Arg1),e);
				if (!IsPrim(a) || !IsFloat(a)) {
					err3(i+1,fnm,"list of floats/4");
					noerr = FALSE;
					break;
				}
				/* sprintf(OutBuf,"%f\n",XtrFloat(a)); PutString(OutBuf); */
				spp->asfloat = XtrFloat(a);
				spp++;
				p = argv(Addr(SkelP(p)->Arg2),e,&e);
			}
			break;
		case ADDR_ARG:
			if (!IsAtomic(ax) || IsNumber(ax)) {
				err3(i+1,fnm,"surface name/1");
				noerr = FALSE;
				break;
			}
			ax = FunctorP(SkelP(ax)->Fn)->atoffe;
			for (s = Surface; s->surface_name[0] != '\0'; s++)
				if (strcmp(AtomP(ax)->stofae,s->surface_name) == 0)
					break;
			if (s->surface_name[0] != '\0')
				argvec[i].asPTR = s->surface;
			else {
				err3(i+1,fnm,"surface name/2");
				noerr = FALSE;
				break;
			}
			argtypes[i] = 4;
			break;
		case STRING_ARG:
			ax = arg(t,g);
			spp = sp[++nspaces].space = getsp(64);
			sp[nspaces].size = 64;
			if (!list_to_string(ax,(char *)spp,255)) {
				err3(i+1,fnm,"string");
				noerr = FALSE;
				break;
			}
			argvec[i].ascharp = (PTR)spp;
			argtypes[i] = 4;
			break;
		case CHAR_ARG:
			ax = arg(t,g);
			if (ax == atomnil) {
				argvec[i].aschar = '\0';
				argtypes[i] = 4;
				break;
			}
			if (IsInp(ax) || Undef(*ax)) {
				err3(i+1,fnm,"character/1");
				noerr = FALSE;
				break;
			}
			e1 = MolP(ax)->Env;
			p = MolP(ax)->Sk;
			if (IsAtomic(p) || IsVar(p) ||
					SkelP(p)->Fn != listfunc) {
				err3(i+1,fnm,"character/2");
				noerr = FALSE;
				break;
			}
			a = arg(Addr(SkelP(p)->Arg1),e1);
			if (!IsInt(a) || (a = XtrInt(a)) < 0 || a > 255) {
				err3(i+1,fnm,"character/3");
				noerr = FALSE;
				break;
			}
			if (argv(Addr(SkelP(p)->Arg2),e1,&e1) != atomnil) {
				err3(i+1,fnm,"character/4");
				noerr = FALSE;
				break;
			}
		/*	sprintf(OutBuf,"char = %d\n",a); PutString(OutBuf); */
			argvec[i].asint = a;
			argtypes[i] = 4;
			break;
		default:
			err3(i+1,fnm,"Internal switch error!");
			noerr = FALSE;
			break;
		}
	}
	if (noerr == TRUE) {
		j = pushargs(cp->Core_func,argvec,arity,argtypes);

		/* reset SIGINT etc since Core has screwed them up */
		CatchSignals();
	}
	else
		j = -1;			/* Return failure */

	for (i = 0; i < arity && j == 0; i++) {
		int *intp,len,n;
		char *charp;
		float *floatp;

		ax = argptr[i];
		switch (cp->Core_arg_type[i]) {
		case INT_PTR:
		case ADDR_PTR:
			intp = argvec[i].asPTR;
		/* sprintf(OutBuf,"Int = %d\n",*intp); PutString(OutBuf); */
			if (!unifyarg(ax,ConsInt(*intp),0)) {
				j = -1;
				break;
			}
			break;
		case FLOAT_PTR:
			floatp = argvec[i].asPTR;
		/* sprintf(OutBuf,"Float = %f\n",*floatp); PutString(OutBuf); */
			if (!unifyarg(ax,ConsFloat(*floatp),0)) {
				j = -1;
				break;
			}
			break;
		case STRING_PTR:
			charp = argvec[i].asPTR;
			len = strlen(charp);
			if (len > 0) {
				p = v + 1;
				n = len + 1;
				while (len-- > 0)
					*++p = ConsInt(*charp++);
				*(p+1) = atomnil;
				p = makelist(n,v+2);
				v1 -= 2;
				if (!unifyarg(ax,MolP(p)->Sk,MolP(p)->Env)) {
					j = -1;
					break;
				}
			}
			else {
				if (!unifyarg(ax,atomnil,0)) {
					j = -1;
					break;
				}
			}
			break;
		default:
			break;
		}
	}
	/* We really want to disable events during the following critical section */
	while (nspaces >= 0) {
		freeblock(sp[nspaces].space,sp[nspaces].size);
		nspaces--;
	}
	resetsigs();
	return(!j);
}

static
catchbus()
{
	signal(SIGBUS,catchbus);
	sprintf(OutBuf,"plgraphics: a SIGBUS has occured\n");
	PutString(OutBuf);
	sprintf(OutBuf,"Current function is %s\n",fnm);
	PutString(OutBuf);
	longjmp(jmpbuf,1);
}

static
catchsegv()
{
	signal(SIGSEGV,catchsegv);
	sprintf(OutBuf,"plgraphics: a SIGSEGV has occured\n");
	PutString(OutBuf);
	sprintf(OutBuf,"Current function is %s\n",fnm);
	PutString(OutBuf);
	longjmp(jmpbuf,1);
}
