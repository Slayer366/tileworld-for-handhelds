/* sdltext.c: Font-rendering functions for SDL.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<SDL.h>
#include	"sdlgen.h"
#include	"../err.h"

//DKS - modified
/* Given a text and a maximum horizontal space to occupy, return the
 * amount of vertical space needed to render the entire text with
 * word-wrapping.
 */
 //DKS: addded "font" parameter and spacing param
 // spacing is number of pixels to space lines by
int measuremltext(char const *text, int len, int maxwidth,
                    SFont_Font *font, int spacing)
{
    int	brk, w, h, n;

    //DKS
    char tmpstr[2];

    if (len < 0)
	len = strlen((char const*)text);
    h = 0;
    brk = 0;
    for (n = 0, w = 0 ; n < len ; ++n) {
    tmpstr[0] = text[n];
    tmpstr[1] = '\0';
    w += SFont_TextWidth(font, tmpstr);

	if (isspace(text[n])) {
	    brk = w;
	} else if (w > maxwidth) {
	    h += SFont_TextHeight(font) + spacing;
	    if (brk) {
		w -= brk;
		brk = 0;
	    } else {
        tmpstr[0] = text[n];
        tmpstr[1] = '\0';
        w = SFont_TextWidth(font, tmpstr);
		brk = 0;
	    }
	}
    }
    if (w)
    h += SFont_TextHeight(font) + spacing;
    return h;
}

/*
 * The main font-rendering functions.
 */

//DKS - modified
/* Draw a single line of text to the screen at the position given by
 * rect. The bitflags in the final argument control the placement of
 * text within rect and what colors to use.
 */
void drawtext(SDL_Surface *sur, SDL_Rect *rect, char const *text,
		     int len, int flags, SFont_Font *font, int spacing)
{
    int		l, r;
    int		n, w;

    //DKS
    char str_to_display[500] = "";

    if (*text == '\0') {
        return;
    }

    if (len < 0)
	len = text ? strlen((char const*)text) : 0;

    //DKS
    if (len > 499) {
        errmsg(NULL, "string too large passed to drawtext: sdltext.c");
        return;
    } else if (spacing < 0) {
        errmsg(NULL, "cannot pass negative line spacings to drawtext: sdltext.c");
        return;
    }

    w = 0;

    w = SFont_TextWidth(font, text);

    if (flags & PT_CALCSIZE) {
    rect->h = SFont_TextHeight(font + spacing);
	rect->w = w;
	return;
    }

    strcpy(str_to_display, text);

    if (w >= rect->w) {
        //text is too wide, limit it to fit
        for (n = len; ((n >= 0) && (w > rect->w)); --n) {
            str_to_display[n] = '\0';
            w = SFont_TextWidth(font, str_to_display);
        }
        l = r = 0;
    } else if (flags & PT_RIGHT) {
	l = rect->w - w;
	r = 0;
    } else if (flags & PT_CENTER) {
	l = (rect->w - w) / 2;
	r = (rect->w - w) - l;
    } else {
	l = 0;
	r = rect->w - w;
    }

    SFont_Write(sur, font, rect->x + l, rect->y, str_to_display);

    if (flags & PT_UPDATERECT) {
    rect->y += (SFont_TextHeight(font) + spacing);
    rect->h -= (SFont_TextHeight(font) + spacing);
    }
}


//DKS - modified
/* Draw one or more lines of text to the screen at the position given by
 * rect. The text is broken up on whitespace whenever possible.
 */
void drawmultilinetext(SDL_Surface *sur, SDL_Rect *rect, char const *text,
			      int len, int flags, SFont_Font *font, int spacing)
{
    if (spacing < 0)
        spacing = 0;

    SDL_Rect	area;
    int		index, brkw, brkn;
    int		w, n;

    if (flags & PT_CALCSIZE) {
	rect->h = measuremltext(text, len, rect->w, font, spacing);
	return;
    }

    if (len < 0)
	len = strlen((char const*)text);

    area = *rect;
    brkw = brkn = 0;
    index = 0;

    //DKS new
    char tmpstr[2];

    for (n = 0, w = 0 ; n < len ; ++n) {
    tmpstr[0] = text[n];
    tmpstr[1] = '\0';
    w += SFont_TextWidth(font, tmpstr);
	if (isspace(text[n])) {
	    brkn = n;
	    brkw = w;
	} else if (w > rect->w) {
	    if (brkw) {
        drawtext(sur, &area, text + index, brkn - index,
				 flags | PT_UPDATERECT, font, spacing);
		index = brkn + 1;
		w -= brkw;
	    } else {
		drawtext(sur, &area, text + index, n - index,
				 flags | PT_UPDATERECT, font, spacing);

		index = n;
		tmpstr[0] = text[n];
		tmpstr[1] = '\0';
		w = SFont_TextWidth(font, tmpstr);
	    }
	    brkw = 0;
	}
    }
    if (w)
	drawtext(sur, &area, text + index, len - index,
            flags | PT_UPDATERECT, font, spacing);

    if (flags & PT_UPDATERECT) {
	*rect = area;
    } else {
//DKS - suspect code, disabling
//	while (area.h)
//	    drawtext(&area, NULL, 0, PT_UPDATERECT);
//	    drawtext(sur, &area, NULL, 0, PT_UPDATERECT, font, spacing);
    }
}

/*
 * The exported functions.
 */

//DKS - modified
/* Lay out the columns of the given table so that the entire table
 * fits within area (horizontally; no attempt is made to make it fit
 * vertically). Return an array of rectangles, one per column. This
 * function is essentially the same algorithm used within printtable()
 * in tworld.c
 */
static SDL_Rect *_measuretable(SDL_Rect const *area, tablespec const *table)
{
	//DKS - we don't use this at all, dunno why I modified it early on but should work

    SDL_Rect		       *colsizes;
    unsigned char const	       *p;
    int				sep, mlindex, mlwidth, diff;
    int				i, j, n, i0, c, w, x;

    if (!(colsizes = malloc(table->cols * sizeof *colsizes)))
	memerrexit();
    for (i = 0 ; i < table->cols ; ++i) {
	colsizes[i].x = 0;
	colsizes[i].y = area->y;
	colsizes[i].w = 0;
	colsizes[i].h = area->h;
    }

    mlindex = -1;
    mlwidth = 0;
    n = 0;
    for (j = 0 ; j < table->rows ; ++j) {
	for (i = 0 ; i < table->cols ; ++n) {
	    c = table->items[n][0] - '0';
	    if (c == 1) {
		w = 0;
		p = (unsigned char const*)table->items[n];
		for (p += 2 ; *p ; ++p)
		    w += SFont_TextHeight(sdlg.font_small);
		if (table->items[n][1] == '!') {
		    if (w > mlwidth || mlindex != i)
			mlwidth = w;
		    mlindex = i;
		} else {
		    if (w > colsizes[i].w)
			colsizes[i].w = w;
		}
	    }
	    i += c;
	}
    }

    sep = SFont_TextHeight(sdlg.font_small) * table->sep;
    w = -sep;
    for (i = 0 ; i < table->cols ; ++i)
	w += colsizes[i].w + sep;
    diff = area->w - w;
    if (diff < 0 && table->collapse >= 0) {
	w = -diff;
	if (colsizes[table->collapse].w < w)
	    w = colsizes[table->collapse].w - SFont_TextHeight(sdlg.font_small);
	colsizes[table->collapse].w -= w;
	diff += w;
    }

    if (diff > 0) {
	n = 0;
	for (j = 0 ; j < table->rows && diff > 0 ; ++j) {
	    for (i = 0 ; i < table->cols ; ++n) {
		c = table->items[n][0] - '0';
		if (c > 1 && table->items[n][1] != '!') {
		    w = sep;
		    p = (unsigned char const*)table->items[n];
		    for (p += 2 ; *p ; ++p)
			w += SFont_TextHeight(sdlg.font_small);
		    for (i0 = i ; i0 < i + c ; ++i0)
			w -= colsizes[i0].w + sep;
		    if (w > 0) {
			if (table->collapse >= i && table->collapse < i + c)
			    i0 = table->collapse;
			else if (mlindex >= i && mlindex < i + c)
			    i0 = mlindex;
			else
			    i0 = i + c - 1;
			if (w > diff)
			    w = diff;
			colsizes[i0].w += w;
			diff -= w;
			if (diff == 0)
			    break;
		    }
		}
		i += c;
	    }
	}
    }
    if (diff > 0 && mlindex >= 0 && colsizes[mlindex].w < mlwidth) {
	mlwidth -= colsizes[mlindex].w;
	w = mlwidth < diff ? mlwidth : diff;
	colsizes[mlindex].w += w;
	diff -= w;
    }

    x = 0;
    for (i = 0 ; i < table->cols && x < area->w ; ++i) {
	colsizes[i].x = area->x + x;
	x += colsizes[i].w + sep;
	if (x >= area->w)
	    colsizes[i].w = area->x + area->w - colsizes[i].x;
    }
    for ( ; i < table->cols ; ++i) {
	colsizes[i].x = area->x + area->w;
	colsizes[i].w = 0;
    }

    return colsizes;
}


/* Free the resources associated with a font.
 */
//DKS - modified
void freefont(void)
{
    if (sdlg.font_tiny->Surface) {
        SFont_FreeFont(sdlg.font_tiny);
    }
    if (sdlg.font_small->Surface) {
        SFont_FreeFont(sdlg.font_small);
    }
    if (sdlg.font_hint->Surface) {
        SFont_FreeFont(sdlg.font_hint);
    }
    if (sdlg.font_big->Surface) {
        SFont_FreeFont(sdlg.font_big);
    }
    if (sdlg.font_led_big->Surface) {
        SFont_FreeFont(sdlg.font_led_big);
    }
}

//DKS - modified
/* Initialize the module.
 */
int _sdltextinitialize(void)
{
    sdlg.measuretablefunc = _measuretable;
    return TRUE;
}
