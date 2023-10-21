main()
{
	int s;


	printf("You should see two sets of duplicate lines\n\n");

	printf("phone: service unknown\n");
	s=phone("stuff_xyzzy",hostname());

	if(s<0) perror("phone");

	s=phone("ftp","non_existant");

	printf("phone: Host is unreachable\n");
	if(s<0) perror("phone");
}
