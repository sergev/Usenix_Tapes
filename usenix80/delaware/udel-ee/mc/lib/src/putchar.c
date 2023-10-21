putchar(c)
char c;
{
 char *p, *con;

 con = 0100010;
 p = 0100011;

 while((*con & 02) == 0);
 *p = c;
 if (c == '\n') putchar('\r');
}
