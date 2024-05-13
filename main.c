#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "computation.h"
#include "event_queue.h"
#include "gui.h"
#include "main.h"
#include "messages.h"
#include "utils.h"

static void process_pipe_message(event *const ev);

void *main_thread(void *d) {
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_out = *(int *)d;
    message msg;
    uint8_t msg_buf[sizeof(message)];
    int msg_len;
    bool quit = false;

    // initialize computation, visualize
    computation_init();
    gui_init();

    do {
        event ev = queue_pop();
        msg.type = MSG_NBR;
        switch (ev.type) {
        case EV_QUIT:
            set_quit();
            debug("Quit received");
            break;
        case EV_GET_VERSION:
            msg.type = MSG_GET_VERSION;
            break;
        case EV_SET_COMPUTE:
            info(set_compute(&msg) ? "Set compute" : "Set compute failed");
            break;
        case EV_COMPUTE:
            enable_comp();
            info(compute(&msg) ? "Compute" : "Compute failed");
            break;
        case EV_COMPUTE_CPU:
            cpu_comp();
            gui_refresh();
            break;
        case EV_ABORT:
            msg.type = MSG_ABORT;
            break;
        case EV_PIPE_IN_MESSAGE:
            process_pipe_message(&ev);
            break;
        case EV_RESET_CHUNK:
            cid_reset();
            break;
        case EV_REFRESH:
            gui_refresh();
            break;
        case EV_CLEAR_BUFFER:
            clear_buffer();
            break;
        default:
            break;
        } // switch end
        if (msg.type != MSG_NBR) {
            my_assert(fill_message_buf(&msg, msg_buf, sizeof(msg_buf), &msg_len), __func__, __LINE__, __FILE__);
            if (write(pipe_out, msg_buf, msg_len) == msg_len) {
                debug("sent date to pipe_out\n");
            } else {
                error("send messange fail\n");
            }
        }
        quit = is_quit();
    } while (!quit);

    // cleanup computation, visualize
    computation_cleanup();
    gui_cleanup();
    return NULL;
}

void process_pipe_message(event *const ev) {
    my_assert(ev != NULL && ev->type == EV_PIPE_IN_MESSAGE && ev->data.msg, __func__, __LINE__, __FILE__);
    ev->type = EV_TYPE_NUM;
    const message *msg = ev->data.msg;
    switch (msg->type) {
    case MSG_OK:
        info("OK");
        break;
    case MSG_COMPUTE_DATA:
        if (!is_abort()) {
            update_data(&(msg->data.compute_data));
        }
        break;
    case MSG_DONE:
        gui_refresh();
        if (is_done()) {
            info("computation_done");
        } else {
            event ev = {.type = EV_COMPUTE};
            queue_push(ev);
        }
        break;
    case MSG_VERSION:
        fprintf(stderr, "INFO: Module version %d.%d-%d\n", msg->data.version.major, msg->data.version.minor,
                msg->data.version.patch);
        break;
    case MSG_ABORT:
        info("Computation aborted");
        abort_comp();
        break;
    // TODO '1',
    case MSG_COMPUTE:
        // TODO
        info("Compute");
        break;
    case MSG_STARTUP:
        fprintf(stderr, "INFO: %s\n", msg->data.startup.message);
        break;
    default:
        fprintf(stderr, "Unhandled pipe message type %d\n", msg->type);
        break;
    }
    free(ev->data.msg);
    ev->data.msg = NULL;
}

/* end of b3b36prg-sem/main.c */
