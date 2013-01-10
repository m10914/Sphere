#include "log.h"


wchar_t *IniRead(wchar_t *filename, wchar_t *section, wchar_t *key){
	wchar_t *out = new wchar_t[512];
	GetPrivateProfileString(
		(LPCWSTR)section, 
		(LPCWSTR)key,
		NULL, 
		out,
		200, 
		(LPCWSTR)filename
	);
	return out;
} 

bool IniWrite(wchar_t *filename, wchar_t *section, wchar_t *key, wchar_t *data){
	return WritePrivateProfileString(
								(LPCWSTR)section,
								(LPCWSTR)key,
								(LPCWSTR)data,
								(LPCWSTR)filename
	);

}