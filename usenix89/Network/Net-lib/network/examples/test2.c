main() { 
	int s;

	s=phone(useport(2501),hostname());
	printf("Done\n");

	pkill(s);
}

