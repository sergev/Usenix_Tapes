getchar()
{
 char *p, *con, c;

 con = 0100010;	/* 8008H */
 p = 0100011;   /* 8009H */

 while((*con & 01) == 0);
 c = *p & 0177;
 if (c == '\r') c = '\n';
 putchar(c);
 return(c);
}
