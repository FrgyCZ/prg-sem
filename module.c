#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "event_queue.h"
#include "keyboard_module.h"
#include "main_module.h"
#include "messages.h"
#include "prg_io_nonblock.h"
#include "utils.h"

#ifndef IO_READ_TIMEOUT_MS
#define IO_READ_TIMEOUT_MS 100
#endif

void *read_pipe_thread(void *d);

int main(int argc, char *argv[]) {
    int ret = EXIT_SUCCESS;
    const char *fname_pipe_in = "/tmp/computational_module.in";
    const char *fname_pipe_out = "/tmp/computational_module.out";
    int pipe_in = io_open_read(fname_pipe_in);
    int pipe_out = io_open_write(fname_pipe_out);

    my_assert(pipe_in != -1 && pipe_out != -1, __func__, __LINE__, __FILE__);

    enum { READ_PIPE_THRD, KEYBOARD_THRD, MAIN_COMP_THRD, NUM_THREADS };
    const char *thrd_names[] = {"ReadPipe", "ReadKeyboard", "MainComp"};
    void *(*thrd_functions[])(void *) = {read_pipe_thread, keyboard_thread, main_module_thread};
    pthread_t threads[NUM_THREADS];
    void *thrd_data[NUM_THREADS];
    thrd_data[READ_PIPE_THRD] = &pipe_in;
    thrd_data[MAIN_COMP_THRD] = &pipe_out;

    for (int i = 0; i < NUM_THREADS; ++i) {
        int r = pthread_create(&threads[i], NULL, thrd_functions[i], thrd_data[i]);
        printf("Create thread '%s' %s\r\n", thrd_names[i], (r == 0 ? "OK" : "FAIL"));
    }

    int *ex;
    for (int i = 0; i < NUM_THREADS; ++i) {
        printf("Call join to the thread %s\r\n", thrd_names[i]);
        int r = pthread_join(threads[i], (void *)&ex);
        printf("Joining the thread %s has been %s\r\n", thrd_names[i], (r == 0 ? "OK" : "FAIL"));
    }

    io_close(pipe_in);
    io_close(pipe_out);

    return ret;
}

void *read_pipe_thread(void *d) {
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_in = *(int *)d;
    fprintf(stderr, "Read_pipe_thread - start\n");
    bool end = false;
    uint8_t msg_buf[sizeof(message)];
    size_t i = 0;
    int len = 0;

    unsigned char c;
    while (io_getc_timeout(pipe_in, IO_READ_TIMEOUT_MS, &c) > 0); // discard garbage

    event startup = {.type = EV_STARTUP};
    queue_push(startup);

    while (!end) {
        int r = io_getc_timeout(pipe_in, IO_READ_TIMEOUT_MS, &c); // nacitame pomoci io_getc_timeout
        if (r > 0) {                                              // char has been read
            if (i == 0) {
                len = 0;
                if (get_message_size(c, &len)) {
                    msg_buf[i++] = c;
                } else {
                    fprintf(stderr, "ERROR: unknown message type 0x%x\n", c);
                }
            } else {
                // read remaining bytes of the message
                msg_buf[i++] = c;
            }
            if (len > 0 && i == len) {
                message *msg = my_alloc(sizeof(message));
                if (parse_message_buf(msg_buf, len, msg)) {
                    event ev;
                    ev.data.msg = msg;
                    switch (msg->type)
                    {
                    case MSG_ABORT:
                        ev.type = EV_ABORT;
                        break;
                    case MSG_GET_VERSION:
                        ev.type = EV_GET_VERSION;
                        break;
                    case MSG_SET_COMPUTE:
                        ev.type = EV_SET_COMPUTE;
                        break;
                    case MSG_COMPUTE:
                        ev.type = EV_COMPUTE;
                    default:
                        break;
                    }
                    queue_push(ev);
                } else {
                    fprintf(stderr, "ERROR: cannot parse message type %d\n", msg_buf[0]);
                    free(msg);
                }
                i = len = 0;
            }
        } else if (r == 0) {
            // TODO timeout
        } else {
            fprintf(stderr, "ERROR: reading from pipe\n");
            set_quit();
            event ev = { .type = EV_QUIT };
            queue_push(ev);
        }
        end = is_quit();
    }
    fprintf(stderr, "Read_pipe_thread - finished\n");
    return NULL;
}
