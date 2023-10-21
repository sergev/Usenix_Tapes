/*
 * RELATIVE TO ABSOLUTE COORDINATES
 */
#include <stdio.h>

main() { 
	int cx = 0, cy = 0, x1, y1, x2, y2; 
	while (scanf("%d %d %d %d\n", &x1, &y1, &x2, &y2) == 4) { 
		cx += x1; 
		cy += y1; 
		printf("%d %d ", cx, cy); 
		cx += x2; 
		cy += y2; 
		printf("%d %d\n", cx, cy); 
	} 
}
