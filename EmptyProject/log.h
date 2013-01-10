#include "common/dxstdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define W(x)		    { hr = x; if( FAILED(hr) ) { HTMLLogW( L"\nError: %s", L#x ); } }

int InitLog(void);

//multibyte
int LogPlease(char* string);
int HTMLLog(char* string, ...);

//widechar
int HTMLLogW(wchar_t* string, ...);

wchar_t *IniRead(wchar_t *filename, wchar_t *section, wchar_t *key);

bool IniWrite(wchar_t *filename, wchar_t *section, wchar_t *key, wchar_t *data);