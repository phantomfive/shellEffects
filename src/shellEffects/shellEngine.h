#ifndef SHELLENGINE_H
#define SHELLENGINE_H

#include "shellEffectsInternal.h"
#include "shellEffects.h"

/** The internal effect structure used by the engine. It includes fields that
  * don't really need to be revealed to effects. Also, since it is not going to
  * be included by any user programs, we can give it a short type name since it
  * will never get in anyone else's namespace. Note that we start it with a 
  * ShellEffect so that we can cast it to a ShellEffect when we desire. In fact,
  * every ShellEffect should allocate this and typecast it to ShellEffect. 
  * It can be easily allocated by calling allocEffect() */
typedef struct Effect_t {
	ShellEffect base;

	int  timeUntilNextDraw;            //decrements until we draw on zero millis
	EffectCompletef completef;         //so we can call it when completed
	void            *completeContext;  //so we can pass it back to the user
} Effect;

//These are the same as the functions declared in shellEffects.h, those just
//pass through to here. We define those in shellEffects.c to make them easy to
//find, but they just pass through to here.
int engineRunShellEffect(ShellEffect *ef);
int engineRunShellEffectAsync(ShellEffect *ef, EffectCompletef func, void*ctx);
void engineStopShellEffect(ShellEffect *ef);
void engineStopAllShellEffects();


//This is the amount of time the engine will sleep between drawing frames.
//Thus, no effect will be called more frequently than this.
#define FRAME_RESOLUTION 10

#endif
