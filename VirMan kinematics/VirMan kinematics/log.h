#include "Windows.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



wchar_t *IniRead(wchar_t *filename, wchar_t *section, wchar_t *key);

bool IniWrite(wchar_t *filename, wchar_t *section, wchar_t *key, wchar_t *data);
