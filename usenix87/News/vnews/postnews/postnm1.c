/*
 * Main routine and error printing routines.
 *
 * Copyright 1984 by Kenneth Almquist.  All rights reserved.
 *     Permission is granted for anyone to use and distribute, but not
 *     sell, this program provided that this copyright notice is retained.
 */

#define EXTERN
#include "postnm.h"


int xxit();


main(argc, argv)
      char **argv;
      {
      int c;
      FILE *fp;
      extern char *optarg;
      extern int optind;

      pathinit();
      getuser();
      while ((c = getopt(argc, argv, "ivBwxc:d:f:n:R:s:t:")) != EOF) {
            switch (c) {
            case 'i':
                  signit = 1;
                  break;
            case 'v':
                  verbose = 1;
                  break;
            case 'B':
                  background = 1;
                  break;
            case 'w':
                  background = 0;
                  break;
            case 'x':
                  debug++;
                  background = 0;
                  break;
            case 'c':
                  sprintf(bfr, "Control: %s\n", optarg);
                  prochdr(bfr, 1);
                  break;
            case 'd':
                  sprintf(bfr, "Distribution: %s\n", optarg);
                  prochdr(bfr, 1);
                  break;
            case 'f':
                  sprintf(bfr, "From: %s\n", optarg);
                  prochdr(bfr, 1);
                  break;
            case 'n':
                  sprintf(bfr, "Newsgroups: %s\n", optarg);
                  prochdr(bfr, 1);
                  break;
            case 'R':
                  sprintf(bfr, "%s\n", optarg);
                  references = savestr(bfr);
                  break;
            case 's':
                  sprintf(bfr, "Subject: %s\n", optarg);
                  prochdr(bfr, 1);
                  break;
            case 't':
                  if (hdrline[HTTO] == NULL) {
                        sprintf(bfr, "To: %s\n", optarg);
                  } else {
                        nstrip(hdrline[HTTO]->text);
                        sprintf(bfr, "%s, %s\n", hdrline[HTTO]->text, optarg);
                        hlzap(HTTO);
                  }
                  prochdr(bfr, 1);
                  break;
            case '?':
                  exit(1);
            }
      }
      if (optind < argc && (infp = fopen(argv[optind], "r")) == NULL)
            xerror("%s: cannot open", argv[optind]);
      if (debug) {
            strcpy(newstemp, "news.tmp");
            strcpy(mailtemp, "mail.tmp");
      } else {
            sprintf(newstemp, "/tmp/pnm%05dn", getpid());
            sprintf(mailtemp, "/tmp/pnm%05dm", getpid());
      }

      readheader();

      checkheader();

      if (signal(SIGINT, SIG_IGN) != SIG_IGN)
            signal(SIGINT, xxit);
      if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
            signal(SIGHUP, xxit);

      if (addrp != addrlist) {
            mailfp = ckfopen(mailtemp, "w");
            genmailhdr(mailfp);
      }
      if (hdrline[HTNEWSGROUPS]) {
            newsfp = ckfopen(newstemp, "w");
            gennewshdr(newsfp);
      }

      if (newsfp == NULL && mailfp == NULL)
            xerror("No to or newsgroup line specified");
      if (nerrors)
            xxit(1);

      addbody(infp);
      if (signit) {
            getuser();
            sprintf(bfr, "%s/.signature", userhome);
            if ((fp = fopen(bfr, "r")) != NULL) {
                  if (mailfp)
                        fputs("--", mailfp);
                  if (newsfp)
                        fputs("-- ", newsfp);
                  addbody(fp);
            }
      }
      if (nerrors)
            xxit(1);

      if (background) {
            if (fork() > 0)
                  exit(0);
            signal(SIGINT, SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
            signal(SIGHUP, SIG_IGN);
#ifdef SIGTSTP
            signal(SIGTSTP, SIG_IGN);
#endif
      }
      if (mailfp) {
            char **pp;
            char *arg[6];

            fclose(mailfp);
            for (pp = addrlist ; pp < addrp ; pp++) {
                  arg[0] = MAILER;
                  arg[1] = *pp;
                  arg[2] = NULL;
                  if (runp(arg, mailtemp))
                        xerror("Mailer failed");
            }
      }
      if (newsfp) {
            char *arg[6];
            struct stat statb;

            fclose(newsfp);
            /* suppress .signature */
            if (userhome == NULL)
                  getuser();
            sprintf(signfile, "%s/.signature", userhome);
            if (stat(signfile, &statb) >= 0) {
                  chmod(signfile, 0);		/* This is sick! */
                  signmode = statb.st_mode;
            }
            if (moderator) {
                  arg[0] = MAILER;
                  arg[1] = moderator;
                  arg[2] = NULL;
                  if (runp(arg, newstemp))
                        xerror("Mail to moderator failed");
            } else {
                  arg[0] = XINEWS;
                  arg[1] = "-h";
                  arg[2] = NULL;
                  if (runp(arg, newstemp))
                        xerror("inews failed");
            }
            savearticle() ;
      }
      xxit(nerrors? 1 : 0);
}



/*
 * Print an error message for an illegal header line.
 */

synerr(fmt, a1, a2, a3, a4)
      char *fmt;
      {
      fprintf(stderr, "postnm: ");
      if (curhdr)
            fprintf(stderr, "line %d: ", curhdr->linno);
      fprintf(stderr, fmt, a1, a2, a3, a4);
      putc('\n', stderr);
      nerrors++;
}


jsynerr(fmt, a1, a2, a3, a4)
      char *fmt;
      {
      synerr(fmt, a1, a2, a3, a4);
      reset();
}



xerror(fmt, a1, a2, a3, a4)
      char *fmt;
      {
      fprintf(stderr, "postnm: ");
      fprintf(stderr, fmt, a1, a2, a3, a4);
      putc('\n', stderr);
      xxit(2);
}



xxit(status) {
      if (!debug) {
            unlink(newstemp);
            unlink(mailtemp);
      }
      if (signmode) {
            chmod(signfile, signmode);
      }
      exit(status);
}



runp(arglist, infile)
      char **arglist;
      char *infile;
      {
      int pid, status, fd;
      extern char *optarg;
      extern int optind;

      if (debug) {
            while (*arglist)
                  printf("%s ", *arglist++);
            if (infile)
                  printf("< %s", infile);
            putchar('\n');
            return 0;
      }
      if (infile && (fd = open(infile, 0)) < 0)
            xerror("Can't open %s", infile);
      if ((pid = fork()) < 0)
            xerror("Can't fork");
      if (pid == 0) {
            if (infile && fd != 0) {
                  close(0);
                  if (dup(fd) != 0)
                        fputs("postnm: dup failed\n", stderr);
                  close(fd);
            }
            execv(arglist[0], arglist);
            fprintf(stderr, "Can't exec %s\n", arglist[0]);
            exit(127);
      }
      close(fd);
      while (wait(&status) != pid);
      if (status & 0177) {
            fprintf(stderr, "postnm: %s died with signal %d", arglist[0], status & 0177);
            if (status & 0200)
                  fprintf(stderr, " - core dumped");
            putc('\n', stderr);
      } else if (status) {
            fprintf(stderr, "postnm: exit status %d from %s\n", (unsigned) status >> 8, arglist[0]);
      }
      return status;
}



/*
 * Save a copy of the article in the users NEWSARCHIVE file.
 * The article is saved only if the user explicitly sets NEWSARCHIVE.
 * Currently, we save USENET articles but not mail, which is
 * rather questionable.
 */
savearticle() {
      register FILE *in, *out;
      register int c;
      time_t timenow, time();
      char *today, *ctime();
      char *ccname;
      char *getenv();

      if ((ccname = getenv("NEWSARCHIVE")) == NULL || ccname[0] == '\0')
            return;
      if ((in = fopen(newstemp, "r")) == NULL) {
            xerror("Can't reopen %s", newstemp);
      }
      if ((out = fopen(ccname, "a")) == NULL) {
            xerror("Can't open %s to save article", ccname);
      }
      timenow = time((time_t *)0);
      today = ctime(&timenow);
      fprintf(out, "From postreply %s", today);
      while ((c=getc(in)) != EOF)
            putc(c, out);
      putc('\n', out);
      fclose(in);
      fclose(out);
}
