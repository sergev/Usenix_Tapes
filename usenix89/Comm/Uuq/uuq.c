

uuq.c:
44d43
< char *touch;
73,75d71
< 		case 't':
< 			touch = &argv[0][2];
< 			break;
84c80
< 	"usage: uuq [-l] [-h] [-ssystem] [-uuser] [-djobno] [-tjobno] [-rspooldir]\n");
---
> 	"usage: uuq [-l] [-h] [-ssystem] [-uuser] [-djobno] [-rspooldir]\n");
192,194d187
< 	} else if (touch) {
< 		if (strcmp(touch, p))
< 			return(0);
227,233d219
< 		if (touch) {
< 			if (W_TYPE[0] == 'S' && !index(W_OPTNS, 'c')) {
< 				utime(W_DFILE,NULL);
< 				fprintf(stderr, "Touching data file %s\n", W_DFILE);
< 			}
< 			continue;
< 		}
265,269d250
< 		exit(0);
< 	}
< 	if (touch) {
< 		utime(filename,NULL);
< 		fprintf(stderr, "Touching command file %s\n", filename);
uuq.1:
5c5
< \fBuuq\fR [-l] [-h] [-ssystem] [-uuser] [-djobno] [-tjobno] [-rsdir]
---
> \fBuuq\fR [-l] [-h] [-ssystem] [-uuser] [-djobno] [-rsdir]
40,43d39
< -t\fIjobno\fR
< Touch (update the modification time of) files associated with
< job number \fIjobno\fR (as obtained from a previous uuq command).
< .TP 10
59,61d54
< .br
< The -t and -d options require write permission on the files involved and
< will silently fail without it.
-- 
