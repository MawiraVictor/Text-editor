/*  Includes */
#include<termios.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<sys/ioctl.h>

//defines
#define CTRL_KEY(k) ((k) & 0x1f)

//data

struct editorConfig {
  struct termios orig_termios;
  int screenrows;  
  int screencols;  
};
struct editorConfig E;

void die (const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}
// terminal
void disableRawMode() {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(){
    if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    
    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
char editorReadKey(){
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
    return -1;
  
}else{
 	*cols = ws.ws_col;
	*rows = ws.ws_row;
	return 0;
 }
}
// append buffer
struct abuf {
	char *b;
	int len;
};
#define ABUF_INIT {NULL, 0}
void abAppend(struct abuf *ab, const char *s, int len) {
	char *new = realloc(ab->b, ab->len + len);

	if (new == NULL) return;
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len += len;
}

void abFree(struct abuf *ab) {
	free(ab->b);
}

// output

void editorDrawRows(){ //function handles each row of the buffer of the text being edited
	int y;
	for (y = 0; y < 24; y++) {
	     if (y == 24 /3) {
		char welcome[80];
		int welcomelen = snprintf(welcome, sizeof(welcome),
			"Kilo editor -- version %s", KILO_VERSION);
		if (welcomelen > 24 ) welcomlen = 24;
		abAppend(ab, welcome, welcomelen);
	     } else {
		abAppend(ab, "~", 1);
	     }

	     abAppend(ab, "\x1b[K, 3");

	     if (y < 24 - 1) {
		     AbAppend(ab, "\r\n", 2);
	     }
	}
}

void editorRefreshScreen(){ //\x1b is an escape character
	write (STDOUT_FILENO, "\x1b[2J", 4); // write 4 bytes out to the terminal
	write (STDOUT_FILENO, "\x1b[H", 3); //reposition the cursor
	
	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3);
}
//input
void editorProcessKeypress(){
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
		 write(STDOUT_FILENO, "\x1b[2J", 4);
		 write(STDOUT_FILENO, "\x1b[h", 3);
		exit(0);
		break;
	}
}

//   init 
int main (){
    enableRawMode();
    
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
        die("getWindowSize");
    }
    
    while (1) {
    editorRefreshScreen();
	  editorProcessKeypress();

	}

    return 0;
}
