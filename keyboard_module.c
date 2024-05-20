#include <stdio.h>
#include <stdlib.h>

#include "event_queue.h"
#include "keyboard_module.h"
#include "utils.h"

void *keyboard_thread(void *d) {
    fprintf(stderr, "Keyboard_thread - start\n");
    call_termios(0);
    int c;
    event ev;
    while ((c = getchar()) != 'q') {
        if (c == 'a')
        {
            ev.type = EV_COMPUTE_ABORT;
            queue_push(ev);
        }
    } // end while
    set_quit();
    ev.type = EV_QUIT;
    queue_push(ev);
    call_termios(1);
    fprintf(stderr, "Keyboard_thread - end\n");
    return NULL;
}