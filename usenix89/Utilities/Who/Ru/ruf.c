/*************************************************************/
/*                                                           */
/*  Copyright (c) 1986   Marc S. Majka                       */
/*                                                           */
/*  Permission is hereby granted to copy all or any part of  */
/*  this program for free distribution.   The author's name  */
/*  and this copyright notice must be included in any copy.  */
/*                                                           */
/*************************************************************/

#include <stdio.h>
#define ASIZE 256

main(argc,argv)
int argc;
char *argv[];
{
  char line[ASIZE], *host[ASIZE], *user[ASIZE], *tty[ASIZE];
  char ustr[64], htstr[64], hstr[64], tstr[64], *malloc();
  int nhost, nuser, nhuser[ASIZE], uhost[ASIZE], huser[ASIZE];
  int i, j, x, nh, ptty, debug, nlog, logs[ASIZE];

  ptty = 0;
  debug = 0;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] ==  '-') {
      switch (argv[i][1]) {
        case 't': ptty = 1; break;
        case 'd': debug = 1; break; /* for hacking */
        default: break;
      }
    }
  }

  nhost = 0;
  nuser = 0;
  for (i = 0; i < ASIZE; i++) {
    nhuser[i] = 0;
    uhost[i] = -1;
  }

  /* read lines from (we hope) rwho */
  while (EOF != scanf("%[^\n]%*c",line)) {

    /* get user and host:tty */
    sscanf(line,"%s%s",ustr,htstr);

    /* separate host and tty */
    for (i = 0; htstr[i] != ':'; i++) hstr[i] = htstr[i];
    hstr[i] = '\0';
    i++;

    /* knock off the "tty" part for brevity */
    if (!strncmp(htstr+i,"tty",3)) i += 3;
    strcpy(tstr,htstr+i);

    /* look through the host name list for this host */
    x = -1;
    for (i = 0; i < nhost; i++) if (!strcmp(host[i],hstr)) x = i;

    /* not found: put the new host on the list */
    if (x < 0) { 
      host[nhost] = malloc(strlen(hstr) + 1);
      strcpy(host[nhost],hstr);
      x = nhost;
      nhost++;
  }

    /* save the tty, user name and host number for the user */
    tty[nuser] = malloc(strlen(tstr) + 1);
    strcpy(tty[nuser],tstr);
    user[nuser] = malloc(strlen(ustr) + 1);
    strcpy(user[nuser],ustr);
    uhost[nuser] = x;
    nuser++;

    /* count number of users for this host */
    nhuser[x]++;
  }

  /* for each host ... */ 
  for (x = 0; x < nhost; x++) {

    /* collect a list of the users for this host (x) */
    nh = 0;
    for (i = 0; i < nuser; i++) if (uhost[i] == x) huser[nh++] = i;

    /* print host name and number of users */
    printf("%s (%d)\n  ",host[x],nh);

    /* for each user ... */ 
    for (i = 0; i < nh; i++) {

      /* count the number of logins for this user (i) */
      if (huser[i] >= 0) {
        nlog = 1;
        logs[0] = huser[i];

        /* look through the rest of the list for more logins */
        for (j = i+1; j < nh; j++) {
          if (huser[j] >= 0 && !strcmp(user[huser[i]],user[huser[j]])) {
            logs[nlog++] = huser[j];
            huser[j] = -1;
          }
        }

        /* print user name and number of logins */
        printf(" %s",user[huser[i]]);
        if (nlog > 1) printf("(%d)",nlog);

        /* print out terminals */
        if (ptty) {
          printf(":%s",tty[logs[0]]);
          for (j = 1; j < nlog; j++) printf(",%s",tty[logs[j]]);
        }
      }
    }

    printf("\n");
  }

  exit(0);
}
