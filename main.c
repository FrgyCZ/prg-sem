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

static bool debug_enabled = false;
static int frame = 0;

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

    display_startup_message();

    do {
        event ev = queue_pop();
        msg.type = MSG_NBR;
        switch (ev.type) {
        case EV_QUIT:
            set_quit();
            debug("Quit received");
            break;
        case EV_GET_VERSION:
            info("Requesting version");
            msg.type = MSG_GET_VERSION;
            break;
        case EV_SET_COMPUTE:
            info(set_compute(&msg) ? "Set compute" : "Set compute failed");
            break;
        case EV_FORCED_COMPUTE:
            enable_comp();
            info(compute(&msg) ? "Compute" : "Compute failed");
            msg.data.compute.forced = true;
            break;
        case EV_COMPUTE:
            info(compute(&msg) ? "Compute" : "Compute failed");
            msg.data.compute.forced = false;
            break;
        case EV_COMPUTE_CPU:
            info("Compute with CPU");
            cpu_comp();
            gui_refresh();
            break;
        case EV_ABORT:
            info("Abort request is being sent");
            msg.type = MSG_ABORT;
            break;
        case EV_PIPE_IN_MESSAGE:
            process_pipe_message(&ev);
            break;
        case EV_RESET_CHUNK:
            info("Reseting cid");
            cid_reset();
            break;
        case EV_REFRESH:
            info("GUI has been refreshed");
            gui_refresh();
            break;
        case EV_CLEAR_BUFFER:
            info("Buffer has been cleared");
            clear_buffer();
            break;
        case EV_ZOOM_IN:
            info("Zoom in");
            zoom(1.0 / ZOOM_COEFFICIENT);
            debug_view();
            break;
        case EV_ZOOM_OUT:
            info("Zoom out");
            zoom(ZOOM_COEFFICIENT);
            debug_view();
            break;
        case EV_SAVE_IMAGE:
            info("Saving image");
            gui_save_image();
            break;
        case EV_UP:
            info("Moved up");
            move(0, MOVE_AMOUNT);
            debug_view();
            break;
        case EV_DOWN:
            info("Moved down");
            move(0, -MOVE_AMOUNT);
            debug_view();
            break;
        case EV_LEFT:
            info("Moved left");
            move(-MOVE_AMOUNT, 0);
            debug_view();
            break;
        case EV_RIGHT:
            info("Moved right");
            move(MOVE_AMOUNT, 0);
            debug_view();
            break;
        case EV_C_RE_INCREASE:
            info("Increased c_re");
            change_c_re(C_CHANGE_COEFFICIENT);
            debug_view();
            break;
        case EV_C_RE_DECREASE:
            info("Decreased c_re");
            change_c_re(-C_CHANGE_COEFFICIENT);
            debug_view();
            break;
        case EV_C_IM_INCREASE:
            info("Increased c_im");
            change_c_im(C_CHANGE_COEFFICIENT);
            debug_view();
            break;
        case EV_C_IM_DECREASE:
            info("Decreased c_im");
            change_c_im(-C_CHANGE_COEFFICIENT);
            debug_view();
            break;
        case EV_RESOLUTION_UP:
            info("Increasing window resolution");
            change_resolution(1);
            clear_buffer();
            gui_refresh();
            break;
        case EV_RESOLUTION_DOWN:
            info("Decreasing window resolution");
            change_resolution(0);
            clear_buffer();
            gui_refresh();
            break;
        case EV_GET_HELP:
            display_startup_message();
            break;
        case EV_TOGGLE_DEBUG:
            debug_enabled = !debug_enabled;
            printf("Debug %s\n", debug_enabled ? "enabled" : "disabled");
            break;
        case EV_SAVE_ANIMATION:
            info("Saving animation");
            system("bash create_animation.sh anim");
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
    case MSG_COMPUTE:
        info("Compute");
        break;
    case MSG_STARTUP:
        fprintf(stderr, "Startup received, module \"%s\" is ONLINE!\n", msg->data.startup.message);
        break;
    default:
        fprintf(stderr, "Unhandled pipe message type %d, oh shit this is the end\n", msg->type);
        break;
    }
    free(ev->data.msg);
    ev->data.msg = NULL;
}

void debug_view(void) {
    if (debug_enabled)
    {
        cpu_comp();
        gui_refresh();
        gui_record_animation(frame++);
    }
}

/* end of b3b36prg-sem/main.c */
