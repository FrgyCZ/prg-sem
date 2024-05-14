
#include <SDL2/SDL.h>
#include "xwin_sdl.h"

#include "utils.h"
#include "computation.h"

#include "event_queue.h"
#include "gui.h"


#ifndef SDL_EVENT_POLL_WAIT_MS
#define SDL_EVENT_POLL_WAIT_MS 1
#endif

static struct {
    int w;
    int h;
    unsigned char *img;
} gui = { .img = NULL };

void gui_init(void)
{
    get_grid_size(&gui.w, &gui.h);
    gui.img = my_alloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__, __LINE__, __FILE__);
}
void gui_cleanup(void)
{
    if (gui.img) {
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close();
}

void gui_refresh(void)
{
    if (gui.img) {
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }
}

void *gui_win_thread(void *d)
{
    info("gui_win_thread - start");
    bool quit = false;
    SDL_Event event_sdl;
    event ev;
    while (!quit) {
        ev.type = EV_TYPE_NUM;
        if (SDL_PollEvent(&event_sdl)) {
            if (event_sdl.type == SDL_QUIT) {
                ev.type = EV_QUIT;
            }
            else if (event_sdl.type == SDL_KEYDOWN) {
                switch (event_sdl.key.keysym.sym) {
                    case SDLK_q:
                        ev.type = EV_QUIT;
                        break;
                    case SDLK_s:
                        ev.type = EV_SET_COMPUTE;
                        break;
                    case SDLK_a:
                        ev.type = EV_ABORT;
                        break;
                    case SDLK_c:
                        ev.type = EV_COMPUTE_CPU;
                        break;
                    case SDLK_g:
                        ev.type = EV_GET_VERSION;
                        break;
                    case SDLK_r:
                        ev.type = EV_RESET_CHUNK;
                        break;
                    case SDLK_p:
                        ev.type = EV_REFRESH;
                        break;
                    case SDLK_l:
                        ev.type = EV_CLEAR_BUFFER;
                        break;
                    case SDLK_KP_1:
                        ev.type = EV_FORCED_COMPUTE;
                        break;
                }
            } else if (event_sdl.type == SDL_KEYUP) {
                info("gui_win_thread - keyup");
            }
            if (event_sdl.type == SDL_MOUSEBUTTONDOWN) {
                info("gui_win_thread - mousebuttondown");
                if (event_sdl.button.button == SDL_BUTTON_LEFT) {
                    int x = event_sdl.button.x;
                    int y = event_sdl.button.y; 
                    fprintf(stderr, "gui_win_thread - mousebuttondown - left - x: %d, y: %d", x, y);
                    //TODO 
                } else if (event_sdl.button.button == SDL_BUTTON_RIGHT) {
                    info("gui_win_thread - mousebuttondown - right");
                }
            }
        }
        if (ev.type != EV_TYPE_NUM) {
            queue_push(ev);
        }
        SDL_Delay(SDL_EVENT_POLL_WAIT_MS);
        quit = is_quit();
    }
    set_quit();
    info("gui_win_thread - end");
    return NULL;
}

/* end of gui.c */
