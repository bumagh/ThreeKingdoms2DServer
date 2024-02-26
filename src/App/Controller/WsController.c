#include "../../Library/Json/Yyjson/yyjson.h"
#include <stdio.h>
#include <string.h>
void handleWs(const char *json, char *ret)
{
    yyjson_doc *doc = yyjson_read(json, strlen(json), 0);
    yyjson_val *root = yyjson_doc_get_root(doc);

    // Get root["name"]
    yyjson_val *type = yyjson_obj_get(root, "type");
    if (strcmp(yyjson_get_str(type), "EnterGame") == 0) {
        strcpy(ret, "EnterGame");
    }
}