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
#define CONFIG_HERO_HEIGHT 50
#define CONFIG_HERO_WIDTH  24
#define CONFIG_HERO_STEP   10
#define CONFIG_BARS_AMOUNT 100
#define CONFIG_BAR_WIDTH   200
#define CONFIG_BAR_HEIGHT  30
#define CONFIG_STARS_AMOUT 100
#define CONFIG_STAR_WIDTH   52
#define CONFIG_STAR_HEIGHT  52
#define CONFIG_GRAVITY 0.5

/* ----------------- */
typedef struct vector {
    double x;
    double y;
} vector_t;

typedef struct {
    vector_t pos;
    int height, width;
    double dx, dy;
    short lives;
    char *name;
    int onBar, isDead;

    int animFrame, facingLeft, slowingDown;
} Hero;

typedef struct {
    int x, y, baseX, baseY, mode;
    float phase;
} Star;

typedef struct {
    int x, y, w, h;
} Bar;

typedef struct {

    float scrollX;

    //Player
    Hero hero;

    //Stars
    Star stars[CONFIG_STARS_AMOUT];

    //Bars
    Bar bars[CONFIG_BARS_AMOUNT];

    //Images
    SDL_Texture *background;
    SDL_Texture *star;
    SDL_Texture *manFrames[2];
    SDL_Texture *fire;
    SDL_Texture *brick;
    SDL_Texture *label;

    //time
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

    surface = IMG_Load("background.png");
    if(!surface){
        printf("Cannot find background.png %d\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }
    game->background = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("obstacle.png");
    if (!surface) {
        printf("Cannot find obstacle.png %s\n", IMG_GetError());
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

    surface = IMG_Load("fire.jpg");
    if (!surface) {
        printf("Cannot find fire.jpg: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }
    game->fire = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("brick.png");
    if (!surface) {
        printf("Cannot find brick.png: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }
    game->brick = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    game->hero.pos.x = 320-40;
    game->hero.pos.y = 240-40;
    game->hero.dx = 0;
    game->hero.dy = 0;
    game->hero.height = CONFIG_HERO_HEIGHT;
    game->hero.width = CONFIG_HERO_WIDTH;
    game->hero.onBar = 0;
    game->hero.animFrame = 0;
    game->hero.facingLeft = 1;
    game->hero.slowingDown = 0;
    game->hero.lives = 3;
    game->hero.isDead = 0;

    game->time = 0;
    game->scrollX = 0;

    //init stars
     for(int i = 0; i < CONFIG_STARS_AMOUT; i++)
     {
        game->stars[i].baseX = 320+random()%38400;
        game->stars[i].baseY = random()%480;
        game->stars[i].mode = random()%2;
        game->stars[i].phase = 2.0*3.14*(random()%360)/360.0f;
     }

    //init bars
    for (int i = 0; i < CONFIG_BARS_AMOUNT; i++) {
        game->bars[i].w = CONFIG_BAR_WIDTH;
        game->bars[i].h = CONFIG_BAR_HEIGHT;
        game->bars[i].x = i*256;
        game->bars[i].y = 400;
    }
    game->bars[99].x = 350;
    game->bars[99].y = 200;

    game->bars[98].x = 350;
    game->bars[98].y = 350;
}


void process(GameState *game) {
    //add time
    game->time++;
    if(!game->hero.isDead){
        //hero movement
        Hero *hero = &game->hero;
        hero->pos.x += hero->dx;
        hero->pos.y += hero->dy;
        if (hero->onBar && !hero->slowingDown && (hero->dx > 0 || hero->dx < 0)) {
            if(game->time % 8 == 0) {
                if(hero->animFrame == 0)
                    hero->animFrame = 1;
                else
                    hero->animFrame = 0;
            }
        }
        hero->dy += CONFIG_GRAVITY;
        //stars movement
        for(int i = 0; i < CONFIG_STARS_AMOUT; i++){
            game->stars[i].x = game->stars[i].baseX;
            game->stars[i].y = game->stars[i].baseY;
            if(game->stars[i].mode == 0)
            {
              game->stars[i].x = game->stars[i].baseX+sinf(game->stars[i].phase+game->time*0.06f)*75;
            }
            else
            {
              game->stars[i].y = game->stars[i].baseY+cosf(game->stars[i].phase+game->time*0.06f)*75;
            }
        }

        //game scrolling
        game->scrollX = -game->hero.pos.x + CONFIG_WINDOW_WIDTH/2;
        if(game->scrollX > 0)
            game->scrollX = 0;
    }
}


//check if 2 figures colliding, for contactHandle
int collide2d(double x1, double y1, double x2, double y2,
              double wt1, double ht1, double wt2, double ht2) {
    return (!((x1 > (x2+wt2)) || (x2 > (x1+wt1)) ||
              (y1 > (y2+ht2)) || (y2 > (y1+ht1))));
}

void contactHandle(GameState *game) {

    //check fell out
    if(game->hero.pos.y > CONFIG_WINDOW_HEIGHT){
        //dead
        game->hero.isDead = 1;
    }

    //check for collision with stars = if dead
    for (int i = 0; i < CONFIG_STARS_AMOUT; i++){
        if(collide2d(game->hero.pos.x, game->hero.pos.y,
                     game->stars[i].x, game->stars[i].y,
                     CONFIG_HERO_WIDTH, CONFIG_HERO_HEIGHT,
                     CONFIG_STAR_WIDTH - 10, CONFIG_STAR_WIDTH - 10)){          //-10 not to die that easy
            //dead
            game->hero.isDead = 1;
        }
    }

    //hero for hero, b for bar
    double hw = game->hero.width, hh = game->hero.height;
    //Check for collision with any bars (brick blocks)
    for (int i = 0; i < CONFIG_BARS_AMOUNT; i++) {
        vector_t *hpos = &game->hero.pos;
        double bx = game->bars[i].x, by = game->bars[i].y,
               bw = game->bars[i].w, bh = game->bars[i].h;
        if (hpos->x + hw/2 > bx && hpos->x + hw/2 < bx + bw) {
            //collision with bottom
            if(hpos->y < by + bh && hpos->y > by && game->hero.dy < 0) {
                //set y
                hpos->y = by + bh;

                //collision with bottom of the bar. stop y velocity
                game->hero.dy = 0;
            }
        }
        if (hpos->x + hw > bx && hpos->x < bx + bw) {
            //collision with the top of the bar
            if(hpos->y + hh > by && hpos->y < by && game->hero.dy > 0) {
                //set y
                hpos->y = by - hh;

                //landed on bar, stop y velocity
                game->hero.dy = 0;
                game->hero.onBar = 1;
            }
        }

        if(hpos->y + hh > by && hpos->y < by + bh) {
            //collision with the right edge
            if(hpos->x < bx + bw && hpos->x + hw > bx + bw && game->hero.dx < 0) {
                //set x
                hpos->x = bx + bw;

                game->hero.dx = 0;
            }
            //collision with the left edge
            else if(hpos->x + hw > bx && hpos->x < bx && game->hero.dx > 0) {
                //set x
                hpos->x = bx - hw;
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

    return done;
}

void render(SDL_Renderer *renderer, GameState *game) {
    //set the drawing color to blue
    SDL_SetRenderDrawColor(renderer, 154, 217, 234, 255);
    //Clear the screen (to blue)
    SDL_RenderClear(renderer);
    //setting background
    SDL_Rect backRect = {
        0, 0, CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT
    };
    SDL_RenderCopy(renderer, game->background, NULL, &backRect);

    //drawing bars
    for(int i = 0; i < CONFIG_BARS_AMOUNT; i++) {
        SDL_Rect barRect = {
            (int)game->scrollX + (int)game->bars[i].x,
            (int)game->bars[i].y,
            game->bars[i].w,
            game->bars[i].h
        };
        SDL_RenderCopy(renderer, game->brick, NULL, &barRect);
    }

    //draw a rectangle at hero's position
    SDL_Rect rect = {
        (int)game->scrollX + (int)game->hero.pos.x,
        (int)game->hero.pos.y,
        game->hero.width,
        game->hero.height
    };
    SDL_RenderCopyEx(renderer, game->manFrames[game->hero.animFrame],
                     NULL, &rect, 0, NULL, (game->hero.facingLeft == 0));
    if(game->hero.isDead){
        SDL_Rect rect = {
            (int)game->scrollX + (int)game->hero.pos.x,
            (int)game->hero.pos.y + CONFIG_HERO_HEIGHT/2,
            game->hero.width,
            game->hero.height/2
        };
        SDL_RenderCopyEx(renderer, game->fire,
                         NULL, &rect, 0, NULL, 0);
    }

    //draw the star image
    for(int i = 0; i < CONFIG_STARS_AMOUT; i++)
    {
        SDL_Rect starRect = {
              (int)game->scrollX + game->stars[i].x,
              (int)game->stars[i].y,
                CONFIG_STAR_WIDTH,
                CONFIG_STAR_HEIGHT
    };
    SDL_RenderCopy(renderer, game->star, NULL, &starRect);
    }

    //done drawing, "present" or show to the screen what we've drawn
    SDL_RenderPresent(renderer);
    if(game->hero.isDead){
        SDL_Delay(1500);
        SDL_Quit();
        exit(-1);
    }
}

int main(void) {
    GameState game;
    SDL_Window *window = NULL;                    // Declare a window
    SDL_Renderer *renderer = NULL;                // Declare a renderer
    int done = 0;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2 Video

    srandom((int)time(NULL));

    //Create an application window with the following settings:
    window = SDL_CreateWindow("Game Window",                        // window title
                              SDL_WINDOWPOS_UNDEFINED,              // initial x position
                              SDL_WINDOWPOS_UNDEFINED,              // initial y position
                              CONFIG_WINDOW_WIDTH,                  // width, in pixels
                              CONFIG_WINDOW_HEIGHT,                 // height, in pixels
                              0                                     // flags
                             );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    game.renderer = renderer;

    loadGame(&game);

    // The window is opened: enter program loop (see SDL_PollEvent)

    //Event loop
    while(!done) {
        //Check for events
        done = processEvents(window, &game);
        process(&game);
        contactHandle(&game);
        //Render display
        render(renderer, &game);
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


