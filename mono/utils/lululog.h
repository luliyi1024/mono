//lulu add for additive functions
#pragma once

#define XLOG_UNKNOWN	0x0
#define XLOG_OK			0x001
#define XLOG_FAILED		0x002
#define XLOG_WARNING	0x004
#define XLOG_LOG		0x008
#define XLOG_DEBUG		0x010

void x_log_init();
void x_log_release();
void x_log(int flag, const char *format, ...);
int x_is_image_buffer_valid(const char* orig_buffer, unsigned int orig_size);
char* x_load_owner_image_buffer(const char *name, unsigned int *size);
void x_release_image_buffer(char *buffer);

const char* x_get_aot_dll_name(const char* assmebly_name);
