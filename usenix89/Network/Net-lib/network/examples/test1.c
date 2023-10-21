main()
{
	int s,q;

	printf("attach\n");

	s=attach(useport(2501));

	printf("accept\n");
	q=answer(s);

	pkill(q);
	pkill(s);
}


