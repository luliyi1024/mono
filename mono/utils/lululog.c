//lulu add for additive functions
#include "lululog.h"
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <Windows.h>
#include <malloc.h>

#ifdef _DEBUG
char *logmode = "DEBUG";
#else
char *logmode = "RELEASE";
#endif

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define FOREGROUND_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_LIGHT_WHITE (FOREGROUND_WHITE | FOREGROUND_INTENSITY)
#define FOREGROUND_LIGHT_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define FOREGROUND_LIGHT_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FOREGROUND_LIGHT_YELLOW (FOREGROUND_YELLOW | FOREGROUND_INTENSITY)

FILE *g_log_file = 0;

int g_log_mask = XLOG_OK | XLOG_FAILED | XLOG_WARNING | XLOG_LOG | XLOG_DEBUG;

void x_log_init()
{
	AllocConsole();
	SetConsoleOutputCP(936);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
	freopen( "CON", "w", stdout );
	g_log_file = _wfopen(L"cout.log",L"w");
	x_log(XLOG_OK,"start monox log %s mode\n",logmode);
}

void x_log_release()
{
	x_log(XLOG_OK,"end monox log\n");
	fclose(g_log_file);
	FreeConsole();
}

void
x_log_mask(int mask)
{
	x_log(XLOG_OK,"set log mask %d\n", mask);
	g_log_mask = mask;
}

int
x_log_wbegin(const wchar_t* module, int flag)
{
	DWORD color = FOREGROUND_WHITE;
	wchar_t* flagStr = L"";

	if ((flag & g_log_mask) == 0)
		return 0;

	switch(flag){
		case XLOG_OK:flagStr =      L"[OK]\t"; color = FOREGROUND_GREEN;break;
		case XLOG_FAILED:flagStr =  L"[FAIL]\t"; color = FOREGROUND_LIGHT_RED; break;
		case XLOG_WARNING:flagStr = L"[WARN]\t"; color = FOREGROUND_LIGHT_YELLOW; break;
		case XLOG_LOG:flagStr =     L"[LOG]\t";break;
		case XLOG_DEBUG:flagStr =   L"[DEBUG]\t";break;
		default: flagStr =          L"[UNKNW]\t"; color = FOREGROUND_LIGHT_RED; break;
	}

	wprintf(L"[%s]\t", module);
	fwprintf(g_log_file,L"[%s]\t", module);

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	
	wprintf(flagStr);
	fwprintf(g_log_file,flagStr);
	return 1;
}

void
x_log_end()
{
	fflush(stdout);
	fflush(g_log_file);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
}

void
x_log_wstring(const wchar_t* module, int flag, const wchar_t* value)
{
	if (x_log_wbegin(module,flag)){

		int len;
		len = WideCharToMultiByte(CP_ACP, 0, value, -1, NULL, 0, NULL, NULL);
		{
			char *str = (char*)alloca(len+1);
			WideCharToMultiByte(CP_ACP, 0, value, -1, str, len + 1, NULL, NULL);
			printf("%s\n",str);
			fprintf(g_log_file,"%s\n",str);
		}
		x_log_end();
	}
}

void
x_log(int flag, const char *format, ...)
{
	va_list args;
	va_start (args, format);
	if (x_log_wbegin(L"mono",flag)){
		vprintf(format,args);
		vfprintf(g_log_file,format,args);
		va_end (args);
		x_log_end();
	}
}

/*
FILE *g_logfile = 0;

void x_log_init()
{
	if (g_logfile == 0){
		g_logfile = fopen("monox.log","w");
		x_log(XLOG_OK,"start monox log %s mode\n",logmode);
	}
}

void x_log_release()
{
	if (g_logfile != 0){
		x_log(XLOG_OK,"end monox log\n");
		fclose(g_logfile);
		g_logfile = 0;
	}
}

void
x_log(int flag, const char *format, ...)
{
	char* flagStr = "";
	va_list args;
	va_start (args, format);

	switch(flag){
		case XLOG_OK:flagStr =      "[OK]      ";break;
		case XLOG_FAILED:flagStr =  "[FAILED]  ";break;
		case XLOG_WARNING:flagStr = "[WARNING] ";break;
		case XLOG_LOG:flagStr =     "[LOG]     ";break;
#ifdef _DEBUG
		case XLOG_DEBUG:flagStr =   "[DEBUG]   ";break;
		default: flagStr =          "[UNKNOWN] ";break;
#else
		default: return;
#endif
	}

	fprintf(g_logfile,flagStr);
	vfprintf(g_logfile,format,args);
	va_end (args);

	fflush(g_logfile);
}
*/
#ifndef _LIB_

int x_is_image_buffer_valid(const char* orig_buffer, unsigned int orig_size)
{
	if (orig_buffer == 0 || orig_size == 0)
		return 0;
	else return 1;
}

char* x_load_owner_image_buffer(const char *name, unsigned int *size)
{
	int l = strlen(name);
	char new_name[1024];
	FILE *fp = 0;
	char *buffer;

	strcpy(new_name,name);
	new_name[l-4]='\0';
	strcat(new_name,"_i.dll");

	fp = fopen(new_name,"rb");
	if (fp == 0)
		return 0;

	fseek(fp,0,SEEK_END);
	*size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	x_log(XLOG_LOG,"replace file mono_image_open: %s %d\n",new_name,*size);

	buffer = (char*)malloc(*size);

	fread(buffer,1,*size,fp);
	fclose(fp);

	return buffer;
}

void x_release_image_buffer(char *buffer)
{
	free(buffer);
}

const char* x_get_aot_dll_name(const char* assmebly_name)
{
	return 0;
}

#endif