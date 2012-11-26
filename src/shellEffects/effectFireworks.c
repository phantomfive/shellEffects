#include "shellEffectsInternal.h"

/** Firework effect
 * Two particle engines, one for fireworks that are launching, and one for
 * fireworks that are in the air, already exploding. If you want to understand
 * this, probably easiest to start from the bottom.
 * ~AT~
 */


//-----------------------------------------------------------------------------
// Data structures
//-----------------------------------------------------------------------------

//This is a struct to make it easy to define a spark
typedef struct SparkDefinition_t {
	int appearance;  //set to SPARK_LIST_END for end
	int ticksToChange;  //set to TICKS_RAND for random. A tick is one frame.
} SparkDefinition;

//An exploder holds the definition for a bunch of sparks
typedef struct ExploderDefinition_t {
	const  SparkDefinition *sparkDefList;
	float  speed;  //set to RANDOM_SPEED (which is zero) for random
	int    direction; //in degrees, RANDOM_DIR (which is zero) for random
	struct ExploderDefinition_t *nextExploder; //set to NULL for none
} ExploderDefinition;

//This is a struct for a moving spark
typedef struct Spark_t {
	const SparkDefinition *sparkDef;
	int sparkDefIndex;
	float x,y;
	float xSpeed;
	float ySpeed;
	int ticksToChange;
	ExploderDefinition *nextExploder;
	char inUse;
} Spark;

typedef struct Launcher_t {
	float x, y;
	float xPointsToMove;  //move this many points on each frame
	float yPointsToMove;  //move this many points on each frame. 1 is 1 character
	int explodeLimit;
	int color;
	char inUse;
	int sleepingTime;
} Launcher;

#define LAUNCHER_ARRAY_SIZE 10
#define SPARK_ARRAY_SIZE LAUNCHER_ARRAY_SIZE * 80 
typedef struct State_t {
	Launcher launchers[LAUNCHER_ARRAY_SIZE];
	Spark sparks[SPARK_ARRAY_SIZE];
} State;

//-----------------------------------------------------------------------------
// Firework type definitions
//-----------------------------------------------------------------------------
//Some useful constants 
#define SPARK_DEF_END -1
#define RAND_TICKS    -3
#define NOT_SLEEPING -1
#define DONE_SLEEPING 0
#define EXPLODER_DEF_END NULL
#define RANDOM_SPEED 0
#define RANDOM_DIR   0
#define ASPECT_RATIO 3.0/6.0
#define SPARK_SPEED_REDUCER 20.0  //slow sparks down by this much
#define NEW_LAUNCHER_FREQUENCY 975    //random percentage, 0-1000
#define LAUNCHER_SLEEP_TIME 30     //how long the launcher should sleep
                                   //at the top of its arc before exploding

//static const ExploderDefinition **nullExploder = NULL;
static int calcRandTicksToChange() {
	return 1;
}
static float calcRandSpeed() {
	return ((float)srRand(0,10)); 
}
static int calcRandDir() {
	//random direction in degrees
	return srRand(0, 360);
}

//Definition of the blue firework
static const SparkDefinition pureBlue[] =
{ {'^'|BLUE_BLACK|A_BOLD,2}, {'#'|BLUE_BLACK|A_BOLD,2}, {'#'|BLUE_BLACK|A_BOLD,2}, {'*'|BLUE_BLACK|A_BOLD,2}, {'x'|BLUE_BLACK|A_BOLD,2}, {'*'|BLUE_BLACK|A_BOLD,2}, {'#'|BLUE_BLACK|A_BOLD,1}, 
  {'*'|BLUE_BLACK|A_BOLD,2}, {'<'|BLUE_BLACK,2}, {'*'|BLUE_BLACK,2}, {'>'|BLUE_BLACK, 2}, {'*'|BLUE_BLACK,1}, {'*'|BLUE_BLACK,1}, {'x'|BLUE_BLACK,1}, {'*'|BLUE_BLACK,1}, {'.'|BLUE_BLACK,1}, 
  {'*'|BLUE_BLACK,1}, {'\''|BLUE_BLACK,1}, {SPARK_DEF_END, SPARK_DEF_END}}; 

static ExploderDefinition blueExploder[] = 
{{pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
 {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue}, {pureBlue},
{EXPLODER_DEF_END}};


//definition of the red and white flower firework
static const SparkDefinition redFlowerScatter[] = 
{ {','|RED_BLACK|A_BOLD,25}, {','|RED_BLACK,25}, {'.'|RED_BLACK,25}, {SPARK_DEF_END, SPARK_DEF_END}};
static const SparkDefinition whiteSlow[] = 
{ {'@'|WHITE_BLACK|A_BOLD,10}, {'O'|WHITE_BLACK|A_BOLD,10}, {'o'|WHITE_BLACK,10}, {'.'|WHITE_BLACK|A_BOLD,10}, {'.'|WHITE_BLACK,10}, {SPARK_DEF_END, SPARK_DEF_END}};
static const SparkDefinition yellowFlower[] = 
{ {' '|WHITE_BLACK, 10}, 
{'X'|YELLOW_BLACK,1}, {'X'|WHITE_BLACK,1},{'*'|YELLOW_BLACK,1},{'@'|WHITE_BLACK,1},{'%'|YELLOW_BLACK,1},{'*'|WHITE_BLACK,1},
{'x'|YELLOW_BLACK,1}, {'O'|WHITE_BLACK,1},{'*'|YELLOW_BLACK,1},{'@'|WHITE_BLACK,1},{'%'|YELLOW_BLACK,1},{'*'|WHITE_BLACK,1},
{'X'|YELLOW_BLACK,1}, {'O'|WHITE_BLACK,1},{'*'|YELLOW_BLACK,1},{'@'|WHITE_BLACK,1},{'#'|YELLOW_BLACK,1},{'*'|WHITE_BLACK,1},
{SPARK_DEF_END, SPARK_DEF_END}};
#define YFSPEED 2
static ExploderDefinition yellowFlowerExploder[] =
{{yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED},
{yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED},
{yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED},
{yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED}, {yellowFlower, YFSPEED},
{EXPLODER_DEF_END}};

#define RFSSPEED 3 
static ExploderDefinition redAndWhiteScatter[] = 
{{redFlowerScatter,RFSSPEED, 27,yellowFlowerExploder}, {redFlowerScatter,RFSSPEED,0, yellowFlowerExploder}, {redFlowerScatter,RFSSPEED,171,yellowFlowerExploder}, {redFlowerScatter,RFSSPEED,243,yellowFlowerExploder}, {redFlowerScatter,RFSSPEED,315,yellowFlowerExploder}, {whiteSlow,.5,90}, {EXPLODER_DEF_END}};



/**Decides which exploder gets launched */
static ExploderDefinition *chooseExploderDefinition() {
	int choice = srRand(0,200);
	if(choice>=0   && choice<=140) return blueExploder;
	if(choice>=141 && choice<=200) return redAndWhiteScatter;
	
	//this will crash it
	return blueExploder;//NULL;
	//try not to get to here
}
static void putLauncher(Launcher *l);

//-----------------------------------------------------------------------------
// Exploder/Spark methods
//-----------------------------------------------------------------------------

/**Checks to see if there is an empty spark and returns it*/
static Spark *getEmptySpark(State *state) {
	for(int i=0;i<SPARK_ARRAY_SIZE;i++) {
		if(state->sparks[i].inUse==0) {
			state->sparks[i].inUse = 1;
			return &state->sparks[i];
		}
	}
	return NULL;
}
/**Frees a spark for later use*/
static void putSpark(Spark *s) {
	s->inUse = false;
}
/**Initializes the Spark array*/
static void initializeSparkArray(State *state) {
	for(int i=0;i<SPARK_ARRAY_SIZE;i++)
		state->sparks[i].inUse = 0;
}

/**Takes an exploder definition and turns it into a bunch of sparks */
static void createExploderFromDefinition(ShellEffect *ef, State *state, int startX,
                                         int startY, ExploderDefinition *e) {
	float speed, radiansDir, dir;

	for(int i=0;e[i].sparkDefList!=NULL;i++) {
		Spark *s = getEmptySpark(state);
		if(s==NULL) break;

		//an exploder is just a bunch of sparks
		s->y = startY;
		s->x = startX;
		s->sparkDef = e[i].sparkDefList;
		s->sparkDefIndex = 0;
		speed = (e[i].speed==RANDOM_SPEED)?calcRandSpeed() : e[i].speed;
		dir   = (e[i].direction==RANDOM_DIR)?calcRandDir() : e[i].direction;
		radiansDir = dir * M_PI/180.0;
		s->xSpeed = cos(radiansDir) * speed/SPARK_SPEED_REDUCER;
		s->ySpeed = sin(radiansDir) * speed/SPARK_SPEED_REDUCER * ASPECT_RATIO;
		s->ticksToChange = (s->sparkDef->ticksToChange==-1) ? 
		                   calcRandTicksToChange() : s->sparkDef->ticksToChange;
		s->nextExploder = (e[i].nextExploder!=NULL)?e[i].nextExploder : NULL;
	}
}

/**takes a launcher and chooses an exploder for it*/
static void createRandomExploder(ShellEffect *ef, State *state, Launcher *l) {
	ExploderDefinition *e = chooseExploderDefinition();
	createExploderFromDefinition(ef, state, l->x, l->y, e);

	//free the launcher for future use
	putLauncher(l);
}

static void drawSparkArray(ShellEffect *ef, State *state) {
	
	for(int i=0; i<SPARK_ARRAY_SIZE;i++) {
		Spark *s = &state->sparks[i];
		if(!s->inUse) continue;
		
		//first, draw it where it is
		srSet(ef->screen, s->x, s->y, s->sparkDef[s->sparkDefIndex].appearance);

		//it moves every tick (that is, every frame)
		s->x += s->xSpeed;
		s->y += s->ySpeed;

		//see if it's time for a sparkList change
		s->ticksToChange--;
		if(s->ticksToChange<=0) {
			s->sparkDefIndex++;
			
			//see if we're at the end of this spark list
			if(s->sparkDef[s->sparkDefIndex].appearance == SPARK_DEF_END) {
				if(s->nextExploder!=NULL) {
					createExploderFromDefinition(ef, state, s->x, s->y, s->nextExploder);	
				}
				putSpark(s);
			}

			//otherwise reset the counter
			s->ticksToChange = s->sparkDef[s->sparkDefIndex].ticksToChange;
		}
	}
}

//-----------------------------------------------------------------------------
// Launcher methods
//-----------------------------------------------------------------------------
/** Initializes the launcher array */
static void initializeLauncherArray(State *state) {
	for(int i=0;i<LAUNCHER_ARRAY_SIZE;i++) {
		state->launchers[i].inUse = 0;
		state->launchers[i].sleepingTime = -1;
	}
}

/**Checks to see if there is an empty launcher in the array and returns it*/
static Launcher *getEmptyLauncher(State *state) {
	for(int i=0;i<LAUNCHER_ARRAY_SIZE;i++) {
		if(state->launchers[i].inUse==0) {
			state->launchers[i].inUse = 1;
			return &state->launchers[i];
		}
	}
	return NULL; //we didn't find one
}

/** Frees a launcher, to be used again later */
static void putLauncher(Launcher *l) {
	l->inUse = 0;
	l->sleepingTime = NOT_SLEEPING;
}

/**Starts a new launcher at the bottom of the screen, gives it a
  *velocity, and sticks it in the Launcher array */
static void createNewLauncher(ShellEffect *ef, State *state) {
	Launcher *l = getEmptyLauncher(state);
	if(l==NULL) return;

	//start at a random point at the bottom of the screen
	l->y = (float)ef->y;
	l->x = (float)srRand(20, ef->x-20);

	//move a sort of slow distance in the x direction each frame
	l->xPointsToMove = ((float)(srRand(0,100)-50))/75;
	float absx = (l->xPointsToMove <0)?l->xPointsToMove*-1:l->xPointsToMove;

	//if we're moving really fast in the x direction, we will move less fast
	//in the y direction. So each launcher goes the same speed, but in
	//different directions
	l->yPointsToMove = .75-(absx/4);

	//how far the launcher should travel before turning into an exploder
	l->explodeLimit = srRand(ef->y/8, ef->y/4*3);
	l->color = YELLOW_BLACK;
}


/**Draws all the launchers that are currently in the array*/
static void drawLaunchingArray(ShellEffect *ef, State *state) {
	//
	for(int i=0;i<LAUNCHER_ARRAY_SIZE;i++) {
		Launcher *l = &state->launchers[i];
		if(!l->inUse) continue;

		//launchers sleep for a bit at the top of their arc.
		if(l->sleepingTime>0) {
			l->sleepingTime--;
			continue;
		}
		//once they're done sleeping turn them into an exploder
		else if(l->sleepingTime==DONE_SLEEPING) {
			createRandomExploder(ef, state, l);
			continue;
		}

		//\\-- first increment them to their new position --\//
		l->x -= l->xPointsToMove;
		l->y -= l->yPointsToMove;

		//\\-- The redraw them at their new position --\// 
		//remove them if the are off the screen
		if(l->x<=0 || l->y<=0 || l->x >= ef->x || l->y>=ef->y)
			putLauncher(l);
		//or put them to sleep if they hit their max 
		else if(l->y<l->explodeLimit)
			l->sleepingTime = LAUNCHER_SLEEP_TIME;	
		else
			//otherwise draw them In practice, it might 
			//not have changed but that's ok.
			srSet(ef->screen, (int)l->x, (int)l->y, '.'|l->color);
	}
}

//-----------------------------------------------------------------------------
// Overridden functions and our constructor
//-----------------------------------------------------------------------------
static int drawScreen(ShellEffect *ef) {
	State *state = (State*)ef->data;

	//create a new firework every once in a while
	if(srRand(0,1000)>NEW_LAUNCHER_FREQUENCY) {
		createNewLauncher(ef, state);
	}

	//clear it all
	srClearScreen(ef->screen);

	//draw the sparks and launchers
	drawLaunchingArray(ef, state);
	drawSparkArray(ef, state);
	return 40;
}

static void effectFree(ShellEffect *ef) {
	free(ef->data);
}

ShellEffect *effectFireworks() {
	State *state = (State*)malloc(sizeof(State));
	if(state==NULL) return NULL;
	initializeLauncherArray(state);
	initializeSparkArray(state);
	return allocEffect(drawScreen, NULL, NULL, effectFree, state);
}

