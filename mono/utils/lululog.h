//lulu
//add for additive functions
#pragma once

void x_log(const char *format, ...);
int x_is_image_buffer_valid(const char* orig_buffer, unsigned int orig_size);
char* x_load_owner_image_buffer(const char *name, unsigned int *size);
void x_release_image_buffer(char *buffer);

const char* x_get_aot_dll_name(const char* assmebly_name);
