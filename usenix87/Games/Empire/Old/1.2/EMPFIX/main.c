#define D_NATSTR
#define D_FILES
#include        "empdef.h"
#include        <stdio.h>

int     xflg, wflg, mflg;

main()
{
        register char   *cp;
        long    now;
        char    *getstri();

	if( privuid != myruid() ) {
		printf("%d != %d\n", myruid(), privuid);
		exit(1);
	}
        natf = open(natfil, O_RDWR);
	for( ;; ) {
		time(&now);
		lseek(natf, 0L, 0);
		if( read(natf, &nat, sizeof(nat)) < sizeof(nat) ) {
			printf("Too few bytes in nation #0...\n");
		}
		up_offset = nat.nat_up_off;
		printf("Curup is %.0f\n", now / 1800. - up_offset);
		cp = getstri("1:nation 2:sector 3:ship 4:treaty 5:loan  : ");
		switch( *cp ) {
		case '\0':
			execl("/bin/date", "", 0);
		case '1':
			nations();
			break;
		case '2':
			sectors();
			break;
		case '3':
			ships();
			break;
		case '4':
			treats();
			break;
		case '5':
			loans();
			break;
		default:
			printf("Huh?\n");
		}
        }
}
