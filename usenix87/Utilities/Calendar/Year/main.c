#include <stdio.h>


main()
{
    int days_in_year();
    int year;
    int country_code;
    
    while (1)
    {
	printf("\nEnter year and country code: ");
	if (scanf("%d %d", &year, &country_code) == EOF)
	    exit(0);

	printf("it has %d days\n", days_in_year(year, country_code));
    }
}
