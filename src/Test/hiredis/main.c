#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <hiredis/hiredis.h>  
#include <yyjson.h>
  int main() {
    // 创建一个JSON对象
    yyjson_doc *doc = yyjson_read("{\"name\":\"John\",\"age\":30}", strlen("{\"name\":\"John\",\"age\":30}"), 0);
    if (!doc) {
        fprintf(stderr, "Failed to parse JSON\n");
        return 1;
    }
    // 序列化JSON对象为字符串
    size_t json_len;
    const char *json_str = yyjson_write_opts(doc, 0, NULL,NULL,NULL);
    if (!json_str) {
        fprintf(stderr, "Failed to serialize JSON\n");
        yyjson_doc_free(doc);
        return 1;
    }

    // 初始化Redis连接
    redisContext *redis_conn = redisConnect("127.0.0.1", 6379);
    if (redis_conn == NULL || redis_conn->err) {
        if (redis_conn) {
            fprintf(stderr, "Redis connection error: %s\n", redis_conn->errstr);
            redisFree(redis_conn);
        } else {
            fprintf(stderr, "Failed to allocate Redis context\n");
        }
        free((void *)json_str);
        yyjson_doc_free(doc);
        return 1;
    }

    // 将JSON字符串存储到Redis中
    redisReply *reply = redisCommand(redis_conn, "SET json_data %s", json_str);
    if (reply == NULL) {
        fprintf(stderr, "Failed to set JSON data in Redis\n");
        free((void *)json_str);
        yyjson_doc_free(doc);
        redisFree(redis_conn);
        return 1;
    }
    freeReplyObject(reply);

    printf("JSON data stored in Redis successfully.\n");

    // 释放资源
    free((void *)json_str);
    yyjson_doc_free(doc);
    redisFree(redis_conn);

    return 0;
}