/*
 *	Convert Phantasia 3.3.1 characs file format to 3.3.1+
 *
 *	Written by Chris Robertson, 1985
 */

#include	<stdio.h>

main()
{
	struct	stats	    	/* player stats -- old format */
	{
		char    name[21];	/* name */
		char    pswd[9];	/* password */
		char    login[10];	/* login */
		double  x;	    	/* x coord */
		double  y;	    	/* y coord */
		double  exp;		/* experience */
		int 	lvl;	   	/* level */
		short   quk;		/* quick */
		double  str;		/* strength */
		double  sin;		/* sin */
		double  man;		/* mana */
		double  gld;		/* gold */
		double  nrg;		/* energy */
		double  mxn;		/* max. energy */
		double  mag;		/* magic level */
		double  brn;		/* brains */
		short   crn;		/* crowns */
		struct
		{
			short	type;
			short	duration;
		}   rng;	    	/* ring stuff */
		char    pal;		/* palantir */
		double  psn;		/* poison */
		short   hw;	     	/* holy water */
		short   amu;		/* amulets */
		char    bls;		/* blessing */
		short   chm;		/* charms */
		double  gem;		/* gems */
		short   quks;		/* quicksilver */
		double  swd;		/* sword */
		double  shd;		/* shield */
		short   typ;		/* character type */
		char    vrg;		/* virgin */
		short   lastused;	/* day of year last used */
		short   status;		/* playing, cloaked, etc. */
		short   tampered;	/* decree'd, etc. flag */
		double  scratch1, scratch2; /* var's for above */
		char    blind;		/* blindness */
		int		wormhole;  	/* # of wormhole, 0 = none */
		long    age;		/* age in seconds */
		short   degen;		/* age/2500 last degenerated */
		short   istat;		/* used for inter-terminal battle */
	} stat;

	struct	nstats    	/* player stats -- new format */
	{
		char    nname[21];	/* name */
		char    npswd[9];	/* password */
		char    nlogin[10];	/* login */
		double  nx;	    	/* x coord */
		double  ny;	    	/* y coord */
		double  nexp;		/* experience */
		int 	nlvl;	   	/* level */
		short   nquk;		/* quick */
		double  nstr;		/* strength */
		double  nsin;		/* sin */
		double  nman;		/* mana */
		double  ngld;		/* gold */
		double  nnrg;		/* energy */
		double  nmxn;		/* max. energy */
		double  nmag;		/* magic level */
		double  nbrn;		/* brains */
		short   ncrn;		/* crowns */
		struct
		{
			short	ntype;
			short	nduration;
		}   nrng;	    	/* ring stuff */
		char    npal;		/* palantir */
		double  npsn;		/* poison */
		short   nhw;	     	/* holy water */
		short   namu;		/* amulets */
		char    nbls;		/* blessing */
		short   nchm;		/* charms */
		double  ngem;		/* gems */
		short   nquks;		/* quicksilver */
		double  nswd;		/* sword */
		double  nshd;		/* shield */
		short   ntyp;		/* character type */
		char    nvrg;		/* virgin */
		short   nlastused;	/* day of year last used */
		short   nstatus;		/* playing, cloaked, etc. */
		short   ntampered;	/* decree'd, etc. flag */
		double  nscr1, nscratch2; /* var's for above */
		char    nblind;		/* blindness */
		int		nwormhole;  	/* # of wormhole, 0 = none */
		long    nage;		/* age in seconds */
		short   ndegen;		/* age/2500 last degenerated */
		short   nistat;		/* used for inter-terminal battle */
		short	lives;		/* number of lives lived */
	} newstat;

	FILE	*oldcharac, *newcharac;

	char	oldpfile[100];
	char	newpfile[100];

	strcpy(oldpfile,PATH/characs");
	strcpy(newpfile,PATH/newcharacs");
	if((oldcharac = fopen(oldpfile,"r")) == NULL)
	{
		fprintf(stderr,"Cannot open original character file!\n");
		exit(1);
	}
	if((newcharac = fopen(newpfile,"w")) == NULL)
	{
		fprintf(stderr,"Cannot create new character file!\n");
		exit(1);
	}
	while (fread((char *) &stat,sizeof(stat),1,oldcharac))
	{
		strcpy (newstat.nname, stat.name);
		strcpy (newstat.npswd, stat.pswd);
		strcpy (newstat.nlogin, stat.login);
		newstat.nx = stat.x;
		newstat.ny = stat.y;
		newstat.nexp = stat.exp;
		newstat.nlvl = stat.lvl;
		newstat.nquk = stat.quk;
		newstat.nstr = stat.str;
		newstat.nsin = stat.sin;
		newstat.nman = stat.man;
		newstat.ngld = stat.gld;
		newstat.nnrg = stat.nrg;
		newstat.nmxn = stat.mxn;
		newstat.nmag = stat.mag;
		newstat.nbrn = stat.brn;
		newstat.ncrn = stat.crn;
		newstat.nrng.ntype = stat.rng.type;
		newstat.nrng.nduration = stat.rng.duration;
		newstat.npal = stat.pal;
		newstat.npsn = stat.psn;
		newstat.nhw = stat.hw;
		newstat.namu = stat.amu;
		newstat.nbls = stat.bls;
		newstat.nchm = stat.chm;
		newstat.ngem = stat.gem;
		newstat.nquks = stat.quks;
		newstat.nswd = stat.swd;
		newstat.nshd = stat.shd;
		newstat.ntyp = stat.typ;
		newstat.nvrg = stat.vrg;
		newstat.nlastused = stat.lastused;
		newstat.nstatus = stat.status;
		newstat.ntampered = stat.tampered;
		newstat.nscr1 = stat.scratch1;
		newstat.nscratch2 = stat.scratch2;
		newstat.nblind = stat.blind;
		newstat.nwormhole = stat.wormhole;
		newstat.nage = stat.age;
		newstat.ndegen = stat.degen;
		newstat.nistat = stat.istat;
		newstat.lives = 0;
		fwrite((char *) &newstat,sizeof(newstat),1,newcharac);
	}
	fclose(oldcharac);
	fclose(newcharac);
}
