#include "log.h"


int InitLog(void)
{
	return 0;

	FILE* log=fopen("log.html", "w+");
	fputs("<html><head><title>WAXIS engine log</title></head><body><h3>WAXIS engine log started</h3>\n", log);
	fclose(log);

	return 0;
}


int LogPlease(char* string)
{
	return 0;

	FILE* log=fopen("log.html","r+");
	while(fgetc(log) != EOF) continue;

	
	for(char* c = string; *c != '\0'; c++)
	{
		if( *c == '\n' ) fputs("<br>", log);
		else if( *c == '-' && *(c+1) == '=' ) fputs("<b>-", log);
		else if( *c == '=' && *(c+1) == '-' ) fputs("=</b>", log);
		else if( *c == '\t' ) fputs("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp;", log);
		else fputc(*c, log);
	}
    //fputs(string,log);
	
	fclose(log);
	return 0;
}


int HTMLLog(char* string, ...)
{
		return 0;

	char text[512];
	
	va_list arg_list;

	va_start(arg_list, string);        
	vsprintf(text, string, arg_list);
	va_end(arg_list);

	LogPlease(text);

	return 0;
}


//--------------
// WideChar
//
//--------------
int HTMLLogW(wchar_t* string, ...)
{
		return 0;
	wchar_t text[512];

	va_list arg_list;

	va_start(arg_list, string);
	vswprintf(text, string, arg_list);
	va_end(arg_list);

	char mbstring[512];
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)text, 512, mbstring, 512, NULL, NULL);

	LogPlease(mbstring);

	return 0;
}


int boxmsg(wchar_t* string, ...)
{
	wchar_t text[512];

	va_list arg_list;

	va_start(arg_list,string);
	vswprintf(text, string, arg_list);
	va_end(arg_list);

	MessageBoxW(0,text,L"Message",0);

	return 0;
}


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