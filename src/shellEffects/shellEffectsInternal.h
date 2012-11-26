#ifndef SHELLEFFECTSINTERNAL_H
#define SHELLEFFECTSINTERNAL_H
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ncurses.h>
#include "shellEffects.h"
#include "shellRenderer.h"

/**The idea is you can have various effects, and enable them or disable them.
 * Each effect is basically a series of function pointers and some data. Each
 * effect is kind of a mini-class.
 * Also, check out shellRenderer.h for drawing functions.
 * The function definitions follow.
 */

#undef SUCCESS
#define SUCCESS 1

//These comments are a contract. If the code deviates from the comments, the
//code is wrong.


//The first section of this header is callbacks that the effects have to write, 
//and the engine will call. Then comes the struct, which will be the method of
//communication between the engine and the effect. Finally, there is the
//allocator, which should get called in the constructor of the effect.

//In all these functions ef is a struct ScreenEffect_t that is returned
//from the constructor. The constructor is custom for each effect.

/** Gets called every few milliseconds to draw the screen. Return a number
  * in milliseconds that will determine approximately how long before this
  * function is called again. Return 0 or less to indicate that this effect
  * is finished and should be removed. */
typedef int  (*EffectDrawScreen)(ShellEffect *ef);

/** This will get called when the screen size changes. newX and newY are the
  * new width and height of the screen, respectively. This function may be
  * set to NULL in the struct if you aren't interested in the screen change.
  * The x and y values in the struct will no be updated until after this
  * function is called. */
typedef void (*EffectScreenSizeChange)(ShellEffect *ef, int newX, int newY);

/** If not NULL, will get called before the effect is paused. If it is NULL, the
  * effect will be paused because DrawScreen() will not be called, but this is
  * a chance to do some cleanup if necessary. */
typedef void (*EffectStop)(ShellEffect *ef);

/** If not NULL, will get called before DrawScreen() is called the first time,
  * and before resuming calls to DrawScreen() if EffectsStop() is called. */
typedef void (*EffectStart)(ShellEffect *ef);

/** Should free any resources held by the effect. Stop need not be called
  * before this is called. The effect itself will be freed by the engine */
typedef void (*EffectFree)(ShellEffect *ef);

/** This is a single effect. Fill in the functions to get them called at
 *  appropriate timings.
 */
struct ShellEffect_t {
	//These will be filled in by the engine. The effect should not change them.
	int x,y;               //The width and height of the screen
	ShellScreen *screen;   //The window to draw on, using ssSet() and ssGet()

	//These following should be filled in by the effect, by calling allocEffect()
	EffectDrawScreen drawScreen;
	EffectScreenSizeChange screenSizeChange;
	EffectStart start;
	EffectStop stop;
	EffectFree free;
	void *data;       //the effect can set this to anything it desires
};


/** Should be called by to allocate a shell effect. Other methods of
  * allocating may go awry. This will fill in the ShellEffect_t struct. You
  * don't need to worry about freeing any memory allocated by calling this
  * function, the engine will do it */
ShellEffect *allocEffect(EffectDrawScreen drawScreen,
                         EffectScreenSizeChange screenSizeChange,
							    EffectStart start, EffectFree free, void *data);

/** Frees an effect. Of course, data will have to be freed separately. *ef
  * will be set to NULL upon returning. NULL may be passed in. The engine
  * will call this, effects should not unless there is an error in the
  * constructor for the effect. It will free everything allocated by
  * allocEffect() */
void freeEffect(ShellEffect **ef);

/** For functions you can call to draw, check out shellRenderer.h */

#endif

