int fd, i, j, wds[2], colcnt 0, bitpos 0;
struct {
	int     width,
		pointer,
		code,
		rastr_width,
		rastr_length,
		left_overlap,
		rows_from_top,
		rastr_size,
		height,
		baseline;
	} chr;
char c;

main(argc, argv)
int argc;
char *argv[];
{

if (argc < 2) {
	printf("useage: descfnt <fontname> [ <sample character> ]\n");
	exit(1);
	}

if (((fd = open(argv[1], 0)) < 0)
	&& ((fd = open(stringf("/lib/fonts/%s", argv[1]), 0)) < 0)
	&& ((fd = open(stringf("%s.fnt", argv[1]), 0)) < 0)
	&& ((fd = open(stringf("/lib/fonts/%s.fnt", argv[1]), 0)) < 0)) {
		printf("can't open ``%s''\n", argv[1]);
		exit(1);
		}

if (argc == 2) {
	printf("characters defined in %s are:\n\n", argv[1]);

	for(i = 0; i < 128; i++) {
		if (read(fd, wds, 4) < 4) {
			printf("premature EOF on font file\n");
			exit(1);
			}

		if(wds[1]) {
			if (i < 040)
				printf("  ^%c", i + 0100);
			else
				printf("   %c", i);
			if (colcnt++ > 8) {
				putchar('\n');
				colcnt = 0;
				}
			}
		}

	read(fd, wds, 4);
	printf("\n\nheight = %d, baseline = %d\n\ndescription:\n", wds[0], wds[1]);
	while(read(fd, &c, 1) > 0 && c)
		putchar(c);

	printf("\n**\n\n");
	}

else {
	seek(fd, (argv[2][0] & 0177) * 4, 0);
	read(fd, &chr.width, 4);
	if(chr.pointer == 0) {
		printf("character ``%c'' is not defined in %s\n", argv[2][0], argv[1]);
		exit();
		}
	seek(fd, 512, 0);
	read(fd, &chr.height, 4);
	seek(fd, chr.pointer, 0);
	read(fd, &chr.code, 12);

	for(i = 0; i < chr.rastr_length; i++) {
		putchar('\t');
		for(j = 0; j < chr.rastr_width; j++)
			putchar(bit()? '*': i == chr.baseline - chr.rows_from_top? '_': ' ');
		putchar('\n');
		}
	}
}

bit() {
register int val;

if (bitpos == 0) {
	bitpos = 0200;
	read(fd, &c, 1);
	}

val = bitpos & c;
bitpos =>> 1;
return(val);
}
