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
                info("Wakey wakey!");
                send_startup_message(&msg);
                break;
            case EV_GET_VERSION:
                info("Get version requested");
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
                    info("New forced compute requested");
                }
                else {
                    if (is_module_aborted())
                    {
                        warn("Compute requested, but the process has been previously aborted");
                        info("If you want to continue, do so by sending a forced compute event");
                    }
                }
                module_compute(ev.data.msg);
                break;
            case EV_COMPUTE_DATA_DONE:
                info("Compute data done");
                msg.type = MSG_DONE;
                break;
            case EV_ABORT:
                info("Abort requested");
                abort_module_compute();
                msg.type = MSG_OK;
                break;
            case EV_COMPUTE_ABORT:
                info("Compute abort requested");
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
    msg->data.startup.message[0] = 'J';
    msg->data.startup.message[1] = 'a';
    msg->data.startup.message[2] = 'k';
    msg->data.startup.message[3] = 'u';
    msg->data.startup.message[4] = 'b';
    msg->data.startup.message[5] = ' ';
    msg->data.startup.message[6] = 'K';
    msg->data.startup.message[7] = 'O';
    msg->data.startup.message[8] = 'L';
    msg->data.startup.message[9] = 'A';
    msg->data.startup.message[10] = 'C';
    msg->data.startup.message[11] = '\0';
    msg->type = MSG_STARTUP;
}
