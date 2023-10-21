write(fildes,buf,num)
char *buf;
{
 int i;
 for (i=0; i<num; i++)
    {
     putchar(*buf);
     buf++;
    }
}
