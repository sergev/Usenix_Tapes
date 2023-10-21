extern  int     sigaddr[];

sigsave()
{
	sigaddr[0] = signal(1, 1);
        sigaddr[1] = signal(2, 1);
        sigaddr[2] = signal(3, 1);
}

sigrest()
{
	signal(1, sigaddr[0]);
        signal(2, sigaddr[1]);
        signal(3, sigaddr[2]);
}
