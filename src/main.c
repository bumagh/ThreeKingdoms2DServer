
#include <errno.h>
#include <stdlib.h>
// #include "Model/model.h"
#include "lwan.h"
#include "lwan-pubsub.h"
static struct lwan_pubsub_topic *chat;

LWAN_HANDLER_ROUTE(ws_write, "/ws-write")
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

static void free_strbuf(void *data)
{
    lwan_strbuf_free((struct lwan_strbuf *)data);
}

LWAN_HANDLER_ROUTE(ws_read, "/ws-read")
{
    enum lwan_http_status status = lwan_request_websocket_upgrade(request);
    struct lwan_strbuf *last_msg_recv;
    int seconds_since_last_msg = 0;

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
            lwan_strbuf_printf(response->buffer,
                               "Last message was received %d seconds ago: %.*s",
                               seconds_since_last_msg,
                               (int)lwan_strbuf_get_length(last_msg_recv),
                               lwan_strbuf_get_buffer(last_msg_recv));
            lwan_response_websocket_write_text(request);

            lwan_request_sleep(request, 1000);
            seconds_since_last_msg++;
            break;

        case 0: /* We got something! Copy it to echo it back */
            lwan_strbuf_set(last_msg_recv,
                            lwan_strbuf_get_buffer(response->buffer),
                            lwan_strbuf_get_length(response->buffer));

            seconds_since_last_msg = 0;

            break;
        }
    }

out:
    /* We abort the coroutine here because there's not much we can do at this
     * point as this isn't a HTTP connection anymore.  */
    coro_yield(request->conn->coro, CONN_CORO_ABORT);
    __builtin_unreachable();
}

static void unsub_chat(void *data1, void *data2)
{
    lwan_pubsub_unsubscribe((struct lwan_pubsub_topic *)data1,
                            (struct lwan_pubsub_subscriber *)data2);
}

static void pub_depart_message(void *data1, void *data2)
{
    char buffer[128];
    int r;

    r = snprintf(buffer, sizeof(buffer), "*** User%d has departed the chat!\n",
                 (int)(intptr_t)data2);
    if (r < 0 || (size_t)r >= sizeof(buffer))
        return;

    lwan_pubsub_publish((struct lwan_pubsub_topic *)data1, buffer, (size_t)r);
}

LWAN_HANDLER_ROUTE(ws_chat, "/ws-chat")
{
    struct lwan_pubsub_subscriber *sub;
    struct lwan_pubsub_msg *msg;
    enum lwan_http_status status;
    static int total_user_count;
    int user_id;
    uint64_t sleep_time = 1000;

    sub = lwan_pubsub_subscribe(chat);
    if (!sub)
        return HTTP_INTERNAL_ERROR;
    coro_defer2(request->conn->coro, unsub_chat, chat, sub);

    status = lwan_request_websocket_upgrade(request);
    if (status != HTTP_SWITCHING_PROTOCOLS)
        return status;

    user_id = ATOMIC_INC(total_user_count);

    lwan_strbuf_printf(response->buffer, "*** Welcome to the chat, User%d!\n",
                       user_id);
    lwan_response_websocket_write_text(request);

    coro_defer2(request->conn->coro, pub_depart_message, chat,
                (void *)(intptr_t)user_id);
    lwan_pubsub_publishf(chat, "*** User%d has joined the chat!\n", user_id);

    while (true) {
        switch (lwan_response_websocket_read(request)) {
        case ENOTCONN:   /* read() called before connection is websocket */
        case ECONNRESET: /* Client closed the connection */
            goto out;

        case EAGAIN: /* Nothing is available from other clients */
            while ((msg = lwan_pubsub_consume(sub))) {
                const struct lwan_value *value = lwan_pubsub_msg_value(msg);

                lwan_strbuf_set(response->buffer, value->value, value->len);

                /* Mark as done before writing: websocket_write() can abort the
                 * coroutine and we want to drop the reference before this
                 * happens. */
                lwan_pubsub_msg_done(msg);

                lwan_response_websocket_write_text(request);
                sleep_time = 500;
            }

            lwan_request_sleep(request, sleep_time);

            /* We're receiving a lot of messages, wait up to 1s (500ms in the
             * loop above, and 500ms in the increment below). Otherwise, wait
             * 500ms every time we return from lwan_request_sleep() until we
             * reach 8s.  This way, if a chat is pretty busy, we'll have a lag
             * of at least 1s -- which is probably fine; if it's not busy, we
             * can sleep a bit more and conserve some resources. */
            if (sleep_time <= 8000)
                sleep_time += 500;
            break;

        case 0: /* We got something! Copy it to echo it back */
            lwan_pubsub_publishf(chat, "User%d: %.*s\n", user_id,
                                 (int)lwan_strbuf_get_length(response->buffer),
                                 lwan_strbuf_get_buffer(response->buffer));
            break;
        }
    }

out:
    /* We abort the coroutine here because there's not much we can do at this
     * point as this isn't a HTTP connection anymore.  */
    coro_yield(request->conn->coro, CONN_CORO_ABORT);
    __builtin_unreachable();
}

LWAN_HANDLER_ROUTE(login, "/login")
{
    request->response.mime_type = "text/html;charset=utf-8";
    char data_buf_fmt[256] = "{\"%s\":\"%s\"}";
    char response_buf_fmt[256] = "{\"code\":%s,\"data\":%s,\"msg\":\"%s\"}";

    if (lwan_request_get_post_param(request, "action") == NULL) {
        char code[] = "202";
        char msg[] = "no action";
        char data_buf[128];
        char response_buf_fmt_buf[256];

        sprintf(data_buf, "\"\"");
        sprintf(response_buf_fmt_buf, response_buf_fmt, code, data_buf, msg);
        lwan_strbuf_set_static(response->buffer, response_buf_fmt_buf,
                               sizeof(response_buf_fmt_buf) - 1);
        return HTTP_OK;
    }
    // if (strcmp(lwan_request_get_post_param(request, "action"), "addUser") ==
    //     0) {
    //     const char *account = lwan_request_get_post_param(request, "account");
    //     const char *pwd = lwan_request_get_post_param(request, "pwd");

    //     if (account == NULL || pwd == NULL) {
    //         char code[] = "203";
    //         char msg[] = "no action";
    //         char data_buf[128];
    //         char response_buf_fmt_buf[256];

    //         sprintf(data_buf, "\"\"");
    //         sprintf(response_buf_fmt_buf, response_buf_fmt, code, data_buf,
    //                 msg);
    //         lwan_strbuf_set_static(response->buffer, response_buf_fmt_buf,
    //                                sizeof(response_buf_fmt_buf) - 1);
    //         return HTTP_OK;
    //     }
    //     if (model_user_add (
    //         lwan_request_get_post_param (request, "account"),
    //         strlen (lwan_request_get_post_param (request, "account")),
    //         lwan_request_get_post_param (request, "pwd"),
    //         strlen (lwan_request_get_post_param (request, "pwd"))) == 0)
    //     {

    //         char code[] = "200";
    //         char msg[] = "";
    //         char data_buf[128];
    //         char response_buf_fmt_buf[256];

    //         sprintf (data_buf, "\"\"");
    //         sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf,
    //                 msg);
    //         lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf,
    //                                sizeof (response_buf_fmt_buf) - 1);
    //     }
    //     else
    //     {
    //         char code[] = "201";
    //         char msg[] = "account exist";

    //         char data_buf[128];
    //         char response_buf_fmt_buf[256];

    //         sprintf (data_buf, "\"\"");
    //         sprintf (response_buf_fmt_buf, response_buf_fmt, code, data_buf,
    //                 msg);
    //         lwan_strbuf_set_static (response->buffer, response_buf_fmt_buf,
    //                                sizeof (response_buf_fmt_buf) - 1);
    //     }
    // }

    return HTTP_OK;
}

int main(void)
{
    chat = lwan_pubsub_new_topic();

    lwan_main();

    lwan_pubsub_free_topic(chat);

    return 0;
}
