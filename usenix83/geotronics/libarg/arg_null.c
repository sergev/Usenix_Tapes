/*
 * arg_null.c:
 * Bagbiting System III printf doesn't known NULL from anything.
 */
#include <stdio.h>

char *arg_null (pointer)
char *pointer;
{
  if (pointer == NULL)
    return ("(null)");
  return (pointer);
}
