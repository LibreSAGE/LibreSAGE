#pragma once

typedef const char* LPCSTR;
typedef char* LPSTR;

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