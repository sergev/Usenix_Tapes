/* TIMEOUT should be one quarter of a second or 250 milliseconds */
#ifdef VMS
#define TIMEOUT "000 00:00:00.25"
#endif
#ifdef BSD_SELECT
#define TIMEOUT 250000		/* microseconds */
#endif
#ifdef UNISOFT_SELECT
#define TIMEOUT 250		/* milliseconds */
#endif
#ifdef CMU_IPC
#define TIMEOUT 250		/* milliseconds */
#endif

#ifdef EUNICE
#define USERNAME_LENGTH	12
#else
#define USERNAME_LENGTH 8
#endif
