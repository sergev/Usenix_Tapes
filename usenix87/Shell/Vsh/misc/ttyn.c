/*
 * Replace ttyn() with logtty() call (faster).
 */
ttyn(fd)
{
	char *logtty();

	if (fd != 2)
		write(2, "ttyn() Botch\r\n", 13);
	return(*logtty());
}

