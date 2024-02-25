#include <stdio.h>  

char* int_to_str (int number, char* str, size_t size)
{
    snprintf (str, size, "%d", number);
    return str;
}