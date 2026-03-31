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
  
  int cx, cy;
  int screenrows;  
  int screencols;  
  struct termios orig_termios;

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

char editorReadKey() {
	int nread;
	char c;
	while ((nread = read(STDERR_FILENO, &c, 1)) != 1) {
		if (mread == -1 && errno != EAGAIN) die("read");
	}
	return c;
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

void editorDrawRows(struct *ab){ //function handles each row of the buffer of the text being edited
	int y;
	for (y = 0; y < 24; y++) {
		abAppend(ab, "~", 1);
		abAppend(&ab, "\x1b[KJ", 3); 

	     if (y == 24 /3) { // we will assume a fixed text editor winngow size of 24 by 24
			abAppend(ab, "\r\n", 2);

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
	struct abuf ab = ABUF_INIT;
	
	abAppend(&ab, "\x1b[?25l", 6);
	abAppend(&ab, "\x1b[H", 3); //reposition the cursor
	
	editorDrawRows(&ab);

	char buf[32];
  	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  	abAppend(&ab, buf, strlen(buf));

	abAppend(&ab, "\x1b[?25h", 6);
	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}
void editorRefreshScreen(){
	write(STDIN_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

}
//input
void editorProcessKeypress(){
	char c  = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
		exit(0);
		break;

	}
}
void editorMoveCursor(char key) {
  switch (key) {
    case 'a':
      E.cx--;
      break;
    case 'd':
      E.cx++;
      break;
    case 'w':
      E.cy--;
      break;
    case 's':
      E.cy++;
      break;
  }
}
void editorProcessKeypress(){
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
		 write(STDOUT_FILENO, "\x1b[2J", 4);
		 write(STDOUT_FILENO, "\x1b[h", 3);
		exit(0);
		break;

	case 'w':
    case 's':
    case 'a':
    case 'd':
      editorMoveCursor(c);
      break;
	  
	}
}

//   init 
void initEditor() {
	E.cx = 0;
	E.cy = 0;

	if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}
int main (){
 enableRawMode();
 
  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

    return 0;
}
