/* this is a local quicky that reads any tape (haha) and puts the files
   into tapeaa, tapeab, tapeac and so on.
		kolstad    6/1 or so/82
	-f option: 6/25/85
 */

#define BUFSZ 100000			/* size of buffer and maximum number 
					   of characters to read */
int     convertit = 0;			  /* = 1 for convert from ebcdic to
					     ascii */
					  /* = 2 for "strip top bit of ascii" 
					  */
char    etoa[] =
{
    0000, 0001, 0002, 0003, 0234, 0011, 0206, 0177,
    0227, 0215, 0216, 0013, 0014, 0015, 0016, 0017,
    0020, 0021, 0022, 0023, 0235, 0205, 0010, 0207,
    0030, 0031, 0222, 0217, 0034, 0035, 0036, 0037,
    0200, 0201, 0202, 0203, 0204, 0012, 0027, 0033,
    0210, 0211, 0212, 0213, 0214, 0005, 0006, 0007,
    0220, 0221, 0026, 0223, 0224, 0225, 0226, 0004,
    0230, 0231, 0232, 0233, 0024, 0025, 0236, 0032,
    0040, 0240, 0241, 0242, 0243, 0244, 0245, 0246,
    0247, 0250, 0133, 0056, 0074, 0050, 0053, 0041,
    0046, 0251, 0252, 0253, 0254, 0255, 0256, 0257,
    0260, 0261, 0135, 0044, 0052, 0051, 0073, 0136,
    0055, 0057, 0262, 0263, 0264, 0265, 0266, 0267,
    0270, 0271, 0174, 0054, 0045, 0137, 0076, 0077,
    0272, 0273, 0274, 0275, 0276, 0277, 0300, 0301,
    0302, 0140, 0072, 0043, 0100, 0047, 0075, 0042,
    0303, 0141, 0142, 0143, 0144, 0145, 0146, 0147,
    0150, 0151, 0304, 0305, 0306, 0307, 0310, 0311,
    0312, 0152, 0153, 0154, 0155, 0156, 0157, 0160,
    0161, 0162, 0313, 0314, 0315, 0316, 0317, 0320,
    0321, 0176, 0163, 0164, 0165, 0166, 0167, 0170,
    0171, 0172, 0322, 0323, 0324, 0325, 0326, 0327,
    0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,
    0340, 0341, 0342, 0343, 0344, 0345, 0346, 0347,
    0173, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
    0110, 0111, 0350, 0351, 0352, 0353, 0354, 0355,
    0175, 0112, 0113, 0114, 0115, 0116, 0117, 0120,
    0121, 0122, 0356, 0357, 0360, 0361, 0362, 0363,
    0134, 0237, 0123, 0124, 0125, 0126, 0127, 0130,
    0131, 0132, 0364, 0365, 0366, 0367, 0370, 0371,
    0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,
    0070, 0071, 0372, 0373, 0374, 0375, 0376, 0377,
};

#include	<stdio.h>
main (argc, argv)
char   *argv[];
{
    int     fid,
            mark,			  /* number of tape marks seen so far 
					  */
            nchars;
    int     nrecs = 0;			  /* how many of this record size
					     we've seen */
    int     lastchars = -4;		  /* most recent record size */
    int     fidout = 0;			  /* output file */
    char    buf[BUFSZ];
    char   *filenum,
           *p;
    int     i;
    char   *tapename = "/dev/rmt8";

    while (argc > 1)
    {
	if (argv[1][0] != '-')
	{
	    fprintf (stderr, "Usage:  taperead [-f tapename] [-e] [-s]\n");
	    exit (1);
	}
	switch (argv[1][1])
	{
	    case 'e': 
		convertit |= 1;
		fprintf (stderr, "I will convert from ebcdic to ascii.\n");
		break;
	    case 'f': 
		tapename = argv[2];
		argc--;
		argv++;
		break;
	    case 's': 
		convertit |= 2;
		fprintf (stderr, "I will strip the top bit from each byte.\n");
		break;
	    default: 
		fprintf (stderr, "Usage:  taperead [-e] [-s]\n");
		exit (1);
	}
	argc--;
	argv++;
    }
    setbuf (stdout, 0);
    filenum = "tapeaa";
    fid = open (tapename, 0);
    if (fid < 0)
    {
	printf ("cant open tape\n");
	exit (2);
    }
    do
    {
	nchars = read (fid, buf, BUFSZ);
	if (fidout == 0)
	    printf ("File: `%s':  ", filenum);
	if (nchars == lastchars)
	    nrecs++;
	if (nchars != lastchars)
	{
	    if (nrecs > 1)
		printf ("*%d", nrecs);
	    nrecs = 1;
	    lastchars = nchars;
	    if (nchars > 0)
	    {
		if (fidout > 0)
		    printf ("+");
		printf ("%d", nchars);
	    }
	}
	if (nchars < 0)
	{
	    printf ("ERROR on tape read\n");
	    continue;
	}
	if (nchars == 0)		  /* EOF? */
	{
	    printf (" === EOF ===\n");
	    mark++;
	    close (fidout);
	    fidout = 0;
	    for (p = filenum + 5; p >= filenum;)
	    {
		if (++*p <= 'z')
		    break;
		*p-- = 'a';
	    }
	    if (mark == 2)
	    {
		printf ("====*****=====EOT========\n");
		exit (0);
	    }
	    continue;
	}
	if (convertit & 1)
	    for (i = 0; i < nchars; i++)
		buf[i] = etoa[buf[i] & 0xFF];

	if (convertit & 2)
	    for (i = 0; i < nchars; i++)
		buf[i] & = 0177;
	if (fidout == 0)
	{
	    fidout = creat (filenum, 0666);
	    if (fidout < 0)
	    {
		printf ("cant open %s\n", filenum);
		exit (1);
	    }
	}
	write (fidout, buf, nchars);
	mark = 0;
    } while (1);
}
