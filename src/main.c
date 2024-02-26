
#include "App/Controller/WsController.h"
#include "App/Model/user.h"
#include "Library/Json/Yyjson/yyjson.h"
#include "lwan.h"
#include "lwan-pubsub.h"
#include <errno.h>
#include <stdlib.h>
static struct lwan_pubsub_topic* notice;
static void free_strbuf (void* data)
{
    lwan_strbuf_free ((struct lwan_strbuf*)data);
}

LWAN_HANDLER_ROUTE (heartbeat, "/heartbeat")
{
    enum lwan_http_status status = lwan_request_websocket_upgrade (request);

    if(status != HTTP_SWITCHING_PROTOCOLS)
        return status;

    while(true)
    {
        lwan_strbuf_printf (response->buffer, "Some random integer: %d", rand ());
        lwan_response_websocket_write_text (request);
        lwan_request_sleep (request, 1000);
    }

    __builtin_unreachable ();
}

LWAN_HANDLER_ROUTE (wsapi, "/wsapi")
{
    enum lwan_http_status status = lwan_request_websocket_upgrade (request);
    struct lwan_strbuf* last_msg_recv;
    int seconds_since_last_msg = 0;
    char data_buf_fmt[256] = "{\"%s\":\"%s\"}";
    char response_buf_fmt[256] = "{\"code\":%s,\"data\":%s,\"msg\":\"%s\"}";
    if(status != HTTP_SWITCHING_PROTOCOLS)
        return status;

    last_msg_recv = lwan_strbuf_new ();
    if(!last_msg_recv)
        return HTTP_INTERNAL_ERROR;
    coro_defer (request->conn->coro, free_strbuf, last_msg_recv);

    while(true)
    {
        switch(lwan_response_websocket_read (request))
        {
        case ENOTCONN:   /* read() called before connection is websocket */
        case ECONNRESET: /* Client closed the connection */
            goto out;

        case EAGAIN: /* Nothing is available */

            lwan_request_sleep (request, 1000);
            // seconds_since_last_msg++;
            break;

        case 0: /* We got something! Copy it to echo it back */
            lwan_strbuf_set (last_msg_recv,
                            lwan_strbuf_get_buffer (response->buffer),
                            lwan_strbuf_get_length (response->buffer));
            lwan_status_info ("%s", lwan_strbuf_get_buffer (last_msg_recv));
            char msgRet[256];
            yyjson_mut_doc* doc = yyjson_mut_doc_new (NULL);
            yyjson_mut_val* root = yyjson_mut_obj (doc);
            yyjson_mut_doc_set_root (doc, root);
            // int playersArr[] = {1,2, 3,4};
            // yyjson_mut_val* players = yyjson_mut_arr_with_sint32 (doc,
            // playersArr, 4);
            handleWsApi (lwan_strbuf_get_buffer (last_msg_recv), msgRet);
            yyjson_mut_obj_add_str (doc, root, "type", msgRet);
            // yyjson_mut_obj_add_val (doc, root, "players", players);
            const char* json = yyjson_mut_write (doc, 0, NULL);

            lwan_strbuf_printf (response->buffer, "%s", json);
            lwan_response_websocket_write_text (request);

            lwan_request_sleep (request, 1000);
            free (json);
            yyjson_mut_doc_free (doc);
            break;
        }
    }

out:
    /* We abort the coroutine here because there's not much we can do at this
     * point as this isn't a HTTP connection anymore.  */
    coro_yield (request->conn->coro, CONN_CORO_ABORT);
    __builtin_unreachable ();
}

static void unsub_chat (void* data1, void* data2)
{
    lwan_pubsub_unsubscribe ((struct lwan_pubsub_topic*)data1,
                            (struct lwan_pubsub_subscriber*)data2);
}

static void pub_depart_message (void* data1, void* data2)
{
    char buffer[128];
    int r;

    r = snprintf (buffer, sizeof (buffer), "*** User%d has departed the chat!\n",
                 (int)(intptr_t)data2);
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
    uint64_t sleep_time = 1000;

    sub = lwan_pubsub_subscribe (notice);
    if(!sub)
        return HTTP_INTERNAL_ERROR;
    coro_defer2 (request->conn->coro, unsub_chat, notice, sub);

    status = lwan_request_websocket_upgrade (request);
    if(status != HTTP_SWITCHING_PROTOCOLS)
        return status;

    user_id = ATOMIC_INC (total_user_count);

    // lwan_strbuf_printf (response->buffer, "*** Welcome to the chat, User%d!\n",
    //                    user_id);
    lwan_response_websocket_write_text (request);

    coro_defer2 (request->conn->coro, pub_depart_message, notice,
                (void*)(intptr_t)user_id);
    lwan_pubsub_publishf (notice, "*** User%d has joined the chat!\n", user_id);

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

                lwan_strbuf_set (response->buffer, value->value, value->len);
                lwan_pubsub_msg_done (msg);
                lwan_response_websocket_write_text (request);
                sleep_time = 500;
            }
            lwan_request_sleep (request, sleep_time);
            if(sleep_time <= 8000)
                sleep_time += 500;
            break;

        case 0:

            lwan_pubsub_publishf (notice, "User%d: %.*s\n", user_id,
                                 (int)lwan_strbuf_get_length (response->buffer),
                                 lwan_strbuf_get_buffer (response->buffer));
            break;
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
