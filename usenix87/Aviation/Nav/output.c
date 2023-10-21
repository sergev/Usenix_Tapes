double Distance ();
double Bearing ();
double Magnetic ();
double Degrees();
double Radians ();

/*
 *	Output formatting and stuff
 */
#include "nav.h"

extern int comflg;
extern int rviaflg;

comment(s)
char *s;
{
	register char *p;

	for (p=s; *p != 0; p++)
		if (*p == '\n')
			*p = 0;
	if (comflg && strlen(s))
		printf("Info: %s \n",s);
}

PrintWp(vortac)
struct vor *vortac;
{
	printf("WayPoint: %s NM\n", vortac->waypoint);
}


PrintVor(vortac)
struct vor *vortac;
{
	if (rviaflg) 
        {
           printf("\nRvor: %s\t\tFreq: %.1f\n", vortac->name,vortac->freq);
           printf("ID: %s\tAltitude: %d feet\n",
		  vortac->id,vortac->alt);
           PrintWp (vortac);
        }
        else
        {
           printf("\nvor %s\t\tfreq: %.1f\n", vortac->name,vortac->freq);
           printf("ID: %s\tAltitude: %d feet\n",
		   vortac->id,vortac->alt);
        }
	comment(vortac->comments);

}

PrintApt(airport)
struct apt *airport;
{
	printf("\n\nAirport: %s\tCity: %s\n", airport->name, airport->city);
	printf("ID: %s\tAltitude: %d feet\n",
		airport->id,airport->alt);
	comment(airport->comments);
}

PrintNav(FromId,ToId,Course,Mag,Dist,TotalDist)
char *FromId, *ToId;
double Course,Mag,Dist,TotalDist;
{
	if (Dist > 0.01){	/* there is some fudge factor here */
		printf("\t\t%s to %s\nMagnetic:\t\t\t\t\t%03.0f\n",
			FromId, ToId, Mag);
		printf("Distance:\t\t\t\t\t%.1f NM\n",
			Dist);
		printf("(true course %03.0f, Total distance %.1f)\n",
			Course,TotalDist);
	}else{	/* special cased since flying 90 deg for 0 NM is silly */
		printf("\n%s is at %s\n",
			FromId, ToId);
		printf("(Total distance %.1f)\n",
			TotalDist);
	}
}

/*
 * Print a reasonable flight plan guide
 *
 */
PrintLine ()
{
char dashes[10];
   strcpy (dashes, "--------");
   printf ("+-%8s%8s+-----+-------+-------+-----+-----+-------+%8s%8s-+\n",
          dashes,dashes,dashes,dashes);
} 
PrintHead ()
{
char from_name[17], to_name[17], from_id[17], to_id[17];
   strcpy (from_name, "  FROM         ");
   strcpy (from_id,   "  FREQ/ID/ALT  ");
   strcpy (to_name,   "  TO           ");
   strcpy (to_id,     "  FREQ/ID/ALT  ");  
   printf ("\n");
   PrintLine();
   printf ("| %-16s| CRS | Wind  |  Leg  | TAS | Rad | Dist  |%16s |\n",
           from_name, to_name);
   printf ("| %-16s| HDG | Alt.  |  Time | GS  | NM  | Time  |%16s |\n",
           from_id, to_id);
   PrintLine();
}

PrintLeg (crs, mag, dist, total, rad, nm, tas, alt, wind, dir, gs, elapse,
          from_name, from_id, from_freq, from_alt,
          to_name, to_id, to_freq, to_alt)
int tas, alt, wind, dir, gs, *elapse, from_alt, to_alt;
double crs, mag, dist, total, rad, nm, from_freq, to_freq;
char *from_name[], *from_id[], *to_name[], *to_id[];
{ int i, t1, t2, t3, t4, w1;

    t1 = 0; t2 = 0; t3 = 0; t4 = 0; 
    if (gs == 0) gs = tas;
    if (gs != 0)
    {
       t1 = (int) ((dist / gs) * 60);
       *elapse += t1 ;
       t2 = t1 % 60;
       t1 = t1 / 60;
       t4 = *elapse % 60;
       t3 = *elapse / 60;
    }
    printf("| %-16s| %03.0f |%3d @%2d| %5.1f | %3d | %03.0f | %5.1f | %-16s|\n",
             from_name, crs, dir, wind, dist, tas, rad, total, to_name);
    printf("| %5.1f/%4s/%-5d| %03.0f | %5d |  %01d:%02d | %3d |%4.1f | %02d:%02d | %5.1f/%4s/%-5d|\n",
             from_freq, from_id, from_alt, mag, alt, t1,t2, 
             gs, nm, t3, t4, to_freq, to_id, to_alt);
    PrintLine();

}
FlightGuide(Start_Airport, To_Airport, 
            fromflg, toflg,
            altflg, AltArray, 
            speedflg, SpeedArray,
            windflg, WindArray,
            viaflg, rviaflg, ViaC, ViaArray)
struct apt *Start_Airport, *To_Airport;
int fromflg, toflg,
    altflg, AltArray[],
    speedflg, SpeedArray[],
    windflg, WindArray[],
    viaflg, rviaflg, ViaC;
struct vor ViaArray[];
{
int c, last_alt, to_alt, v;
int TAS, ALT, WIND, DIR, GS, ELAPSE;
double Radial, Course, CRS, HDG, Mag, NM, 
       Dist, TotalDist, last_var, last_freq, to_freq;
char last_name[NAMLEN], to_name[NAMLEN];
char last_id[IDLEN], to_id[IDLEN];
struct fix last_fix;
 
        c = 0;
        v = 0;
        printf ("\n\n\n\t\t\tFlight Guide\n\n");
	if (fromflg != 0)
        {
           strcpy (last_id, Start_Airport->id);
           strcpy (last_name, Start_Airport->name);
           last_alt = Start_Airport->alt;
           last_fix = Start_Airport->loc;
           last_var = Start_Airport->var;
           last_freq = 0.0; 
        }
        else
        {  
           strcpy (last_id, ViaArray[v].id);
           strcpy (last_name, ViaArray[v].name);
           last_alt = ViaArray[v].alt;
           last_fix = ViaArray[v].loc;
           last_var = ViaArray[v].var;
           last_freq = ViaArray[v].freq;
           PrintHead ();
           v++;
        }
        Dist = 0.0; TotalDist = 0.0; Course = 0.0; Mag = 0.0;
        TAS = 0; ELAPSE = 0; GS = 0; 
        for (v = v; v <= ViaC; v++)
        {

           if ((v < ViaC) || ((v = ViaC) && (toflg == 0)))
           {
              if ((v % 5) == 0) PrintHead ();
              Dist = Distance (last_fix.lat.rad, last_fix.lon.rad,
                        ViaArray[v].loc.lat.rad, ViaArray[v].loc.lon.rad);
              Course = Bearing (last_fix.lat.rad, last_fix.lon.rad,
                        ViaArray[v].loc.lat.rad, ViaArray[v].loc.lon.rad);
              strcpy (to_name, ViaArray[v].name);
              strcpy (to_id, ViaArray[v].id);
              to_freq = ViaArray[v].freq;
              to_alt = ViaArray[v].alt;
           }
           else
           if (toflg != 0)
           {
              if ((v % 5) == 0) PrintHead ();
              Dist = Distance (last_fix.lat.rad, last_fix.lon.rad,
                             To_Airport->loc.lat.rad,To_Airport->loc.lon.rad);
              Course = Bearing (last_fix.lat.rad, last_fix.lon.rad,
                             To_Airport->loc.lat.rad,To_Airport->loc.lon.rad);
              strcpy (to_name, To_Airport->name);
              strcpy (to_id, To_Airport->id);
              to_freq = 0.0;
              to_alt = To_Airport->alt;
           }   
              TotalDist += Dist;
              if (rviaflg)
              {
                 Radial = ViaArray[v].loc.radial;
                 NM = ViaArray[v].loc.nm;
              }
              else
              {
                 Radial = 0.0;
                 NM = 0.0;
              }
              if (speedflg) 
                 TAS = SpeedArray[v] ;
              else TAS = 0;
              if (altflg) 
                 ALT = AltArray[v] ;
              else ALT = 0; 
              if (windflg) 
                 WIND = WindArray[(v*2)+1] ;
              else WIND = 0;
              if (windflg) 
                 DIR = WindArray[(v*2)] ;
              else DIR = 0;
              GS = 0;
              CRS = Magnetic (Course, last_var);
	      Mag = Course;
              if ((windflg != 0) && (speedflg != 0))
                WindCorrection (TAS, &Mag, WIND, DIR, &GS, last_var);
              HDG = Magnetic (Mag, last_var);
              PrintLeg (CRS, HDG, Dist, TotalDist, 
                        Radial, NM, TAS, ALT,
                        WIND, DIR, GS, &ELAPSE,
                        last_name, last_id, last_freq, last_alt,
                        to_name, to_id, to_freq, to_alt);
              strcpy (last_name, ViaArray[v].name);
              strcpy (last_id, ViaArray[v].id);
              last_fix = ViaArray[v].loc;
              last_alt = ViaArray[v].alt;
              last_var = ViaArray[v].var;
              last_freq = ViaArray[v].freq;
        }
/*	if (toflg != 0)
        {
           strcpy (to_name, To_Airport->name);
           strcpy (to_id, To_Airport->id);
           PrintHead (last_name, last_id, to_name, to_id); 
           last_alt = To_Airport->alt;
        }
*/
        printf ("\n\n");
        if (comflg != 0)
        {
           if (fromflg != 0)
              PrintApt (Start_Airport);
           for (v = 0; v < ViaC; v++)
              PrintVor (&ViaArray[v]);
           if (toflg !=0)
              PrintApt (To_Airport);
        }
}
dummy ()
{int i;
	i = 1;
}
