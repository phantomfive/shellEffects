#ifdef MAKE_TEST_PROGRAM

#include <stdio.h>
#include <unistd.h>
#include "shellEffects.h"

const char sprite[] =
"  //  \\\\  "
" _\\\\()//_ "
"/ //  \\\\ \\"
" | \\__/ | ";

const char sprite2[] =
"  ,;;;, ,;;;, "
" ;;;' ';' ';;;"
" ;;;       ;;;"
"  ';;,   ,;;' "
"    ';;,;;'   "
"     ';'      ";

EffectTravelingSpriteDef spriteDef[] = {{100, 0, 10, 4, sprite,  .1,0, .1,.5 },
                                        {400, 0, 10, 4, sprite, .1,.7, .15,.5},
                                        {400, 0, 10, 4, sprite, .2,.7, .25,.5},
                                        {400, 0, 10, 4, sprite, .3,.7, .35,.5},
                                        {400, 0, 14, 6, sprite2, .4,.7, .45,.5},
                                        {400, 0, 14, 6, sprite2, .5,.7, .55,.5},
                                        {400, 0, 14, 6, sprite2, .6,.7, .65,.5},
                                        {400, 0, 14, 6, sprite2, .7,.7, .75,.5,  .8, .7, SPRITE_END}};

char *args[] = {"/bin/cat","testFile", NULL};

int main(void) {
	ShellEffect *effect;
	//effect = effectRunCommand("/bin/cat", args);
	//runShellEffectAsync( effect, NULL, NULL );
	//effect = effectLeftRight( 200 );
	//runShellEffectAsync( effect, NULL, NULL );
	effect = effectFireworks();
	runShellEffectAsync( effect, NULL, NULL );
	effect = effectTravelingSprite(spriteDef);
	//runShellEffectAsync( effect, NULL, NULL );
	//effect = effectUpDown(500);
	runShellEffect( effect );
	//sleep(1000);
	stopAllShellEffects();
	return 0;
}

#endif

