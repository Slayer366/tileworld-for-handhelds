/* res.c: Functions for loading resources from external files.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	"defs.h"
#include	"fileio.h"
#include	"err.h"
#include	"oshw.h"
#include	"unslist.h"
#include	"res.h"

/*
 * The resource ID numbers
 */

#define	RES_IMG_BASE		0
#define	RES_IMG_TILES		(RES_IMG_BASE + 0)
#define	RES_IMG_FONT		(RES_IMG_BASE + 1)
#define	RES_IMG_LAST		RES_IMG_FONT

#define	RES_CLR_BASE		(RES_IMG_LAST + 1)
#define	RES_CLR_BKGND		(RES_CLR_BASE + 0)
#define	RES_CLR_TEXT		(RES_CLR_BASE + 1)
#define	RES_CLR_BOLD		(RES_CLR_BASE + 2)
#define	RES_CLR_DIM		    (RES_CLR_BASE + 3)
#define	RES_CLR_LAST		RES_CLR_DIM

#define	RES_TXT_BASE		(RES_CLR_LAST + 1)
#define	RES_TXT_UNSLIST		(RES_TXT_BASE + 0)
#define	RES_TXT_LAST		RES_TXT_UNSLIST

#define	RES_SND_BASE		    (RES_TXT_LAST + 1)
#define	RES_SND_CHIP_LOSES	    (RES_SND_BASE + SND_CHIP_LOSES)
#define	RES_SND_CHIP_WINS	    (RES_SND_BASE + SND_CHIP_WINS)
#define	RES_SND_TIME_OUT	    (RES_SND_BASE + SND_TIME_OUT)
#define	RES_SND_TIME_LOW	    (RES_SND_BASE + SND_TIME_LOW)
#define	RES_SND_DEREZZ		    (RES_SND_BASE + SND_DEREZZ)
#define	RES_SND_CANT_MOVE	    (RES_SND_BASE + SND_CANT_MOVE)
#define	RES_SND_IC_COLLECTED	(RES_SND_BASE + SND_IC_COLLECTED)
#define	RES_SND_ITEM_COLLECTED	(RES_SND_BASE + SND_ITEM_COLLECTED)
#define	RES_SND_BOOTS_STOLEN	(RES_SND_BASE + SND_BOOTS_STOLEN)
#define	RES_SND_TELEPORTING	    (RES_SND_BASE + SND_TELEPORTING)
#define	RES_SND_DOOR_OPENED	    (RES_SND_BASE + SND_DOOR_OPENED)
#define	RES_SND_SOCKET_OPENED	(RES_SND_BASE + SND_SOCKET_OPENED)
#define	RES_SND_BUTTON_PUSHED	(RES_SND_BASE + SND_BUTTON_PUSHED)
#define	RES_SND_TILE_EMPTIED	(RES_SND_BASE + SND_TILE_EMPTIED)
#define	RES_SND_WALL_CREATED	(RES_SND_BASE + SND_WALL_CREATED)
#define	RES_SND_TRAP_ENTERED	(RES_SND_BASE + SND_TRAP_ENTERED)
#define	RES_SND_BOMB_EXPLODES	(RES_SND_BASE + SND_BOMB_EXPLODES)
#define	RES_SND_WATER_SPLASH	(RES_SND_BASE + SND_WATER_SPLASH)
#define	RES_SND_SKATING_TURN	(RES_SND_BASE + SND_SKATING_TURN)
#define	RES_SND_BLOCK_MOVING	(RES_SND_BASE + SND_BLOCK_MOVING)
#define	RES_SND_SKATING_FORWARD	(RES_SND_BASE + SND_SKATING_FORWARD)
#define	RES_SND_SLIDING		    (RES_SND_BASE + SND_SLIDING)
#define	RES_SND_SLIDEWALKING	(RES_SND_BASE + SND_SLIDEWALKING)
#define	RES_SND_ICEWALKING	    (RES_SND_BASE + SND_ICEWALKING)
#define	RES_SND_WATERWALKING	(RES_SND_BASE + SND_WATERWALKING)
#define	RES_SND_FIREWALKING	    (RES_SND_BASE + SND_FIREWALKING)
#define	RES_SND_LAST    		RES_SND_FIREWALKING

#define	RES_COUNT		        (RES_SND_LAST + 1)

/* Place for storing the resource values.
 */
typedef	char	resourceitem[256];

/* The complete list of resource names.
 */
static char const *rclist[] = {
    "tileimages",
    "font",
    "backgroundcolor",
    "textcolor",
    "boldtextcolor",
    "dimtextcolor",
    "unsolvablelist",
    "chipdeathsound",
    "levelcompletesound",
    "chipdeathbytimesound",
    "ticksound",
    "derezzsound",
    "blockedmovesound",
    "pickupchipsound",
    "pickuptoolsound",
    "thiefsound",
    "teleportsound",
    "opendoorsound",
    "socketsound",
    "switchsound",
    "tileemptiedsound",
    "wallcreatedsound",
    "trapenteredsound",
    "bombsound",
    "splashsound",
    "blockmovingsound",
    "skatingforwardsound",
    "skatingturnsound",
    "slidingsound",
    "slidewalkingsound",
    "icewalkingsound",
    "waterwalkingsound",
    "firewalkingsound",
};

/* The complete collection of resource values.
 */
static resourceitem	allresources[Ruleset_Count][RES_COUNT];

/* The resource values for the current ruleset.
 */
static resourceitem    *resources = NULL;

/* The ruleset-independent resource values.
 */
static resourceitem    *globalresources = allresources[Ruleset_None];

/* The active ruleset.
 */
static int		currentruleset = Ruleset_None;

/* The directory containing all the resource files.
 */
static char const      *resdir = NULL;

/* Getting and setting the resource directory.
 */
char const *getresdir(void)		{ return resdir; }
void setresdir(char const *dir)		{ resdir = dir; }

//DKS - added this to ensure tile images are loaded only once
int		images_loaded = 0;

//DKS modified to only use atiles.png
/* A few resources have non-empty default values.
 */
static void initresourcedefaults(void)
{
    strcpy(allresources[Ruleset_None][RES_IMG_TILES], "atiles.png");
    strcpy(allresources[Ruleset_None][RES_IMG_FONT], "font.bmp");
    strcpy(allresources[Ruleset_None][RES_CLR_BKGND], "000000");
    strcpy(allresources[Ruleset_None][RES_CLR_TEXT], "FFFFFF");
    strcpy(allresources[Ruleset_None][RES_CLR_BOLD], "FFFF00");
    strcpy(allresources[Ruleset_None][RES_CLR_DIM], "C0C0C0");
    memcpy(&allresources[Ruleset_MS], globalresources,
				sizeof allresources[Ruleset_MS]);
    memcpy(&allresources[Ruleset_Lynx], globalresources,
				sizeof allresources[Ruleset_Lynx]);
}

/* Iterate through the lines of the rc file, storing the values in the
 * allresources array. Lines consisting only of whitespace, or with an
 * octothorpe as the first non-whitespace character, are skipped over.
 * Lines containing a ruleset in brackets introduce ruleset-specific
 * resource values. Ruleset-independent values are copied into each of
 * the ruleset-specific entries. FALSE is returned if the rc file
 * could not be opened.
 */
static int readrcfile(void)
{
    resourceitem	item;
    fileinfo		file;
    char		buf[256];
    char		name[256];
    char	       *p;
    int			ruleset;
    int			lineno, i, j;

    memset(&file, 0, sizeof file);
	if (!openfileindir(&file, resdir, "rc", "r", "can't open"))
		return FALSE;

    ruleset = Ruleset_None;
	for (lineno = 1 ; ; ++lineno) {
		i = sizeof buf - 1;
		if (!filegetline(&file, buf, &i, NULL))
			break;
		for (p = buf ; isspace(*p) ; ++p) ;
		if (!*p || *p == '#')
			continue;
		if (sscanf(buf, "[%[^]]]", name) == 1) {
			for (p = name ; (*p = tolower(*p)) != '\0' ; ++p) ;
			if (!strcmp(name, "ms"))
				ruleset = Ruleset_MS;
			else if (!strcmp(name, "lynx"))
				ruleset = Ruleset_Lynx;
			else if (!strcmp(name, "all"))
				ruleset = Ruleset_None;
			else
				warn("rc:%d: syntax error", lineno);
			continue;
		}
		if (sscanf(buf, "%[^=]=%s", name, item) != 2) {
			warn("rc:%d: syntax error", lineno);
			continue;
		}
		for (p = name ; (*p = tolower(*p)) != '\0' ; ++p) ;
		for (i = sizeof rclist / sizeof *rclist - 1 ; i >= 0 ; --i)
			if (!strcmp(name, rclist[i]))
				break;
		if (i < 0) {
			warn("rc:%d: illegal resource name \"%s\"", lineno, name);
			continue;
		}
		strcpy(allresources[ruleset][i], item);
		if (ruleset == Ruleset_None)
		    for (j = Ruleset_None ; j < Ruleset_Count ; ++j)
			strcpy(allresources[j][i], item);
	}

    fileclose(&file, NULL);
    return TRUE;
}

/*
 * Resource-loading functions
 */

/* Parse the color-definition resource values.
 */
//DKS - don't need this anymore..
//static int loadcolors(void)
//{
//    long	bkgnd, text, bold, dim;
//    char       *end;
//
//    bkgnd = strtol(resources[RES_CLR_BKGND].str, &end, 16);
//    if (*end || bkgnd < 0 || bkgnd > 0xFFFFFF) {
//	warn("rc: invalid color ID for background");
//	bkgnd = -1;
//    }
//    text = strtol(resources[RES_CLR_TEXT].str, &end, 16);
//    if (*end || text < 0 || text > 0xFFFFFF) {
//	warn("rc: invalid color ID for text");
//	text = -1;
//    }
//    bold = strtol(resources[RES_CLR_BOLD].str, &end, 16);
//    if (*end || bold < 0 || bold > 0xFFFFFF) {
//	warn("rc: invalid color ID for bold text");
//	bold = -1;
//    }
//    dim = strtol(resources[RES_CLR_DIM].str, &end, 16);
//    if (*end || dim < 0 || dim > 0xFFFFFF) {
//	warn("rc: invalid color ID for dim text");
//	dim = -1;
//    }
//
//    setcolors(bkgnd, text, bold, dim);
//    return TRUE;
//}

/* Attempt to load the tile images.
 */
static int loadimages(void)
{
    char       *path;
    int		f;

    f = FALSE;
    path = getpathbuffer();
	if (*resources[RES_IMG_TILES]) {
		combinepath(path, resdir, resources[RES_IMG_TILES]);
		printf("loadimages path: %s\n", path);
		f = loadtileset(path, TRUE);
	}
	if (!f && resources != globalresources
			&& *globalresources[RES_IMG_TILES]) {
		combinepath(path, resdir, globalresources[RES_IMG_TILES]);
		f = loadtileset(path, TRUE);
	}
	free(path);

	if (!f)
		errmsg(resdir, "no valid tilesets found");
	return f;
}

//DKS - won't need this anymore
/* Load the font resource.
 */
//static int loadfont(void)
//{
//    char       *path;
//    int		f;
//
//    f = FALSE;
//    path = getpathbuffer();
//    if (*resources[RES_IMG_FONT].str) {
//	combinepath(path, resdir, resources[RES_IMG_FONT].str);
//	f = loadfontfromfile(path, TRUE);
//    }
//    if (!f && resources != globalresources
//	   && *globalresources[RES_IMG_FONT].str) {
//	combinepath(path, resdir, globalresources[RES_IMG_FONT].str);
//	f = loadfontfromfile(path, TRUE);
//    }
//    free(path);
//
//    if (!f)
//	errmsg(resdir, "no valid font found");
//    return f;
//}

/* Load the list of unsolvable levels.
 */
static int loadunslist(void)
{
    char const *filename;

	if (*resources[RES_TXT_UNSLIST])
		filename = resources[RES_TXT_UNSLIST];
	else if (resources != globalresources
			&& *globalresources[RES_TXT_UNSLIST])
		filename = globalresources[RES_TXT_UNSLIST];
	else
		return FALSE;

    return loadunslistfromfile(filename);
}

/* Load all of the sound resources.
 */
static int loadsounds(void)
{
    char       *path;
    int		count;
    int		n, f;

    path = getpathbuffer();
    count = 0;
	for (n = 0 ; n < SND_COUNT ; ++n) {
		f = FALSE;
		if (*resources[RES_SND_BASE + n]) {
			combinepath(path, resdir, resources[RES_SND_BASE + n]);
			f = loadsfxfromfile(n, path);
		}
		if (!f && resources != globalresources
				&& *globalresources[RES_SND_BASE + n]) {
			combinepath(path, resdir, globalresources[RES_SND_BASE + n]);
			f = loadsfxfromfile(n, path);
		}
		if (f)
			++count;
	}
    free(path);
    return count;
}

/* Load all resources that are available. FALSE is returned if the
 * tile images could not be loaded. (Sounds are not required in order
 * to run, and by this point we should already have a valid font and
 * color scheme set.)
 */
int loadgameresources(int ruleset)
{
    currentruleset = ruleset;
    resources = allresources[ruleset];

	//DKS - fonts and colors are handled differently now
	//loadcolors();
	//loadfont();

	//DKS - having horrible segfaults only on GP2X when tiles are freed,
	//I am putting a new static variable in to ensure loadimages is loaded
	//once and only once for entire program.
	//Both rulesets will use atiles.png.
	if (images_loaded == 0) {
		if (!loadimages())
			return FALSE;
		images_loaded = 1;
	}

	if (loadsounds() == 0)
		setaudiosystem(FALSE);
    return TRUE;
}

/* Parse the rc file and load the font and color scheme. FALSE is returned
 * if an error occurs.
 */
//DKS - modified
int initresources(void)
{
    initresourcedefaults();
    resources = allresources[Ruleset_None];

	//DKS - fonts and colors are handled differently now
	//    if (!readrcfile() || !loadcolors() || !loadfont())
	if (!readrcfile() )
		return FALSE;
    loadunslist();
    return TRUE;
}

//DKS - modified
/* Free all resources.
 */
void freeallresources(void)
{
    //int	n;

    freefont();

	if (images_loaded == 1) {
		freetileset();
	}

    clearunslist();

	//DKS - this is now done explicitly in setaudiosystem
	//    for (n = 0 ; n < SND_COUNT ; ++n)
	//		 freesfx(n);
}
