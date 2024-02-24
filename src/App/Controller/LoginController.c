#include "lwan.h"
#include "../Model/user.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
LWAN_HANDLER_ROUTE (login, "/login")
{
    request->response.mime_type = "text/html;charset=utf-8";
    char data_buf_fmt[256] = "{\"%s\":\"%s\"}";
    char response_buf_fmt[256] = "{\"code\":%s,\"data\":%s,\"msg\":\"%s\"}";

    if(lwan_request_get_post_param (request, "action") == NULL)
    {
        char code[] = "202";
        char msg[] = "no action";
        char data_buf[128];
        char response_buf_fmt_buf[256];

        sprintf (data_buf, "\"\"");
        sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf, msg);
        lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf,
                               strlen (response_buf_fmt_buf));
        return HTTP_OK;
    }
    if(strcmp (lwan_request_get_post_param (request, "action"), "addUser") ==
        0)
    {
        const char* account = lwan_request_get_post_param (request, "account");
        const char* pwd = lwan_request_get_post_param (request, "pwd");

        if(account == NULL || pwd == NULL)
        {
            char code[] = "203";
            char msg[] = "no action";
            char data_buf[128];
            char response_buf_fmt_buf[256];

            sprintf (data_buf, "\"\"");
            sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf,
                    msg);
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf,
                                   strlen (response_buf_fmt_buf));
            return HTTP_OK;
        }
        if(addUser (
            lwan_request_get_post_param (request, "account"),
            strlen (lwan_request_get_post_param (request, "account")),
            lwan_request_get_post_param (request, "pwd"),
            strlen (lwan_request_get_post_param (request, "pwd"))) == 0)
        {

            char code[] = "200";
            char msg[] = "";
            char data_buf[128];
            char response_buf_fmt_buf[256];

            sprintf (data_buf, "\"\"");
            sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf,
                    msg);
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf,
                                   strlen (response_buf_fmt_buf));
        }
        else
        {
            char code[] = "201";
            char msg[] = "account exist";

            char data_buf[128];
            char response_buf_fmt_buf[256] = {0};

            sprintf (data_buf, "\"\"");
            sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf,
                    msg);
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
        }
    }
    free_strbuf ();
    lwan_strbuf_free (response->buffer);
    return HTTP_OK;
}