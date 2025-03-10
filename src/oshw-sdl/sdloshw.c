/* sdloshw.c: Top-level SDL management functions.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<SDL.h>
#include	"sdlgen.h"
#include	"../err.h"
#include 	"port_cfg.h"

/* Values global to this library.
 */
oshwglobals	sdlg;

/* This is an automatically-generated file, which contains a
 * representation of the program's icon.
 */
#include	"ccicon.c"

//DKS - modified
/* Dispatch all events sitting in the SDL event queue.
 */
static void _eventupdate(int wait)
{
    static int	mouselastx = -1, mouselasty = -1;
    SDL_Event	event;

	if (wait) {
		SDL_WaitEvent(NULL);
		int eventoccured = 0;
		while (!eventoccured) {
			eventoccured = SDL_PollEvent(NULL);
			if (!eventoccured) {
				SDL_Delay(10);
			}
		}
	}

	SDL_PumpEvents();
	while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				if (windowmappos(mouselastx, mouselasty) < 0)


					SDL_ShowCursor(SDL_DISABLE);
				keyeventcallback(event.key.keysym.sym, TRUE);
				if (event.key.keysym.unicode
						&& event.key.keysym.unicode != event.key.keysym.sym) {
					keyeventcallback(event.key.keysym.unicode, TRUE);
					keyeventcallback(event.key.keysym.unicode, FALSE);
				}
				break;
			case SDL_KEYUP:
				if (windowmappos(mouselastx, mouselasty) < 0)


					SDL_ShowCursor(SDL_DISABLE);

				keyeventcallback(event.key.keysym.sym, FALSE);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				SDL_ShowCursor(SDL_ENABLE);
				break;
			case SDL_MOUSEMOTION:
				SDL_ShowCursor(SDL_ENABLE);
				break;
			case SDL_QUIT:
				exit(EXIT_SUCCESS);
		}
	}
}

/* Alter the window decoration.
 */
void setsubtitle(char const *subtitle)
{
    char	buf[270];

	if (subtitle && *subtitle) {
		sprintf(buf, "Tile World - %.255s", subtitle);
		SDL_WM_SetCaption(buf, "Tile World");

	} else {
		SDL_WM_SetCaption("Tile World", "Tile World");
	}
}

//DKS - modified, we're going to explicity shutdown SDL, not with atexit()
/* Shut down SDL.
 */
/*
static void shutdown(void)
{
    SDL_Quit();
}
*/

//DKS - modified
/* Initialize SDL, create the program's icon, and then initialize
 * the other modules of the library.
 */
int oshwinitialize(int silence, int showhistogram, int fullscreen)
{
//	SDL_Surface	       *icon;

	sdlg.eventupdatefunc = _eventupdate;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		errmsg(NULL, "Cannot initialize SDL system: %s\n", SDL_GetError());
		return FALSE;
	}
	//atexit(shutdown);



	return _sdltimerinitialize(showhistogram)
		&& _sdltextinitialize()
		&& _sdltileinitialize()
		&& _sdlinputinitialize()
		&& _sdloutputinitialize(fullscreen)
		&& _sdlsfxinitialize(silence);
}

/* The real main().
 */
int main(int argc, char *argv[])
{
    return tworld(argc, argv);
}
