/*  Includes */
#include<termios.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<ctype.h>
#include<sys/ioctl.h>

//defines
#define CTRL_KEY(k) ((k) & 0x1f)

//data

struct editorConfig {
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
// output

void editorDrawRows(){ //function handles each row of the buffer of the text being edited
	int y;
	for (y = 0; y < 24; y++) {
	     write(STDOUT_FILENO, "~\r\n", 3);
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
    
    
    while (1) {
    editorRefreshScreen();
	  editorProcessKeypress();

	}

    return 0;
}
