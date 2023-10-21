/*
 * String functions.
 */

char *memcpy(/*char *dst, const char *src, int size*/);
char *memccpy(/*char *dst, const char *src, int ucharstop, int size*/);
char *strcpy(/*char *dst, const char *src*/);
char *strncpy(/*char *dst, const char *src, int size*/);
char *strcat(/*char *dst, const char *src*/);
char *strncat(/*char *dst, const char *src, int size*/);
int memcmp(/*const char *s1, const char *s2, int size*/);
int strcmp(/*const char *s1, const char *s2*/);
int strncmp(/*const char *s1, const char *s2, int size*/);
char *memchr(/*const char *s, int ucharwanted, int size*/);
char *strchr(/*const char *s, int charwanted*/);
int strcspn(/*const char *s, const char *reject*/);
char *strpbrk(/*const char *s, const char *breakat*/);
char *strrchr(/*const char *s, int charwanted*/);
int strspn(/*const char *s, const char *accept*/);
char *strstr(/*const char *s, const char *wanted*/);
char *strtok(/*char *s, const char *delim*/);
char *memset(/*char *s, int ucharfill, int size*/);
int strlen(/*const char *s*/);

/*
 * V7 and Berklix compatibility.
 */
char *index(/*const char *s, int charwanted*/);
char *rindex(/*const char *s, int charwanted*/);
int bcopy(/*const char *src, char *dst, int length*/);
int bcmp(/*const char *s1, const char *s2, int length*/);
int bzero(/*char *dst, int length*/);

/*
 * Putting this in here is really silly, but who am I to argue with X3J11?
 */
char *strerror(/*int errnum*/);
