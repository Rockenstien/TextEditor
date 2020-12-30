#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#define ctrl_key(k) ((k) & 0x1f)

struct 
editorConfig{
  int screenrows;
  int screencols;
  struct termios original_term;
};

struct editorConfig E;
void
die(const char *s){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void
disableRawMode(){
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original_term) == -1){
    die("tcsetattr");
  }
}

void
enableRawMode(){
  if(tcgetattr(STDIN_FILENO, &E.original_term) == -1){
    die("tcgetattr");
  };
  atexit(disableRawMode);
  struct termios raw = E.original_term;
  
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if(tcsetattr(STDIN_FILENO,  TCSAFLUSH, &raw) == -1){
    die("tcsetattr");
  }
}

char
editorReadKey(){
  int nread;
  char c;
  while((nread = read(STDIN_FILENO, &c, 1)) != 1){
    if(nread == -1 && errno != EAGAIN)  die("read");
  }
  return c;
}

void
editorKeyPress(){
  char c = editorReadKey();
  switch(c){
    case ctrl_key('q')  :
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

void editorDrawRows(){
  int y;
  for(y=0; y<E.screenrows; y++){
    write(STDOUT_FILENO, "~\r\n",3);
  }
}

int getWindowSize(int *rows, int *cols){
  struct winsize ws;
  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
    return -1;
  }
  else{
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void initEditor(){
  if(getWindowSize(&E.screenrows,&E.screencols) == -1) die("Init");
}

void editorRefreshScreen(){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[3", 3);

  editorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

int
main() {
  enableRawMode();
  initEditor();
  while(1){
    editorRefreshScreen();
    editorKeyPress();
  }
  return 0;
}