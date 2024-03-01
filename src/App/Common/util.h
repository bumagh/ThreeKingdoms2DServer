
#ifndef util_h
#define util_h
#include <stdio.h>  
char* int_to_str (int number, char* str, size_t size);
static void free_strbuf (void* data);
static void unsub_chat(void *data1, void *data2);
#endif