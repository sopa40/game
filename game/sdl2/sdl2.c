#define _POSIX_C_SOURCE 2
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Configurable items */
#define CONFIG_PROGNAME sdl2_test
#define CONFIG_POLL_RATE_MS (5)
#define CONFIG_WINDOW_WIDTH 800
#define CONFIG_WINDOW_HEIGHT 600
/* ----------------- */

/* utility macros */
#define xstr(s) str(s)
#define str(s) #s
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

/* Global constants, typed for non-strings, macros for some strings to use compile-time concat */
#define PROGNAME xstr(CONFIG_PROGNAME)
static const int POLL_RATE_MS = CONFIG_POLL_RATE_MS;
static const int WINDOW_WIDTH = CONFIG_WINDOW_WIDTH;
static const int WINDOW_HEIGHT = CONFIG_WINDOW_HEIGHT;

/* Global variables, shouldn't be many */

/* Handles events
 * returns -1 for quit event
 */




typedef struct
{
    int x, y;
    short life;
    char *name;
} Man;

void render(SDL_Renderer *renderer, Man *man)
{
    SDL_SetRenderDrawColor(renderer, 0, 0 ,255, 255);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_Rect rect = { man->x, man->y, 200, 200 };
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);
}

int processEvents(SDL_Window *window, Man *man)
{
    SDL_Event event;
    int done = 0;

    while(SDL_PollEvent(&event)){
        switch(event.type)
        {
            case SDL_WINDOWEVENT_CLOSE:
            {
                if(window){
                    SDL_DestroyWindow(window);
                    window = NULL;
                    done = 1;
                }
            }
            break;
            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        done = 1;
                    break;
                    case SDLK_RIGHT:
                        man->x += 10;
                    break;
                    case SDLK_LEFT:
                        man->x += -10;
                    break;
                    case SDLK_DOWN:
                        man->y += +10;
                    break;
                    case SDLK_UP:
                        man->y += -10;
                    break;
                }
                break;
                case SDL_QUIT:
                    done = 1;
                break;
            }
        }
    }

    return done;
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Init(SDL_INIT_VIDEO);

    Man man;
    man.x = 220;
    man.y = 140;

    window = SDL_CreateWindow("Game Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640,
                              480,
                              0
                              );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int done = 0;

    while(!done){
        done = processEvents(window, &man);

        render(renderer,&man);

        SDL_Delay(100);

    }


    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    SDL_Quit();
    return 0;
}


