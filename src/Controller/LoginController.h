#ifndef user_mgr_h
#define user_mgr_h

#include "lwan.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
LWAN_HANDLER_ROUTE(login, "/login")
{
    response->mime_type = "text/html;charset=utf-8";
    const char *action = (char *)malloc(64 * sizeof(char));
    if (!action) {
        return HTTP_OK;
    }
    action = lwan_request_get_post_param(request, "action");
    char data_buf_fmt[256] = "{\"%s\":\"%s\"}";
    char response_buf_fmt[256] = "{\"code\":%s,\"data\":%s,\"msg\":\"%s\"}";
    if (strcmp(action, "addUser") == 0) {
        const char *account = lwan_request_get_post_param(request, "account");
        const char *pwd = lwan_request_get_post_param(request, "pwd");

        if (account == NULL || pwd == NULL) {
            static const char noarg[] = "2";
            lwan_strbuf_set_static(response->buffer, noarg, sizeof(noarg) - 1);
            return HTTP_OK;
        }

        if (model_user_add(account, strlen(account), pwd, strlen(pwd)) == 0) {

            char code[] = "200";
            char msg[] = "";
            char data_buf[128];
            char response_buf_fmt_buf[256];

            sprintf(data_buf, "\"\"");
            sprintf(response_buf_fmt_buf, response_buf_fmt, code, data_buf,
                    msg);
            lwan_strbuf_append_printf(response->buffer, response_buf_fmt_buf);
        } else {
            char code[] = "201";
            char msg[] = "account exist";

            char data_buf[128];
            char response_buf_fmt_buf[256];

            sprintf(data_buf, "\"\"");
            sprintf(response_buf_fmt_buf, response_buf_fmt, code, data_buf,
                    msg);

            lwan_strbuf_append_printf(response->buffer, response_buf_fmt_buf);
        }
    }

    return HTTP_OK;
}

#endif