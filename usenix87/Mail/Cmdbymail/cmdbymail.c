#include        <stdio.h>

char msgbuffer[1024000];

main(ac, av)
char **av;
{
        char    *trusted_user,
                *mail_file,
                *subject;

        if (ac == 1) {
                printf("doing settup\n");
                setup(av[0]);
                exit(0);
        }
        trusted_user = av[1];
        subject = av[2];
        mail_file = av[3];

        collect_message();
        if (command_message(trusted_user,subject))
                do_command();
        save_message(mail_file);
}

setup(pname)
{
        FILE *fd;
        register char *run;
        char ubuf[256], sbuf[256], mbuf[256];
        char fbuf[256];

        sprintf(fbuf, "%s/.forward", getenv("HOME"));
        if ((fd = fopen(fbuf, "w")) == NULL) {
                printf("cannot open .forward file for writing\n");
                exit(0);
        }

        printf("enter full mail address of trusted user: ");
        run = ubuf;
        for (*run = getchar(); *run != '\n'; *run = getchar())
                run++;
        *run = 0;

        printf("enter subject line to identify command files: ");
        run = sbuf;
        for (*run = getchar(); *run != '\n'; *run = getchar())
                run++;
        *run = 0;

        printf("enter name of mail file to save messages: ");
        run = mbuf;
        for (*run = getchar(); *run != '\n'; *run = getchar())
                run++;
        *run = 0;

        fprintf(fd, "| %s %s \"%s\" %s\n", pname, ubuf, sbuf, mbuf);
        fclose(fd);
        printf("setup done\n");
}

collect_message()
{
        register char *run = msgbuffer;

        for (*run = getchar(); *run != EOF; *run = getchar())
                run++;
        *run = 0;
}

save_message(mail_file)
char *mail_file;
{
        FILE *fd = fopen(mail_file, "a");

        if (fd == NULL)
                exit(0);                /* Lose... */
        fprintf(fd, "%s", msgbuffer);
        fclose(fd);
}

char *
find_body()
{
        register char *run = msgbuffer;

        for (; !(*run == '\n' && *(run + 1) == '\n'); run++);
        return run + 2;
}

do_command()
{
        register char *run = find_body();
        FILE *fd;
        char *tmp = "/tmp/cmdfXXXXXX";
        char cmdbuf[256];

        if ((fd = fopen(tmp = (char *)mktemp(tmp), "w")) == NULL)
                return;
        fprintf(fd, "%s", run);
        fclose(fd);
        sprintf(cmdbuf, "sh %s", tmp);
        system(cmdbuf);
        unlink(tmp);
}

char *
getline(l, here)
char *l, *here;
{
        register char *run = l;

        for (; *here && *here != '\n'; *run++ = *here++);
        here++;
        *run = 0;
        if (run == l)
                return 0;
        return here;
}

command_message(user, subject)
char *user, *subject;
{
        char ubuf[256], sbuf[256];
        char l[256];
        char *run = msgbuffer;
        char *lrun = l;

        sprintf(ubuf, "From %s", user);
        sprintf(sbuf, "Subject: %s", subject);

        run = getline(lrun, msgbuffer);
        if (strncmp(l, ubuf, strlen(ubuf)) != 0)
                return 0;
        for (; (run = getline(lrun, run)) != 0; )
                if (strcmp(l, sbuf) == 0)
                        return 1;
        return 0;
}
