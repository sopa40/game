#define _POSIX_C_SOURCE 2
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include "SDL2/SDL_image.h"
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
#include <time.h>


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

/* Global constants, typed for non-strings, macros for some strings to use compile-time concat
#define PROGNAME xstr(CONFIG_PROGNAME)
static const int POLL_RATE_MS = CONFIG_POLL_RATE_MS;
static const int WINDOW_WIDTH = CONFIG_WINDOW_WIDTH;
static const int WINDOW_HEIGHT = CONFIG_WINDOW_HEIGHT; */

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

typedef struct
{
    int x, y;
} Star;

typedef struct
{

    Man man;
    Star stars[10];
    SDL_Texture *star;
    SDL_Renderer *renderer;

} Gamestate;

void loadGame(Gamestate *game){
    SDL_Surface *starSurface = NULL;
    starSurface = IMG_Load("star.png");
    if(starSurface == NULL){
        printf("Can not find star.png!\n");
        SDL_Quit();
        exit(1);
    }
    game->star = SDL_CreateTextureFromSurface(game->renderer, starSurface);
    SDL_FreeSurface(starSurface);
    game->man.x = 220;
    game->man.y = 140;

    for(int i = 0; i < 10; i++){
        game->stars[i].x = random()%640;
        game->stars[i].y = random()%480;
    }
}

void render(SDL_Renderer *renderer, Gamestate *game)
{
    SDL_SetRenderDrawColor(renderer, 0, 0 ,255, 255);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_Rect rect = { game->man.x, game->man.y, 100, 100 };
    SDL_RenderFillRect(renderer, &rect);


    for(int i = 0; i < 10; i++){
        SDL_Rect starRect = { game->stars[i].x, game->stars[i].y, 64, 64};
        SDL_RenderCopy(renderer, game->star, NULL, &starRect);

    }
    SDL_RenderPresent(renderer);

}

int processEvents(SDL_Window *window, Gamestate *game)
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
                }
                break;
                case SDL_QUIT:
                    done = 1;
                break;
            }
        }
    }
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_LEFT])
        game->man.x -= 10;
    if(state[SDL_SCANCODE_RIGHT])
        game->man.x += 10;
    if(state[SDL_SCANCODE_DOWN])
        game->man.y += 10;
    if(state[SDL_SCANCODE_UP])
        game->man.y -= 10;
    return done;
}

int main(void)
{
    Gamestate game;
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Init(SDL_INIT_VIDEO);
    srandom((int)time(NULL));

    window = SDL_CreateWindow("Game Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640,
                              480,
                              0
                              );



    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    game.renderer = renderer;
    loadGame(&game);

    int done = 0;

    while(!done){
        done = processEvents(window, &game);
        render(renderer,&game);
        SDL_Delay(100);
    }

    SDL_DestroyTexture(game.star);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    SDL_Quit();
    return 0;
}


