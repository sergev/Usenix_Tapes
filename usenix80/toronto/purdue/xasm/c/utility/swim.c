	extern	fout;
main()
{

	register x,y,i;

	shell("stty nl");
	fout = dup(1);
	x = 10;	y = 12;
	putchar(26);		/* clear screen	*/
    while(x < 75){
	lsimov(x++,y);
	fish();
	fry();
	}
	lsimov(1,20);
	flush();
	shell("stty -nl");
}



lsimov(x,y)
	int x,y;
{
	putchar('\033');	putchar('=');
	putchar(y + 040);	putchar(x + 040);
}
fish()
{
	register char c;
	register i,j;

	char up,down,left,right;

	up = 013;	left = 010;	right = 014;	down = 012;

	for(i = 0;i != 5;i++)putchar(right);
	for(i = 0;i != 3;i++)putchar(up);
	printf("o%c%c%c",left,down,down);
	for(i = 0;i != 12;i++)putchar(left);
	printf("\\\\  \\\\\\\\  o");
	putchar(down);
	for(i = 0;i != 10;i++)putchar(left);
	printf(">>|   '<");
	putchar(down);
	for(i = 0;i != 9;i++)putchar(left);
	printf("//  '~~'%c%c",up,left);
}
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
