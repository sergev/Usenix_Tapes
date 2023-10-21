/* findfile.c
 * Copyright 1985 Massachusetts Institute of Technology
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include "findfile.h"

int
findfileindir(area, name, mag, s, nname, nmag)
     char *area,*name,*s,*nname;
     int mag,*nmag;
{
    FILE *f;
    char buf[BUFSIZ];
    int found = 0;
    char *rindex();
    
    sprintf(s,"%s/SUBDIR",area);
    if (!access(s,0)) sprintf(s,"%s/%s/%s.%dpxl",area,name,name,mag);
    else sprintf(s,"%s/%s.%dpxl",area,name,mag);
    if (!access(s,4)) {
	strcpy(nname,name);
	*nmag = mag;
	return(-1);
    }
    else {
	/* look for the "pk" version */
	(void) sprintf(rindex(s, '.'), ".%dpk", (2*mag+5)/10);
	if (!access(s, 4)) {
	    (void) strcpy(nname, name);
	    *nmag = mag;
	    return (1);
	}
	else {
	    sprintf(buf,"%s/NEEDED",area);
	    if (!access(buf,2)) {
		sprintf(s,"%s.%dpxl\n",name,mag);
		f = fopen(buf,"r+");
		while (fgets(buf,sizeof(buf),f)) if (!strcmp(buf,s)) found++;
		if (!found) fputs(s,f);
		fclose(f);
	    }
	    return(0);
	}
    }
    
}


/* -1 for a pxl file, +1 for a pk file, otherwise 0; name in s */
int findfile(dirvec,dirveclen,area, name, mag, s, nname, nmag)
     char *dirvec[],*area,*name,*s,*nname;
     int dirveclen,mag,*nmag;
{
    int i,point;
    char family[128];

    strcpy(nname,name);
    *nmag = mag;
    point = -1;
    (void) sscanf(name,"%[^0123456789.]%d",family,&point);

    /* First check dir area given in DVI file */
    if (*area && (i = findfileindir(area, name, mag, s, nname, nmag)))
	return(i);
  
    /* Then check along dirvec */
    for (i = 0; i < dirveclen; i++) 
	if (i = findfileindir(dirvec[i], name, mag, s, nname, nmag))
	    return(i);

    /* next check for closest magnification along dirvec */
    return(findanyfile(dirvec,dirveclen,family,point,mag,name,s,nname,nmag));
}
  
int strdiff(s1,s2)
     char *s1,*s2;
{
  register int diff = 0;

  while (*s1 && *s2) diff += abs(*s1++ - *s2++);
  while (*s1) diff += *s1++;
  while (*s2) diff += *s2++;
  return(diff);
}

scanpdir(dir,name,
	 family,point,mag,
	 bestfamily,bestname,bestpoint,bestmag,
	 min_df,min_dp,min_dm)
     char *dir,*name,*family,*bestfamily,*bestname;
     int point,mag,*bestpoint,*bestmag,*min_df,*min_dp,*min_dm;
{
  DIR *dirstream;
  struct direct *dirrecord;
  char qfamily[128];
  char type[4];
  int qpoint,qmag,df,dp,dm;

  if (dirstream = opendir(dir)) {
    while (dirrecord = readdir(dirstream)) {
	if (!strcmp(dirrecord->d_name+dirrecord->d_namlen-3,"pxl")
	    || !strcmp(dirrecord->d_name+dirrecord->d_namlen-2,"pk")) {
	qpoint = -1; qmag = -1;
	(void) sscanf(dirrecord->d_name,"%[^0123456789.]%d.%d%3s",
		      qfamily,&qpoint,&qmag,type);
	if (!strcmp(type, "pk"))
	    qmag *= 5;
	df = strdiff(family,qfamily);
	dp = abs(point - qpoint);
	dm = abs(mag - qmag);
	if ((df < *min_df)
	    || (df == *min_df && dp < *min_dp)
	      || (df == *min_df && dp == *min_dp && dm < *min_dm)) {
		sprintf(bestname,"%s/%s",dir,dirrecord->d_name);
		strcpy(bestfamily,qfamily);
		*bestpoint = qpoint;
		*bestmag = qmag;
		*min_df = df;
		*min_dp = dp;
		*min_dm = dm;
	  }
      }
    }
    closedir(dirstream);
  }
}

scandir(dir,name,
	family,point,mag,
	bestfamily,bestname,bestpoint,bestmag,
	min_df,min_dp,min_dm)
     char *dir,*name,*family,*bestfamily,*bestname;
     int point,mag,*bestpoint,*bestmag,*min_df,*min_dp,*min_dm;
{
  DIR *dirstream;
  struct direct *dirrecord;
  int df;
  char pdir[MAXNAMLEN];

  if (dirstream = opendir(dir)) {
    while (dirrecord = readdir(dirstream)) {
      if (dirrecord->d_name[0] != '.') {
	df = strdiff(name,dirrecord->d_name);
	if (df <= *min_df) {
	  sprintf(pdir,"%s/%s",dir,dirrecord->d_name);
	  scanpdir(pdir,name,
		   family,point,mag,
		   bestfamily,bestname,bestpoint,bestmag,
		   min_df,min_dp,min_dm);
	}
      }
    }
    closedir(dirstream);
  }
}


/* finds the best match to the desired font */
int findanyfile(dirvec,dirveclen,family,point,mag,name,s,nname,nmag)
     char *dirvec[],*family,*name,*s,*nname;
     int dirveclen,point,mag,*nmag;
{
  char foo[MAXNAMLEN],bestname[MAXNAMLEN],bestfamily[128];
  int min_df,min_dp,min_dm,df,dp,dm,i,bestpoint,bestmag;
  
  bestname[0] = '\0'; 
  min_df = min_dp = min_dm = 9999999;
  for (i = 0; i < dirveclen; i++) {
    sprintf(foo,"%s/SUBDIR",dirvec[i]);
    if (!access(foo,0)) scandir(dirvec[i],name,
				family,point,mag,
				bestfamily,bestname,&bestpoint,&bestmag,
				&min_df,&min_dp,&min_dm);
    else scanpdir(dirvec[i],name,
		  family,point,mag,
		  bestfamily,bestname,&bestpoint,&bestmag,
		  &min_df,&min_dp,&min_dm);
  }
  if (bestname[0]) {
    if (bestpoint > 0) sprintf(nname,"%s%d",bestfamily,bestpoint);
    else strcpy(nname,bestfamily);
    *nmag = bestmag;
    strcpy(s,bestname);
    if ((strcmp(bestfamily,family)
	 || bestpoint != point || abs(bestmag - mag) > 2)) 
      fprintf(stderr,
	      "Substituted font %s at mag %d for %s at mag %d.\n",
	      nname,(bestmag * 4 + 3) / 6,
	      name,(mag * 4 + 3) / 6);
    return strcmp(&bestname[strlen(bestname)-3], "pxl") ? (1) : (-1);
  }
  return(0);
}
  
