#include <stdio.h>
#include <math.h>

#define mapx(lon) (int)(((lon)-Llon)*Xfud)
#define mapy(lat) (int)(Yfud*(Merc?(log(tan((45.0+(lat)/2.0)*Drad))-Mlat):((lat)-Llat)))

double	Drad, Llat, Ulat, Llon, Rlon;
double	Xfud, Yfud, Mlat, atof(), fabs();
int	Ext, Merc;

main(argc, argv)
int	argc;
char	**argv;
{
	int	i, chr;

	openpl();
	erase();
	space(0, 0, 3520, 3520);
	move(0, 0);
	cont(3519, 0);
	cont(3519, 3519);
	cont(0, 3519);
	cont(0, 0);
	Ext = 0;
	Rlon = 180.0;
	Llon = -180.0;
	Ulat = 90.0;
	Llat = -90.0;
	Merc = i = 0;
	if (argc > 1 && *argv[1] == '-') {
		argc--;
		argv++;
		while (chr = *++*argv) switch (chr) {
		  case 'm':
			Merc = 1;
			Ulat = 75.0;
			Llat = -60.0;
			break;
		  case 'e':
			Ext = 1;
			break;
		  case 'l':
			i = 1;
			break;
		}
	}
	if (i && argc > 4) {
		argc -= 4;
		Llon = atof(*++argv);
		Rlon = atof(*++argv);
		Llat = atof(*++argv);
		Ulat = atof(*++argv);
	}
	Xfud = 3520.0/(Rlon-Llon);
	if (Merc) {
		Drad = 3.14159265358979323/180.0;
		Mlat = log(tan((45.0+Llat/2.0)*Drad));
		Yfud = 3520/(log(tan((45.0+Ulat/2.0)*Drad))-Mlat);
	} else {
		Yfud = 3520/(Ulat-Llat);
	}
	while (--argc)
		dfile(*++argv);
	pworld();
	closepl();
	exit(0);
}

dfile(file)
char	*file;
{
	FILE	*fp;
	double	lat, lon, alat, alon;

	if (!(fp = fopen(file, "r"))) {
		perror(file);
		return;
	}
	while (fscanf(fp, "%lf %lf %lf %lf", &lat, &lon, &alat, &alon) != EOF)
		if (Ext || (lat >= Llat && lat <= Ulat && lon >= Llon
		    && lon <= Rlon && alat >= Llat && alat <= Ulat
		    && alon >= Llon && alon <= Rlon))
			line(mapx(lon), mapy(lat), mapx(alon), mapy(alat));
	fclose(fp);
}

#ifdef	WDB
pworld()
{
	FILE	*fp, *ip;
	char	line[80];
	int	cnt, pos;
	float	lon, lat, alon, alat;

	if (!(ip = fopen("worldI.ind", "r"))) {
		perror("index");
		exit(1);
	}
	if (!(fp = fopen("worldI.dat", "r"))) {
		perror("data");
		exit(2);
	}
	while (fgets(line, 80, ip)) {
		sscanf(line, "%*d %*d %d %d %f %f %f %f", &pos, &cnt,
			&lat, &lon, &alat, &alon);
		if (Llon > alon || Rlon < lon || Llat > alat || Ulat < lat)
			continue;
		fseek(fp, --pos*8l, 0);
		fread(&lat, sizeof lat, 1, fp);
		fread(&lon, sizeof lon, 1, fp);
		move(mapx(lon), mapy(lat));
		while (--cnt) {
			fread(&alat, sizeof lat, 1, fp);
			fread(&alon, sizeof lon, 1, fp);
			if (fabs(alon-lon) < 300.0)
				cont(mapx(alon), mapy(alat));
			else
				move(mapx(alon), mapy(alat));
			lon = alon;
		}
	}
	fclose(fp);
	fclose(ip);
}
#else
pworld()
{
	char		line[40];
	FILE		*df;
	auto	int	con;
	register int	aout, out;
	auto	float	lat, lon, alat, alon;

	if (!(df = fopen("world", "r"))) {
		perror("world");
		exit(1);
	}
	alon = -9999.0;
	aout = 15;
	while (fgets(line, 40, df)) {
		sscanf(line, "%d %f %f", &con, &lat, &lon);
		if (Llon > lon)
			out = 1;
		else if (Rlon < lon)
			out = 2;
		else
			out = 0;
		if (Llat > lat)
			out |= 4;
		else if (Ulat < lat)
			out |= 8;
		if (con == 2 && !(aout & out) && fabs(alon-lon) < 300.0) {
			if (!(aout & 16))
				move(mapx(alon), mapy(alat));
			cont(mapx(lon), mapy(lat));
			out |= 16;
		}
		aout = out;
		alat = lat;
		alon = lon;
	}
	fclose(df);
}
#endif
