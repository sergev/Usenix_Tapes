#include <descrip.h>

main(argc,argv)
int argc;
char *argv[];
{
char input[32],output[32];
if(argc < 3) strcpy(output,"nl:"); else strcpy(output,argv[3]);
if(argc < 2) strcpy(input,"nl:"); else strcpy(input,argv[2]);
printf("cmd:%s in:%s out:%s\n",argv[1],input,output);
spawn(argv[1],input,output);
}

spawn(command,input,output)
char *command,*input,*output;
{
int ret;
$DESCRIPTOR(tmp1,command);
$DESCRIPTOR(tmp2,input);
$DESCRIPTOR(tmp3,output);
tmp1.dsc$w_length = strlen(command);
tmp2.dsc$w_length = strlen(input);
tmp3.dsc$w_length = strlen(output);
ret=lib$spawn(&tmp1,&tmp2,&tmp3,0,0,0,0,0,0,0);
lib$signal(ret);
}

