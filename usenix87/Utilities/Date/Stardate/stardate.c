				/* Number of seconds from		*/
#define	EPOCH	104538600	/* 20:30:00 EST, Thu. September 8, 1966	*/
				/* to the UNIX epoch of 00:00:00 GMT,	*/
				/* January 1, 1970.			*/
main()
{
double stardate;
stardate = (time((long *) 0) + EPOCH) / 86400.0;
printf("%.5f\n", stardate);
}
