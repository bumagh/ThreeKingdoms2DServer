#include <stdio.h>  
#include "lwan.h"
#include "lwan-pubsub.h"
char* int_to_str (int number, char* str, size_t size)
{
    snprintf (str, size, "%d", number);
    return str;
}
 void free_strbuf (void* data)
{
    lwan_strbuf_free ((struct lwan_strbuf*)data);
}
 void unsub_chat(void *data1, void *data2)
{
    lwan_pubsub_unsubscribe((struct lwan_pubsub_topic *)data1,
                            (struct lwan_pubsub_subscriber *)data2);
}

