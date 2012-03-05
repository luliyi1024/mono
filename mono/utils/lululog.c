//lulu add for additive functions
#include "lululog.h"
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _DEBUG

static FILE *g_logfile = 0;

void
x_log(int flag, const char *format, ...)
{
	char* flagStr = "";
	va_list args;
	va_start (args, format);

	if (g_logfile == 0){
		g_logfile = fopen("lululog.txt","w");
	}

	switch(flag){
		case XLOG_OK:flagStr =      "[OK]      ";break;
		case XLOG_FAILED:flagStr =  "[FAILED]  ";break;
		case XLOG_WARNING:flagStr = "[WARNING] ";break;
		case XLOG_LOG:flagStr =     "[LOG]     ";break;
	}

	fprintf(g_logfile,flagStr);
	vfprintf(g_logfile,format,args);
	va_end (args);

	fflush(g_logfile);
}

#else

void x_log(const char *format, ...){}

#endif


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

	buffer = malloc(*size);

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