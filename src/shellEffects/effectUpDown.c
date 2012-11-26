#include "shellEffectsInternal.h"
#define WIDTH 2

static int dd;
static int pos = 0;
static int drawScreen(ShellEffect *ef) {
	if(pos>ef->y)
		return -1;
	srSet(ef->screen, WIDTH, pos, 'U'|RED_BLUE|A_BOLD);
	if(pos>0)
	   srSet(ef->screen, WIDTH, pos-1, -1);
	pos++;
	return dd;
}


ShellEffect *effectUpDown(int delay)
{
	ShellEffect *rv = allocEffect(drawScreen, NULL, NULL, NULL, NULL);
	dd = delay;
	return rv;
}

