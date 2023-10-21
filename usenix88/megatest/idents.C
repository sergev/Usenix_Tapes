//  COPYRIGHT 1988 Megatest Corp.

#include <ctype.h>
#include "idents.H"

inline unsigned char
LOWER(unsigned char ch)  { return (isupper(ch)?tolower(ch):(ch)); }

Idents_UC::equiv(register  char* str1, register  char* str2)
{
  while( isalnum(*str1) || *str1 == '_' )
    if(*str1++ != *str2++)
      return 0;
  return !(isalnum(*str2) || *str2 == '_');
}

Idents_uC::equiv(register  char* str1, register  char* str2)
{
  while( isalnum(*str1))
    if(*str1++ != *str2++)
      return 0;
  return !isalnum(*str2);
}

Idents_uc::equiv(register  char* str1, register  char* str2)
{
  while( isalnum(*str1))
    if( LOWER(*str1) != LOWER( *str2))
      return 0;
    else
      { str1++; str2++; }
  return !isalnum(*str2);
}

Idents_Uc::equiv(register  char* str1, register  char* str2)
{
  while(isalnum(*str1) || *str1 == '_' )
    if( LOWER(*str1) != LOWER( *str2))
      return 0;
    else
      { str1++; str2++; }
  return !(isalnum(*str2) || *str2 == '_');
}

Idents_Tc::equiv(register  char* str1, register  char* str2)
{
  do
    if( LOWER(*str1) != LOWER(*str2))
      return 0;
    else
      { do str1++; while (*str1 == '-');
	do str2++; while (*str2 == '-');
      }
  while(isalnum(*str1));
  return !(isalnum(*str2));
}

Idents_UC::hash(register  char* str)
{
  register int hash = 0;
  while( isalnum(*str) || *str == '_' )
    hash += hash + (unsigned char)(*str++);
  return hash;
}

/* underscores not valid. case matters. */
Idents_uC::hash(register  char* str)
{
  register int hash = 0;
  while( isalnum(*str))
      { hash += hash + (unsigned char)(*str++); }
  return hash;
}


/* underscores are not valid. case does not matter. (Standard Pascal) */
Idents_uc::hash(register  char* str)
{
  register int hash = 0;
  while( isalnum(*str))
      { hash += hash + LOWER((unsigned char)(*str++));
      }
  return hash;
}

/* Telephone scheme */
Idents_Tc::hash(register  char* str)
{
  register int hash = 0;
  do
      { hash += hash + LOWER((unsigned char)(*str));
	do str++; while (*str == '-');
      }
  while( isalnum(*str));
  return hash;
}

/* underscores are valid. case does not matter. */
Idents_Uc::hash(register  char* str)
{
  register int hash = 0;
  while(isalnum(*str) || *str == '_' )
    { hash += hash + LOWER((unsigned char)(*str++));
    }
  return hash;
}
