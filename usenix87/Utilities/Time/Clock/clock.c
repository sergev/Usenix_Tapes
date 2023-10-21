   
/*
 * Writen by Dan Heller one bored evening in Spring '85 */
 *    argv@sri-spam.arpa
 *  Clock.c
 *    12
 *
 * 9  +--3
 *    |
 *    6
*/
#include <sys/time.h>

main()
{
    long x;
    short pm;
    struct tm *T;

    pm = 0;
    time(&x);
    T = localtime(&x);

    if(T->tm_hour >= 12) pm = 1;
    if(T->tm_hour > 12) T->tm_hour -= 12;
    if(T->tm_hour == 0) T->tm_hour = 12;

    printf("---------\n");
    printf("|   12  |\n|  ");

    if((T->tm_hour == 9 && T->tm_min > 30) || (T->tm_hour == 10) || 
       (T->tm_hour == 11 && T->tm_min <= 30) || 
       (T->tm_min >= 48 && T->tm_min <= 54))
	putchar('\\');
    else putchar(' ');

    if((T->tm_hour == 11 && T->tm_min > 30) || (T->tm_hour == 12 && 
	T->tm_min <= 30) || (T->tm_min >= 55 || T->tm_min <= 5))
        putchar('|');
    else putchar(' ');

    if((T->tm_hour == 12 && T->tm_min > 30) || (T->tm_hour == 1) ||
       (T->tm_hour == 2 && T->tm_min <= 30) ||
       (T->tm_min >= 6 && T->tm_min <= 12))
        putchar('/');
    else putchar(' ');

    printf("  |\n|9");

    if((T->tm_hour == 8 && T->tm_min > 30) || 
       (T->tm_hour == 9 && T->tm_min <= 30) ||
       (T->tm_min >= 43 && T->tm_min <= 47))
	printf("--");
    else printf("  ");

    putchar('+');

    if((T->tm_hour == 2 && T->tm_min > 30) || 
       (T->tm_hour == 3 && T->tm_min <= 30) ||
       (T->tm_min >= 13 && T->tm_min <= 17))
	printf("--");
    else printf("  ");

    printf("3|\n|  ");

    if((T->tm_hour == 6 && T->tm_min > 30) || (T->tm_hour == 7) ||
       (T->tm_hour == 8 && T->tm_min <= 30) ||
       (T->tm_min >= 36 && T->tm_min <= 42))
	putchar('/');
    else putchar(' ');

    if((T->tm_hour == 5 && T->tm_min > 30) ||
       (T->tm_hour == 6 && T->tm_min <= 30) ||
       (T->tm_min >= 25 && T->tm_min <= 35))
        putchar('|');
    else putchar(' ');

    if((T->tm_hour == 3 && T->tm_min > 30) || (T->tm_hour == 4) ||
       (T->tm_hour == 5 && T->tm_min <= 30) ||
       (T->tm_min >= 18 && T->tm_min <= 24))
        putchar('\\');
    else putchar(' ');

    printf("  |\n|   6   |\n---------\n");
    printf("%d:%02d %cm\n", T->tm_hour, T->tm_min, (pm ? 'p' : 'a'));
}
