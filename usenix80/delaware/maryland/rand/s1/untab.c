struct buf
{	int fildes;
	int nleft;
	char *nextp;
	char buffer[512];
} ibuf, obuf;
int posit {0};

main(argc, argv)
int argc;
char **argv;
{	ibuf.fildes = ibuf.nleft = obuf.nleft = 0;
	ibuf.nextp = ibuf.buffer;
	obuf.fildes = 1;
	obuf.nextp = obuf.buffer;
	if (argc <= 1)
		pfile();
	else
	{	while (argc-- > 1)
			if ((ibuf.fildes = open(*++argv, 0)) != -1)
			{	pfile();
				close(ibuf.fildes);
			}
	}
}

pfile()
{	register int pos, nspaces;
	register int c;

	pos = posit;
	while ((c = getc(&ibuf)) != -1)
	{	if (c=='\t')
		{	nspaces = 8 - (pos&07);
			pos =+ nspaces;
			while (nspaces--) putc(' ', &obuf);
		}
		else
		{	putc(c, &obuf);
			if(c=='\n' || c=='\r') pos=0;
			else ++pos;
		}
	}
	fflush(&obuf);
}
