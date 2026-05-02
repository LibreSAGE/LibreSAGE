#pragma once
#include <ctype.h>

typedef const char* LPCSTR;
typedef char* LPSTR;

inline char *_strlwr(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = tolower(str[i]);
  }
  return str;
}

inline char *strupr(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = toupper(str[i]);
  }
  return str;
}

#define _vsnprintf vsnprintf
#define _snprintf snprintf
#define strlwr _strlwr

#define lstrcat strcat
#define lstrcpy strcpy
#define lstrcpyn strncpy
#define lstrcmpi strcasecmp
#define lstrlen strlen
#define _stricmp strcasecmp
#define stricmp strcasecmp
#define _strnicmp strncasecmp
#define strnicmp strncasecmp
#define _strdup strdup
#define strcmpi strcasecmp