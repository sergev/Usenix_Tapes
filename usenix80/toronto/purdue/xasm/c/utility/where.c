	char mes[] "You are \0                                       ";
	char wh[] "/usr/bin/where \0                            ";
main(argc,argv)
	int argc;	char **argv;
{
	register i,j,k;
	char s[100];
	if(argc > 2){
		shell("pwd");
		return;
		}
	cats(wh,argv[1]);
	shell(wh);
}
