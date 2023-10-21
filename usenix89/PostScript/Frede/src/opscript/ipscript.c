/*
 *	ipscript.c - version 3.2
 *
 *	Convert bitmap image to postscript
 *
 *		- Stephen Frede, UNSW, Australia (stephenf@elecvax.oz)
 *			...!seismo!munnari!elecvax!stephenf
 *		- fixes MWR, UNSW (michaelr@elecvax.oz)
 */

#ifndef	lint
static char id[] = "ipscript - version 3.2; UNSW, Australia";
#endif	lint

#include	<stdio.h>
#include	"pscript.h"

/* default values for the input bitmap data */
#define	N_SCANSPERIMAGE	256	/* no. scan lines */
#define	N_PIXELSPERSCAN	256	/* pixels per scanline */
#define	N_SCANRES	8	/* no. bits per pixel (input) */
#define	PAPERWIDTH	19.3	/* width (short axis) of paper (cm) */
#define	PAPERLENGTH	28.3	/* length (long axis) of paper (cm) */

/* possible input formats */
#define	I_BYTE		'b'
#define	I_HEX		'x'

#define	BITSINBYTE	8

int	ScansPerImage = N_SCANSPERIMAGE,	/* no. lines in image */
	PixelsPerScan = N_PIXELSPERSCAN,	/* no. pixels in each line */
	scanres = N_SCANRES,		/* input resolution (no. grey levels) */
	CellsPerInch = 60,		/* halftone cells/inch */
	outres = 8,			/* 2^outres output grey levels */
	CellRotation = 45,		/* halftone cell rotation */
	rotation = 0,			/* image rotation */
	headerskip = 0,			/* no. bytes to skip at start */
	greyoffset = 0,			/* brightness */
	PaperType =
#ifdef	ALW
		PT_A4,
#else
		PT_DEFAULT,
#endif	ALW
	inpformat = I_BYTE;		/* format of input data */
float	aspect = 1.5,		/* aspect ratio (x:y) */
	width = 0.0,		/* output width (cm) */
	height = 0.0;		/* output height (cm) */
bool	ManualFeed = FALSE,	/* manual or auto (paper tray) feed */
	NegativeImage = FALSE;	/* reverse black and white */

double	atof();

/* ARGSUSED */
main(argc, argv)
int	argc;
char	**argv;
{
	char	c;
	bool	filesgiven = FALSE,
		status = 0;

	postr = stdout;
	ScansPerImage = N_SCANSPERIMAGE;
	PixelsPerScan = N_PIXELSPERSCAN;
	argv++;		/* skip command name */

	while(*argv)
	{
		if(**argv == '-')
		{
			(*argv)++;	/* skip the '-' */
			c = **argv;	/* command arg */
			(*argv)++;	/* skip arg letter */
			switch(c)
			{
			case 'i':	/* input format */
				if(**argv == 'b')
					inpformat = I_BYTE;
				else if(**argv == 'x')
					inpformat = I_HEX;
				else
				{
					fprintf(stderr, "input format (-i option) only allows 'b' or 'x'\n");
					status++;
					break;
				}
				(*argv)++;
				if(**argv == '\0')
				{
					fprintf(stderr, "input format - expected scan resolution (bits) after `-%c'\n", (*argv)[-1]);
					status++;
					break;
				}
				scanres = atoi(*argv);
				if(scanres < 1 || scanres > 8)
				{
					fprintf(stderr, "input format: illegal scan resolution must be in range 1 - 8\n");
					status++;
				}
				break;

			case 's':
				headerskip = atoi(*argv);
				break;

			case 'g':	/* grey offset (brightness) */
				greyoffset = atoi(*argv);
				break;

			case 'b':	/* bits resolution out */
				outres=atoi(*argv);
				if(outres != 1 && outres != 2 &&
					outres != 4 && outres != 8)
				{
					fprintf(stderr,
						"Illegal output resolution - must be 1, 2, 4 or 8 bits\n");
					status++;
				}
				break;

			case 'y':	/* no. scan lines input */
				ScansPerImage=atoi(*argv);
				break;

			case 'x':	/* pixels per scanline */
				PixelsPerScan = atoi(*argv);
				break;

			case 'a':	/* aspect ratio */
				aspect = atof(*argv);
				break;

			case 'w':	/* width (cm) */
				width = atof(*argv);
				break;

			case 'h':	/* height (cm) */
				height = atof(*argv);
				break;

			case 'f':	/* output pixels/inch */
				CellsPerInch = atoi(*argv);
				break;

			case 'p':	/* pixel rotation */
				CellRotation = atoi(*argv);
				break;

			case 'r':	/* image rotation */
				if(**argv)
					rotation = atoi(*argv);
				else
					rotation = 90;
				break;

			case 'L':	/* 'legal' paper size */
				PaperType = PT_LEGAL;
				break;

			case 'n':	/* negative image */
				NegativeImage = TRUE;
				break;

			case 'S':	/* manual feed */
				ManualFeed = TRUE;
				break;

			default:
				fprintf(stderr, "Unknown option '%c'\n", c);
				status++;
			}
		}
		else
		{
			if(status)
				usage();
			filesgiven = TRUE;
			status += DoImage(*argv);
		}
		argv++;
	}
	if(status && ! filesgiven)
		usage();
	if(! filesgiven)
		status += DoImage("-");

	exit(status);
	/* NOTREACHED */
}

char	*usagetab[] = {
	"Usage: ipscript option file ...",
	"valid options:",
	"\t-r[angle]		output image rotation (degrees)",
	"\t-wWidth			output image width (in cm)",
	"\t-hHeight		output image height (in cm)",
	"\t-aAspect		aspect ratio (width/height)",
	"\t-yScanlines		no. scan lines in input",
	"\t-xScanlength		no. pixels in an input scan line",
	"\t-iFormat		input format: {'b'|'x'}bitsresolution",
	"\t-sSkip			no. bytes header to skip",
	"\t-gGrey			output grey level offset (signed)",
	"\t-bbits			output resolution = 2^bits greylevels",
	"\t-fFrequency		output frequency (pixels/inch)",
	"\t-pAngle			pixel grid rotation (degrees)",
	"\t-fFrequency		output frequency (dots/inch)",
	"\t-n			negative image",
	"\t-S			Use manual feed",
	(char *) 0
};

usage()
{
	char	**p;

	p = usagetab;
	while(*p)
		fprintf(stderr, "%s\n", *p++);
	exit(1);
	/* NOTREACHED */
}

/* postscript initialisation for options currently in effect
 * Assumes default LaserWriter conditions (ie start of job)
 */
init(filename, BytesPerScan)
char	*filename;
int	BytesPerScan;
{
	pcomminit(0.0, (float) rotation, PaperType, ManualFeed,
		"", filename, "ipscript bit image");
	fprintf(postr, "gsave %d %d translate\n", rotation == 0 ? 25 : 18,
						  rotation == 0 ? 0  : 5);

	if(height == 0.0 && width == 0.0)
		width = rotation == 0 ? PAPERWIDTH : PAPERLENGTH;
	if(height == 0.0)
		height = width / aspect;
	else if(width == 0.0)
		width = height * aspect;
	fprintf(postr, "%.1f %.1f scale\n", width * PU_CM, height * PU_CM);

	fprintf(postr, "0 setlinewidth\n");
	fprintf(postr, "/picstr %d string def\n", BytesPerScan);
	fprintf(postr, "/doimage {\n");
	fprintf(postr, "%d %d %d [ %d 0 0 %d 0 %d ]\n",
		PixelsPerScan, ScansPerImage, outres,
		PixelsPerScan, - ScansPerImage, ScansPerImage);
	fprintf(postr, "{ currentfile picstr readhexstring pop } image\n");
	fprintf(postr, "} def\n");
	if(CellsPerInch != PD_PFREQUENCY || CellRotation != PD_PROTATION)
		fprintf(postr, "%d %d {dup mul exch dup mul add 1 exch sub} setscreen\n", CellsPerInch, CellRotation);
	if(NegativeImage)
		fprintf(postr, "{1 exch sub} settransfer\n");
}

DoImage(filename)
char	*filename;
{
	register int	outval,
			PixelInByte,
			ByteInScan,
			PixelInScan,
			ScanInImage;
	int		div,
			inval,
			max;
	FILE		*istr;
	int		BytesPerScan;	/* no. bytes/scanline output */
	int		PixelsInByte;	/* no. pixels/byte output */
	bool		OutRange = FALSE;

	if(strcmp(filename, "-") == 0)
		istr = stdin;
	else if((istr = fopen(filename, "r")) == NULL)
	{
		perror(filename);
		return(1);
	}

	PixelsInByte = BITSINBYTE / outres;
	BytesPerScan = (PixelsPerScan + PixelsInByte - 1) / PixelsInByte;

	init(filename, BytesPerScan);
	max = 1 << scanres;
	div = scanres - outres;

	fprintf(postr, "%% Resolution: %d bits\n",
		outres);
	fprintf(postr, "(bit image start ...\\n) ps\n");
	fprintf(postr, "%%%%EndProlog\n");
	fprintf(postr, "%%%%Page 1 1\n");

	fprintf(postr, "doimage\n");

	for(ByteInScan = 0; ByteInScan < headerskip; ByteInScan++)
		getc(istr);

	for(ScanInImage = 0; ScanInImage < ScansPerImage; ScanInImage++)
	{
		for(ByteInScan = 0, PixelInScan = 0;
			ByteInScan < BytesPerScan; ByteInScan++)
		{
			outval = 0;
			/* build up an output byte */
			for(PixelInByte = 0;
				PixelInByte < PixelsInByte && PixelInScan < PixelsPerScan;
				PixelInByte++, PixelInScan++)
			{
				if(inpformat == I_BYTE)
					inval = getc(istr);
				else
				{	/* I_HEX */
					if(scanres <= 4)
						fscanf(istr, "%1X", &inval);
					else
						fscanf(istr, "%2X", &inval);
				}
				if(feof(istr))
				{
					fprintf(stderr, "Error: insufficient input\n");
					return(1);
				}
				if(greyoffset)
				inval += greyoffset;
				if(inval < 0)
				{
					inval = 0;
					OutRange = TRUE;
				}
				else if(inval >= max)
				{
					inval = max-1;
					OutRange = TRUE;
				}
				if(div >= 0)
					outval = (outval << outres) | (inval >> div);
				else
					outval = (outval << outres) | (inval << -div);
			}
			/* make up a whole byte at the end of a scan line */
			for(; PixelInByte < PixelsInByte; PixelInByte++)
				outval = outval << outres;
			/* do a little bit of formatting to make it readable */
			if(ByteInScan % 10 == 0)
				if(ByteInScan % 30 == 0) putc('\n', postr);
				else putc(' ', postr);
			fprintf(postr, "%X%X", outval >> 4, outval & 017);
		}
		/* end of a scan line */
		fprintf(postr, "\n");
	}

	/* draw a border around the picture */
	if(NegativeImage)
		fprintf(postr, "{} settransfer\n");	/* restore transfer func */
	fprintf(postr,
		"newpath 0 0 moveto 1 0 lineto 1 1 lineto 0 1 lineto closepath stroke\n");

	fprintf(postr, "\ngrestore\nshowpage\n");
	pcommfinish(-1, "");
	if(getc(istr) != EOF)
		fprintf(stderr,
			"Warning: input data after end of image\n");
	if(OutRange)
		fprintf(stderr,
			"Warning: input greyscale range too large - change 'i' or 'g' options\n");
	return(0);
}
