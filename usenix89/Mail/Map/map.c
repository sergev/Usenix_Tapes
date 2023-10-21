#include <ctype.h>
#include <stdio.h>

#define	HSIZE	20000

struct	site {
	char	sname[24];
	float	lat, lon;
} *Htab[HSIZE];

main(argc, argv)
int	argc;
char	*argv[];
{
	int	i, s;
	char	*ca[500];
	char	*edges;

	edges = *++argv;
	while (--argc)
		est(*++argv);
	dumpedges(edges);
	exit(0);
}

est(file)
char	*file;
{
	FILE	*fp;
	char	line[100], sname[24];
	int	count;
	double	lat, lon, slat, slon;

	fp = fopen(file, "r");
	slat = slon = count = 0;
	while (fgets(line, 100, fp))
		if (line[0] == '#') switch(line[1]) {
		  case 'L':
			if (*sname && ploc(line+2, &lat, &lon))
				add(sname, lat, lon);
			*sname = 0;
			break;
		  case 'N':
			sscanf(line+2, "%23s", sname);
			break;
		}
	fclose(fp);
}

add(sname, lat, lon)
char	*sname;
double	lat, lon;
{
	unsigned 	hkey, chr;
	char		*cp, *malloc();
	struct	site	*sst;

	hkey = 0;
	for (cp = sname; chr = *cp; cp++) {
		hkey <<= 3;
		hkey += chr;
	}
	hkey %= HSIZE;
	while (Htab[hkey])
		if (++hkey == HSIZE)
			hkey = 0;
	Htab[hkey] = sst = (struct site *)malloc(sizeof(struct site));
	strcpy(sst->sname, sname);
	sst->lat = lat;
	sst->lon = lon;
}

ploc(line, lat, lon)
char	*line;
double	*lat, *lon;
{
	char	*cp, *index(), *cvtn();
	double	nums[32], work;
	int	i, j, cnt = 0;

	for (cp = line; *cp; cp++) {
		if (isspace(*cp))
			continue;
		if (index("NSEW", *cp)) {
			nums[cnt++] = - *cp;
			continue;
		}
		if (isdigit(*cp))
			cp = cvtn(cp, nums + cnt++);
	}
	if (cnt < 4)
		return(0);
	nums[cnt] = -1;
	if (nums[0] < 0.0) {
		i = 0;
		while (nums[++i] >= 0.0)
			continue;
		work = nums[i];
		nums[i] = nums[0];
		while (nums[++i] >= 0.0)
			continue;
		nums[i] = work;
		i = 1;
	} else
		i = 0;
	j = 1;
	work = 0.0;
	*lat = *lon = -999.0;
	while (i < cnt) {
		if (nums[i] < 0.0)
			if (j == 1)
				goto prob;
			else {
				j = 1;
				switch((int)-nums[i]) {
				  case 'N':
					*lat = work;
					break;
				  case 'S':
					*lat = - work;
					break;
				  case 'E':
					*lon = work;
					break;
				  case 'W':
					*lon = - work;
					break;
				}
				if (*lat > -990.0 && *lon > -990.0)
					return(1);
			}
		else switch (j) {
		  case 1:
			work = nums[i];
			j = 2;
			break;
		  case 2:
			work += nums[i] / 60.0;
			j = 3;
			break;
		  case 3:
			work += nums[i] / 3600.0;
			j = 4;
			break;
		  case 4:
			goto prob;
		}
		i++;
	}
prob:
	return(0);
}

char	*
cvtn(cp, np)
char	*cp;
double	*np;
{
	double	atof();
	int	i;

	for (i = 0; isdigit(cp[i]); i++)
		continue;
	if (i > 3) {
		*np = (cp[0]-'0')*10 + (cp[1]-'0');
		return(cp+1);
	}
	*np = atof(cp);
	while (isdigit(*cp) || *cp == '.')
		cp++;
	return(cp-1);
}

dumpedges(edges)
char	*edges;
{
	FILE		*fp;
	struct	site	*fst, *sst, *find();
	char		line[50], *cp;

	fp = fopen(edges, "r");
	while (fgets(line, 50, fp)) {
		if (cp = index(line, '\n'))
			*cp = '\0';
		if (cp = index(line, '>')) {
			*cp++ = '\0';
			if ((fst = find(line)) && (sst = find(cp)))
				report(fst, sst);
		}
	}
	fclose(fp);
}

struct	site *
find(sname)
char	*sname;
{
	unsigned	hkey, chr;
	char		*cp;

	hkey = 0;
	for (cp = sname; chr = *cp; cp++) {
		hkey <<= 3;
		hkey += chr;
	}
	hkey %= HSIZE;
	while (Htab[hkey]) {
		if (!strcmp(Htab[hkey]->sname, sname))
			return(Htab[hkey]);
		if (++hkey == HSIZE)
			hkey = 0;
	}
	return(NULL);
}

report(f, d)
struct	site	*f, *d;
{
	if (f->lon == d->lon && f->lat == d->lat)
		return;
	if (f->lat > d->lat)
		printf("%g\t%g\t%g\t%g\n", f->lat, f->lon, d->lat, d->lon);
	else
		printf("%g\t%g\t%g\t%g\n", d->lat, d->lon, f->lat, f->lon);
}
