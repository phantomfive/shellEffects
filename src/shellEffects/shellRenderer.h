#ifndef SHELL_RENDERER_H
#define SHELL_RENDERER_H

/** Written to handle all drawing to the screen in a terminal.
  * Handles drawing from several different screens.
  * Author: Andrew
  */

typedef struct shellScreen_t ShellScreen;

//---------------------------------------------------------------------
// These methods are used by the individual effects
//---------------------------------------------------------------------

/**
  * Sets the character at x,y on the screen to value c. To
  * represent a transparent character, use -1.
  */
void srSet(ShellScreen *screen, int x, int y, int c);

/** returns the character at x,y.  -2 will be returned on error.  */
int srGet(ShellScreen *screen, int x, int y);

/** clears the screen */
void srClearScreen(ShellScreen *screen);

/** Returns a random number between start and end, inclusive */
int srRand(int startInclusive, int endInclusive);

//--------------------------------------------------------------------
// Colors!
//--------------------------------------------------------------------
/** Here colors will be listed in alphabetical order. Eventually we
 *  may have all color pairs added, but I am too lazy to add them
 *  all at once, so if you need one that is not on the list, add it
 *  here and initialize it in shellRenderer.c::initColors()
 * 
 *  Convention is FOREGROUND_BACKGROUND
 *
 *  To use a color, when you call srSet() you can | the color with
 *  the character you desire to use, like this:
 *
 *  srSet(scr, x,y, 'c' | RED_BLUE );
 *
 *  To make it bold, you can | it with A_BOLD.
 *  To make it dim,  you can | it with A_DIM.
 *  To underline,    you can | it with A_UNDERLINE.
 *  To super-bold,   you can | it with A_STANDOUT.
 *  To reverse,      you can | it with A_REVERSE.
 *  To blink,        you can | it with A_BLINK.
 *
 *  Not all of those are guaranteed to work on all systems.
 */

#define BLUE_BLACK   COLOR_PAIR(3)
#define RED_BLUE     COLOR_PAIR(1)
#define RED_BLACK    COLOR_PAIR(4)
#define WHITE_BLACK  COLOR_PAIR(5)
#define YELLOW_BLACK COLOR_PAIR(2)

//--------------------------------------------------------------------
// These methods will be called by the ShellEffects engine
//--------------------------------------------------------------------

/**Gets called when the screen size changes */
typedef void(*srScreenSizeChanged)(int oldX, int oldY, int newX, int newY);

/** Returns SUCCESS on success, or another number on error. Set the callback
  * if you want notification of screen size changes. */
int srInit(srScreenSizeChanged callback);
void srShutdown();

/** Allocates a screen, or frees it. Returns NULL on error */
ShellScreen *srAllocScreen();
void srFreeScreen(ShellScreen **screen);

/** Sets x and y to the screen size. Don't pass NULL */
void srGetScreenSize(ShellScreen *screen, int *x, int *y);

/** Draws a screen. Will replace anything that is currently on the screen,
  * unless those squares are transparent (-1). In other words, the screen
  * you want on top should be drawn last */
void srDrawScreen(ShellScreen *screen);

/**This one actually causes all the previous draws to be pushed to the shell*/
void srFlush();



#endif
