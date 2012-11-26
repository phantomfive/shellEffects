#ifndef SHELLEFFECTS_H
#define SHELLEFFECTS_H
/** shellEffects.h - the public API *\
  * A library for coordinating graphics in a shell. This file is the defining
  * API document.
  *
  * To use: allocate a ShellEffect using one of the constructors, and call
  * either runShellEffect() or runShellEffectAsync(). That's really all you
  * have to do. If you add more than one effect, they will be drawn back to
  * front, with the ones you add first being occluded, or covered, by the ones
  * you add later. Each function has an explanation of its usage, so be sure
  * to read that.    -AT                                                      */

#if __cplusplus
extern "C" {  //this is lame
#endif

typedef struct ShellEffect_t ShellEffect;


/** Runs a single shell effect. If NULL is passed in, nothing will happen.
  * returns 0 on error, or true on success. ef will be freed by the library. */
int runShellEffect(ShellEffect *ef);

/** Called in response to a completion of asyncronous effect, as explained in
  * the comments to runShellEffectAsync(). Note: this function is called from
  * a separate thread, so make sure you don't do anything more complicated
  * than setting a variable or something: treat it like an interrupt handler
  * and you will avoid subtle synchronization issues */
typedef void (*EffectCompletef)(ShellEffect *ef, void *context);

/** Also runs a single shell effect. If ef is NULL, nothing will happen.
  * When the effect is finished running (note: not all effects finish, as
  * explained in the comments to the individual effects), func will be called,
  * with ef and context as parameters. The effect will be be freed by the
  * library after func() returns. If func is NULL, of course it will not be 
  * called. */
int runShellEffectAsync(ShellEffect *ef, EffectCompletef func, void *context);

/** If you've held on to the pointer to a screen effect, you can call this to
  * stop the effect. If it was an asynchronous effect, then the 
  * effectCompletef() will be called, if it is a synchronous effect, then
  * runShellEffect() will return. If NULL is passed in, nothing will happen. */
void stopShellEffect(ShellEffect *ef);

/** Stops all effects as though calling stopShellEffect() for each one, and
  * also frees any resources allocated for running the library. */
void stopAllShellEffects();


//----------------------------------------------------------------------------
// Available Effects
//----------------------------------------------------------------------------

ShellEffect *effectLeftRight(int delay);
ShellEffect *effectUpDown(int delay);

//--
/** Flashes fireworks on the screen at random intervals until stopped */
ShellEffect *effectFireworks();

//--
/** Runs a command and sends the output to the screen */
ShellEffect *effectRunCommand(const char *command, char *const args[]);

//--
/** This structure is used to describe the sprite and how it will move. You can
  * You will pass in an array of these, with the end of the array being denoted by
  *  end set to non-zero. */
typedef struct EffectTravelingSpriteDefStruct{
   //sprite speed
	int speed;               //on a scale of 0-1000, 0 being no motion
	int accelleration;       //on a scale of 0-1000, 0 being no acceleration
	
	//sprite appearance
	int spriteWidth;         //indicates the width of the sprite array
	int spriteHeight;        //indicates the height of the sprite array
	const char *spriteArray; //a width*height array that represents the sprite.
	                         //This is an ascii-art array.

   //sprite path.
	//The sprite path is defined by a quadratic bezier curve. The point meanings
	//are explained briefly below. The sprite will be drawn from tthe upper left 
	//corner of the sprite, (0,0) represents the top left corner of the screen, 
	//and (1,1) represents the bottom right corner of the screen. 
	float x1,y1;        //The starting point for the sprite.
	float x2,y2;        //The control point. The sprite will move close to here.
	float x3,y3;        //The ending point for the sprite. If 'end' is set to 0,
	                    //then this will be ignored, and the starting point for 
							  //the next element in the array will be used.
	
	//Set this to non-zero to denote the end of the array. Can use SPRITE_END
	char  end; 
} EffectTravelingSpriteDef;
#define SPRITE_END 1

/** Moves a sprite around on the screen. See the comments for the previous
  * struct for instructions on how to use. */
ShellEffect *effectTravelingSprite(const EffectTravelingSpriteDef list[]);

#ifdef __cplusplus
}
#endif
#endif

