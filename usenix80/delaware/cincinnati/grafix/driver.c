int fin;
float deltx;
float delty;
main(argc,argv)
char **argv;
{
	char c;
	char s[50];
	int xi,yi,x0,y0,x1,y1,r,dx,n,i;
	int pat[256];
	while(argc-- > 1) {
		if(*argv[1] == '-')
			switch(argv[1][1]) {
			case 'x':
				deltx = atoi(&argv[1][2]) - 1;
				break;
			case 'y':
				delty = atoi(&argv[1][2]) - 1;
				break;
			}
		argv++;
	}
	fin = dup(0);
	close(0);
	openpl();
	while((c=getchar()) != 0){
		switch(c){
		case 'm':
			xi = getwd();
			yi = getwd();
			move(xi,yi);
			break;
		case 'l':
			x0 = getwd();
			y0 = getwd();
			x1 = getwd();
			y1 = getwd();
			line(x0,y0,x1,y1);
			break;
		case 't':
			gets(s);
			label(s);
			break;
		case 'e':
			erase();
			break;
		case 'p':
			xi = getwd();
			yi = getwd();
			point(xi,yi);
			break;
		case 'n':
			xi = getwd();
			yi = getwd();
			cont(xi,yi);
			break;
		case 's':
			x0 = getwd();
			y0 = getwd();
			x1 = getwd();
			y1 = getwd();
			space(x0,y0,x1,y1);
			break;
		case 'a':
			xi = getwd();
			yi = getwd();
			x0 = getwd();
			y0 = getwd();
			x1 = getwd();
			y1 = getwd();
			arc(xi,yi,x0,y0,x1,y1);
			break;
		case 'c':
			xi = getwd();
			yi = getwd();
			r = getwd();
			circle(xi,yi,r);
			break;
		case 'f':
			gets(s);
			linemod(s);
			break;
		case 'd':
			xi = getwd();
			yi = getwd();
			dx = getwd();
			n = getwd();
			for(i=0; i<n; i++)pat[i] = getwd();
			dot(xi,yi,dx,n,pat);
			break;
		}
	}
	closepl();
	return(0);
}
getwd(){
	int a;
	char *c;
	c = &a;
	*c++ = getchar();
	*c = getchar();
	return(a);
}
gets(s)
char *s;
{
	for( ; *s = getchar(); s++)
		if(*s == '\n')
			break;
	*s = '\0';
	return;
}
