#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "computation_module.h"
#include "main_module.h"
#include "event_queue.h"
#include "messages.h"
#include "utils.h"

void *main_module_thread(void *d)
{
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_out = *(int *)d;
    message msg;
    uint8_t msg_buf[sizeof(message)];
    int msg_len;
    bool quit = false;
    do {
        event ev = queue_pop();
        msg.type = MSG_NBR;
        switch (ev.type) {
            case EV_QUIT:
                set_quit();
                debug("Quit received");
                break;
            case EV_STARTUP:
                send_startup_message(&msg);
                break;
            case EV_GET_VERSION:
                msg.type = MSG_VERSION;
                msg.data.version.major = 4;
                msg.data.version.minor = 20;
                msg.data.version.patch = 69;
                break;
            case EV_SET_COMPUTE:
                info(set_module_compute(ev.data.msg, d) ? "Set compute" : "Set compute failed");
                break;
            case EV_COMPUTE:
                if (ev.data.msg->data.compute.forced)
                {
                    enable_module_compute();
                }
                module_compute(ev.data.msg);
                break;
            case EV_COMPUTE_DATA_DONE:
                msg.type = MSG_DONE;
                break;
            case EV_ABORT:
                abort_module_compute();
                msg.type = MSG_OK;
                break;
            case EV_COMPUTE_ABORT:
                abort_module_compute();
                msg.type = MSG_ABORT;
                break;
            default:
                break;
        } // switch end
        if (!(ev.type == EV_QUIT || ev.type == EV_COMPUTE_ABORT))
        {
            free(ev.data.msg);
        }
        if (msg.type != MSG_NBR) {
            my_assert(fill_message_buf(&msg, msg_buf, sizeof(msg_buf), &msg_len), __func__, __LINE__, __FILE__);
            if (!(write(pipe_out, msg_buf, msg_len) == msg_len)) {
                error("send message fail\n");
            }
        }
        quit = is_quit();
    } while (!quit);
    return NULL;
}

void send_startup_message(message *msg) {
    msg->data.startup.message[0] = 'M';
    msg->data.startup.message[1] = 'R';
    msg->data.startup.message[2] = 'D';
    msg->data.startup.message[3] = 'A';
    msg->data.startup.message[4] = 'T';
    msg->data.startup.message[5] = '\0';
    msg->type = MSG_STARTUP;
}
