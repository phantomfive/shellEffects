#include <sys/select.h>
#include <pthread.h>
#include "shellEngine.h"
#include "shellEffectsInternal.h"

typedef struct List_t List;
#include "shellCrazyList.h"

//put all local variables here so it will be easy 
//to make this more than a singleton if we later desire.
typedef struct EngineState_t {
	char initialized;
	List effectList;  //all our effects are placed here
	int  shouldStop;  //set to 1 to stop the main loop
	ShellEffect *newEffect;
	ShellEffect *effectToRemove;
	volatile int shouldRemoveAllEffects;
	pthread_mutex_t lock; //locks out user threads
} EngineState;
static EngineState state_ = {0};
static EngineState *state = &state_; //because I like using -> more than .


//a mostly portable way of sleeping
static void portableSleep(int milliseconds){
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = milliseconds * 100;
	select(0, NULL, NULL, NULL, &tv);
}


//-----------------------------------------------------------------
// These functions belong to the mainLoop thread
//-----------------------------------------------------------------

//in order to avoid locking, when the user threads want to add an effect,
//they set the newEffect variable. Then when the main loop gets around to it,
//it removes it from the variable and sets the variable to NULL. If another
//thread tries to add a new effect before it is NULL, then that thread will
//block.
static void checkForNewEffects() {
	if(state->newEffect != NULL) {
		pushBack(&state->effectList, state->newEffect);
		state->newEffect = NULL;
	}
}

static void removeOneEffect(ShellEffect *ef) {
	Effect *f = (Effect*)ef;

	//notify the user
	if(f->completef!=NULL)
		f->completef(ef, f->completeContext);

	srFreeScreen(&ef->screen);
	removeElement(&state->effectList, ef);
	if(f->base.free!=NULL)
		f->base.free(ef);
	freeEffect(&ef);
}

//similar thing with the old effects. If state->shouldRemoveAllEffects
//or state->effectToRemove is NULL, then we will remove it. Then set it
//to 0 (or NULL) to let the waiting thread know the deed is done.
static void removeOldEffects() {
	
	//remove all if we should
	if(state->shouldRemoveAllEffects) {
		ShellEffect *temp;

		while((temp=popBack(&state->effectList)) != NULL)
		{
			removeOneEffect(temp);
		}
		state->shouldRemoveAllEffects = 0;
	}
	
	//remove a single one if we should
	else if(state->effectToRemove != NULL){
		removeOneEffect(state->effectToRemove);
		state->effectToRemove = NULL;
	}

	//now remove the ones that are finished
	listForeach(effect, state->effectList){
		if(effect->timeUntilNextDraw<=0)
		{
			removeInForeach(effect, state->effectList);
			removeOneEffect((ShellEffect*)effect);
		}
	}
}

//this should be called every millisecond
static void drawOneLoop() {
	//draw them
	listForeach(effect, state->effectList) {
		effect->timeUntilNextDraw-=FRAME_RESOLUTION;
		if(effect->timeUntilNextDraw<=0) {
			effect->timeUntilNextDraw = effect->base.drawScreen(&effect->base);
		}
		srDrawScreen(effect->base.screen);
	}
	srFlush();
}

//This is the main work loop thread
static void *mainLoop(void *v) {
	while(!state->shouldStop) {
		checkForNewEffects();
		removeOldEffects();
		drawOneLoop();
		portableSleep(FRAME_RESOLUTION);
	}
	state->shouldStop = 0;
	return NULL;
}

//------------------------------------------------------------------
// These functions are to be accessed by user threads
//------------------------------------------------------------------
//returns true on success, 0 on failure
static int initialize() {
	if(state->initialized) return 1;

	pthread_mutex_init(&state->lock, NULL);

	listInit(&state->effectList);
	state->shouldStop  = 0;
	state->effectToRemove = NULL;
	state->shouldRemoveAllEffects = 0;
	state->initialized = 1;

	srInit(NULL);

	pthread_t thread;
	pthread_create(&thread, NULL,mainLoop, NULL);

	return 1;
}

//right now this only works if called after all the effects have been stopped
static void shutdown() {
	if(!state->initialized) return;

	state->shouldStop = 1;
	while(state->shouldStop)
		;
	
	srShutdown();
	pthread_mutex_destroy(&state->lock);

	state->initialized = 0;
}

//----------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------
//This one isn't public, but is closely related to engineRunShellEffect()
//so I'll put it here
struct Waiting{
	pthread_cond_t cond;
	char done;
	pthread_mutex_t lock;
};

//gets called by engineRunShellEffect()
static void shortEffectCompletef(ShellEffect *ef, void *context) {
	struct Waiting *w = (struct Waiting*)context;
	w->done = 1;
	pthread_mutex_lock(&w->lock);
	pthread_cond_signal(&w->cond);
	pthread_mutex_unlock(&w->lock);
}
int engineRunShellEffect(ShellEffect *ef) {
	if(ef==NULL) return SUCCESS;
	int rv;
	struct Waiting w;
	w.done = 0;
	pthread_cond_init(&w.cond, NULL);
	pthread_mutex_init(&w.lock, NULL);
	pthread_mutex_lock(&w.lock);

	//call the async function, and as the EffectCompletef callback, use
	//a short function that just wakes our thread up
	if((rv=engineRunShellEffectAsync(ef, shortEffectCompletef, &w))!=SUCCESS)
		return rv;

	while(!w.done)
		pthread_cond_wait(&w.cond, &w.lock);
	pthread_mutex_unlock(&w.lock);

	pthread_mutex_destroy(&w.lock);
	pthread_cond_destroy(&w.cond);
	return SUCCESS;
}

int engineRunShellEffectAsync(ShellEffect *ef, EffectCompletef func, void*ctx){
	if(ef==NULL) return SUCCESS;
	
	pthread_mutex_lock(&state->lock);

	initialize();

	Effect *eff = (Effect*)ef;
	while(state->newEffect != NULL) //wait until main thread picks it up
		;
	eff->completef = func;
	eff->timeUntilNextDraw = 1;
	eff->completeContext = ctx;
	eff->base.screen = srAllocScreen();
	srGetScreenSize(eff->base.screen, &eff->base.x, &eff->base.y);
	state->newEffect = ef;

	pthread_mutex_unlock(&state->lock);
	return SUCCESS;
}
void engineStopShellEffect(ShellEffect *ef) {
	if(ef==NULL) return;
	pthread_mutex_lock(&state->lock);

	state->effectToRemove = ef;
	while(state->effectToRemove!=NULL)
		;

	pthread_mutex_unlock(&state->lock);
}

void engineStopAllShellEffects() {
		pthread_mutex_lock(&state->lock);
		
		state->shouldRemoveAllEffects = 1;
		while(state->shouldRemoveAllEffects)
			;
		
		shutdown();
		pthread_mutex_unlock(&state->lock);
}

