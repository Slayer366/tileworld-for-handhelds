/* sdlsfx.c: Creating the program's sound effects.
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
#include	"../state.h"

#include 	<sys/ioctl.h>
#include	<sys/soundcard.h>
#include	<unistd.h>
#include	<fcntl.h>

//DKS - new conversion of sound to SDL_mixer
#include    "SDL_mixer.h"

//DKS
/* Some generic default settings for the audio output.
 */
#define DEFAULT_SND_FMT		AUDIO_S16SYS
#define DEFAULT_SND_FREQ	22050
#define	DEFAULT_SND_CHAN	2
#define DEFAULT_SND_CHUNKS	2048

// This is TRUE if music is enabled or FALSE if disabled by user
static int music_enabled = TRUE;
//DKS New - what song in the songs[] array are we playing?
static int gamemusicplaying = 0;
static int menumusicplaying = 0;

//DKS -  for playing music:
static Mix_Music *current_music = NULL;

//DKS - modified
/* The data needed for each sound effect's wave.
 */
typedef	struct sfxinfo {
	Mix_Chunk   *wave;   /* the actual wave data */
    Uint32      len;     /* size of the wave data */
    int         pos;     /* how much has been played already */
	int	        playing; /* is the wave currently playing? */
	int	        channel; /* channel this sound is playing on	*/
} sfxinfo;

/* The data needed to talk to the sound output device.
 */
//static SDL_AudioSpec	spec;

/* All of the sound effects.
 */
static sfxinfo		sounds[SND_COUNT];

/* TRUE if sound-playing has been enabled.
 */
static int		enabled = FALSE;

/* TRUE if the program is currently talking to a sound device.
 */
static int		hasaudio = FALSE;

/* The volume level.
 */
//static int	volume = SDL_MIX_MAXVOLUME;
static int		volume = 100;		//overall volume 0 - 100
static int 		musicvolume = 55;	//volume of music 0 - 128

//DKS - modified and had to add declaration here:
static void shutdownsound(void);

/*
 * The exported functions.
 */

//DKS - modified version for SDL_mixer use:
/* Activate or deactivate the sound system. When activating for the
 * first time, the connection to the sound device is established. When
 * deactivating, the connection is closed.
 */
int setaudiosystem(int active)
{
	if (!enabled)
		return !active;

	if (!active) {
		if (hasaudio) {
	    hasaudio = FALSE;
			shutdownsound();
		}
		return TRUE;
	}

	if (!SDL_WasInit(SDL_INIT_AUDIO)) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
			 warn("Cannot initialize audio output: %s", SDL_GetError());
			 return FALSE;
		}
	}

	if (hasaudio)
		return TRUE;

	if (Mix_OpenAudio(DEFAULT_SND_FREQ, DEFAULT_SND_FMT, DEFAULT_SND_CHAN, DEFAULT_SND_CHUNKS) < 0) {
		warn("Mix_OpenAudio: %s\n", Mix_GetError());
		music_enabled = 0;
		return FALSE;
    }

	// load support for the OGG music
	int flags = MIX_INIT_OGG;
	int initted = Mix_Init(flags);
	if ((initted&flags) != flags) {
		 warn("Mix_Init: Failed to init ogg music support.\n");
		 warn("Mix_Init: %s\n", Mix_GetError());
		 music_enabled = 0;
	}

    hasaudio = TRUE;
    return TRUE;
}


//DKS - SDL_mixer version:
/* Load a single wave file into memory. The wave data is converted to
 * the format expected by the sound device.
 */
int loadsfxfromfile(int index, char const *filename)
{
	if (!filename) {
		freesfx(index);
		return TRUE;
	}

	if (!enabled)
		return FALSE;
    if (!hasaudio && !setaudiosystem(TRUE))
			 return FALSE;

	sounds[index].wave = Mix_LoadWAV(filename);
	sounds[index].playing = FALSE;
	if(!sounds[index].wave) {
		 warn("Mix_LoadWAV: %s\n", Mix_GetError());
		 return FALSE;
	}
	return TRUE;
}

//DKS
void playgamesongs(void) {
	if (!hasaudio || !music_enabled || gamemusicplaying) {
		return;
	}
	gamemusicplaying = 1;
	menumusicplaying = 0;
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	if (current_music) {
		Mix_FreeMusic(current_music);
		current_music = NULL;
	}
	if (music_enabled) {
		current_music = Mix_LoadMUS("music/music.ogg");
		Mix_VolumeMusic(musicvolume);
		if (current_music) {
			// play music forever if it loaded successfully
			if( Mix_PlayMusic(current_music, -1) == -1) {
				 printf("Error: Mix_PlayMusic: %s\n", Mix_GetError());
			}
		} else {
			printf("Error: Mix_LoadMUS(\"music/music.ogg\"): %s\n", Mix_GetError());
			music_enabled = 0;
		}
	}
}

//DKS new function gets called in main menu
void playmenusong(void) {
	if (!hasaudio || !music_enabled ) {
		return;
	}
	gamemusicplaying=0;
	menumusicplaying=1;
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	if (current_music) {
		Mix_FreeMusic(current_music);
		current_music = NULL;
	}
	if (music_enabled) {
		current_music = Mix_LoadMUS("music/menumusic.ogg");
		Mix_VolumeMusic(musicvolume);
		if (current_music) {
			// play music forever if it loaded successfully
			if( Mix_PlayMusic(current_music, -1) == -1) {
				 printf("Error: Mix_PlayMusic: %s\n", Mix_GetError());
			}
		} else {
			printf("Error: Mix_LoadMUS(\"music/menumusic.ogg\"): %s\n", Mix_GetError());
			music_enabled = 0;
		}
	}
}


//DKS new function gets called if user disabled music on main menu
void disablemusic(void) {
	if (!hasaudio || !music_enabled) {
		return;
	}
	music_enabled = 0;
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	if (current_music) {
		Mix_FreeMusic(current_music);
		current_music = NULL;
	}
}

//DKS new function gets called if user disabled music on main menu
void enablemusic(void) {
	if (!hasaudio || music_enabled) {
		return;
	}
	music_enabled = 1;
	playmenusong();
}

//DKS new function to tell if music is enabled or not (used in main menu)
int ismusicenabled(void) {
	return (music_enabled);
}

//DKS - modified version for use with SDL_mixer
/* Select the sounds effects to be played. sfx is a bitmask of sound
 * effect indexes. Any continuous sounds that are not included in sfx
 * are stopped. One-shot sounds that are included in sfx are
 * restarted.
 */
void playsoundeffects(unsigned long sfx)
{
	unsigned long	flag;
	int			i;

	if (!hasaudio || !volume)
		return;

	for (i = 0, flag = 1 ; i < SND_COUNT ; ++i, flag <<= 1) {
		// first, check if the channel has finished playing this sound and mark it as no longer playing if so
		if (sounds[i].playing) {
			if (sounds[i].channel >= 0) {
				 if (!Mix_Playing(sounds[i].channel)) {
					sounds[i].channel = -1;
					sounds[i].playing = 0;
					Mix_HaltChannel(sounds[i].channel);
				}
			} else {
				// somehow, a sound's channel got marked -1 but still marked as playing (shouldn't happen)
				sounds[i].playing = 0;
				Mix_HaltChannel(sounds[i].channel);
			}
		}
		if (sfx & flag) {
			if (i < SND_ONESHOT_COUNT) {
				if (sounds[i].playing && sounds[i].channel != -1)
					Mix_HaltChannel(sounds[i].channel);
				sounds[i].playing = 1;
				sounds[i].channel = Mix_PlayChannel(-1, sounds[i].wave, 0);
				if (sounds[i].channel < 0) {
					printf("Mix_PlayChannel: %s\n",Mix_GetError());
					sounds[i].playing = 0;
				}
			} else {
				// repeating sound, only start playing if it's not already
				if (!sounds[i].playing) {
					sounds[i].channel = Mix_PlayChannel(-1, sounds[i].wave, -1);
					sounds[i].playing = 1;
				}
			}
		} else {
			sounds[i].playing = FALSE;
			if (i >= SND_ONESHOT_COUNT && sounds[i].channel > -1) {
				Mix_HaltChannel(sounds[i].channel);
				sounds[i].channel = -1;
			}
		}
	}
}

//DKS modified for SDL_mixer:
/* If action is negative, stop playing all sounds immediately.
 * Otherwise, just temporarily pause or unpause sound-playing.
 */
void setsoundeffects(int action)
{
	int	i;

	if (!hasaudio || !volume)
		return;

	if (action < 0) {
		// halt all sound playback
		for (i = 0 ; i < SND_COUNT ; ++i) {
			sounds[i].playing = FALSE;
			Mix_HaltChannel(-1);
		}
	} else {
		if (action > 0) {
			// unpause sound playback on all channels
			Mix_Resume(-1);
			if (music_enabled)
				Mix_ResumeMusic();
		} else if (action == 0) {
			// pause sound playback on all channels
			Mix_Pause(-1);
			if (music_enabled && Mix_PlayingMusic())
				Mix_PauseMusic();
		}
	}
}

//DKS - added for SDL_mixer:
void freemusic(void)
{
	if (!hasaudio) return;
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	if (current_music) {
		Mix_FreeMusic(current_music);
		current_music = NULL;
	}
	music_enabled = 0;
}

//DKS - modified for SDL_mixer: NOTE: -1 for index frees all sounds
/* Release all memory for the given sound effect.
 */
void freesfx(int index)
{
	if (!hasaudio) return;
	Mix_HaltChannel(-1); // make sure all sounds are halted before freeing any chunks
	if (index < 0) {
		int i;
		for (i = 0; i < SND_COUNT ; ++i) {
			if (sounds[i].wave)
				Mix_FreeChunk(sounds[i].wave);
			sounds[i].wave = NULL;
			sounds[i].playing = FALSE;
			sounds[i].channel = -1;
		}
	} else {
		if (sounds[index].wave) {
			Mix_FreeChunk(sounds[index].wave);
			sounds[index].wave = NULL;
			sounds[index].playing = FALSE;
			sounds[index].channel = -1;
		}
	}
}

//DKS modified
/* Set the current volume level to v. If display is true, the
 * new volume level is displayed to the user.
 */
int setvolume(int v, int display)
{
    char	buf[16];

    if (!hasaudio)
	return FALSE;
    if (v < 0)
	v = 0;
    else if (v > 10)
	v = 10;
    if (!volume && v)
//	displaysoundeffects(0, FALSE);
    volume = (SDL_MIX_MAXVOLUME * v + 9) / 10;
    if (display) {
	sprintf(buf, "Volume: %d", v);
	setdisplaymsg(buf, 1000, 1000);
    }
    return TRUE;
}

//DKS modified
/* Change the current volume level by delta. If display is true, the
 * new volume level is displayed to the user.
 */
int changevolume(int delta, int display)
{
    return setvolume(((10 * volume) / SDL_MIX_MAXVOLUME) + delta, display);
}

//DKS - modified for SDL_mixer and music:
/* Shut down the sound system.
 */
static void shutdownsound(void)
{
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	freesfx(-1);
	freemusic();
	Mix_CloseAudio();
	Mix_Quit();

	if (SDL_WasInit(SDL_INIT_AUDIO))
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	hasaudio = FALSE;
	music_enabled = 0;
}

//DKS - modified.  We're now explicity shutting down SDL systems ourselves, not
// with atexit()
/* Initialize the module. If silence is TRUE, then the program will
 * leave sound output disabled.
 */
int _sdlsfxinitialize(int silence)
{
//    atexit(shutdown);
    enabled = !silence;
//    initonomatopoeia();
    if (enabled)
	setaudiosystem(TRUE);
    return TRUE;
}
