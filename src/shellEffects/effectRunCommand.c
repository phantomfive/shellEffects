#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include "shellEffectsInternal.h"

typedef struct sei {
	int cursorRow;
	int cursorCol;
	int childFd;
	int childPid;
} Sei;

//----------------------------------------------------------------------------
// Functions for dealing with terminal output
//----------------------------------------------------------------------------

/**Scrolls the screen up one line, or moves the cursor down one line */
static void scrollScreen(ShellEffect *ef, Sei *state) {
	int i, j;
	if(state->cursorRow<ef->y-1) {
		state->cursorRow++;
		return;
	}

	for(i=1; i<ef->y; i++) {
		for(j=0; j<ef->x; j++) {
			srSet(ef->screen, j, i-1, srGet(ef->screen, j, i) );
		}
	}

	//clear the bottom row
	for(j=0;j<ef->x;j++)
		srSet(ef->screen, j, ef->y-1, -1);
}

/**Make sure 'line' is NULL terminated. */
static void addLineToScreen(ShellEffect *ef, Sei *state, const char *line) {
	int i;
	for(i=0;line[i]!=0;i++) {
		if(line[i]=='\n' || state->cursorCol >= ef->x) {
			scrollScreen(ef, state);
			state->cursorCol=0;
			continue;
		}
		srSet(ef->screen, state->cursorCol, state->cursorRow, line[i]);
		state->cursorCol++;
	}

}

//--------------------------------------------------------------------------
// Functions for running a process
//--------------------------------------------------------------------------
/**Forks and execs the process 'proc,' passes in the parameters in 'argv'.
 * Of course parameter [0] should be the same as proc. Returns 0 on success,
 * negative on failure, and errno will be set */
static int forkProcess(const char *proc, char*const argv[], 
                       int *childFd, int *childPid) {
	int socks[2];

	//create a double sided socket: one for us, one for the child's stdout
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, socks)<0) {
		return -1;
	}
	*childFd = socks[0];
	*childPid = fork();
	if(*childPid<0) {
		close(socks[0]); close(socks[1]);
		return -1;
	}

	if(*childPid==0) {
		//This stuff is complicated. We are the child process, so we set
		//STDOUT to one end of the double-sided socket, then exec() the program,
		//which will inherit all our open sockets.
		if(dup2(socks[1], STDOUT_FILENO) < 0) {
			exit(-1);
		}
		execv(proc, argv);
		perror("This is an error: ");
		fprintf(stderr, "proc %s, argv[0] %s\n", proc, argv[0]);
		//if exec succeeded, it will never get here
		exit(-1);
		return -1;
	}
	else {
		//This is the parent process. Close one end of the socket (it won't be
		//closed in the child, we don't need two of those hanging around), and
		//return
		close(socks[1]);
	}	
	return 0;
}

static void killChild(int childFd, int childPid) {
	if(kill(childPid, 9)<0)
		;//error, but what to do?
	if(close(childFd)<0)
		;//another error, but again what to do?
}

//--------------------------------------------------------------------------
// Standard overridden effect functions
//--------------------------------------------------------------------------

static int drawScreen(ShellEffect *ef) {
	char buf[1000];
	int charsRead;
	Sei *state = (Sei*)ef->data;

	//see if there's anything to read
	fd_set fds;
	struct timeval t = {0};
	FD_ZERO(&fds);
	FD_SET(state->childFd, &fds);
	int result = select(state->childFd+1, &fds, NULL, NULL, &t);
	if( result==0 ) return 100;  //no data available to read

	//read output from the process
	if((charsRead = read(state->childFd, buf, sizeof(buf)-1)) <= 0)
		return -1; //cleanup in free method

	//add it to the screen
	buf[charsRead] = 0;
	addLineToScreen(ef, state, buf);
	return 200;
}

static void effectFree(ShellEffect *ef) {
	Sei *state = (Sei*)ef->data;
	killChild(state->childFd, state->childPid);
	free(state);
}

ShellEffect *effectRunCommand(const char *command, char *const args[]) {

	//alloc the effect
	Sei *state = (Sei*)malloc(sizeof(struct sei));
	if(state==NULL) return NULL;
	ShellEffect *rv = allocEffect(drawScreen, NULL, NULL, effectFree, state);
	if(rv==NULL) return NULL;
	state->cursorCol = 0;
	state->cursorRow = 0;
	rv->data = state;

	//run the shell command
	if(forkProcess(command, args, &state->childFd, &state->childPid)<0) {
		free(state);
		freeEffect(&rv);
		return NULL;
	}
	return rv;
}
