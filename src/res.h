/* res.h: Functions for loading resources from external files.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#ifndef	_res_h_
#define _res_h_

/* Get and set the directory containing all the resource files.
 */
extern char const *getresdir(void);
extern void setresdir(char const *dir);

/* Parse the rc file and initialize the resources that are needed at
 * the start of the program (i.e., the font and color settings).
 * FALSE is returned if the rc file contained errors or if a resource
 * could not be loaded.
 */
extern int initresources(void);

/* Load all resources, using the settings for the given ruleset. FALSE
 * is returned if any critical resources could not be loaded.
 */
extern int loadgameresources(int ruleset);

//DKS - added this to ensure tile images are loaded only once
extern int		images_loaded;

/* Release all memory allocated for the resources.
 */
extern void freeallresources(void);

#endif
