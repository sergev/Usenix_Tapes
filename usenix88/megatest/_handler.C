typedef void (*VOID_FUNC)();

int write( int , char *, unsigned int );
void exit(int);

static void
default_new_handler()
{
  static char message[] = "new: Out of memory.\n";
  write(2, message, sizeof(message));
  exit(12);
}

extern VOID_FUNC
_new_handler = &default_new_handler;

extern VOID_FUNC
set_new_handler(VOID_FUNC handler)
{
  VOID_FUNC old_handler = _new_handler;
  _new_handler = handler;
  return old_handler;
}
