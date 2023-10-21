/*
 *	builtin.c
 *
 *	initialize builtin functions
 */

# include	"ic.h"
# include	<math.h>

# define PI	3.14159265358979323846

struct fbuiltin {
	double	(*bf_func)();
	char	*bf_name;
	int	bf_argc;
};

struct vbuiltin {
	double	bv_value;
	char	*bv_name;
};

double	dowrt(), dowrtln(), doprintf(), doscanf();
double	sinD(), cosD(), tanD(), asinD(), acosD(), atanD(), atan2D();
double	dist();

struct fbuiltin funcs[] = {
	dowrt,		"write",	-1,
	dowrtln,	"writeln",	-1,
	doprintf,	"printf",	-1,
	doscanf,	"scanf",	-1,
	exp,		"exp",		1,
	log,		"log",		1,
	log10,		"log10",	1,
	pow,		"pow",		2,
	sqrt,		"sqrt",		1,
	fabs,		"abs",		1,
	floor,		"floor",	1,
	ceil,		"ceil",		1,
	hypot,		"hypot",	2,
	j0,		"j0",		1,
	j1,		"j1",		1,
	jn,		"jn",		256 | 2,
	y0,		"y0",		1,
	y1,		"y1",		1,
	yn,		"yn",		256 | 2,
	sinD,		"sin",		1,
	cosD,		"cos",		1,
	tanD,		"tan",		1,
	asinD,		"asin",		1,
	acosD,		"acos",		1,
	atanD,		"atan",		1,
	atan2D,		"atan2",	2,
	sinh,		"sinh",		1,
	cosh,		"cosh",		1,
	tanh,		"tanh",		1,
	dist,		"dist",		4,
	0,		0,		0,
};

struct vbuiltin vars[] = {
	3.1415926535897932384626433,	"pi",
	2.7182818284590452353602874,	"e",
	0.0,				0,
};

initbuiltin ()
{
	register struct fbuiltin	*f;
	register struct vbuiltin	*v;
	register symbol			*s;
	symbol				*insertSym();

	for (f = funcs; f->bf_name; f++) {
		s = insertSym (f->bf_name);
		s->s_type = BUILTIN;
		s->s_level = -1;
		s->s_builtin = f->bf_func;
		s->s_argc = f->bf_argc;
	}
	for (v = vars; v->bv_name; v++) {
		s = insertSym (v->bv_name);
		s->s_type = VARTYPE;
		s->s_level = 0;
		s->s_value = v->bv_value;
	}
}

double
dowrt (n, p)
int	n;
double	*p;
{
	while (n--) {
		printf ("%.15g ", *p++);
	}
	return 1.0;
}

double
dowrtln (n, p)
int n;
double	*p;
{
	dowrt (n, p);
	putchar ('\n');
	return 1.0;
}

double
doprintf (n, p)
int n;
double	*p;
{
	char	*fmt;
	char	**strings;
	extern char	**stringsp;
	
	strings = stringsp;
	++p;
	for (fmt = *strings++; *fmt; ++fmt) {
		switch (*fmt) {
		case '%':
			switch (*++fmt) {
			case 'd':
				printf ("%.0f", *p);
				break;
			case 's':
				printf ("%s", *strings++);
				break;
			case 'f':
				printf ("%f", *p);
				break;
			case 'e':
				printf ("%e", *p);
				break;
			case 'g':
				printf ("%g", *p);
				break;
			case 'c':
				printf ("%c", (char) *p);
				break;
			case 'o':
				printf ("%lo", (long) *p);
				break;
			case 'x':
				printf ("%lx", (long) *p);
				break;
			default:
				putchar (*fmt);
				continue;
			}
			++p;
			break;
		default:
			putchar (*fmt);
		}
	}
}

double
doscanf (n, p)
{
	return 0.0;
}

double
sinD(a)
double	a;
{
	return sin (a * PI / 180);
}

double
cosD(a)
double	a;
{
	return cos (a * PI / 180);
}

double
tanD(a)
double	a;
{
	return tan (a * PI / 180);
}

double
asinD(a)
double	a;
{
	return asin (a) * 180/PI;
}

double
acosD(a)
double	a;
{
	return acos (a) * 180/PI;
}

double
atanD(a)
double	a;
{
	return atan (a) * 180/PI;
}

double
atan2D(a,b)
double	a,b;
{
	return atan2 (a,b) * 180/PI;
}
