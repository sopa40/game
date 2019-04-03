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
#include <math.h>


/* Configurable items */
#define CONFIG_POLL_RATE_MS  (5)
#define CONFIG_WINDOW_WIDTH  640
#define CONFIG_WINDOW_HEIGHT 480
#define CONFIG_HERO_HEIGHT 40
#define CONFIG_HERO_WIDTH  40
#define CONFIG_HERO_STEP   10
#define CONFIG_BARS_AMOUNT 10
#define CONFIG_BAR_WIDTH   80
#define CONFIG_BAR_HEIGHT  40
#define CONFIG_STARS_AMOUT 1
#define CONFIG_GRAVITY 0.5

/* ----------------- */
typedef struct {
    double x, y;
    double dx, dy;
    short life;
    char *name;
    int onBar;

    int animFrame, facingLeft, slowingDown;
} Man;

typedef struct {
    int x, y;
} Star;

typedef struct {
    int x, y, w, h;
} Bar;

typedef struct {
    //Players
    Man hero;

    //Stars
    Star stars[100];

    //Bars
    Bar ledges[100];

    //Images
    SDL_Texture *star;
    SDL_Texture *manFrames[2];
    SDL_Texture *brick;

    int time;

    //Renderer
    SDL_Renderer *renderer;
} GameState;

void loadGame(GameState *game);
void contactHandle(GameState *game);
void process(GameState *game);
void render(SDL_Renderer *renderer, GameState *game);
int collide2d(double x1, double y1, double x2, double y2,
              double wt1, double ht1, double wt2, double ht2);
int processEvents(SDL_Window *window, GameState *game);

void loadGame(GameState *game) {
    SDL_Surface *surface = NULL;

    //Load images and create rendering textures from them
    surface = IMG_Load("star.png");
    if (!surface) {
        printf("Cannot find star.png %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }

    game->star = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("hero.png");
    if (!surface) {
        printf("Cannot find hero.png: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }

    game->manFrames[0] = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("hero_m.png");
    if (!surface) {
        printf("Cannot find hero_m.png: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }

    game->manFrames[1] = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("brick.png");
    if (!surface) {
        printf("Cannot find brick.png: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }
    game->brick = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    game->hero.x = 320-40;
    game->hero.y = 240-40;
    game->hero.dx = 0;
    game->hero.dy = 0;
    game->hero.onBar = 0;
    game->hero.animFrame = 0;
    game->hero.facingLeft = 1;
    game->hero.slowingDown = 0;

    game->time = 0;

    //init stars
    /* for(int i = 0; i < 100; i++)
     {
       game->stars[i].x = random()%640;
       game->stars[i].y = random()%480;
     }*/

    //init ledges
    for (int i = 0; i < 100; i++) {
        game->ledges[i].w = 256;
        game->ledges[i].h = 64;
        game->ledges[i].x = i*256;
        game->ledges[i].y = 400;
    }
    game->ledges[99].x = 350;
    game->ledges[99].y = 200;

    game->ledges[98].x = 350;
    game->ledges[98].y = 350;
}

//check if 2 figures colliding
int collide2d(double x1, double y1, double x2, double y2,
              double wt1, double ht1, double wt2, double ht2) {
    return (!((x1 > (x2+wt2)) || (x2 > (x1+wt1)) ||
              (y1 > (y2+ht2)) || (y2 > (y1+ht1))));
}

void process(GameState *game) {
    //add time
    game->time++;

    //hero movement
    Man *hero = &game->hero;
    hero->x += hero->dx;
    hero->y += hero->dy;
    if (hero->onBar && !hero->slowingDown && (hero->dx > 0 || hero->dx < 0)) {
        if(game->time % 8 == 0) {
            if(hero->animFrame == 0)
                hero->animFrame = 1;
            else
                hero->animFrame = 0;
        }
    }

    hero->dy += CONFIG_GRAVITY;
}

void contactHandle(GameState *game) {
    //Check for collision with any ledges (brick blocks)
    for (int i = 0; i < 100; i++) {
        double mw = 48, mh = 48;
        double mx = game->hero.x, my = game->hero.y;
        double bx = game->ledges[i].x, by = game->ledges[i].y,
               bw = game->ledges[i].w, bh = game->ledges[i].h;

        if (mx + mw/2 > bx && mx + mw/2 < bx + bw) {
            //are we bumping our head?
            if(my < by + bh && my > by && game->hero.dy < 0) {
                //correct y
                game->hero.y = by + bh;
                my = by + bh;

                //bumped our head, stop any jump velocity
                game->hero.dy = 0;
                game->hero.onBar = 1;
            }
        }
        if (mx + mw > bx && mx < bx + bw) {
            //are we landing on the ledge
            if(my + mh > by && my < by && game->hero.dy > 0) {
                //correct y
                game->hero.y = by - mh;
                my = by - mh;

                //landed on this ledge, stop any jump velocity
                game->hero.dy = 0;
                game->hero.onBar = 1;
            }
        }

        if(my + mh > by && my < by + bh) {
            //rubbing against right edge
            if(mx < bx + bw && mx + mw > bx + bw && game->hero.dx < 0) {
                //correct x
                game->hero.x = bx + bw;
                mx = bx + bw;

                game->hero.dx = 0;
            }
            //rubbing against left edge
            else if(mx + mw > bx && mx < bx && game->hero.dx > 0) {
                //correct x
                game->hero.x = bx - mw;
                mx = bx - mw;

                game->hero.dx = 0;
            }
        }
    }
}

int processEvents(SDL_Window *window, GameState *game) {
    SDL_Event event;
    int done = 0;

    while(SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_WINDOWEVENT_CLOSE:
            if(window) {
                SDL_DestroyWindow(window);
                window = NULL;
                done = 1;
            }
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                done = 1;
            else if (event.key.keysym.sym == SDLK_UP && game->hero.onBar) {
                game->hero.dy = -8;
                game->hero.onBar = 0;
            }
            break;
        case SDL_QUIT:
            //quit out of the game
            done = 1;
            break;
        default:
            break;
        }
    }

    //better jump
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP])
        game->hero.dy -= 0.2f;

    //Walking
    if (state[SDL_SCANCODE_LEFT]) {
        game->hero.dx -= 0.5;
        if(game->hero.dx < -6)
            game->hero.dx = -6; 

        game->hero.facingLeft = 1;
        game->hero.slowingDown = 0;
    }
    else if (state[SDL_SCANCODE_RIGHT]) {
        game->hero.dx += 0.5;
        if(game->hero.dx > 6)
            game->hero.dx = 6;

        game->hero.facingLeft = 0;
        game->hero.slowingDown = 0;
    }
    else {
        game->hero.animFrame = 0;
        game->hero.dx *= 0.8f;
        game->hero.slowingDown = 1;
        if (fabsf((float)(game->hero.dx)) < 0.1f)
            game->hero.dx = 0;
    }

//  if(state[SDL_SCANCODE_UP])
//  {
//    game->hero.y -= 10;
//  }
//  if(state[SDL_SCANCODE_DOWN])
//  {
//    game->hero.y += 10;
//  }

    return done;
}

void render(SDL_Renderer *renderer, GameState *game) {
    //set the drawing color to blue
    SDL_SetRenderDrawColor(renderer, 128, 128, 255, 255);

    //Clear the screen (to blue)
    SDL_RenderClear(renderer);

    //set the drawing color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for(int i = 0; i < 100; i++) {
        SDL_Rect ledgeRect = {
            (int)game->ledges[i].x,
            (int)game->ledges[i].y,
            game->ledges[i].w,
            game->ledges[i].h
        };
        SDL_RenderCopy(renderer, game->brick, NULL, &ledgeRect);
    }

    //draw a rectangle at hero's position
    SDL_Rect rect = {
        (int)game->hero.x,
        (int)game->hero.y,
        48,
        48
    };
    SDL_RenderCopyEx(renderer, game->manFrames[game->hero.animFrame],
                     NULL, &rect, 0, NULL, (game->hero.facingLeft == 0));

    //draw the star image
//  for(int i = 0; i < 100; i++)
//  {
//    SDL_Rect starRect = { game->stars[i].x, game->stars[i].y, 64, 64 };
//    SDL_RenderCopy(renderer, game->star, NULL, &starRect);
//  }


    //We are done drawing, "present" or show to the screen what we've drawn
    SDL_RenderPresent(renderer);
}

int main(void) {
    GameState game;
    SDL_Window *window = NULL;                    // Declare a window
    SDL_Renderer *renderer = NULL;                // Declare a renderer
    int done = 0;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

// srandom((int)time(NULL));

    //Create an application window with the following settings:
    window = SDL_CreateWindow("Game Window",                     // window title
                              SDL_WINDOWPOS_UNDEFINED,           // initial x position
                              SDL_WINDOWPOS_UNDEFINED,           // initial y position
                              CONFIG_WINDOW_WIDTH,                               // width, in pixels
                              CONFIG_WINDOW_HEIGHT,                               // height, in pixels
                              0                                  // flags
                             );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    game.renderer = renderer;

    loadGame(&game);

    // The window is open: enter program loop (see SDL_PollEvent)

    //Event loop
    while(!done) {
        //Check for events
        done = processEvents(window, &game);

        process(&game);
        contactHandle(&game);

        //Render display
        render(renderer, &game);

        //don't burn up the CPU
        //SDL_Delay(10);
    }


    //Shutdown game and unload all memory
    SDL_DestroyTexture(game.star);
    SDL_DestroyTexture(game.manFrames[0]);
    SDL_DestroyTexture(game.manFrames[1]);
    SDL_DestroyTexture(game.brick);

    // Close and destroy the window
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    // Clean up
    SDL_Quit();
    return 0;
}

