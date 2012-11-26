#include "shellEffectsInternal.h"




//This one only has a few differences from the TravelingSpriteDefStruct.
typedef struct TravelingSpriteStruct {
	float speed;
	float accelleration;
	float location;   //between 0-1, according to the definition of bezier curves
	
	int width;
	int height;
	const char *spriteArray;
	
	float x1,y1;
	float x2,y2;
	float x3,y3;

	int oldx, oldy;
	
	const EffectTravelingSpriteDef *nextSprite;
} TravelingSprite;

typedef struct StateStruct {
	EffectTravelingSpriteDef *spriteDefinition;
	TravelingSprite currentSprite;
} State;
//---------------------------------------------------------------------------------------
// Copying, data around. And freeing it
//---------------------------------------------------------------------------------------
static char copySpriteDef(const EffectTravelingSpriteDef *original,
                                EffectTravelingSpriteDef *copy) {
	memcpy(copy, original, sizeof(EffectTravelingSpriteDef));
	copy->spriteArray = strdup(original->spriteArray);
	if(copy->spriteArray==NULL) return 0;
	else                        return 1;
}
static EffectTravelingSpriteDef *copySpriteDefList(const EffectTravelingSpriteDef *list) {
	int len=0;
	EffectTravelingSpriteDef *rv;
	while(!list[len].end)
		len++;
	
	//create an array the size of the sprite def, then copy it over one at a time
	rv = malloc(sizeof(EffectTravelingSpriteDef)*len);
	for(int i=0;i<len; i++) {
		if(!copySpriteDef(&list[i], &rv[i])){ 
			free(rv);  
			return NULL;
		}
	}
	return rv;	
}
static void freeSpriteDefList(EffectTravelingSpriteDef *list) {
	do{
		free((char*)list->spriteArray);
	}while(!list->end);
	free(list);
}

//This is a little non-generic, but this method expects 'spriteDef' to be a part of
//an array. If it happens to be a random spriteDefinition from nowhere, this won't work.
static void setCurrentSprite(ShellEffect *ef, const EffectTravelingSpriteDef *spriteDef) {
	State *state = (State*)ef->data;
	TravelingSprite *currentSprite = &state->currentSprite;

	currentSprite->speed         = spriteDef->speed/20000.0;
	currentSprite->accelleration = spriteDef->accelleration/20000.0;
	currentSprite->location      = 0;
	currentSprite->width         = spriteDef->spriteWidth;
	currentSprite->height        = spriteDef->spriteHeight;
	currentSprite->spriteArray   = spriteDef->spriteArray;
	currentSprite->x1            = spriteDef->x1;
	currentSprite->y1            = spriteDef->y1;
	currentSprite->x2            = spriteDef->x2;
	currentSprite->y2            = spriteDef->y2;
	currentSprite->x3            = spriteDef->x3;
	currentSprite->y3            = spriteDef->y3;
	currentSprite->nextSprite    = NULL;


	//if we're not at the end, we need to set nextSprite, and a different x3 and y3
	if(!spriteDef->end) {
		const EffectTravelingSpriteDef *nextSprite = spriteDef + 1;
		currentSprite->nextSprite = nextSprite;
		currentSprite->x3 = nextSprite->x1;
		currentSprite->y3 = nextSprite->y1;
	}
}

//----------------------------------------------------------------------------------------
// Miscellaneous drawing functions
//----------------------------------------------------------------------------------------
void drawSpriteAtPoint(const char *spriteArray, int width, int height, int x, 
                              int y, ShellScreen *screen) {
	int i,j;
	for(j=0;j<height;j++) {
		for(i=0;i<width;i++) {
			srSet(screen, i+x, j+y, spriteArray[i+j*width]);
		}
	}
}
static void clearSpriteAtPoint(int width, int height, int x, int y, ShellScreen *screen) {
	int i, j;

	for(j=0;j<height; j++) {
		for(i=0;i<width; i++) {
			srSet(screen, i+x, j+y, -1);
		}
	}
}

static int drawScreen(ShellEffect *ef) {
	float x,y;
	State *state = (State*)ef->data;
	TravelingSprite *sprite = &state->currentSprite;
	
	//Calculate x and y
	float t = sprite->location;
	x = ((1-t)*(1-t))*sprite->x1 + 2*t*(1-t)*sprite->x2 + t*t*sprite->x3;
	y = ((1-t)*(1-t))*sprite->y1 + 2*t*(1-t)*sprite->y2 + t*t*sprite->y3;
	x *= ef->x;
	y *= ef->y;

	//draw it
	clearSpriteAtPoint(sprite->width, sprite->height, sprite->oldx, sprite->oldy, ef->screen);
	drawSpriteAtPoint(sprite->spriteArray,sprite->width, sprite->height, x, y, ef->screen);
	sprite->oldx = x; sprite->oldy = y;

	//update the location and speed
	sprite->location += sprite->speed;
	sprite->speed    += sprite->accelleration;

	if(sprite->location > 1) {
		if(sprite->nextSprite==NULL)
			return -1; //we are done

		//Otherwise, if there is another sprite, lets move on to that one
		setCurrentSprite(ef, sprite->nextSprite);
		srClearScreen(ef->screen);
	}
	
	return 100;
}

//----------------------------------------------------------------------------------------
// Constructor/Destructor
//----------------------------------------------------------------------------------------

static void effectFree(ShellEffect *ef) {
	State *state = (State*)ef->data;
	freeSpriteDefList(state->spriteDefinition);
	free(state);
}

ShellEffect *effectTravelingSprite(const EffectTravelingSpriteDef list[]) {
	State *state = (State*)malloc(sizeof(State));
	if(state==NULL) return NULL;
	state->spriteDefinition = copySpriteDefList(list);

	ShellEffect *rv = allocEffect(drawScreen, NULL, effectFree, NULL, state);
	if(rv==NULL) return rv;

	setCurrentSprite(rv, list);
	return rv;
}
