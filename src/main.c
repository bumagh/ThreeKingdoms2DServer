
#include "App/Controller/WsController.h"
#include "App/Model/user.h"
#include "Library/Json/Yyjson/yyjson.h"
#include "lwan.h"
#include <errno.h>
#include <stdlib.h>
static void free_strbuf(void *data)
{
    lwan_strbuf_free((struct lwan_strbuf *)data);
}

LWAN_HANDLER_ROUTE(heartbeat, "/heartbeat")
{
    enum lwan_http_status status = lwan_request_websocket_upgrade(request);

    if (status != HTTP_SWITCHING_PROTOCOLS)
        return status;

    while (true) {
        lwan_strbuf_printf(response->buffer, "Some random integer: %d", rand());
        lwan_response_websocket_write_text(request);
        lwan_request_sleep(request, 1000);
    }

    __builtin_unreachable();
}

static void handleMsg(void *data) {}
LWAN_HANDLER_ROUTE(wsapi, "/wsapi")
{
    enum lwan_http_status status = lwan_request_websocket_upgrade(request);
    struct lwan_strbuf *last_msg_recv;
    int seconds_since_last_msg = 0;
    char data_buf_fmt[256] = "{\"%s\":\"%s\"}";
    char response_buf_fmt[256] = "{\"code\":%s,\"data\":%s,\"msg\":\"%s\"}";
    if (status != HTTP_SWITCHING_PROTOCOLS)
        return status;

    last_msg_recv = lwan_strbuf_new();
    if (!last_msg_recv)
        return HTTP_INTERNAL_ERROR;
    coro_defer(request->conn->coro, free_strbuf, last_msg_recv);

    while (true) {
        switch (lwan_response_websocket_read(request)) {
        case ENOTCONN:   /* read() called before connection is websocket */
        case ECONNRESET: /* Client closed the connection */
            goto out;

        case EAGAIN: /* Nothing is available */

            lwan_request_sleep(request, 1000);
            // seconds_since_last_msg++;
            break;

        case 0: /* We got something! Copy it to echo it back */
            lwan_strbuf_set(last_msg_recv,
                            lwan_strbuf_get_buffer(response->buffer),
                            lwan_strbuf_get_length(response->buffer));
            lwan_status_info("%s", lwan_strbuf_get_buffer(last_msg_recv));
            char msgRet[256];
            yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
            yyjson_mut_val *root = yyjson_mut_obj(doc);
            yyjson_mut_doc_set_root(doc, root);
            // // int playersArr[] = {1,2, 3,4};
            // // yyjson_mut_val* players = yyjson_mut_arr_with_sint32 (doc,
            // playersArr, 4);
            handleWs(lwan_strbuf_get_buffer(last_msg_recv), msgRet);
            yyjson_mut_obj_add_str(doc, root, "type", msgRet);
            // // yyjson_mut_obj_add_val (doc, root, "players", players);
            const char *json = yyjson_mut_write(doc, 0, NULL);
            // char code[] = "202";
            // char msg[] = "no action";
            // char data_buf[128];
            // char response_buf_fmt_buf[256];

            // sprintf (data_buf, "\"\"");
            // sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf,
            // msg); seconds_since_last_msg = 0;
            lwan_strbuf_printf(response->buffer, "%s", json);
            lwan_response_websocket_write_text(request);

            lwan_request_sleep(request, 1000);
            free(json);
            yyjson_mut_doc_free(doc);
            break;
        }
    }

out:
    /* We abort the coroutine here because there's not much we can do at this
     * point as this isn't a HTTP connection anymore.  */
    coro_yield(request->conn->coro, CONN_CORO_ABORT);
    __builtin_unreachable();
}

int main(void)
{
    lwan_main();
    return 0;
}
