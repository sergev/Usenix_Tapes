# Given the UUCP/USENET map, generate a USENET edge list
#
/^#N[	 ]/{
	system = $2;
}
/^#U[	 ]/{
	if (NF > 1) {
		for(x = 2; x <= NF; x++) {
			printf("%s>%s\n", system, $(x));
		}
	}
}
