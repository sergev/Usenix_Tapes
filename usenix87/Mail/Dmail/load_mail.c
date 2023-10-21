/*
 *  LOAD_MAIL.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  file-io routines to scan the mail file and load required information.
 *
 *
 *  Global Routines:    HOLD_LOAD()         hold on loading mail after change
 *                      NOHOLD_LOAD()       hold off.. load if changes
 *                      LOAD_CHANGES()      reload mail if changed 
 *                      LOAD_MAIL()         load/reload mail
 *                      SAVE_FILE()         save mail items back to spool
 *                      CHECK_NEW_MAIL()    check for new mail
 *                      WRITE_FILE()        append mail items to a file
 *                      GET_EXTRA_OVR()     ret index of Field (create if not)
 *                      ADD_EXTRA()         add another field (reloads mail)
 *                      DELETE_EXTRA()      delete a field
 *                      GET_EXTRA()         ret index of Field, or error
 *                      M_SELECT()          select on current message list
 *
 *
 *  Static Routines:    LOAD_HASH()         load hash table from fields list
 *                      FREE_ENTRY()        unload EVERYTHING
 *                      FREE_TABLE()        unload all Fields table
 *                      LOAD_FILE()         raw file loading/counting
 *                      
 *
 */

#include <stdio.h>
#include <sys/file.h>
#include "dmail.h"

#define NOHOLD  0
#define HOLD    1

#define NO_BASE     0
#define NO_FIELDS   1
#define ENTRY_OK    2

struct FIND Find[MAXTYPE + 1] = {
        "From:"   , 5, 1,
        "To:"     , 3, 1,
        "Subject:", 8, 1 };

static int  File_size;
static int  changed, load_hold;
static int  Hash[256];


hold_load()
{
    load_hold = 1;
}


nohold_load()
{
    load_hold = 0;
    load_changes();
}


load_changes()
{
    if (changed  &&  !load_hold)
        load_mail(Entries, 1);
}

initial_load_mail()
{
    if (load_mail (0, 0) < 0)
        return (-1);
    return ((Entries) ? 1 : -1);
}


static
load_mail(at, from0)
{
    FILE *fi;
    int i, count, file_size;

    if (No_load_mail)
        return (-1);
    load_hash();
    if (from0)
        free_table (0, HOLD);
    else
        free_table (at, NOHOLD);
    fi = fopen (mail_file, "r");
    if (m_fi != NULL)
        fclose (m_fi);
    m_fi = fopen (mail_file, "r");
    if (fi == NULL  ||  m_fi == NULL)
        return (-1);
    flock (m_fi->_file, LOCK_EX);
    if (at)
        fseek (fi, Entry[at].fpos, 0);
    else
        fseek (fi, 0, 0);
    count = Entries;
    while (search_from(fi))
        ++count;
    if (Entries != count) {
        printf ("%d New Items loaded\n", count - Entries);
        Entry = (struct ENTRY *)realloc (Entry, sizeof(*Entry) * (count + 1));
    }
    Entries = count;
    for (i = at; i < Entries; ++i)
        Entry[i].no  =  Entry[i].status = 0;
    Entry[i].fpos = File_size = file_size = ftell (fi);
    fclose (fi);
    load_file ((from0) ? 0 : at);
    if (file_size != File_size) {       /* Last entry incomplete?       */
        free_table (Entries - 1, NOHOLD);
    }
    changed = 0;
    flock (m_fi->_file, LOCK_UN);
    return (1);
}


static
load_file(at)
int at;
{
    FILE *fi;
    char *next, *ptr;
    int i, bit, maxbit, len, count, havefrom;

    maxbit = 0;
    for (i = 0; Find[i].search != NULL; ++i) 
        maxbit = (maxbit << 1) | 1;
    fi = fopen (mail_file, "r");
    count = -1;
    havefrom = 0;
    while (havefrom  ||  search_from (fi)) {
        havefrom = 0;
        if (++count >= Entries)
            break;
        len = strlen(Buf) - 1;
        Buf[len] = '\0';
        next = next_word(Buf);
        len -= next - Buf;
        Entry[count].fpos = ftell (fi);
        Entry[count].from = malloc (len + 1);
        bcopy (next, Entry[count].from, len + 1);

        /* SEARCH FIELD LIST */

        bit = 0;
        if (Debug)
            printf ("No %d  ---------------------\n", count + 1);
        while (fgets (Buf, MAXFIELDSIZE, fi) != NULL) {
            if (Buf[0] == '\n')
                break;
            if (isfrom(Buf)) {
                havefrom = 1;
                break;
            }
            len = strlen(Buf) - 1;
            Buf[len] = '\0';
            if (Debug)
                printf ("CHECK: %s\n", Buf);
            next = next_word(Buf);
            len -= next - Buf;
            if (Hash[*Buf] == 0)
                continue;
            if (Hash[*Buf] > 0) {
                i = Hash[*Buf] & 0xff;
                if (strncmp (Find[i].search, Buf, Find[i].len) == 0) 
                    goto found;
                continue;
            }
            for (i = -Hash[*Buf] & 0xff; Find[i].search; ++i) {
                if (*Find[i].search != *Buf)
                    break;
                if (strncmp (Find[i].search, Buf, Find[i].len) == 0)
                    goto found;
            }
            continue;
found:
            if (Debug)
                printf ("Found: %d %s\n", i, Buf);
            if (Find[i].notnew == 0) {
                Find[i].notnew = 1;
                ptr = Buf;
                while (*ptr  &&  *ptr != ':')
                    ++ptr;
                ++ptr;
                Find[i].search =
                        realloc (Find[i].search, ptr - Buf + 1);
                strncpy (Find[i].search, Buf, ptr - Buf);
                *(Find[i].search + (ptr - Buf)) = '\0';
                Find[i].len = strlen(Find[i].search);
            }
            compile_field (Buf, fi);
            Entry[count].fields[i] = 
                    malloc (strlen(next) + 1);
            strcpy (Entry[count].fields[i], next);
            if ((bit |= (1 << i)) == maxbit) 
                break;
        }
        if (bit != maxbit) {
            for (i = 0; Find[i].search != NULL; ++i) {
                if (((1 << i) & bit) == 0) {
                    Entry[count].fields[i] = malloc (1);
                    *(Entry[count].fields[i]) = '\0';
                }
            }
        }
    } 
    File_size = ftell (fi);
    fclose (fi);
    return (1);
}


static
load_hash()
{
    register int i, c;

    bzero (Hash, sizeof(Hash));
    for (i = 0; Find[i].search; ++i) {
        c = *Find[i].search;
        if (Hash[c] > 0)
            Hash[c] = -Hash[c];
        if (Hash[c] == 0)
            Hash[c] = i | 0x100;
    }
}


free_entry()
{
    free_table(0, NOHOLD);
    Entry = (struct ENTRY *)realloc (Entry, sizeof(*Entry));
    File_size = Entries = 0;
    Entry->status = Entry->no = Entry->fpos = Current = 0;
    if (m_fi)
        fclose (m_fi);
}


static
free_table(at, hold)
{
    int i, j;

    for (i = at; i < Entries; ++i) {
        free (Entry[i].from);
        for (j = 0; Find[j].search != NULL; ++j) 
            free (Entry[i].fields[j]);
    }
    Entries = (hold == HOLD) ? Entries : at;
    File_size = (at) ? Entry[Entries].fpos : 0;
}

static
search_from(fi)
FILE *fi;
{
    while (fgets (Buf, MAXFIELDSIZE, fi) != NULL) {
        if (isfrom (Buf))
            return (1);
    }
    return (0);
}


save_file(reload, mark, notmark)
{
    FILE *fi, *fiscr;
    int fd, fdscr;
    int new_size, i;
    char scratch[64];
    char buf[MAXFIELDSIZE];
    char *avnul[3];

    avnul[0] = "";
    avnul[1] = "";
    avnul[2] = NULL;
    if (m_fi)
        fclose (m_fi);
    for (i = 0; i < Entries; ++i) {
        if ((Entry[i].status & mark) != mark  ||
                (~Entry[i].status & notmark) != notmark) 
            break;
    }
    if (i == Entries) {
        m_select (avnul, M_RESET);
        puts ("No Changes Made");
        return (Entries);
    }
    sprintf (scratch, "/tmp/dmail%d", getpid());
    fd = open (mail_file, O_RDWR, 0);
    if (fd < 0)
        return (-1);
    flock (fd, LOCK_EX);
    fdscr = open (scratch, O_RDWR | O_CREAT | O_TRUNC, MAILMODE);
    fi    = fdopen (fd, "r+");
    fiscr = fdopen (fdscr, "a+");
    for (i = 0; i < Entries; ++i) {
        if ((Entry[i].status & mark) == mark  &&
                (~Entry[i].status & notmark) == notmark) {
            fputs ("From ", fiscr);
            fputs (Entry[i].from, fiscr);
            fputc ('\n', fiscr);
            fseek (fi, Entry[i].fpos, 0);
            while (fgets (buf, MAXFIELDSIZE, fi) != NULL) {
                if (isfrom(buf))
                    break;
                fputs (buf, fiscr);
            }
        }
    }

    /* If NEW MAIL has come in, append that to the scratch file also */

    new_size = fseek (fi, 0, 2);
    if (File_size != new_size) {
        fseek (fi, File_size, 0);
        while (fgets (buf, MAXFIELDSIZE, fi) != NULL)
            fputs (buf, fiscr);
    }

    /* Write scratch file back to mail file, or try to */

    fflush (fi);
    fflush (fiscr);
    fseek (fi   , 0, 0);
    fseek (fiscr, 0, 0);
    lseek (fd   , 0, 0);
    lseek (fdscr, 0, 0);
    while ((i = read (fdscr, buf, MAXFIELDSIZE)) > 0) 
        write (fd, buf, i);
    ftruncate (fd, lseek (fd, 0, 1));
    if (lseek (fd, 0, 2) == 0  &&  !reload) {
        printf ("%s  Removed\n", mail_file);
        unlink (mail_file);
    }
    fclose (fi);
    fclose (fiscr);
    unlink (scratch);
    if (reload) {
        free_entry();
        load_mail(0, 0);
    }
    m_select (avnul, M_RESET);
    return (0);
}


check_new_mail()
{
    push_break();
    if (m_fi == NULL) {
        m_fi = fopen (mail_file, "r");
        if (m_fi == NULL) {
            pop_break();
            return;
        }
    }
    if (fseek (m_fi, 0, 2) != File_size)
        load_mail(Entries, 1);
    pop_break();
}


write_file(file, modes, mark, notmark)
char *file;
{
    int i, fd, notopen = 1;
    FILE *fi;
    char buf[MAXFIELDSIZE];

    for (i = 0; i < Entries; ++i) {
        if ((Entry[i].status & mark) == mark  &&
                (~Entry[i].status & notmark) == notmark) {
            if (notopen) {
                notopen = 0;
                fd = open (file, O_APPEND | O_WRONLY | modes, MAILMODE);
                if (fd < 0)
                    return (-1);
                flock (fd, LOCK_EX);
                fi = fdopen (fd, "a");
            }
            fputs ("From ", fi);
            fputs (Entry[i].from, fi);
            fputc ('\n', fi);
            if (m_fi) {
                fseek (m_fi, Entry[i].fpos, 0);
                while (fgets (buf, MAXFIELDSIZE, m_fi) != NULL) {
                    if (isfrom(buf))
                        break;
                    fputs (buf, fi);
                }
            }
        }
    }
    if (!notopen)
        fclose (fi);
    return (1);
}


get_extra_ovr(str)
char *str;
{
    register int i;

    i = get_extra (str);
    if (i < 0) {
        i = add_extra (str);
        load_changes();
    }
    return (i);
}


add_extra(str)
char *str;
{
    int i, j;

    for (i = 0; i < MAXTYPE; ++i) {
        if (Find[i].search == NULL)
            break;
    }
    if (i == MAXTYPE)
        i = EXSTART;
    for (; i < MAXTYPE; ++i) {
        for (j = 0; j < Listsize; ++j) {
            if (i == header[j])
                break;
        }
        if (j == Listsize)
            break;
    }
    if (i >= MAXTYPE)
        return (-1);
    if (Find[i].search != NULL)
        free (Find[i].search);
    Find[i].len = strlen(str);
    Find[i].search = malloc (Find[i].len + 1);
    Find[i].notnew = 0;
    strcpy (Find[i].search, str);
    changed = 1;
    return (i);
}


delete_extra(str)
char *str;
{
    int i;

    for (i = EXSTART; Find[i].search; ++i) {
        if (strncmp (Find[i].search, str, strlen(str)) == 0) {
            free (Find[i].search);
            do {
                Find[i].search = Find[i + 1].search;
            } while (Find[++i].search);
            changed = 1;
            return (i);
        }
    }
    return (-1);
}


delete_all()
{
    int i;

    for (i = EXSTART; Find[i].search; ++i) {
        free (Find[i].search);
        changed = 1;
    }
    return (1);
}


get_extra(str)
char *str;
{
    int i;

    for (i = 0; Find[i].search; ++i) {
        if (strncmp (str, Find[i].search, strlen(str)) == 0)
            return (i);
    }
    return (-1);
}


m_select(sav, mode)
register char *sav[];
{
    char *ptr, *dest;
    char l_map[256];
    int idx[MAXLIST], ix = 0;
    int ok, not, len, scr;
    register int i, j, avi;
        
    for (i = 0;i < 256; ++i)
        l_map[i] = i;
    for (i = 'A'; i <= 'Z'; ++i)
        l_map[i] += 'a' - 'A';
    hold_load();
    i = 0;
    idx[ix++] = get_extra_ovr (sav[i++]);
    for (; sav[i]; ++i) {
        if (strcmp (sav[i], ",") == 0  &&  sav[i + 1])
            idx[ix++] = get_extra_ovr (sav[++i]);
    }
    idx[ix] = -1;
    nohold_load();
    j = 1;
    for (i = 0; i < Entries; ++i) {
        if (mode == M_CONT  &&  Entry[i].no == 0)
            continue;
        ix = ok = 0;
        avi = 1;
        while ((ptr = sav[avi]) != NULL) {
            if (ptr[0] == ','  &&  ptr[1] == '\0' && sav[avi+1]) {
                ++ix;
                avi += 2;
                continue;
            }
            if (not = (*ptr == '!'))
                ++ptr;
            len = strlen (ptr);
            dest = Entry[i].fields[idx[ix]];
            if (*ptr == '\0') {
                ok = 1;
                goto gotit;
            }
            while (*dest) {
                scr = 0;
                while (l_map[dest[scr]] == l_map[ptr[scr]] && ptr[scr])
                    ++scr;
                if (ptr[scr] == '\0') {
                    ok = 1;
                    goto gotit;
                }
                ++dest;
            }
            ++avi;
        }
gotit:
        Entry[i].no = (ok ^ not) ? j++ : 0;
    }
    if (Entry[Current].no == 0) {
        Current = indexof (1);
        if (Current < 0) {
            Current = 0;
            return (-1);
        }   
    }
    return (1);
}
