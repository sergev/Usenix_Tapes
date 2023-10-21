#include <stdio.h>
#include <math.h>
#include "nav.h"

double Distance();
double Bearing();
double Magnetic();
double Degrees();
double Radians();
double LatIntercept();

int comflg;
int rviaflg;
int altflg;
int windflg;
int speedflg;

main(ac,av)
int ac;
char **av;
{
	struct apt AptArray[MAXAPTS];
	struct vor VorArray[MAXVORS];
	struct vor ViaArray[MAXVIA];
        int AltArray[MAXVORS];
        int WindArray[MAXVORS*2];
        int SpeedArray[MAXVORS];
	int AltC, AptC, SpeedC, VorC, ViaC, WindC;
	FILE *fpa, *fpv;
	struct apt From, To;
	double Dist, TotalDist, Course, WpLat, WpDist, WpRad;
        double A, B, C, WpLon;
        int AltDiff;
	char sysbuf[BUFSIZ];

	struct fix LastFix;
	double LastVar;
	char LastId[NAMLEN];

	register int v;
	register int i;
	int  fromflg, toflg, viaflg;

	comflg = 0;
	fromflg = 0;
	toflg = 0;
	viaflg = 0;
	rviaflg = 0;
        altflg = 0;
        windflg = 0;
        speedflg = 0;

	fpa = fopen(AIRPORTS,"r");
	if (fpa == NULL){
		perror(AIRPORTS);
		exit(1);
	}
	fpv = fopen(VORS,"r");
	if (fpv == NULL){
		perror(VORS);
		exit(1);
	}
	AptC = ParseApt(AptArray,fpa);
	if (! AptC){
		oops("Trouble parsing airports");
	}
	fclose(fpa);
	VorC = ParseVor(VorArray,fpv);
	if (! VorC){
		oops("Trouble parsing VORs");
	}
	fclose(fpv);

	ac--;av++;
	while(ac){
		if (strcmp(*av,"-from") == 0){
			ac--;av++;
			if(!ac || **av == '-')
				usage("from: Missing departure ID");
			if (fromflg)
				usage("from: Two departure points?");
			if (FindApt(&From,*av,AptArray,AptC) != 0)
				oops("Couldn't find your departure airport");
			fromflg++;
			ac--;av++;
		}else if (strcmp(*av,"-to") == 0){
			ac--;av++;
			if(!ac || **av == '-')
				usage("to: Missing destination ID");
			if (toflg)
				usage("to: Two destinations?");
			if (FindApt(&To,*av,AptArray,AptC) != 0)
				oops("Couldn't find your destination airport");
			toflg++;
			ac--;av++;
		}else if (strcmp(*av,"-via") == 0){
			ac--;av++;
			if(!ac || **av == '-')
				usage("via: Missing VOR ID(s)");
			if (viaflg)
				usage("via: One set of 'via' VORs please");
			for(ViaC=0;
			ac && (**av != '-') && (ViaC < MAXVIA);
			ViaC++){
				if (FindVor(&ViaArray[ViaC],*av,VorArray,VorC)
				!= 0){
					fprintf(stderr,
						"via: %s: VOR not found\n",*av);
					exit(1);
				}
				ac--;av++;
			}
			viaflg++;
		}else if (strcmp(*av,"-rvia") == 0){
			ac--;av++;
			if(!ac || **av == '-')
				usage("rvia: Missing VOR ID(s)");
			if (rviaflg)
				usage("rvia: One set of 'rvia' VORs please");
			for(ViaC=0;
			ac && (**av != '-') && (ViaC < MAXVIA);
			ViaC++){
				if (FindVor(&ViaArray[ViaC],*av,VorArray,VorC)
				!= 0){
					fprintf(stderr,
						"rvia: %s: VOR not found\n",*av);
					exit(1);
				}
				ac--;av++;
			}
			rviaflg++;
		}else if (strcmp(*av,"-alt") == 0){
			ac--;av++;
			if(!ac || **av == '-')
				usage("alt: Missing ALTITUDE ");
			if (altflg)
				usage("alt: One set of ALTITUDEs please");
			for(AltC=0;
			ac && (**av != '-') && (AltC < MAXVIA);
			AltC++){
				if (StoreAlt(AltArray,AltC,*av)
				!= 0){
					fprintf(stderr,
						"alt: Altitude not found\n",*av);
					exit(1);
				}
				ac--;av++;
			}
			altflg++;
		}else if (strcmp(*av,"-wind") == 0){
			ac--;av++;
			if(!ac || **av == '-')
				usage("wind: Missing WINDS ALOFT ");
			if (windflg)
				usage("wind: One set of WINDS ALOFT please");
			for(WindC=0;
			ac && (**av != '-') && (WindC < MAXVIA);
			WindC=WindC+2){
				if (StoreWind(WindArray,&WindC,*av)
				!= 0){
					fprintf(stderr,
						"wind: WINDS ALOFT error\n",*av);
					exit(1);
				}
				ac--;av++;
			}
			windflg++;
		}else if (strcmp(*av,"-speed") == 0){
			ac--;av++;
			if(!ac || **av == '-')
				usage("speed: Missing AIRSPEED ");
			if (speedflg)
				usage("speed: One set of AIRSPEEDs please");
			for(SpeedC=0;
			ac && (**av != '-') && (SpeedC < MAXVIA);
			SpeedC++){
				if (StoreSpeed(SpeedArray,SpeedC,*av)
				!= 0){
					fprintf(stderr,
						"speed: AIRSPEED not found\n",*av);
					exit(1);
				}
				ac--;av++;
			}
			speedflg++;
		}else if (strcmp(*av,"-com") == 0){
			comflg++;
			ac--;av++;
		}else if (strcmp(*av,"-airports") == 0){
			sprintf(sysbuf,"/usr/ucb/more %s",AIRPORTS);
			system(sysbuf);
			exit(0);
		}else if (strcmp(*av,"-vors") == 0){
			sprintf(sysbuf,"/usr/ucb/more %s",VORS);
			system(sysbuf);
			exit(0);
		}else if (strcmp(*av,"-k") == 0){
			ac--;av++;
			if (!ac) usage("-k: missing keyword or pattern");
			sprintf(sysbuf,"/bin/grep -i %s %s",
				*av,AIRPORTS);
			system(sysbuf);
			sprintf(sysbuf,"/bin/grep -i %s %s",
				*av,VORS);
			system(sysbuf);
			exit(0);
		}
		else
/*
 *	Default action here
 */
			usage("Plan a flight...");
	}

	if	( (fromflg && !(toflg || viaflg))	||
		  ( toflg && !(fromflg || viaflg))	||
		  (viaflg && rviaflg)			||
		  (rviaflg && !(fromflg && toflg))	||
                  (windflg && !(speedflg))	        ||
		  ((!fromflg && !toflg) && !viaflg) )
		usage("Plan a flight...");
/*
 *	Now that we are done loading the data into the structs
 *	and parsing the command line, we can have some fun.
 */

	TotalDist = Dist = 0.;
	if(fromflg){
		LastFix = From.loc;
		LastVar = From.var;
		strcpy(LastId,From.id);
	}
	if (rviaflg) {
	   viaflg = rviaflg;
	   for (v=0; v < ViaC; v++) 
              if (ViaArray[v].alt != 0) {
           
                /* Compute a point that lies on the Great Circle Route
                   at the same longitude as the vor that we are using
                */
			WpLat =
			   LatIntercept(From.loc.lat.rad, From.loc.lon.rad,
					To.loc.lat.rad, To.loc.lon.rad,
					ViaArray[v].loc.lon.rad);

                /* Compute the slope of the line from From to the
                    point that is on the Great Circle Route
                */
                        B = ViaArray [v].loc.lon.rad - From.loc.lon.rad;
                        A = -(WpLat - From.loc.lat.rad);
                        C = -((A*From.loc.lon.rad)+(B*From.loc.lat.rad));

                /* Compute the coordinates of a point on the Great
                    Circle Route that is perpendicular to the VOR
                */
                        WpLon = (((B*B)*ViaArray[v].loc.lon.rad) -
                                 (A*B*ViaArray[v].loc.lat.rad) - A*C) /
                                 ((A*A)+(B*B));
                        WpLat = -(((A*B*ViaArray[v].loc.lon.rad) -
                                   ((A*A)*ViaArray[v].loc.lat.rad) + B*C) /
                                   ((A*A)+(B*B)));

                 /* Compute the distance from the VOR to the point
                     that lies on the Great Circle Route
                 */
                        WpDist = Distance (ViaArray[v].loc.lat.rad,
                                           ViaArray[v].loc.lon.rad,
                                           WpLat, WpLon);
                        if (altflg) {
                            AltDiff = abs(ViaArray[v].alt - AltArray[v]);
                            AltDiff = AltDiff / (5280 * 1.15);
                            WpDist = sqrt((WpDist*WpDist) +
                                          (AltDiff*AltDiff));
                        }
                 /* Finally, compute the bearing from the VOR to
                     the point that will be our WayPoint.
                 */
                        WpRad  = Bearing (From.loc.lat.rad,
                                          From.loc.lon.rad,
                                          To.loc.lat.rad,
                                          To.loc.lon.rad) - 90.;
                        if (WpRad < 0.) WpRad = WpRad + 360.;
                        if (WpLon < ViaArray[v].loc.lon.rad) 
                            WpRad = WpRad+180.;
                        WpRad  = Magnetic (WpRad, ViaArray[v].var);

			ViaArray[v].loc.lat.rad = WpLat;
			ViaArray[v].loc.lat.deg = Degrees(WpLat);
                        ViaArray[v].loc.lon.rad = WpLon;
                        ViaArray[v].loc.lon.deg = Degrees(WpLon);
                        ViaArray[v].loc.radial  = WpRad;
                        ViaArray[v].loc.nm      = WpDist;  
			sprintf(ViaArray[v].waypoint,
				"%s R-%05.1f / %.1f",
				ViaArray[v].id, WpRad, WpDist);
			sprintf(ViaArray[v].id,
				"WP%d",
				v+1);
    	    }
	}
	if (viaflg){
		v=0;
		if (!fromflg){
			LastFix = ViaArray[v].loc;
			LastVar = ViaArray[v].var;
			strcpy(LastId,ViaArray[v].id);
			v++;
		}else{
			Dist = Distance(LastFix.lat.rad,LastFix.lon.rad,
			    ViaArray[v].loc.lat.rad,ViaArray[v].loc.lon.rad);
			Course= Bearing(LastFix.lat.rad,LastFix.lon.rad,
			    ViaArray[v].loc.lat.rad,ViaArray[v].loc.lon.rad);
			TotalDist += Dist;
			LastFix = ViaArray[v].loc;
			LastVar = ViaArray[v].var;
			strcpy(LastId,ViaArray[v].id);
			v++;
		}

	}

   for (v = 0; v <= ViaC; v++)
   {
       if (speedflg)
          if (SpeedArray[v] == 0)
             if (v > 0)
                SpeedArray[v] = SpeedArray[v-1];
       if (altflg)
          if (AltArray[v] == 0)
             if (v > 0)
                AltArray[v] = AltArray[v-1];
       if (windflg)
          if (WindArray[v*2] == 0)
             if (v > 0)
             {
                WindArray[v*2] = WindArray[(v-1)*2];
                WindArray[v*2+1] = WindArray[(v-1)*2+1];
             }
   }
   FlightGuide (&From, &To,
                fromflg, toflg,
                altflg, AltArray, 
                speedflg, SpeedArray,
                windflg, WindArray,
                viaflg, rviaflg, ViaC, ViaArray);
   

}

usage(s)
char *s;
{
	fprintf(stderr,"nav: %s\n\n",s);
	fprintf(stderr,
	"Usage: nav [-from airport_id -to airport_id] [-[r]via vor [vor...]] [-com]\n");
	fprintf(stderr,
	"(The order of the 'from', 'to' and '[r]via' parts may be arbitrary)\n");
	fprintf(stderr,
	"Or:\nnav -k string: grep a string from the databases ignoring case\n");
	fprintf(stderr,
	"nav -airports : print out the airport database\n");
	fprintf(stderr,
	"nav -vors : print out the vor database\n");
	fprintf(stderr,"\nUse the abbreviated IDs found in the databases.\n");
	exit(1);
}

oops(s)
char *s;
{
	fprintf(stderr,"nav: %s\n",s);
	exit(1);
}
