#
# define CR	13
# define NL	10

main(argc,argv)
int argc;
char *argv[]; {
	int id, oid;
	char buf[512];
	char *temp, *index();
	char *end = "\r\n";
	int ln, cnt;
	register char *ptr, *od;
	register i, count;

	for (i = 1; i < argc; i++) {
		if ((id = open(argv[i],0)) < 0) {
			printf("CAN NOT OPEN FILE `%s'. . . . .\n",argv[i]);
			break;
		} else {
			temp = "tempXXXXXX";
			mktemp(temp);
			oid = creat(temp,0644);
			
			ptr = buf;
			while (ln = read(id,ptr,512)) {
				count = 0;
				while ( od = index(ptr,CR) ) {
					if (od > &buf[ln]) break;
					*od = '\0';
					cnt = strlen(ptr);
					count += cnt;
					write(oid, ptr, cnt);
					write(oid, end, 2);
					ptr = ++od;
					count++;
				}
				if (count = ln - count)
					write(oid,ptr,count);
				ptr = buf;
			}

			close(oid); close(id);
			strcpy(buf,"mv ");
			strcat(buf,temp);
			strcat(buf," ");
			strcat(buf,argv[i]);
			system(buf);
			unlink(temp);
		}
	}
}
