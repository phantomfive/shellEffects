#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "shellRenderer.h"
#include "shellEffectsInternal.h"

struct shellScreen_t {
	int *chars; //these two are a double array representing the screen

	int x,y; //width and height of the screen
};

//put all local variables here so it will be easy
//to make this more than a singleton if we later desire.
struct things{
	int x, y;  //screen width and height

	srScreenSizeChanged screenSizeChanged;

	int *chars;   //our 'active' screen
	int *oldChars; //our 'old' screen, so we only draw changes
};
static struct things state_ = {0};
static struct things*state  = &state_; //because I like using -> more than .

#define posCvrt(i, j) ((i) + (j)*state->x)

static void initColors();
static int redraw = 0;
//---------------------------------------------------------------------
// These methods are used by the individual effects
//---------------------------------------------------------------------

/**
  * Sets the character at x,y on the screen to value c. To
  * represent a transparent character, use -1.
  */
void srSet(ShellScreen *screen, int x, int y, int c)
{
	if(x>=screen->x || y>=screen->y || x<0 || y<0) return;
	screen->chars[posCvrt(x,y)] = c;
	redraw = 1;
}

/**
  * returns the character at x,y. 'flags' will be set to the flags if
  * it is not NULL. -2 will be returned on error.
  */
int srGet(ShellScreen *screen, int x, int y)
{
	if(x>=screen->x || y>=screen->y || x<0 || y<0) return -2;
	return screen->chars[posCvrt(x,y)];
}

void srClearScreen(ShellScreen *s) {
	memset(s->chars, -1, s->x*s->y*sizeof(int));
/*	for(int i=0;i<s->x;i++) {
		for(int j=0;j<s->x; j++)
			s->chars[posCvrt(i,j)] = -1;
	}*/
}

int srRand(int startInclusive, int endInclusive) {
	double end = endInclusive, start = startInclusive;
	return (int)(end-start)*(rand()/(float)RAND_MAX) + start;
}

//--------------------------------------------------------------------
// These methods will be called by the SellEffects engine
//--------------------------------------------------------------------

/*//Useful for debugging
static void printScreen()
{
	int i,j;
	printf("------------------------------------------------\n");
	for(i=0;i<state->y;i++)
	{
		for(j=0;j<state->x;j++)
		{
			printf("%d ", state->chars[posCvrt(j,i)]);
		}
		printf("\n");
	}
}*/

int srInit(srScreenSizeChanged callback) {
	int i,j;
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	start_color();
	initColors();
	state->screenSizeChanged = callback;
	getmaxyx(stdscr, state->y, state->x);

	state->chars = malloc(sizeof(int)*state->x*state->y);
	state->oldChars = malloc(sizeof(int)*state->x*state->y);
	for(j=0;j<state->y;j++) {
		for(i=0;i<state->x; i++) {
			state->oldChars[posCvrt(i,j)] = -1;
			state->chars[posCvrt(i,j)] = -1;
		}
	}
	return SUCCESS;
}

void srShutdown() {
	free(state->chars);
	free(state->oldChars);
	endwin();
}

ShellScreen *srAllocScreen() {
	int i,j;
	ShellScreen *screen = malloc(sizeof(ShellScreen));
	if(screen!=NULL) {
		screen->chars = malloc(sizeof(int)*state->x*state->y);
		if(screen->chars!=NULL) {
				for(j=0;j<state->y;j++)
					for(i=0;i<state->x;i++)
						screen->chars[posCvrt(i,j)] = -1;
				screen->x = state->x;
				screen->y = state->y;
				return screen; //success
	   }
		free(screen);
	}
	return NULL;
}

void srFreeScreen(ShellScreen **screen) {
	if(screen==NULL || *screen==NULL) return;
	free((*screen)->chars);
	free(*screen);
	*screen = NULL; 
}

void srGetScreenSize(ShellScreen *screen, int *x, int *y) {
	*x = screen->x;
	*y = screen->y;
}

void srDrawScreen(ShellScreen *screen) {
	int i, j;
	int maxX = (screen->x > state->x)?state->x:screen->x; //min x
	int maxY = (screen->y > state->y)?state->y:screen->y; //min y
	for(i=0;i<maxX; i++) {
		for(j=0;j<maxY; j++) {
			if(screen->chars[posCvrt(i,j)]>=0){
				state->chars[posCvrt(i,j)] = screen->chars[posCvrt(i,j)];
			}
		}
	}
}

void srFlush() {
	int i,j;
	char needsRefresh = 0;

	//if(!redraw) return;
	redraw = 0;

	for(i=0;i<state->x;i++){
		for(j=0;j<state->y;j++){
			if(state->chars[posCvrt(i,j)]!=state->oldChars[posCvrt(i,j)]) {
				if(state->chars[posCvrt(i,j)]>=0)
					mvaddch(j, i, state->chars[posCvrt(i,j)]);
				else
					mvaddch(j, i, ' ');


				state->oldChars[posCvrt(i,j)] = state->chars[posCvrt(i,j)];
				needsRefresh = 1;
			}

			//we need to clear chars[] every time so we can redraw it
			state->chars[posCvrt(i,j)] = -1;
		}
	}

	move(0,0);
	if(needsRefresh)
		refresh();

}

//---------------------------------------------------------------------------
// Internal static stuff
//---------------------------------------------------------------------------
static void initColors() {
	//annoying, but necessary

	//Here please put them in numerical order of their values
	init_pair(1, COLOR_RED,    COLOR_BLUE);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(3, COLOR_BLUE,   COLOR_BLACK);
	init_pair(4, COLOR_RED,    COLOR_BLACK);
	init_pair(5, COLOR_WHITE,  COLOR_BLACK);
}
