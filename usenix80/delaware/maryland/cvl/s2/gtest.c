int t[2] {
	0124000,        /* SPD | SLFTST */
	0130000         /* LPA */
};
char buf[2];
main()
{
	register d;

	d = open("/dev/gr",1);

	write(1,"\nHit RETURN for each test\n",26);
	readnl(0,buf,2);
	write(d,t,4);
	write(1,"\007",1);
	readnl(0,buf,2);
	t[1]++;
	write(d,t,4);
	write(1,"\007",1);
	readnl(0,buf,2);
	t[1]++;
	write(d,t,4);
	write(1,"\007",1);
	readnl(0,buf,2);
	t[1]++;
	write(d,t,4);
	write(1,"\007",1);
}
