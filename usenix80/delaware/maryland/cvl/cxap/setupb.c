#
#include "/mnt/phil/cxap/area.define"

int setupb(area,name,win,sh,numro,numco,ptr,buf,collen)
int     *area;                  /*work area for C-xap                   */
char    *name;                  /*input file                            */
int     *win;                   /*window for input picture              */
int     sh;                     /*shift                                 */
int     numro;                  /*number of rows in neighborhood        */
int     numco;                  /*number of columns in neighborhood     */
int     *ptr;                   /*vector of pointers                    */
int     *buf;                   /*input buffer                          */
int     collen;                 /*column length of a row                */
{

	register
	int     bufof,          /*buffer offset                         */
		poscntr,        /*position counter                      */
		status;         /*status of function call               */

	bufof = (numco + 1) / 2 - 1;

	status = setupr(area,name,win,sh,numro,ptr,buf + bufof,collen);

	if ((status == ERROR) || (status == EOF))
	{
		printf(2,"Setupr error in Bread\n");
		return(status);
	}

	WROWS    = win[3];
	LEFTBOR  = bufof;
	RIGHTBOR = numco / 2;
	TOPBOR   = (numro + 1) / 2 - 1;
	BOTBOR   = numro / 2;
	POSITION = 0;
	OUTCOL   = numco + NCOL - 1;
	RBORLOC  = LEFTBOR + NCOL;

	if (BOTBOR < 1)
		return(EOF);

	for (poscntr = 1; poscntr <= BOTBOR; poscntr++)
	{
		POSITION = poscntr;

		if (POSITION <= WROWS)
		       rex(area,buf,ptr);

		if (POSITION <= TOPBOR)
		       copyup(area,buf,ptr);

		if (POSITION > WROWS)
		       copydn(area,buf,ptr);
	}

	return(SUCCES);

} /*end setupb*/



