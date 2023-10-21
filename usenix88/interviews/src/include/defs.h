#ifndef defs_h
#define defs_h

typedef enum { false, true} boolean;
typedef int Coord;

extern double inch, inches, cm, point, points;
static const int pixels = 1;

#define nil 0

inline int min (int a, int b) {
    return a < b ? a : b;
}

inline int max (int a, int b) {
    return a > b ? a : b;
}

inline int round (double x) {
    return x > 0 ? int(x+0.5) : -int(-x+0.5);
}

#endif
