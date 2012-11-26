#include <stdlib.h>

#include "shellEffectsInternal.h"
#include "shellEngine.h"

//--------------------------------------------------------------------------
// Functions defined in shellEffectsInternal.h
//--------------------------------------------------------------------------
ShellEffect *allocEffect(EffectDrawScreen drawScreen,
                         EffectScreenSizeChange screenSizeChange,
								 EffectStart start, EffectFree free, void *data) {
	ShellEffect *rv;
	rv = (ShellEffect *)malloc(sizeof(struct Effect_t));
	if(rv != NULL) {
		rv->drawScreen = drawScreen;
		rv->screenSizeChange = screenSizeChange;
		rv->start = start;
		rv->free  = free;
		rv->data  = data;
	}

	return rv;
}

void freeEffect(ShellEffect **ef) {
	if(ef!=NULL && *ef!=NULL) {
		free(*ef);
		*ef = NULL;
	}
}

//-------------------------------------------------------------------------
// functions defined in shellEffects.h
//-------------------------------------------------------------------------
int runShellEffect(ShellEffect *ef) {
	return engineRunShellEffect(ef);
}
int runShellEffectAsync(ShellEffect *ef, EffectCompletef func, void *context) {
	return engineRunShellEffectAsync(ef, func, context);
}
void stopShellEffect(ShellEffect *ef) {
	engineStopShellEffect(ef);
}
void stopAllShellEffects() {
	engineStopAllShellEffects();
}

