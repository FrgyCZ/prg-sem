/*
 * Filename: event_queue.h
 * Date:     2017/04/15 12:41
 * Author:   Jan Faigl
 */

#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "messages.h"

typedef enum {
    EV_COMPUTE,     // request compute on nucleo with particular
    EV_RESET_CHUNK, // reset the chunk id
    EV_ABORT,
    EV_GET_VERSION,
    EV_THREAD_EXIT,
    EV_QUIT,
    EV_SERIAL,
    EV_SET_COMPUTE,
    EV_COMPUTE_CPU,
    EV_CLEAR_BUFFER,
    EV_REFRESH,
    EV_PIPE_IN_MESSAGE,
    EV_STARTUP,
    EV_COMPUTE_DATA_DONE,
    EV_FORCED_COMPUTE,
    EV_ZOOM_IN,
    EV_ZOOM_OUT,
    EV_SAVE_IMAGE,
    EV_UP,
    EV_DOWN,
    EV_RIGHT,
    EV_LEFT,
    EV_C_RE_INCREASE,
    EV_C_RE_DECREASE,
    EV_C_IM_INCREASE,
    EV_C_IM_DECREASE,
    EV_TOGGLE_DEBUG,
    EV_CLOSE_STARTUP,
    EV_RESOLUTION_UP,
    EV_RESOLUTION_DOWN,
    EV_COMPUTE_ABORT,
    EV_TYPE_NUM
} event_type;

typedef struct {
    int param;
} event_keyboard;

typedef struct {
    message *msg;
} event_serial;

typedef struct {
    // event_source source;
    event_type type;
    union {
        int param;
        message *msg;
    } data;
} event;

void queue_init(void);
void queue_cleanup(void);

event queue_pop(void);

void queue_push(event ev);

bool is_quit();

void set_quit();

#endif

/* end of event_queue.h */
