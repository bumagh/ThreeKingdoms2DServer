
#include "App/Common/util.h"
#include "App/Controller/WsController.h"
#include "App/Model/user.h"
#include "Library/Json/Yyjson/yyjson.h"
#include "lwan.h"
#include "lwan-pubsub.h"
#include <errno.h>
#include <stdlib.h>
static struct lwan_pubsub_topic* notice;

void handleNoticeApi (const char* json, char* ret)
{
    yyjson_doc* doc = yyjson_read (json, strlen (json), 0);
    yyjson_val* root = yyjson_doc_get_root (doc);
    yyjson_val* type = yyjson_obj_get (root, "type");
    strcpy (ret, yyjson_get_str (type));
    if(strcmp (yyjson_get_str (type), "EnterGame") == 0)
    {
        strcpy (ret, "EnterGame");
    }
}
static void pub_depart_message (void* data1, void* data2)
{
    char buffer[128];
    int r;
    char response_buf_fmt[256] =
        "{\"type\":\"%s\",\"data\":{\"pid\":\"%d\"},\"msg\":\"%s\"}";
    r = snprintf (buffer, sizeof (buffer), response_buf_fmt, "leave",
                 (int)(intptr_t)data2, "leave");

    if(r < 0 || (size_t)r >= sizeof (buffer))
        return;

    lwan_pubsub_publish ((struct lwan_pubsub_topic*)data1, buffer, (size_t)r);
}

LWAN_HANDLER_ROUTE (wsnotice, "/wsnotice")
{
    struct lwan_pubsub_subscriber* sub;
    struct lwan_pubsub_msg* msg;
    enum lwan_http_status status;
    static int total_user_count;
    int user_id;
    uint64_t sleep_time = 100;

    sub = lwan_pubsub_subscribe (notice);
    if(!sub)
        return HTTP_INTERNAL_ERROR;
    coro_defer2 (request->conn->coro, unsub_chat, notice, sub);

    status = lwan_request_websocket_upgrade (request);
    if(status != HTTP_SWITCHING_PROTOCOLS)
        return status;

    user_id = ATOMIC_INC (total_user_count);

    // lwan_strbuf_printf (response->buffer, "*** Welcome to the chat,
    // User%d!\n",
    //                    user_id);
    // lwan_response_websocket_write_text(request);

    coro_defer2 (request->conn->coro, pub_depart_message, notice,
                (void*)(intptr_t)user_id);
    char response_buf_fmt[256] =
        "{\"type\":\"%s\",\"data\":{\"pid\":\"%d\"},\"msg\":\"%s\"}";

    lwan_pubsub_publishf (notice, response_buf_fmt, "enter", user_id, "enter");
    while(true)
    {
        switch(lwan_response_websocket_read (request))
        {
        case ENOTCONN:   /* read() called before connection is websocket */
        case ECONNRESET: /* Client closed the connection */
            goto out;

        case EAGAIN: /* Nothing is available from other clients */
            while((msg = lwan_pubsub_consume (sub)))
            {
                const struct lwan_value* value = lwan_pubsub_msg_value (msg);
                lwan_status_info (value->value);
                lwan_strbuf_set (response->buffer, value->value, value->len);
                lwan_pubsub_msg_done (msg);
                lwan_response_websocket_write_text (request);
                sleep_time = 100;
            }
            lwan_request_sleep (request, sleep_time);
            if(sleep_time <= 8000)
                sleep_time += 100;
            break;

        case 0:
        {
            char response_buf_fmt[256] =
                "{\"type\":\"%s\",\"data\":\"%s\",\"pid\":\"%d\"}";
            char buf[256] = {0};
            char bufRet[256] = {0};
            sprintf (buf, "%.*s", lwan_strbuf_get_length (response->buffer), lwan_strbuf_get_buffer (response->buffer));
            handleNoticeApi (buf, bufRet);
            // lwan_pubsub_publishf (notice, response_buf_fmt, "notice", bufRet, user_id);
            lwan_pubsub_publishf (notice, buf);
            break;
        }
        }
    }

out:
    /* We abort the coroutine here because there's not much we can do at this
     * point as this isn't a HTTP connection anymore.  */
    coro_yield (request->conn->coro, CONN_CORO_ABORT);
    __builtin_unreachable ();
}

int main (void)
{
    notice = lwan_pubsub_new_topic ();

    lwan_main ();

    lwan_pubsub_free_topic (notice);
    return 0;
}
