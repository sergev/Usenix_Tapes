s/^#ifndef[ 	]*\([A-Z][A-Z]*\)H[ 	]*$/#ifndef	\1_H/w changes
s/^#define[ 	]*\([A-Z][A-Z]*\)H[ 	]*$/#define	\1_H/w changes
/^#define[ 	]*\([A-Z][A-Z]*\)H$/a\
// $Header$
/^Modification History:/a\
\
$Log$
