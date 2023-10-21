# include	"tables.h"	/* IBM_ASCII conversion tables */

ibmtoa(cp, n)
char	*cp;
int	n;
/*
**  Converts n IBM characters of cp to their ASCII equivalent.
*/
{
	for (; n--; cp++)
		*cp = ibm_a[*cp & 255];
}
	


atoibm(cp, n)
char	*cp;
int	n;
/*
**  Converts n ASCII characters of cp to their IBM-EBCDIC equivalent.
*/
{
	for (; n--; cp++)
		*cp = a_ibm[*cp & 255];
}


atoe(cp, n)
char	*cp;
int	n;
/*   
**  Converts n EBCDIC characters of cp to their ASCII equivalent.
*/
{
	for (; n--; cp++)
		*cp = a_ebc[*cp & 255];
}



etoa(cp, n)
char	*cp;
int	n;
/*
**  Converts n ASCII characters of cp to their EBCDIC equivalent.
*/
{
	for (; n--; cp++)
		*cp = ebc_a[*cp & 255];
}
