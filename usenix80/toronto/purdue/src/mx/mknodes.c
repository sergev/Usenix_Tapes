main()
{
	char c;
	for(c=1 ; c<33 ;c++)
		printf("/etc/mknod \"/dev/mx/%c\" c 24 %d\n",c+'@',c);
}
