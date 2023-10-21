/*
 *	Defines and structs for nav navigation program
 */

#define AIRPORTS	"airports"
#define VORS		"vors"


#define pi (3.1415926535897932384626433)

#define MAXAPTS 512	/* Maximum number of airports in APTarray */
#define MAXVORS 512	/* Maximum number of VORs in VORarray */
#define MAXVIA 64	/* Maximum number of VORs in ViaArray */
#define NAMLEN 33	/* Size of name field + EOS */
#define COMLEN 65	/* Size of comment field + EOS */
#define IDLEN 6		/* Size of ID field + EOS */
#define WPLEN 65	/* Size of WP field + EOS */

struct lat_lon{
	double rad;	/* radians */
	double deg;	/* decimal degrees */
};
struct fix{
	struct lat_lon lat;
	struct lat_lon lon;
        double radial;
        double nm;
};
struct vor{
	char id[IDLEN];		/* Three letter ID */
	char name[NAMLEN];	/* Full name of VOR */
	struct fix loc;		/* Location of the VOR */
	double var;		/* Magnetic variation in decimal degrees */
	float freq;		/* Radio frequency */
	int alt;		/* Altitude of vor in feet */
                                /* vors = 0 vortacs != 0 */
	char comments[COMLEN];	/* Comments (optional) */
	char waypoint[WPLEN];	/* Waypoint (not in VORS entry) */
};
struct apt{
	char id[IDLEN];		/* Three letter ID */
	char city[NAMLEN];	/* Name of nearest city */
	char name[NAMLEN];	/* Name of airport */
	int alt;		/* Altitude of airport in feet */
	struct fix loc;		/* Location of the airport */
	double var;		/* Magnetic variation in decimal degrees */
	char comments[COMLEN];	/* Comments (optional) */
};
