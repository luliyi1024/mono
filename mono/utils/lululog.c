//lulu add for additive functions
#include "lululog.h"
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <Windows.h>

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

void x_log_init()
{
	AllocConsole();
	SetConsoleOutputCP(936);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
	freopen( "CON", "w", stdout );
	x_log(XLOG_OK,"start monox log %s mode\n",logmode);
}

void x_log_release()
{
	x_log(XLOG_OK,"end monox log\n");
	FreeConsole();
}

void
x_log(int flag, const char *format, ...)
{
	DWORD color = FOREGROUND_WHITE;
	char* flagStr = "";
	va_list args;
	va_start (args, format);


	switch(flag){
		case XLOG_OK:flagStr =      "[OK]\t"; color = FOREGROUND_GREEN;break;
		case XLOG_FAILED:flagStr =  "[FAIL]\t"; color = FOREGROUND_LIGHT_RED; break;
		case XLOG_WARNING:flagStr = "[WARN]\t"; color = FOREGROUND_LIGHT_YELLOW; break;
		case XLOG_LOG:flagStr =     "[LOG]\t";break;
#ifdef _DEBUG
		case XLOG_DEBUG:flagStr =   "[DEBUG]\t";break;
		default: flagStr =          "[UNKNW]\t"; color = FOREGROUND_LIGHT_RED; break;
#else
		default: return;
#endif
	}

	printf("[mono]\t");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	
	printf(flagStr);
	vprintf(format,args);
	va_end (args);

	fflush(stdout);

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
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