#include "lwan.h"
#include "../Model/user.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
LWAN_HANDLER_ROUTE (login, "/login")
{
    response->mime_type = "text/html;charset=utf-8";
    char data_buf_fmt[256] = "{\"%s\":\"%s\"}";
    char response_buf_fmt[256] = "{\"code\":%s,\"data\":%s,\"msg\":\"%s\"}";
    char code[] = "404";
    char msg[] = "no action";
    char data_buf[128] = "\"\"";
    char response_buf_fmt_buf[256];
    char* action = lwan_request_get_post_param (request, "action");
    if(action == NULL)
    {
        sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf, msg);
        lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
        return HTTP_OK;
    }
    if(strcmp (action, "addUser") == 0)
    {
        const char* account = lwan_request_get_post_param (request, "account");
        const char* pwd = lwan_request_get_post_param (request, "pwd");

        if(account == NULL || pwd == NULL)
        {
            sprintf (response_buf_fmt_buf, response_buf_fmt, "203", data_buf, "account or pwd is null");
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
            return HTTP_OK;
        }
        if(addUser (account, strlen (account), pwd, strlen (pwd)) == 0)
        {
            sprintf (response_buf_fmt_buf, response_buf_fmt, "200", "addUser success", msg);
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
        }
        else
        {
            char response_buf_fmt_buf[256] = {0};
            sprintf (response_buf_fmt_buf, response_buf_fmt, "201", data_buf, "account exist");
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
        }
    }
    else if(strcmp (action, "loginUser") == 0)
    {
        const char* account = lwan_request_get_post_param (request, "account");
        const char* pwd = lwan_request_get_post_param (request, "pwd");

        if(account == NULL || pwd == NULL)
        {
            sprintf (response_buf_fmt_buf, response_buf_fmt, "203", data_buf, "account or pwd is null");
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
            return HTTP_OK;
        }

        int loginRet = loginUser (account, strlen (account), pwd, strlen (pwd));
        if(loginRet > 0)
        {
            char data_key[64];
            char data_val[64];
            char data_buf[128];
            char response_buf_fmt_buf[256];
            sprintf (data_key, "%s", "id");
            sprintf (data_val, "%d", loginRet);
            sprintf (data_buf, data_buf_fmt, data_key, data_val);
            sprintf (response_buf_fmt_buf, response_buf_fmt, "200", data_buf, "login success");
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
        }
        else
        {
            sprintf (response_buf_fmt_buf, response_buf_fmt, "201", data_buf, "err or not exist");
            lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf, strlen (response_buf_fmt_buf));
        }
    }
    lwan_strbuf_free (response->buffer);
    return HTTP_OK;
}