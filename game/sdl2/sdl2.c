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
static int event_handler(const SDL_Event *const ev)
{
	if (ev->type == SDL_QUIT) {
		return (-1);
	}

	switch (ev->type) {
	case SDL_KEYDOWN:
		/* ev is a union, member SDL_KeyboardEvent key is only activated
		 * if some keyboard event did happen.
		 * key has member keysym.scancode which describes the physical
		 * key that was pressed
		 */
		SDL_Log("SDL_KeyboardEvent:KEYDOWN: %s\n",
				SDL_GetScancodeName(ev->key.keysym.scancode));
		break;
	default:
		break;
	}

	return (0);
}

int main(void)
{
	int exit_code = 0;

	/* log pid */
	pid_t pid = getpid();
	fprintf(stderr, PROGNAME": pid = %"PRIdMAX"\n", (intmax_t) pid);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit_code = 1;
		goto exit_main;
	}

	SDL_Window *window;
	SDL_Renderer *renderer;
	if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindowAndRenderer: %s",
				SDL_GetError());
		exit_code = 1;
		goto quit_sdl;
	}

	while (true) {
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);

		SDL_Event ev;
		SDL_WaitEventTimeout(&ev, POLL_RATE_MS);
		if (event_handler(&ev) == -1) {
			SDL_Log(PROGNAME": Ending program");
			goto destroy_window_renderer;
		}
	}

destroy_window_renderer:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
quit_sdl:
	SDL_Quit();
exit_main:
	return (exit_code);
}
