
int tsts[]
{
	0124000,
	0130000,
	0130001,
};

main(argv, argc)
int argv, argc;
{
	register int fil;

	fil = open("/dev/gr", 1);
	while(1) {
		write(fil, tsts, sizeof tsts);
		sleep(5);
	}
}
