/*
 * RawConsole.c
 *
 * Shell 2.06M	28-May-87
 * console handling, command line editing support for Shell
 * using new console packets from 1.2.
 * Written by Steve Drew. (c) 14-Oct-86.
 * 16-Dec-86 Slight mods to rawgets() for Disktrashing.
 *
 */

extern int aux; /* for use with aux: */

#include "shell.h"

void
setraw(onoff)
{
    if (onoff) set_raw();
    else set_con();
}

char *
rawgets(line,prompt)
char *line, *prompt;
{
    char *get_var();
    char *gets();
    register int n, pl;
    register int max, i;
    unsigned char c1,c2,c3;
    char fkeys[5];
    char *s;
    int fkey;
    int insert = 1;
    char rep[14];
    static int width;
    int recall = -1;
    struct HIST *hist;

    if (aux) {
	printf("%s",prompt);
	fflush(stdout);
    }
    if (!IsInteractive(Input()) || aux ) return(gets(line));
    if (WaitForChar((long)Input(), 100L) ||   /* don't switch to 1L ...*/
	   stdin->_bp < stdin->_bend) {	    /* else causes read err's*/
    /*	printf("%s",prompt); */
	gets(line);
	return(line);
    }
    setraw(1);
    printf("%s",prompt);
    max = pl = i = strlen(prompt);
    strcpy(line,prompt);
    if (!width) width = 77;
    if (s = get_var (LEVEL_SET, "_insert"))
	insert = atoi(s) ? 1 : 0;

    while((c1 = getchar()) != 255) {
	if (c1 < 156) switch(c1) {
	    case 155:
		 c2 = getchar();
		 switch(c2) {
		     case 'A':			/* up arrow   */
			n = ++recall;
		     case 'B':			/* down arrow */
			line[pl] = '\0';
			if (recall >= 0 || c2 == 'A') {
			    if (c2 == 'B') n = --recall;
			    if (recall >= 0) {
				for(hist = H_head; hist && n--;
				    hist = hist->next);
				if (hist) strcpy(&line[pl],hist->line);
				else recall = H_len;
			    }
			}
			if (i != pl)
			    printf("\233%dD",i);
			printf("\015\233J%s",line);
			i = max = strlen(line);
			break;
		     case 'C':			/* right arrow*/
			if (i < max) {
			    i++;
			    printf("\233C");
			}
			break;
		     case 'D':			/* left arrow */
			if (i > pl) {
			    i--;
			    printf("\233D");
			}
			break;
		    case 'T':			/* shift up   */
		    case 'S':			/* shift down */
			break;
		    case ' ':			/* shift -> <-*/
			c3 = getchar();
			break;
		    default:
			c3 = getchar();
			if (c3 == '~') {
			    fkey = c2;
			    fkeys[0] = 'f';
			    if (c2 == 63) {
				strcpy(&line[pl],"help");
				goto done;
			    }
			}
			else if (getchar() != '~') { /* window was resized */
			    while(getchar() != '|');
			    printf("\2330 q"); /* get window bounds */
			    n = 0;
			    while((rep[n] = getchar()) != 'r' && n++ < 14 );
			    width = (rep[n-3] - 48) * 10 + rep[n-2] - 48;
			    rep[n-1] = '\0';
			    set_var (LEVEL_SET, "_width", &rep[n-3]);
			    break;
			}
			else {
			    fkey = c3;
			    fkeys[0] = 'F';
			}
			sprintf(fkeys+1,"%d",fkey - 47);
			if (!(s = get_var(LEVEL_SET, fkeys))) break;
			strcpy(&line[pl], s);
			printf("%s",&line[pl]);
			goto done;
			break;
		    }
		break;
	    case 8:
		if (i > pl) {
		    i--;
		    printf("\010");
		}
		else break;
	    case 127:
		if (i < max) {
		    int j,t,l = 0;
		    movmem(&line[i+1],&line[i],max-i);
		    --max;
		    printf("\233P");
		    j = width - i % width - 1;	 /* amount to end     */
		    t = max/width - i/width;	 /* no of lines	      */
		    for(n = 0; n < t; n++) {
			l += j;			 /* no. of char moved */
			if (j) printf("\233%dC",j); /* goto eol	      */
			printf("%c\233P",line[width*(i/width+n+1)-1]);
			j = width-1;
		    }
		    if (t)
		    printf("\233%dD",l+t);   /* get back */
		}
		break;
	    case 18:
		n = i/width;
		if (n) printf("\233%dF",n);
		printf("\015\233J%s",line);
		i = max;
		break;
	    case 27:
	    case 10:
		break;
	    case 1:
		insert ^= 1;
		break;
	    case 21:
	    case 24:
	    case 26:
		if (i > pl)
		    printf("\233%dD",i-pl);
		i = pl;
		if (c1 == 26) break;
		printf("\233J");
		max = i;
		line[i] = '\0';
		break;
	    case 11:	    /* ^K */
		printf("\233J");
		max = i;
		line[i] = '\0';
		break;
	    case 28:	    /* ^\ */
		setraw(0);
		return(NULL);
	    case 5:
		printf("\233%dC",max - i);
		i = max;
		break;
	    case 13:
		line[max] = '\0';
done:		printf("\233%dC\n",max - i);

		setraw(0);
		strcpy(line, &line[pl]);
		return(line);
	    default:
		if (c1 == 9) c1 = 32;
		if (c1 > 31 & i < 256) {
		    if (i < max && insert) {
			int j,t,l = 0;
			movmem(&line[i], &line[i+1], max - i);
			printf("\233@%c",c1);
			t = max/width - i/width;
			j = width - i % width - 1;
			for(n = 0; n < t; n++) {
			    l += j;
			    if (j) printf("\233%dC",j);
			    printf("\233@%c",line[width*(i/width+n+1)]);
			    j = width-1;
			}
			if (t) printf("\233%dD",l + t);
			++max;
		    }
		    else {
			if (i == pl && max == i) printf("\015%s",line);
			putchar(c1);
		    }
		    line[i++] = c1;
		    if (max < i) max = i;
		    line[max] = '\0';
		}
	}
    }
    setraw(0);
    return(NULL);
}

