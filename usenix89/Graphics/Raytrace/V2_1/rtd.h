struct color {
int r;int g;int b;};

struct vector {
double x;
double y;
double z;
double l;
double xzl;} ;

struct ray {
struct vector org;
struct vector dir;} ;

struct sphere {
struct vector cent;
double rad;} ;

struct ball {
struct sphere s;
double ior;
double rfr;
double rfl;
double dif;
double amb;
};

struct mat {
struct vector x;  /* first !row! */
struct vector y;  /*second !row! */
struct vector z;}; /* third !row! */

