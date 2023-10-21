#include <curses.h>
#include <time.h>
#include <math.h>
#include <signal.h>

struct tm *localtime();

#define excursion

#define ltoyl(x)		(x)
#define ltoxl(x)		((x) * 7.0 / 3.0)

#define C_X		40.0
#define C_Y		12.0
#define L_Y		ltoyl(10.0)
#define L_X		ltoxl(10.0)
#define S_HR		0.0
#define L_HR		4.0
#define S_MIN		0.0
#define L_MIN		9.0
#define S_SEC		7.0
#define L_SEC		2.0
#define S_TICK		10.0

#define HR_HAND		"@"
#define MIN_HAND	"*"
#define SEC_HAND 	"."
#define CENTER		"#\b"

#define QUANTUM		10	/* # seconds between redraws */

double rad = 3.14159265359 / 180.0;

int cleanup();
int byebye();

main() {
    long clock;
    struct tm *now;
    double angle, distance;
    extern int ospeed;

    initscr();
    signal(SIGINT, byebye);
    signal(SIGTERM, byebye);
    /* if you want to trap SIGTSTP, you can do it yourself!  --bsa */
    signal(SIGQUIT, cleanup);
    if (ospeed == 15)
        ospeed = 13;
    for (;;) {
        time(&clock);
        now = localtime(&clock);
        if (now->tm_hour >= 12)
            now->tm_hour -= 12;
        erase();
        excursion {
            double x, y;

            for (x = -L_X; x <= L_X; x += 0.1) {
                mvaddch(round(C_Y - L_Y), round(C_X + x), '-');
                mvaddch(round(C_Y + L_Y), round(C_X + x), '-');
            }
            for (y = -L_Y; y <= L_Y; y += 0.1) {
                mvaddch(round(C_Y + y), round(C_X - L_X), '|');
                mvaddch(round(C_Y + y), round(C_X + L_X), '|');
            }
            mvaddch(round(C_Y - L_Y), round(C_X - L_X), '+');
            mvaddch(round(C_Y - L_Y), round(C_X + L_X), '+');
            mvaddch(round(C_Y + L_Y), round(C_X + L_X), '+');
            mvaddch(round(C_Y + L_Y), round(C_X - L_X), '+');
        }
        standout();
        for (angle = 0; angle < 12; angle++) {
            char tick[3];

            sprintf(tick, "%.0f", (angle == 0.0? 12.0: angle));
            dot(S_TICK, angle * 360 / 12, tick);
        }
        for (distance = S_MIN; distance < S_MIN + L_MIN; distance += 0.25)
            dot(distance, (double) ((now->tm_min + now->tm_sec / 60) * 360 / 60), MIN_HAND);
        for (distance = S_HR; distance < S_HR + L_HR; distance += 0.25)
            dot(distance, (double) ((now->tm_hour + now->tm_min / 60) * 360 / 12), HR_HAND);
        for (distance = S_SEC; distance < S_SEC + L_SEC; distance += 0.25)
            dot(distance, (double) (now->tm_sec * 360 / 60), SEC_HAND);
        standend();
        mvaddstr(round(C_Y), round(C_X), CENTER);
        refresh();
        sleep(QUANTUM);
    }
}

round(x)
double x; {
    double rndx;
    int intx;
    
    rndx = x + 0.5;
    intx = (int) rndx;
    return intx;
}

dot(d, theta, str)
double d, theta; {
    int x, y;

    y = y_loc(d, theta);
    x = x_loc(d, theta);
    mvaddstr(y, x, str);
}

x_loc(d, theta)
double d, theta; {
    int x;
    double xl;

    xl = ltoxl(d);
    if (theta < 0.001)
        theta = 0;
    x = round(xl * sin(theta * rad));
    x += round(C_X);
    return x;
}

y_loc(d, theta)
double d, theta; {
    int y;
    double yl;

    yl = ltoyl(d);
    if (theta < 0.001)
        theta = 0;
    y = -round(yl * cos(theta * rad));
    y += round(C_Y);
    return y;
}

byebye() {
    standend();
    clear();
    refresh();
    endwin();
    system("stty echoe");
    exit(0);
}

cleanup() {
    clearok(stdscr, TRUE);
}
