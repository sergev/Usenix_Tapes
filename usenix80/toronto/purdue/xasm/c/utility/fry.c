fry()
{
	register char c;
	register i,j;
	char up,down,left,right;

	up = 013;	left = 010;	right = 014;	down = 012;

	for(i = 0;i != 5;i++)putchar(right);
	for(i = 0;i != 3;i++)putchar(up);
	printf(" %c%c%c",left,down,down);
	for(i = 0;i != 12;i++)putchar(left);
	printf("           ");
	putchar(down);
	for(i = 0;i != 10;i++)putchar(left);
	printf("        ");
	putchar(down);
	for(i = 0;i != 9;i++)putchar(left);
	printf("        %c%c",up,left);
}
