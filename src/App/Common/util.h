
#ifndef util_h
#define util_h
#include <stdio.h>  
char* int_to_str (int number, char* str, size_t size);
void free_strbuf (void* data);
void unsub_chat(void *data1, void *data2);
#endif