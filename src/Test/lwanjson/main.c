#include<stdio.h>
#include"json.h"
#include <stdbool.h>

static const char hello_world[] = "Hello, World!";

struct hello_world_json
{
    const char* message;
};
static int append_to_strbuf (const char* bytes, size_t len, void* data)
{
    struct lwan_strbuf* strbuf = data;

    return !lwan_strbuf_append_str (strbuf, bytes, len);
}
static const struct json_obj_descr hello_world_json_desc[] = {
    JSON_OBJ_DESCR_PRIM (struct hello_world_json, message, JSON_TOK_STRING),
};

int main ()
{
    struct hello_world_json j = {.message = hello_world};
    const char buf[60];
    json_obj_encode_full (hello_world_json_desc, N_ELEMENTS (hello_world_json_desc), "123", append_to_strbuf, buf, false);
    return 0;
}