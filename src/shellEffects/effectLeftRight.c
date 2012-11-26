#include "shellEffectsInternal.h"
#define HEIGHT 2

//You might say we store the class data here. Store any local variables you need in a struct
//that is allocated dynamically. That way you can have more than one instance of our effect
//running at a time.
typedef struct StateStruct {
	int delay;
	int pos;
} State;

/** This method is where we do the drawing. It gets called every few milliseconds.
  * Here we draw a T moving across the screen */
static int drawScreen(ShellEffect *ef) {
	//Get the state for this effect
	State *state = (State*)ef->data;

	//Erase the character from where it was before
	srSet(ef->screen, state->pos-1, HEIGHT, -1);

	//if we are done, return -1 to stop
	if(state->pos > ef->x)
		return -1;

	//Draw the character at the new location`
	srSet(ef->screen, state->pos, HEIGHT, 'T');
	state->pos++;

	//return the delay in milliseconds before this method should be called again
	return state->delay;
}

/** Cleanup all our state when the effect is finished */
static void effectFree(ShellEffect *ef) {
	State *state = ef->data;
	free(state);
}

/** This is the constructor. This is the only method that should NOT be static (unless, of course,
  * you wish to expose other methods as a public API */
ShellEffect *effectLeftRight(int delay)
{
	//allocate our internal state
	State *state = malloc(sizeof(State));
	if(state==NULL) return NULL;
	state->pos = 0;
	state->delay = delay;

	//once everything is initialized, we can finish setting things up
	ShellEffect *rv = allocEffect(drawScreen, NULL, effectFree, NULL, state);
	return rv;
}

